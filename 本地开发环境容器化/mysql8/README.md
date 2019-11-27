## docker 中启动mysql方法


**通过下面的方式启动mysql的docker容器**


```bash
# way 1
docker run --name mysql -v /home/mysql/data:/var/lib/mysql -e MYSQL_ROOT_PASSWORD=root -p 3306:3306 -d mysql:8

# way 2
docker-compose up -d
```

容器第一次启动时, 只能够通过root用户登入数据库，此时root用户是未设置密码的
```
docker run -it mysql mysql -uroot
```
通过上面的命令就进入了mysql的命令行交互模式:
通过创建用户，用户权限管理，赋予用户远程访问的能力

这样就能够在本地宿主机上访问mysql了
