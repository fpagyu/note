## Mysql -- for update



Mysql的存储引擎是InnoDB时for update有行锁和表锁。



**用法**:

```sql
select ... for update
```



**Tips**:

> 仅适用于InnoDB, 必须在事务块begin/commit中才有效
>
> 当有明确的主键/索引时是行级锁， 否则是表级别锁tips

