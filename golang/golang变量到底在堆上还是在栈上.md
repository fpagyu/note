## Golang 变量到底在堆上还是在栈上

源于一个问题，代码如下

```go
// demo.go
package main

import "fmt"

type Stu struct {
    age int
}

func (stu *Stu) test(s string) {
    fmt.Println(s)
    // fmt.Println(stu)
}

func main() {
    var stu Stu
    
    stu.test()
    fmt.Println(&stu)
}
```



我迫切希望知道在main函数中，以

> var stu Stu

方式声明的变量stu，内存是如何分配的。

我尝试使用go tool 工具查看其汇编代码:

```
go tool compile -N -S demo.go // 或者使用go run -gcflags "-N -S" demo.go
```

得到下面的汇编代码:

```assembly
"".main STEXT size=89 args=0x0 locals=0x28
        0x0000 00000 (demo.go:13)       TEXT    "".main(SB), $40-0
        0x0000 00000 (demo.go:13)       MOVQ    (TLS), CX
        0x0009 00009 (demo.go:13)       CMPQ    SP, 16(CX)
        0x000d 00013 (demo.go:13)       JLS     82
        0x000f 00015 (demo.go:13)       SUBQ    $40, SP
        0x0013 00019 (demo.go:13)       MOVQ    BP, 32(SP)
        0x0018 00024 (demo.go:13)       LEAQ    32(SP), BP
        0x001d 00029 (demo.go:13)       FUNCDATA        $0, gclocals·33cdeccccebe80329f1fdbee7f5874cb(SB)
        0x001d 00029 (demo.go:13)       FUNCDATA        $1, gclocals·33cdeccccebe80329f1fdbee7f5874cb(SB)
        0x001d 00029 (demo.go:13)       FUNCDATA        $3, gclocals·9fb7f0986f647f17cb53dda1484e0f7a(SB)
        0x001d 00029 (demo.go:14)       PCDATA  $2, $0
        0x001d 00029 (demo.go:14)       PCDATA  $0, $0
        0x001d 00029 (demo.go:14)       MOVQ    $0, "".stu+24(SP)
        0x0026 00038 (demo.go:16)       MOVQ    $0, (SP)
        0x002e 00046 (demo.go:16)       PCDATA  $2, $1
        0x002e 00046 (demo.go:16)       LEAQ    go.string."test"(SB), AX
        0x0035 00053 (demo.go:16)       PCDATA  $2, $0
        0x0035 00053 (demo.go:16)       MOVQ    AX, 8(SP)
        0x003a 00058 (demo.go:16)       MOVQ    $4, 16(SP)
        0x0043 00067 (demo.go:16)       CALL    "".Stu.test(SB)
        0x0048 00072 (demo.go:18)       MOVQ    32(SP), BP
        0x004d 00077 (demo.go:18)       ADDQ    $40, SP
        0x0051 00081 (demo.go:18)       RET
        0x0052 00082 (demo.go:18)       NOP
        0x0052 00082 (demo.go:13)       PCDATA  $0, $-1
        0x0052 00082 (demo.go:13)       PCDATA  $2, $-1
        0x0052 00082 (demo.go:13)       CALL    runtime.morestack_noctxt(SB)
        0x0057 00087 (demo.go:13)       JMP     0
```

发现go代码的16行var stu Stu 对应的汇编代码中并没有调用runtime.newobject(SB), 因此此时的stu是分配在栈上的。然而当我尝试在main函数中打印&stu时, 我得到了另一段汇编代码:

