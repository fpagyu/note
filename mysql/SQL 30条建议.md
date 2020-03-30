## SQL 30条建议

**数据库连接池大小设置**
```
连接数 = ((核心数 * 2) + 有效磁盘数)
```
该公式有Postgres提供, 核心数不应该包括超线程


[后端程序员必备：书写高质量SQL的30条建议](https://juejin.im/post/5e624d156fb9a07ca80ab6f2)

[mysql优化原则_小表驱动大表IN和EXISTS的合理利用](https://segmentfault.com/a/1190000014509559)

