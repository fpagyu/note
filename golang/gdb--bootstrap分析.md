# gdb--bootstrap分析

阅读过雨枫老师的Go源码分析就知道如何使用gdb分析一个go程序的启动过程。

首先给出一个简单的go程序, 这个程序啥也不做， 就只有一个空的main函数:

```golang
// demo.go
package main

func main() {
    
}
```

编译：

> go build -gcflags "-N -l" -o demo demo.go

-gcflags "-N -l" 参数会关闭编译器代码优化和函数内联， 避免断点和单步执行无法准确对应源码行， 避免小函数和局部变量被优化掉。

在命令行中输入**gdb ./demo**进入gdb调试模式：

```
(gdb) info file
Symbols from "/home/jvm/code/go/src/grpc_study/.example/demo".
Local exec file:
        `/home/jvm/code/go/src/grpc_study/.example/demo', file type elf64-x86-64.
        Entry point: 0x44a2e0
        0x0000000000401000 - 0x000000000044ea30 is .text
        0x000000000044f000 - 0x0000000000478d73 is .rodata
        0x0000000000478f20 - 0x00000000004795f4 is .typelink
        0x00000000004795f8 - 0x0000000000479600 is .itablink
        0x0000000000479600 - 0x0000000000479600 is .gosymtab
        0x0000000000479600 - 0x00000000004b6a96 is .gopclntab
        0x00000000004b7000 - 0x00000000004b7c08 is .noptrdata
        0x00000000004b7c20 - 0x00000000004b9950 is .data
        0x00000000004b9960 - 0x00000000004d5c70 is .bss
        0x00000000004d5c80 - 0x00000000004d8378 is .noptrbss
        0x0000000000400f9c - 0x0000000000401000 is .note.go.buildid
(gdb) b *0x44a2e0
Breakpoint 1 at 0x44a2e0: file /usr/local/go/src/runtime/rt0_linux_amd64.s, line 8.
```

info file 让我们找到程序的Entry point， 当然直接在命令行中输入readelf -l ./demo 也能够让我们找到这个Entry point, 它是所有go程序的入口地址，是一段汇编代码。在Entry point处打断点。

打开源文件/usr/local/go/src/runtime/rt0_linux_amd64.s，

```assembly
#include "textflag.h"

TEXT _rt0_amd64_linux(SB),NOSPLIT,$-8
	JMP	_rt0_amd64(SB)
```

_rt0_amd64就是我们要执行的地方， 启动程序, 运行到断点处， 输入s(step into), 进入函数内部：

```
(gdb) r
Starting program: /home/jvm/code/go/src/grpc_study/.example/demo 

Breakpoint 1, _rt0_amd64_linux () at /usr/local/go/src/runtime/rt0_linux_amd64.s:8
8               JMP     _rt0_amd64(SB)
(gdb) s
_rt0_amd64 () at /usr/local/go/src/runtime/asm_amd64.s:15
15              MOVQ    0(SP), DI       // argc
```

_rt0_amd64函数：

```assembly
TEXT _rt0_amd64(SB),NOSPLIT,$-8
	MOVQ	0(SP), DI	// argc
	LEAQ	8(SP), SI	// argv
	JMP	runtime·rt0_go(SB)
```

使用上面同样的办法， 进入到runtime.rt0_go函数的内部:

```assembly
TEXT runtime·rt0_go(SB),NOSPLIT,$0
	...

	CLD				// convention is D is always left cleared
	CALL	runtime·check(SB)

	MOVL	16(SP), AX		// copy argc
	MOVL	AX, 0(SP)
	MOVQ	24(SP), AX		// copy argv
	MOVQ	AX, 8(SP)
	CALL	runtime·args(SB)
	CALL	runtime·osinit(SB)
	CALL	runtime·schedinit(SB)

	// create a new goroutine to start program
	MOVQ	$runtime·mainPC(SB), AX		// entry
	PUSHQ	AX
	PUSHQ	$0			// arg size
	CALL	runtime·newproc(SB)     // 创建一个新的goroutine, 用于运行runtime.main
	POPQ	AX
	POPQ	AX

	// start this M, 启动m(可视为系统线程), 绑定p已运行runtime.main
	CALL	runtime·mstart(SB)

	CALL	runtime·abort(SB)	// mstart should never return
	RET

	// Prevent dead-code elimination of debugCallV1, which is
	// intended to be called by debuggers.
	MOVQ	$runtime·debugCallV1(SB), AX
	RET
```

我们忽略前面部分函数参数入栈及硬件信息检查等部分的代码， 重点关注runtime.args, runtime.osinit, runtime.schedinit, runtime.mainPC, runtime.newproc的函数。

runtime.args

```golang
func args(c int32, v **byte) {
	argc = c
	argv = v
	sysargs(c, v)
}
```

只是简单的参数处理

runtime.osinit

```golang
func osinit() {
	ncpu = getproccount()
}
```

获取cpu数量, cpu的数量约等于cpu的核数，如果cpu带超线程功能， 则cpu的数量等于核数 x 2。

runtine.schedinit, 初始化运行时环境

```golang
// The bootstrap sequence is:
//
//	call osinit
//	call schedinit
//	make & queue new G
//	call runtime·mstart
//
// The new G calls runtime·main.
func schedinit() {
	// raceinit must be the first call to race detector.
	// In particular, it must be done before mallocinit below calls racemapshadow.
	_g_ := getg()
	if raceenabled {
		_g_.racectx, raceprocctx0 = raceinit()
	}
	
	// 设置最大系统线程数量限制
	sched.maxmcount = 10000
	
	// go变量的初始化发生在运行时启动以后，
	// schedinit调用该函数使得变量在运行时启动之前就被初始化，
	// 而不是在变量申明之后才被初始化
	tracebackinit()
	moduledataverify()
	
	// 栈，内存分配器，调度器相关初始化
	stackinit()
	mallocinit()
	mcommoninit(_g_.m)
	
	cpuinit()       // must run before alginit
	alginit()       // maps must not be used before this call
	modulesinit()   // provides activeModules
	typelinksinit() // uses maps, activeModules
	itabsinit()     // uses activeModules

	msigsave(_g_.m)
	initSigmask = _g_.m.sigmask

	goargs()
	goenvs()
	parsedebugvars()
	gcinit()

	sched.lastpoll = uint64(nanotime())
	procs := ncpu
	if n, ok := atoi32(gogetenv("GOMAXPROCS")); ok && n > 0 {
		procs = n
	}
	if procresize(procs) != nil {
		throw("unknown runnable goroutine during bootstrap")
	}

	// For cgocheck > 1, we turn on the write barrier at all times
	// and check all pointer writes. We can't do this until after
	// procresize because the write barrier needs a P.
	if debug.cgocheck > 1 {
		writeBarrier.cgo = true
		writeBarrier.enabled = true
		for _, p := range allp {
			p.wbBuf.reset()
		}
	}

	if buildVersion == "" {
		// Condition should never trigger. This code just serves
		// to ensure runtime·buildVersion is kept in the resulting binary.
		buildVersion = "unknown"
	}
}
```



## Tips

**如何找到程序的Entry point**

1. gdb <a.out>， 通过info file获取Entry Point
2. readelf -l <a.out>
3. objdump -f <a.out>, 中的start address