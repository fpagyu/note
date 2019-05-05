## django group 查询

## 一个简单的例子



```python
class Entry(models.Model):
    field1 = IntegerField()
	
# group by 查询例子
r = Entry.objects.all().only("field1")
r.query.group_by = ["field1"]

```



