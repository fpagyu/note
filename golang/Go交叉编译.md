# Go交叉编译



可以使用go tool dist list 命令查看交叉编译支持的操作系统列表。



交叉编译的命令:

```bash
CGO_ENABLED=0 GOOS=linux GOARCH=amd64 go build main.go
```

> CGO_ENABLED=0, 关闭cgo. go的交叉编译不支持cgo， 因此需要关闭cgo

> GOOS=linux, 设置目标操作系统

> GOARCH=amd64, 设置目标的cpu体系架构

