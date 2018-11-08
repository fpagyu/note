## RediSearch 构建搜索服务

最近一个星期手头上有一些开发任务，所以没有时间来写一些东西，趁着周末时间来总结一下最近接触的一些东西。

缘起于最近的一个需求，需要对一批数据打标签，并对外提供一个标签搜索服务。需求清晰并不困难，直接上搜索，反正开源的解决方案如此多：ElasticSearch，Sphinx，Redis-Search或者类似于Segmentfault一样直接在redis上构建。但具体使用哪一种解决方案，仍然需要考虑。由于数据量并不会非常大，顶多也就几万的数据，加之当前服务器的配置并不算高，因此es的方案直接pass掉了。Sphinx和Redis-Search这两个项目足够轻量，功能也可以，但两个项目都已经很久没有维护了，而直接基于Redis构建的方案足够简单，而且redis服务本身已经存在于当前的服务组件中了，因此不需要引入新的服务组件。恰在此时发现了Redisearch这个东西(和上文的Redis-Serach是不同的)，api丰富，基于redis且部署方便，因此就选择了它。

- [Elasticsearch](https://www.elastic.co/products/elasticsearch)
- [Sphinx](http://sphinxsearch.com/)
- [Redis-Search](https://github.com/huacnlee/redis-search)
- [Redisearch](https://github.com/RedisLabsModules/RediSearch)



RediSearch 网上的资料不多，好在东西比较简单，官方文档基本上够用了，github上有各种语言的实现api，其中有demo可供研究。

Redisearch并没有使用redis内建的数据结构，直接使用redis-cli查看并不直观，前期研究的时候，用的python调试比较简单。Redisearch中有两个概念index，schema。对比传统的关系型数据库，我倾向于将index一个表空间，处于传统数据库的database和table之间的一个东西，而schema定义这个表的结构。因此可以将index和schema的组合看作是传统数据库中的表，schema则是定义了这个表的结构，index标识了这个表的名字。

Reidsearch中有几种字段类型：numeric, text, tag。

### Tag 类型字段搜索语法

> @<field_name>:{ <tag> | <tag> | ...}

表示搜索符合其中任意<tag>的结果，这里是OR的关系。如果要实现AND的关系，则需要这样做：

> @<field_name>:{<tag> | ...} @<field_name>:{<tag> |...}

其他的搜索语法可参见官方文档：

https://oss.redislabs.com/redisearch/Query_Syntax/



此外，需要说明一点的是，通常新增一条document和删除一条document都比较简单，有直接的指令支持，而更新document的命令则与增加document的命令相同，"FT.ADD", 当更新操作需要额外指定两个参数：[REPLACE], [PARTIAL].



下面放出一段简单的go的例子：

```go
package main

import (
	"fmt"
	"log"

	"github.com/RedisLabs/redisearch-go/redisearch"
)

func main() {
    // 申明index，并定义schema
	client := redisearch.NewClient("localhost:6382", "btTheme")

	schema := redisearch.NewSchema(redisearch.DefaultOptions).
		AddField(redisearch.NewNumericFieldOptions("id", redisearch.NumericFieldOptions{Sortable: true, NoIndex: false})).
		AddField(redisearch.NewNumericFieldOptions("online", redisearch.NumericFieldOptions{Sortable: false, NoIndex: false})).
		AddField(redisearch.NewNumericFieldOptions("sort", redisearch.NumericFieldOptions{Sortable: true, NoIndex: true})).
		AddField(redisearch.NewTextFieldOptions("path", redisearch.TextFieldOptions{Sortable: false, NoIndex: true})).
		AddField(redisearch.NewTextFieldOptions("envelope", redisearch.TextFieldOptions{Sortable: false, NoIndex: true})).
		AddField(redisearch.NewTagField("lang")).
		AddField(redisearch.NewTagField("tags"))
	
    // 如果index存在则删除
	client.Drop()
    
	if err := client.CreateIndex(schema); err != nil {
		log.Fatal(err)
	}

	doc := redisearch.NewDocument("103", 1.0)
	doc.Set("id", 103).Set("online", 1).Set("sort", 1).
		Set("path", "/100/15234727b1f_94d.png").
		Set("lang", "英文,中文").
		Set("envelope", "https://cdn.invit.vip/100/1_8d7_fa6.jpeg").
		Set("tags", "春节")
	// // doc.Set("id", 102).Set("lang", "简中").Set("tags", "春节,节日")

	docs := []redisearch.Document{doc}
    // 设置Replace和Partial为True，如果文档存在则执行更新操作，否则创建新文档
	err := client.IndexOptions(redisearch.IndexingOptions{
		NoSave:  false,
		Replace: true,
		Partial: true,
	}, docs...)

	if err != nil {
		log.Fatal(err)
	}

	docs, total, err := client.Search(redisearch.NewQuery("@lang:{英文} @tags:{毕业季|春节}").SetSortBy("id", false).Limit(0, 10))
	if err != nil {
		log.Fatal(err)
	}

	log.Println("total: ", total)
	if total > 0 {
		fmt.Println("docs: ", docs)
	}
}
```



