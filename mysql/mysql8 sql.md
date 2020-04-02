## mysql8 sql

### 创建用户

```sql
-- 创建用户
create user 'test'@'%' identified by '123456';
-- 创建数据库
create database testdb;
-- 授权用户操作数据库的权限
grant all on testdb.* to 'test'@'%' with grant option;
```



### 创建表

```sql
CREATE TABLE `mt_A` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(20) NOT NULL DEFAULT '',
  `timestamp` bigint(20) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8
```

### 添加表索引

```sql
-- 修改表结构
ALTER TABLE mt_A ADD INDEX ref_timestamp_1234(timestamp);
-- 创建索引
CREATE INDEX ref_timestamp_1234 ON mt_A(timestamp); 
-- 如果索引字段是CHAR， VARCHAR...
CREATE INDEX ref_timestam_1234 ON mt_A(<columnName>(<length>));
```