```assembly
"".main STEXT size=227 args=0x0 locals=0x78
        0x0000 00000 (demo.go:15)       TEXT    "".main(SB), $120-0
        0x0000 00000 (demo.go:15)       MOVQ    (TLS), CX
        0x0009 00009 (demo.go:15)       CMPQ    SP, 16(CX)
        0x000d 00013 (demo.go:15)       JLS     217
        0x0013 00019 (demo.go:15)       SUBQ    $120, SP
        0x0017 00023 (demo.go:15)       MOVQ    BP, 112(SP)
        0x001c 00028 (demo.go:15)       LEAQ    112(SP), BP
        0x0021 00033 (demo.go:15)       FUNCDATA        $0, gclocals·3e27b3aa6b89137cce48b3379a2a6610(SB)
        0x0021 00033 (demo.go:15)       FUNCDATA        $1, gclocals·304e99e2f81ebc884eaa547114e5fb65(SB)
        0x0021 00033 (demo.go:15)       FUNCDATA        $3, gclocals·f6aec3988379d2bd21c69c093370a150(SB)
        0x0021 00033 (demo.go:16)       PCDATA  $2, $1
        0x0021 00033 (demo.go:16)       PCDATA  $0, $0
        0x0021 00033 (demo.go:16)       LEAQ    type."".Stu(SB), AX
        0x0028 00040 (demo.go:16)       PCDATA  $2, $0
        0x0028 00040 (demo.go:16)       MOVQ    AX, (SP)
        0x002c 00044 (demo.go:16)       CALL    runtime.newobject(SB)
        0x0031 00049 (demo.go:16)       PCDATA  $2, $1
        0x0031 00049 (demo.go:16)       MOVQ    8(SP), AX
        0x0036 00054 (demo.go:16)       PCDATA  $0, $1
        0x0036 00054 (demo.go:16)       MOVQ    AX, "".&stu+64(SP)
        0x003b 00059 (demo.go:16)       PCDATA  $2, $0
        0x003b 00059 (demo.go:16)       MOVQ    $0, (AX)
        0x0042 00066 (demo.go:18)       PCDATA  $2, $1
        0x0042 00066 (demo.go:18)       MOVQ    "".&stu+64(SP), AX
        0x0047 00071 (demo.go:18)       PCDATA  $2, $0
        0x0047 00071 (demo.go:18)       MOVQ    AX, (SP)
        0x004b 00075 (demo.go:18)       PCDATA  $2, $1
        0x004b 00075 (demo.go:18)       LEAQ    go.string."test"(SB), AX
        0x0052 00082 (demo.go:18)       PCDATA  $2, $0
        0x0052 00082 (demo.go:18)       MOVQ    AX, 8(SP)
        0x0057 00087 (demo.go:18)       MOVQ    $4, 16(SP)
        0x0060 00096 (demo.go:18)       CALL    "".(*Stu).test(SB)
        0x0065 00101 (demo.go:19)       PCDATA  $2, $1
        0x0065 00101 (demo.go:19)       PCDATA  $0, $0
        0x0065 00101 (demo.go:19)       MOVQ    "".&stu+64(SP), AX
        0x006a 00106 (demo.go:19)       PCDATA  $2, $0
        0x006a 00106 (demo.go:19)       PCDATA  $0, $2
        0x006a 00106 (demo.go:19)       MOVQ    AX, ""..autotmp_3+56(SP)
        0x006f 00111 (demo.go:19)       PCDATA  $0, $3
        0x006f 00111 (demo.go:19)       XORPS   X0, X0
        0x0072 00114 (demo.go:19)       MOVUPS  X0, ""..autotmp_2+72(SP)
        0x0077 00119 (demo.go:19)       PCDATA  $2, $1
        0x0077 00119 (demo.go:19)       LEAQ    ""..autotmp_2+72(SP), AX
        0x007c 00124 (demo.go:19)       MOVQ    AX, ""..autotmp_5+48(SP)
        0x0081 00129 (demo.go:19)       TESTB   AL, (AX)
        0x0083 00131 (demo.go:19)       PCDATA  $2, $2
        0x0083 00131 (demo.go:19)       PCDATA  $0, $4
        0x0083 00131 (demo.go:19)       MOVQ    ""..autotmp_3+56(SP), CX
        0x0088 00136 (demo.go:19)       PCDATA  $2, $3
        0x0088 00136 (demo.go:19)       LEAQ    type.*"".Stu(SB), DX
        0x008f 00143 (demo.go:19)       PCDATA  $2, $2
        0x008f 00143 (demo.go:19)       MOVQ    DX, ""..autotmp_2+72(SP)
        0x0094 00148 (demo.go:19)       PCDATA  $2, $1
        0x0094 00148 (demo.go:19)       MOVQ    CX, ""..autotmp_2+80(SP)
        0x0099 00153 (demo.go:19)       TESTB   AL, (AX)
        0x009b 00155 (demo.go:19)       JMP     157
        0x009d 00157 (demo.go:19)       MOVQ    AX, ""..autotmp_4+88(SP)
        0x00a2 00162 (demo.go:19)       MOVQ    $1, ""..autotmp_4+96(SP)
        0x00ab 00171 (demo.go:19)       MOVQ    $1, ""..autotmp_4+104(SP)
        0x00b4 00180 (demo.go:19)       PCDATA  $2, $0
        0x00b4 00180 (demo.go:19)       MOVQ    AX, (SP)
        0x00b8 00184 (demo.go:19)       MOVQ    $1, 8(SP)
        0x00c1 00193 (demo.go:19)       MOVQ    $1, 16(SP)
        0x00ca 00202 (demo.go:19)       CALL    fmt.Println(SB)
        0x00cf 00207 (demo.go:20)       PCDATA  $0, $0
        0x00cf 00207 (demo.go:20)       MOVQ    112(SP), BP
        0x00d4 00212 (demo.go:20)       ADDQ    $120, SP
        0x00d8 00216 (demo.go:20)       RET
        0x00d9 00217 (demo.go:20)       NOP
```

很显然，此时的stu是有runtime.newobject生成的，它是分配在堆上的。



查看了一些文章，其中[Golang逃逸分析](https://studygolang.com/articles/10026)从内存逃逸分析的角度给出了另外的解释, 其使用的工具

> go tool compile -m demo.go

对程序进行了内存逃逸分析，得出了和上述汇编一致的结果。

在golang的[FAQ](https://golang.org/doc/faq)中对此也有说明:

>  From a correctness standpoint, you don't need to know. Each variable in Go exists as long as there are references to it. The storage location chosen by the implementation is irrelevant to the semantics of the language. 
>
>  The storage location does have an effect on writing efficient programs. When possible, the Go compilers will allocate variables that are local to a function in that function's stack frame.  However, if the compiler cannot prove that the variable is not referenced after the function returns, then the compiler must allocate the variable on the garbage-collected heap to avoid dangling pointer errors. Also, if a local variable is very large(>32k), it might make more sense to store it on the heap rather than the stack. 
>
>  In the current compilers, if a variable has its address taken, that variable is a candidate for allocation on the heap. However, a basic *escape analysis* recognizes some cases when such variables will not live past the return from the function and can reside on the stack. 



**参考资料**

[Golang逃逸分析](https://studygolang.com/articles/10026)

[Frequently Asked Questions (FAQ)](https://golang.org/doc/faq)