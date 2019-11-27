# Mysql8.0



## 账户管理

#### 用户创建和授权 

**before mysql8.0**

```sql
mysql> grant all privileges on *.* to 'yuzj'@'%' on identified by 'password';
```

**since mysql8.0**

```sql
mysql> create user 'yuzj'@'%' identified by 'password';
mysql> grant all privileges on *.* to 'yuzj'@'%';
```

#### 认证插件更新

**密码认证插件**

mysql8.0 默认的身份认证插件是caching_sha2_password,

替代了之前的mysql_native_password.

```sql
mysql> show variables like 'default_authentication%';
mysql> select user,host,plugin from mysql.user;
```

**密码管理**

mysql8.0开始允许限制重复使用以前的密码.

```sql
mysql> show variables like 'password%';
```

password_history=3     // 新密码不能与最近3次使用的密码相同, 默认是0

password_reuse_interval=90 // 新密码不能与90天内使用过的密码相同,默认是0

password_require_current=ON  // 修改密码时需要输入当前的密码, 默认是OFF

**since mysql8.0 可以持久化的动态修改全局变量，在服务重启之后配置不丢失**

```sql
mysql> set persist password_history=5;
```

在8.0之前的版本中通过set global password_history=5;在mysql服务重启之后配置就会丢失.

**修改密码**

```
mysql> alter user 'yuzj'@'%' identified by 'new_password'
```

**权限管理**

mysql8.0提供角色管理功能， 角色是一组权限的集合.

在mysql8.0之前，用户和权限之间是直接关联的，mysql8.0在用户和权限之间增加了一层角色,

通过角色可以更方便的管理用户权限.

```sql
mysql> create database testdb;
mysql> create table testdb.t1(id integer(11));
mysql> create role 'write_role';   -- 创建角色
mysql> grant insert,update,delete on testdb.* to  'write_role'; --赋予角色插入，更新，删除权限
mysql> grant select on testdb.* to 'write_role'; --赋予角色查询权限
mysql> create user 'user1'@'%' identified by 'user1'; --创建用户
mysql> grant 'write_role' to 'user1'@'%';                   -- 赋予用户角色权限
mysql> show grants for 'user1';                         -- 查看用户权限
mysql> set default role 'write_role' to 'user1';        -- 设置用户默认的角色
mysql> revoke delete,update on testdb.* from 'write_role'; --撤销角色权限
mysql> show grants for user1;
mysql> show grants for user1 using 'write_role';
```

## 优化器索引

#### 隐藏索引

mysql8.0开始支持隐藏索引(invisible index), 对优化器来说不可见.

隐藏索引不会被优化使用，但仍需维护.

应用场景: 软删除(软删除索引)、灰度发布。

#### 降序索引

mysql8.0开始真正支持降序索引(descending index).

只有InnoDB存储引擎支持降序索引， 只支持BTREE降序索引

mysql8.0不在对Group BY操作进行隐式排序

#### 函数索引

