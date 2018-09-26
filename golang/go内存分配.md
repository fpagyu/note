## golang内存分配算法



### 基本策略

1. 每次从操作系统申请一大块内存(比如1MB), 以减少系统调用。
2. 将申请到的大块内存按照特定大小预先切分成小块， 构成链表。
3. 为对象分配内存时，只需从大小合适的链表提取一个小块即可。
4. 回收对象内存是，将小块内存重新归还到原链表， 以便复用。
5. 如果闲置内存过多，则尝试归还部分内存给操作系统， 降低整体开销。



**runtime.stackinit()**

```
var stackpool [_NumStackOrders]mSpanList
stackpool 是 span全局的stack池。

stackpool[0]的大小等于FixedStack,之后每一个order都是前面的2倍。在linux/darwin/bsd等操作系统中fixedstack=2k，为了保证最大cache size一致， 因此不同的操作系统_NumStackOrders和FixedStack大小都不一样, 如下表:
//   OS               | FixedStack | NumStackOrders
//   -----------------+------------+---------------
//   linux/darwin/bsd | 2KB        | 4
//   windows/32       | 4KB        | 3
//   windows/64       | 8KB        | 2
//   plan9            | 4KB        | 3

因此我们可以看到，在linux中有2k， 4k, 8k, 16k 四种大小的stack, go中最大允许缓存的stack大小就是16k,
如果需要更大的stack，将会由程序直接分配。

我们需要使用哪一个order上的stack由所需stack的大小决定, 计算方式如下：
		order = log_2(size/FixedStack)
```

golang 中page的大小:

```
_PageShift = 13
_PageSize = 1 << _PageShift // 1 << 13, 8k
```

### 内存块

分配器将其管理的内存块分为2种：

- span: 由多个地址连续的页(page) 组成的大块内存
- object: 将span按特定大小切分成多个小块， 每个小块可存储一个对象



**分配流程**

1. 计算待分配对象对应规格(size class)
2. 从cache.alloc 数组中找到规格相同的span
3. 从span.freelist 链表中提取可用object
4. 如果span.freelist 为空， 从central获取新的span
5. 如果central.nonempty为空， 从heap.free/freelarge获取， 并切分成object链表
6. 如果heap中没有大小合适的闲置span， 向操作系统申请新内存块



**释放流程**

1. 将标记为可回收的object交还给所属span.freelist
2. 该span被放回central， 可供任意cache重新获取使用
3. 如果span已收回全部object， 则将其交还给heap， 以便重新切分复用
4. 定期扫描heap里长时间闲置的span， 释放其占用内存

以上不包括大对象， 它直接从heap分配和回收



