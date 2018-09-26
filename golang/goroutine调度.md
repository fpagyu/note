## goroutine调度



### Go调度器模型

#### G-M模型

#### G-P-M模型

下图所示来自《go语言学习笔记》中的GPM基本示意图:

![GPM](https://github.com/fpagyu/notebook/raw/master/images/g-m-p-schedule.png)

G, P, M 定义在[runtime/runtime2.go](https://golang.org/src/runtime/runtime2.go)中, 其中

1. G 是goroutine的抽象， 它不是一个执行体，但它保存了一个goroutine运行的上下文信息，包括goroutine的状态，栈信息等。G一旦创建就会进入P本地队列或全局队列, 如上图所示。
2. P(processor)，可以将它类比计算机CPU的核, 用来控制可同时并发执行的任务数量。P中包含了G的队列，为线程(M)提供执行资源。
3. M 代表系统线程, 是真正的执行单元。每一个M都必须要绑定一个P, 以进入schedule循环，不断执行G并发任务。schedule循环的机制大致是从各种队列、p的本地队列中获取G，切换到G的执行栈上并执行G的函数，调用goexit做清理工作并回到m，如此反复。M并不保留G状态，这是G可以跨M调度的基础。

P的数量大致与CPU核数相当(有可能更多/更少)，M则是按需创建。





**相关资源**

[Scalable Go Scheduler Design Doc](https://docs.google.com/document/d/1TTj4T2JO42uD5ID9e89oa0sLKhJYD0Y_kqxDv3I3XMw/edit#heading=h.mmq8lm48qfcw)

[《go语言学习笔记》-- 雨痕老师](https://github.com/qyuhen/book)

[也谈goroutine调度器](https://tonybai.com/2017/06/23/an-intro-about-goroutine-scheduler/)



