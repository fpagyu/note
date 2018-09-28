## python 元类编程

python的元类编程虽然在大多数场景下并非是必须的，但在一些框架中(比如django的orm)中还是有大量的应用。了解元类编程对阅读源码和理解python中一切皆对象大有裨益。

本文基于python3.6， python3中的元类编程与python2中有些许不同， 这里不做赘述。



### type 和 object

在正式讲解元类编程之前， 首先了解一下type和object。通常情况下我们可以通过type获取某个对象的类型信息, 但同时type也是一个类。我们都知道在Python的新式类中， object是所有类的基类, 同理，type作为一个类，那么object必定也是type的基类。

```python
>> type.__base__
<type 'object'>
```

type这个类具有创建其他类的能力，实际上就是我们要说的元类。如果我们需要自己实现一个元类，那么就需要继承type类。object 也是有type创建的。

```python
>> type(object)
<type 'type'>
```

上面就是type和object大致上的关系。额外补充说明，要想看一个类的继承链(mro顺序), 通过CLASS.\_\_mro\_\_查看即可。



### 使用type创建类

type是如何创建一个类的？ 通过在Python解释器终端上输入help(type), 我们看到type的两种用法，我们需要的就是它的第二种用法：

> class type(object)
>  |  type(object) -> the object's type
>  |  type(name, bases, dict) -> a new type

有三个参数，name表示新创建类的名字， bases表示新类的基类， dict表示类的属性和方法,。我们可以用下面的方法创建一个User类(基类是object， 没有属性和方法):

```
>> User = type('User', (), {})
```

### 使用metaclass影响类的创建过程

元类可以创建类对象，那么我们就可以通过元类控制类的生成过程, 而实例的生成则是依赖与\_\_new\_\_方法。下面举个例子：

```python
class MetaClass(type):
	def __new__(mcs, name, bases, attrs):
        # do-something
        return super().__new__(mcs, name, bases, attrs)

# in python3
class User(metaclass=MetaClass):
    def __init__(self, name, age):
        self.name = name
        self.age = age
        
    def __str__(self):
        return self.name
```

在这里如果想看到User的创建过程可以在class User 处打上断点, 通过pycharm进行单步调试。User是MetaClass这个类的实例， 实际上在生成User时，会调用MetaClass中的\_\_new\_\_方法(new方法的作用与上面type的作用类似)。因此我们可以在\_\_new\_\_中增加一些逻辑，以处理User类。django中的orm就是一个很典型的例子。



### 为什么要用元类

据说99%的python程序员是不需要使用元类的。讲真，我没有办法说出1,2,3...条原因来说明为什么需要使用元类。但在某些情况下使用元类，确实减少了很多重复性的工作。

[orm.py](https://gist.github.com/fpagyu/cd42d8a7615232cf0c3f6b24260c302d) 是一个使用元类和描述器相关知识实现的一个orm demo,  可以作为元类使用的一个学习例子。

