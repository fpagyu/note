## Mysql -- for update



Mysql的存储引擎是InnoDB时for update有行锁和表锁。



**使用场景**:

存在高并发并且对于数据的准确性很有要求的场景

比如涉及到金钱、库存等。一般这些操作都是很长一串并且是开启事务的。如果库存刚开始读的时候是1， 而另一个进程立马进行了update将库存更新为0了，而事务还没有结束， 会将错的数据一致执行下去，就会有问题。所以需要for update进行数据加锁防止高并发时候数据出错。



Mysql的存储引擎是InnoDB时for update有行锁和表锁。



**用法**:

```sql
select ... for update
```



**Tips**:

> 仅适用于InnoDB, 必须在事务块begin/commit中才有效
>
> 当有明确的主键/索引时是行级锁， 否则是表级别锁tips



**用法**:

```sql
select ... for update
```



**Tips**:

> 仅适用于InnoDB, 必须在事务块begin/commit中才有效
>
> 当有明确的主键/索引时是行级锁， 否则是表级别锁tips

