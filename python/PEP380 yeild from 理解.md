## PEP380 yield from理解

yield from 是python3.3中新加入的语法，旨在通过提供一个委托生成器来提高生成器的灵活性。很长的一段时间内对yield from都不甚理解， 主要是英语渣， 没有好好通读PEP380。



### 协程生成器的基本行为





参考文档:

[PEP380 Syntax for Delegating to a Subgenerator](https://www.python.org/dev/peps/pep-0380/)

[python协程1：yield的使用](https://mp.weixin.qq.com/s?__biz=MzAwNjI5MjAzNw==&mid=2655751983&idx=1&sn=e4c093c6e5d6e4e8281d76db7c67eb23)

[python协程2：yield from 从入门到精通](https://segmentfault.com/a/1190000009781688)

