# Redis笔记(01)


## string

> set key value [ex 秒数] | [px 毫秒数]
```
# 设置name="fpgayu"
set name fpgayu

# 设置name="fpgayu", 并设置过期时间为100s
set name fpgayu ex 100
```
