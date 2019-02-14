## elasticsearch API汇总

**文档(基于6.5版本)**

> [官方文档](https://www.elastic.co/guide/en/elasticsearch/reference/6.5/getting-started-search-API.html)

## cluster 健康

>  [`_cat` API

**es 集群健康**

```shell
curl -X GET "localhost:9200/_cat/health?v"
```

**es 集群节点**

```shell
curl -X GET "localhost:9200/_cat/nodes?v"
```

**es indices**

```shell
# 查看所有index
curl -X GET "localhost:9200/_cat/indices?v"
```

## Index

**创建索引**

```shell
# 创建一个名为customer的index(对比mysql,相当于添加了一张名为customer的表)
curl -X PUT "localhost:9200/customer?pretty"
```
**删除index**

```shell
# 删除名为customer的index
curl -X DELETE "localhost:9200/customer?pretty"
```

## doc操作

**往index中添加doc(INSERT, REPLACE)**

```shell
# 往名为customer的index中添加doc, 并指定id=1(对比mysql, 相当于在表中添加了一行记录)
# 如果customer中存在这个id的doc，则会替换旧的doc
curl -X PUT "localhost:9200/customer/_doc/1?pretty" -H 'Content-Type: application/json' -d'
{
  "name": "John Doe"
}
'

# 直接添加文档，不指定id
curl -X POST "localhost:9200/customer/_doc?pretty" -H 'Content-Type: application/json' -d'
{
  "name": "Jane Doe"
}
'

```

**获取doc(SELECT)**

```shell
# 类比mysql, 相当于在customer表中获取id=1的记录
curl -X GET "localhost:9200/customer/_doc/1?pretty"
```

**更新doc(UPDATE)**

```shell
# 更新操作，这里添加一个age字段
curl -X POST "localhost:9200/customer/_doc/1/_update?pretty" -H 'Content-Type: application/json' -d'
{
  "doc": { "name": "Jane Doe", "age": 20 }
}
'

# 等价于: update set age = age + 5 from customer where id = 1
curl -X POST "localhost:9200/customer/_doc/1/_update?pretty" -H 'Content-Type: application/json' -d'
{
  "script" : "ctx._source.age += 5"
}
'
```

**删除doc(DELETE)**

```shell
curl -X DELETE "localhost:9200/customer/_doc/2?pretty"
```

**批量操作(batch)**

> [`_bulk` API](https://www.elastic.co/guide/en/elasticsearch/reference/6.5/docs-bulk.html)

```shell
# 批量添加id=1和id=2的doc
curl -X POST "localhost:9200/customer/_doc/_bulk?pretty" -H 'Content-Type: application/json' -d'
{"index":{"_id":"1"}}
{"name": "John Doe" }
{"index":{"_id":"2"}}
{"name": "Jane Doe" }
'

# 更新id=1的doc，并删除id=2的doc
curl -X POST "localhost:9200/customer/_doc/_bulk?pretty" -H 'Content-Type: application/json' -d'
{"update":{"_id":"1"}}
{"doc": { "name": "John Doe becomes Jane Doe" } }
{"delete":{"_id":"2"}}
'

```

