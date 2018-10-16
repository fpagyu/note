# python协程1：yield的使用(转)

说明: 本文转自[python协程1：yield的使用](https://mp.weixin.qq.com/s?__biz=MzAwNjI5MjAzNw==&mid=2655751983&idx=1&sn=e4c093c6e5d6e4e8281d76db7c67eb23)。非常感谢这篇文章的作者，这篇文章写的非常不错，建议阅读作者原文。



> 最近找到一本python好书《流畅的python》，是到现在为止看到的对python高级特性讲述最详细的一本。 看了协程一章，做个读书笔记，加深印象。

### 协程定义

协程的底层架构是在pep342 中定义，并在python2.5 实现的。

python2.5 中，yield关键字可以在表达式中使用，而且生成器API中增加了 .send(value)方法。生成器可以使用.send(...)方法发送数据，发送的数据会成为生成器函数中yield表达式的值。

协程是指一个过程，这个过程与调用方协作，产出有调用方提供的值。因此，生成器可以作为协程使用。

> 除了 .send(...)方法，pep342 和添加了 .throw(...)（让调用方抛出异常，在生成器中处理）和.close()（终止生成器）方法

python3.3后，pep380对生成器函数做了两处改动：

- 生成器可以返回一个值；以前，如果生成器中给return语句提供值，会抛出SyntaxError异常。
- 引入yield from 语法，使用它可以把复杂的生成器重构成小型的嵌套生成器，省去之前把生成器的工作委托给子生成器所需的大量模板代码。

### 协程生成器的基本行为

首先说明一下，协程有四个状态，可以使用inspect.getgeneratorstate(...)函数确定：

- GEN_CREATED # 等待开始执行
- GEN_RUNNING # 解释器正在执行（只有在多线程应用中才能看到这个状态）
- GEN_SUSPENDED # 在yield表达式处暂停
- GEN_CLOSED # 执行结束

```python
#! -*- coding: utf-8 -*-

import inspect

# 协程使用生成器函数定义：定义体中有yield关键字。

def simple_coroutine():
    print('-> coroutine started')    # yield 在表达式中使用；如果协程只需要从客户那里接收数据，yield关键字右边不需要加表达式（yield默认返回None）
    x = yield
    print('-> coroutine received:', x)


my_coro = simple_coroutine()# 和创建生成器的方式一样，调用函数得到生成器对象。
print(inspect.getgeneratorstate(my_coro))
# 协程处于 GEN_CREATED (等待开始状态)my_coro.send(None)
# 首先要调用next()函数，因为生成器还没有启动，没有在yield语句处暂停，所以开始无法发送数据
# 发送 None 可以达到相同的效果 my_coro.send(None) 

next(my_coro)# 此时协程处于 GEN_SUSPENDED (在yield表达式处暂停)

print(inspect.getgeneratorstate(my_coro))
# 调用这个方法后，协程定义体中的yield表达式会计算出42；现在协程会恢复，一直运行到下一个yield表达式，或者终止。

my_coro.send(42)
print(inspect.getgeneratorstate(my_coro))
```

运行上述代码，输出结果如下

```python
GEN_CREATED
-> coroutine started
GEN_SUSPENDED
-> coroutine received: 42# 这里，控制权流动到协程定义体的尾部，导致生成器像往常一样抛出StopIteration异常

Traceback (most recent call last):
  File "/Users/gs/coroutine.py", line 18, in <module> 
    my_coro.send(42)
StopIteration
```

> send方法的参数会成为暂停yield表达式的值，所以，仅当协程处于暂停状态是才能调用send方法。 如果协程还未激活（GEN_CREATED 状态）要调用next(my_coro) 激活协程，也可以调用my_coro.send(None)

如果创建协程对象后立即把None之外的值发给它，会出现下述错误：

```python
>>> my_coro = simple_coroutine()
>>> my_coro.send(123)

Traceback (most recent call last):
  File "/Users/gs/coroutine.py", line 14, in <module>
    my_coro.send(123)
TypeError: can't send non-None value to a just-started generator
```

仔细看错误消息

> can't send non-None value to a just-started generator

最先调用next(my_coro) 这一步通常称为”预激“（prime）协程---即，让协程向前执行到第一个yield表达式，准备好作为活跃的协程使用。

再看一个两个值的协程

```python
def simple_coro2(a):
    print('-> coroutine started: a =', a)
    b = yield a
    print('-> Received: b =', b)
    c = yield a + b
    print('-> Received: c =', c)

my_coro2 = simple_coro2(14)
print(inspect.getgeneratorstate(my_coro2))# 这里inspect.getgeneratorstate(my_coro2) 得到结果为 GEN_CREATED （协程未启动）

next(my_coro2)
# 向前执行到第一个yield 处 打印 “-> coroutine started: a = 14”
# 并且产生值 14 （yield a 执行 等待为b赋值）

print(inspect.getgeneratorstate(my_coro2))
# 这里inspect.getgeneratorstate(my_coro2) 得到结果为 GEN_SUSPENDED （协程处于暂停状态）

my_coro2.send(28)
# 向前执行到第二个yield 处 打印 “-> Received: b = 28”
# 并且产生值 a + b = 42（yield a + b 执行 得到结果42 等待为c赋值）

print(inspect.getgeneratorstate(my_coro2))
# 这里inspect.getgeneratorstate(my_coro2) 得到结果为 GEN_SUSPENDED （协程处于暂停状态）

my_coro2.send(99)
# 把数字99发送给暂停协程，计算yield 表达式，得到99，然后把那个数赋值给c 打印 “-> Received: c = 99”
# 协程终止，抛出StopIteration
```

运行上述代码，输出结果如下

```python
GEN_CREATED
-> coroutine started: a = 14
GEN_SUSPENDED
-> Received: b = 28
-> Received: c = 99
Traceback (most recent call last):
  File "/Users/gs/coroutine.py", line 37, in <module>
    my_coro2.send(99)
StopIteration
```

simple_coro2 协程的执行过程分为3个阶段，如下图所示

![img](http://mmbiz.qpic.cn/mmbiz_png/ea0GXSono0UKbibn6XsvecpHT7QB8mOiatKUmcaoLqh1ibV4ZlmhfFP6H1k0RvsZRMfVyjBYibjXibjxIGqs7PW8ngQ/640?wx_fmt=png&wxfrom=5&wx_lazy=1&wx_co=1)

1. 调用next(my_coro2)，打印第一个消息，然后执行yield a，产出数字14.
2. 调用my_coro2.send(28)，把28赋值给b，打印第二个消息，然后执行 yield a + b 产生数字42
3. 调用my_coro2.send(99)，把99赋值给c，然后打印第三个消息，协程终止。

#### 使用装饰器预激协程

我们已经知道，协程如果不预激，不能使用send() 传入非None 数据。所以，调用my_coro.send(x)之前，一定要调用next(my_coro)。 为了简化，我们会使用装饰器预激协程。

```python
from functools import wraps

def coroutinue(func):
    '''
    装饰器： 向前执行到第一个`yield`表达式，预激`func`
    :param func: func name
    :return: primer
    '''    @wraps(func)
    def primer(*args, **kwargs):
        # 把装饰器生成器函数替换成这里的primer函数；调用primer函数时，返回预激后的生成器。
        gen = func(*args, **kwargs)        # 调用被被装饰函数，获取生成器对象
        next(gen)  # 预激生成器
        return gen  # 返回生成器
    return primer

# 使用方法如下

@coroutinue
def simple_coro(a):
    a = yield

simple_coro(12)  # 已经预激
```

#### 终止协程和异常处理

协程中，为处理的异常会向上冒泡，传递给next函数或send方法的调用方，未处理的异常会导致协程终止。

看下边这个例子

```python
#! -*- coding: utf-8 -*-

from functools import wraps

def coroutinue(func):
    '''
    装饰器： 向前执行到第一个`yield`表达式，预激`func`
    :param func: func name
    :return: primer
    '''    
    @wraps(func)
    def primer(*args, **kwargs):
        # 把装饰器生成器函数替换成这里的primer函数；调用primer函数时，返回预激后的生成器。
        gen = func(*args, **kwargs)        # 调用被被装饰函数，获取生成器对象
        next(gen)  # 预激生成器
        return gen  # 返回生成器
    return primer

@coroutinue
def averager():
    # 使用协程求平均值
    total = 0.0
    count = 0
    average = None
    while True:
        term = yield average
        total += term
        count += 1
        average = total/count

coro_avg = averager()
print(coro_avg.send(40))
print(coro_avg.send(50))
print(coro_avg.send('123')) # 由于发送的不是数字，导致内部有异常抛出。
```

执行上述代码结果如下

```python
40.0
45.0

Traceback (most recent call last):
  File "/Users/gs/coro_exception.py", line 37, in <module>
    print(coro_avg.send('123'))
  File "/Users/gs/coro_exception.py", line 30, in averager
    total += term
TypeError: unsupported operand type(s) for +=: 'float' and 'str'
```

出错的原因是发送给协程的'123'值不能加到total变量上。
出错后，如果再次调用 coro_avg.send(x) 方法 会抛出 StopIteration 异常。

由上边的例子我们可以知道，如果想让协程退出，可以发送给它一个特定的值。比如None和Ellipsis。（推荐使用Ellipsis，因为我们不太使用这个值）
从Python2.5 开始，我们可以在生成器上调用两个方法，显式的把异常发给协程。
这两个方法是throw和close。

```python
generator.throw(exc_type[, exc_value[, traceback]])
```

这个方法使生成器在暂停的yield表达式处抛出指定的异常。如果生成器处理了抛出的异常，代码会向前执行到下一个yield表达式，而产出的值会成为调用throw方法得到的返回值。如果没有处理，则向上冒泡，直接抛出。

```python
generator.close()
```

生成器在暂停的yield表达式处抛出GeneratorExit异常。 如果生成器没有处理这个异常或者抛出了StopIteration异常，调用方不会报错。如果收到GeneratorExit异常，生成器一定不能产出值，否则解释器会抛出RuntimeError异常。

**示例： 使用close和throw方法控制协程**

```python
import inspect

class DemoException(Exception):
    pass


@coroutinue
def exc_handling():
    print('-> coroutine started')    
    while True:        
        try:
            x = yield
        except DemoException:
            print('*** DemoException handled. Conginuing...')
        else:            # 如果没有异常显示接收到的值
            print('--> coroutine received: {!r}'.format(x))    
    raise RuntimeError('This line should never run.')  # 这一行永远不会执行

exc_coro = exc_handling()

exc_coro.send(11)
exc_coro.send(12)
exc_coro.send(13)
exc_coro.close()
print(inspect.getgeneratorstate(exc_coro))
```

> raise RuntimeError('This line should never run.') 永远不会执行，因为只有未处理的异常才会终止循环，而一旦出现未处理的异常，协程会立即终止。

执行上述代码得到结果为：

```python
-> coroutine started
--> coroutine received: 11
--> coroutine received: 12
--> coroutine received: 13
GEN_CLOSED    # 协程终止
```

上述代码，如果传入DemoException，协程不会中止，因为做了异常处理。

```python
exc_coro = exc_handling()

exc_coro.send(11)
exc_coro.send(12)
exc_coro.send(13)
exc_coro.throw(DemoException) # 协程不会中止，但是如果传入的是未处理的异常，协程会终止
print(inspect.getgeneratorstate(exc_coro))
exc_coro.close()
print(inspect.getgeneratorstate(exc_coro))

## output-> coroutine started
--> coroutine received: 11
--> coroutine received: 12
--> coroutine received: 13
*** DemoException handled. Conginuing...
GEN_SUSPENDED
GEN_CLOSED
```

如果不管协程如何结束都想做些处理工作，要把协程定义体重的相关代码放入try/finally块中。

```python
@coroutinue
def exc_handling():
    print('-> coroutine started')
    try:
        while True:
            try:
                x = yield
            except DemoException:
                print('*** DemoException handled. Conginuing...')
            else:                # 如果没有异常显示接收到的值
                print('--> coroutine received: {!r}'.format(x))
    finally:
        print('-> coroutine ending')
```

