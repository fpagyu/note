## python 描述器



python描述器相关内容其实在《Python核心编程》(第二版)中有较为详细的描述。python写了两三年，时间久了，这些基础反而有些遗忘。恰好看到 **Python描述器引导(https://pyzh.readthedocs.io/en/latest/Descriptor-HOW-TO-Guide.html)**这篇文章， 其中对python描述器协议的描述已经相当的透彻了，特做此笔记。



### Python属性查找顺序

默认对属性的访问控制是从对象的字典(\_\_dict\_\_)中获取(get), 设置(set)和删除(delete)的。举例来说，a.x的查找顺序是a.\_\_dict\_\_['x'], 然后type(a).\_\_dict\_\_['x'], 然后找type(a)的父类(不包括元类)。如果查找到的值是一个描述器， Python就会调用描述器的方法来重写默认的控制行为。这个重写发生在这个查找环节的哪里取决于定义了哪个描述器方法。注意， 只有在新式类中描述器才会起作用。描述器正是属性， 实例方法， 静态方法， 类方法和super背后的实现机制。



### 描述器协议方法

> \_\_get\_\_(self, obj, type=None) --> value
> \_\_set\_\_(self, obj, value) --> None
> \_\_delete\_\_(self, obj) --> None

一个对象只要实现了上述任意一个方法，就会成为描述器。如果一个对象同时定义了\_\_get\_\_()和\_\_set\_\_()方法， 它就是资料描述器(data descriptor)。仅定义了\_\_get\_\_()的描述器叫非资料描述器(常用于方法)。

资料描述器和非资料描述器的重要区别是： 相对于实例的字典的优先级。如果实例字典中有与描述器同名的属性， 如果描述器是资料描述器， 优先使用资料描述器，如果是非资料描述器， 优先使用字典中的属性。

要想制作一个只读的资料描述器，需要同时定义\_\_set\_\_和\_\_get\_\_， 并在\_\_set\_\_中引发一个AttributeError异常。



**参考资料**

[Python描述器引导](https://pyzh.readthedocs.io/en/latest/Descriptor-HOW-TO-Guide.html)

[Python描述器引导-en](https://docs.python.org/3/howto/descriptor.html)

