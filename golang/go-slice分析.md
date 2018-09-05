在线上项目写go已经一年多了， go语言一直有一种很独特的魅力，让我有种回到写C的感觉，同时又有种python的舒适感。写go的代码， 不需要太担心底层的一些东西，语言本身以及编译器都为你做了很好的处理，但细究其细节，对理解语言本身，写出更好的代码以及更高效的排除bug都有非常大的好处。因此准备写几篇文章整理和总结一下，更重要的是督促自己离开一个舒适区，这也是一个深入学习go的过程。文章的主题没有计划，遇到一些有价值的内容就整理一下， 开篇以slice为主题。



## slice的结构

在go的runtime源码包中定义了slice的结构，详见[源码](https://golang.org/src/runtime/slice.go):

```go
type slice struct {
	array unsafe.Pointer
    len int
    cap int
}
```

array 是一个指向了长度为cap的数组。我们可以通过对一个slice反射来进行分析,其[反射结构](https://golang.org/pkg/reflect/#SliceHeader)定义如下：

```go
type SliceHeader struct {
    Data uintptr
    Len int
    Cap int
}
```

要拿到一个SliceHeader对象, 以下为例：

```go
b := make([]byte, 0, 10)

ptr := (*reflect.SliceHeader)(unsafe.Pointer(&b))
```



## slice的切片

从上一节中我们知道， slice底层其实是指向了一个数组的。我们在对一个slice进行切片操作的时候(我们称这个过程叫re-slicing)， 其本质其实是指向底层数组的指针发生了改变，这篇[文章](https://blog.golang.org/go-slices-usage-and-internals)中的几幅示例图很形象的描述了这个过程,  我们通过程序来验证一下:

```go
func main() {
	s := []string{"a", "b", "c", "d", "e"}
	s1 := s[2:4]

	// 可以看到，打印结果是true
	fmt.Println(&s[2] == &s1[0])

	// 可以看到改变s1[0]， s[2]也发生了相同的变化
	s1[0] = "s"
	fmt.Println(s[2])

	// 切片的len， cap变化
	p := (*reflect.SliceHeader)(unsafe.Pointer(&s))
	// 打印 s.Len:  5   s.Cap:  5
	fmt.Println("s.Len: ", p.Len, "  s.Cap: ", p.Cap)

	// cap的大小是底层指向数组的长度
	p1 := (*reflect.SliceHeader)(unsafe.Pointer(&s1))
	// 打印： s.Len:  5   s.Cap:  5
	fmt.Println("s1.Len: ", p1.Len, "  s1.Cap: ", p1.Cap)
}
```



在这里我们可以看到re-slicing并不会拷贝底层的数组， cap的大小其实就是底层指向的数组的长度。



## slice 的内存分配规则

### 初始化slice

我们定义slice有几种方式， 这里我们选择一种比较特殊的slice类型, []byte:

```go
// 通常情况下定义一个slice有两种办法
s := make([]T, len, cap)  // 直接使用make函数
s := []T{}                // 显式初始化

// []byte 比较特殊，可以将一个字符串转换为slice
s := []byte("")
```

make真正调用的是makeslice方法, 该方法位于runtime包中 [详见](https://golang.org/src/runtime/slice.go), 下面是makeslice的实现:

```
func makeslice(et *_type, len, cap int) slice {
	maxElements := maxSliceCap(et.size)
	if len < 0 || uintptr(len) > maxElements {
		panic(errorString("makeslice: len out of range"))
	}

	if cap < len || uintptr(cap) > maxElements {
		panic(errorString("makeslice: cap out of range"))
	}

	p := mallocgc(et.size*uintptr(cap), et, true)
	return slice{p, len, cap}
}
```

maxSliceCap能够计算出slice允许的最大长度，这和具体的slice类型有关。通过允许分配的最大内存_MaxMem除以每一个slice元素的大小， 就可以得到这个值. mallocgc 这个函数用于在go中分配内存和触发gc, 在这里我们只需知道它为我们的slice分配了指定大小的内存。[]T{}显示初始化这种方式没有深究， 我猜测应该仍是调用了makeslice, 其len和cap的值都是初始化语句中元素的个数.



这里谈一下[]byte("")， 因为它可能会引发一些很奇怪的[现象](https://github.com/mushroomsir/blog/blob/master/Go%E4%B8%ADstring%E8%BD%AC%5B%5Dbyte%E7%9A%84%E9%99%B7%E9%98%B1.md)。它实际上是通过调用[stringtoslicebyte](https://github.com/golang/go/blob/master/src/runtime/string.go#L154)方法实现的：

```go
const tmpStringBufSize = 32
type tmpBuf [tmpStringBufSize]byte

func stringtoslicebyte(buf *tmpBuf, s string) []byte {
	var b []byte
	if buf != nil && len(s) <= len(buf) {
		*buf = tmpBuf{}
		b = buf[:len(s)]
	} else {
		b = rawbyteslice(len(s))
	}
	copy(b, s)
	return b
}
```

这个tmpBuf是一个32字节长度的byte数组。我们可以看到当len(s) <= 32 时， 我们会得到一个len=len(s),  cap=32的[]byte。反之，将会调用rawbytesslice得到[]byte：

```go
func rawbyteslice(size int) (b []byte) {
	// roundupsize 会进行内存对齐， 因此cap>=size
	cap := roundupsize(uintptr(size))
	p := mallocgc(cap, nil, false)
	if cap != uintptr(size) {
		memclrNoHeapPointers(add(p, uintptr(size)), cap-uintptr(size))
	}

	*(*slice)(unsafe.Pointer(&b)) = slice{p, size, int(cap)}
	return
}
```

这段代码比较好理解，就不赘述了。



### slice的Cap增长

谈到slice的cap增长， 就要说一下append这个函数了。通过下面一个小例子感受一下:

```go
package main

func main() {
    s := make([]byte, 0)
    s = append(s, 'a')
}
```

执行 go run -gcflag '-S -S' demo1.go 得到：

```
 0x001d 00029 (/home/jvm/demo1.go:5)        LEAQ    type.uint8(SB), AX
 0x0024 00036 (/home/jvm/demo1.go:5)        MOVQ    AX, (SP)
 0x0028 00040 (/home/jvm/demo1.go:5)        LEAQ    ""..autotmp_1+64(SP), AX
 0x002d 00045 (/home/jvm/demo1.go:5)        MOVQ    AX, 8(SP)
 0x0032 00050 (/home/jvm/demo1.go:5)        XORPS   X0, X0
 0x0035 00053 (/home/jvm/demo1.go:5)        MOVUPS  X0, 16(SP)
 0x003a 00058 (/home/jvm/demo1.go:5)        MOVQ    $1, 32(SP)
 0x0043 00067 (/home/jvm/demo1.go:5)        PCDATA  $0, $0
 0x0043 00067 (/home/jvm/demo1.go:5)        CALL    runtime.growslice(SB)
 0x0048 00072 (/home/jvm/demo1.go:5)        MOVQ    40(SP), AX
 0x004d 00077 (/home/jvm/demo1.go:5)        MOVB    $97, (AX)
```

当超出s的cap时， 会调用runtime包中的[growslice](https://golang.org/src/runtime/slice.go#L89), 源码较长就不复制粘贴了。

从源码中可以看到几个关键点， growslice的参数中会指定一个最小期望的cap, old.cap是原slice的cap,

假如这个cap> 2 * old.cap，那么新的cap就是growslice函数参数中指定的cap大小。

反之当cap <= 2 * old.cap时, 有两种情况：

假如原slice的长度小于1024, 那么新的cap就会扩大为原来的两倍。

如果原slice的长度大于或等于1024，那么就会在执行newcap += newcap / 4， 直到newcap > cap为止, 

newcap的基础值是old.cap, 如果在累加的过程中出现newcap = 0或者其他溢出的情况,newcap会直接设置

为cap的大小。

做完上述处理之后， 程序还会再做一次内存对齐处理和内存溢出检测， 如果都没有问题， 就会创建一个新的slice,同时将旧的slice数据拷贝到新的slice中。

### 参考:

https://blog.golang.org/go-slices-usage-and-internals

https://github.com/mushroomsir/blog/blob/master/Go%E4%B8%ADstring%E8%BD%AC%5B%5Dbyte%E7%9A%84%E9%99%B7%E9%98%B1.md