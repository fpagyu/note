---

---

## 红黑树(Red-Black-Tree) PART I

红黑树是一种特殊的二叉搜索树. 红黑树满足二叉搜索树的一切特征(对任意节点A, 其左子树中节点值皆小于A的值, 其右子树中的值都大于A的值).普通二叉搜索树在某些情况下会退化为线性结构, 导致其搜索性能下降. 为了解决这一问题, 引入了红黑树这种结构.



### 红黑树的性质

> 1. 树中任意节点的颜色非红即黑
> 2. 树的根节点一定是黑色的
> 3.  NIL节点(null, nil 或none)是黑色的
> 4. 如果一个节点是红色的，则它的子节点必须是黑色的
> 5. 从一个节点到该节点的子孙节点的所有路径上包含相同数目的黑节点

红黑树的上述性质保证了红黑树具有下面的特征

> 定理: 一棵含有n个节点的红黑树的高度至多为2log(n+1), 即h <= 2log(n+1)



### 红黑树定义

```go
const (
	RED uint8 = 0
    BLACK uint8 = 255
)

type RBNode struct {
    left *RBNode
    right *RBNode
    parent *RBNode
    
    key int
    color uint8
}

type RBTree struct {
    root *RBNode
}
```



### 搜索

搜索过程同二叉查找树, 比较简单, 代码如下:

```go
func (t *RBTree) search(node *RBNode, key int) *RBNode {
    for node != nil {
        if key == node.key {
            break
        }
        if key < node.key {
            node = node.left
        } else {
            node = node.right
        }
    }
    return node
}

func (t *RBTree) Search(key int) *RBNode {
    return t.search(t.root, key)
}
```

### 节点颜色

```go
// 设置节点颜色
func (t *RBTree) setColor(node *RBNode, color uint8) {
    if node != nil {
        node.color = color
    }
}

func (t *RBTree) colorOf(node *RBNode) uint8 {
    if node == nil || node.color == 255 {
        return BLACK
    } else {
        return RED
    }
}
```





### 插入和删除

#### 左旋

```go
func (t *RBTree) leftRotate(node *RBNode) *RBNode {
	r := node.right
	node.right = r.left
	if node.right != nil {
		node.right.parent = node
	}
    
    r.parent = node.parent
    if r.parent == nil {
        t.root = node
    } else {
        if node == node.parent.left {
            node.parent.left = r
        } else {
            node.parent.right = r
        }
    }
    node.parent = r
    r.left = node
    
    return r
}
```

### 右旋

```go
func (t *RBTree) rightRotate(node *RBNode) *RBNode {
    l := node.left
    node.left = l.right
    if node.left != nil {
        node.left.parent = node
    }
    l.parent = node.parent
    if node.parent == nil {
        t.root = l
    } else {
        if node = node.parent.left {
            node.parent.left = l
        } else {
            node.parent.right = l
        }
    }
    l.right = node
    node.parent = l
    
    return l
}
```

### 插入

**step 1:** 将节点插入到红黑树中, 此过程与普通二叉查找树的插入并无区别

**step 2:** 检查红黑树的性质是否被破坏, 如果被破坏, 则需要修正红黑树的结构

> case 1: 新插入的节点作为树的根节点, 只需将新节点的颜色设置为黑色即可
>
> case 2: 新插入节点的父亲节点是黑节点, 此时二叉树性质没有发生改变, 不需要做任何处理
>
> case 3: 新插入节点的父亲节点是红节点, 违反了性质4, 需要做如下处理. 分两种情况, 1)父亲节点是祖父节点的左孩子; 2)父亲节点是祖父节点的右孩子;两种情况的处理方式是类似的, 下面就按情况1)进行讨论

| case   | 描述                                                         | 操作                                                         |
| :----- | ------------------------------------------------------------ | ------------------------------------------------------------ |
| case 1 | 新插入的节点为树的根节点                                     | 1)将树根节点设置为黑色                                       |
| case 2 | 新插入节点的父亲节点是黑节点                                 | 无需处理                                                     |
| case 3 | 新插入节点的父亲节点是红节点,破坏了性质4, 且其叔叔节点是红节点 | 1) 将父亲节点设置为黑色; 2) 将叔叔节点设置为黑色; 3)将祖父节点设置为红色 |
| case 4 | 新插入节点的父亲节点是红节点,破坏了性质4; 且新节点是其父亲节点的右孩子 | 处理思路: 将其转换为case5. 以其父节点为失衡点, 左旋          |
| case 5 | 新插入节点的父亲节点是红节点,破坏了性质4; 且新节点是其父亲节点的左孩子 | 1)  将当前节点的父节点设置为黑色; 2) 将当前节点的祖父节点设置为红色; 3) 以当前节点的祖父节点为失衡点右旋 |

下面是针对case3, case4, case5 做的图示例:

![rbtree_insertfix](https://github.com/fpagyu/notebook/raw/master/images/rbtree_insertfix.png)

```go
func (t *RBTree) Insert(key int) *RBNode {
    node := t.root
    var parent *RBNode
    for node != nil {
        if key == node.key {
            // 树中已存在相同key的节点
            return
        }
        parent = node
        if key < node.key {
            node = node.left
        } else {
            node = node.right
        }
    }
    
    node = &RBNode{key: key, parent: parent, color: RED}
    if parent == nil {
        t.root = node
    } else {
        if key < parent.key {
            parent.left = node
        } else {
            parent.right = node
        }
    }
    
    // 至此已经完成插入操作, 至此已经完成了插入操作
    // 但插入操作可能会改变红黑树的性质, 需要重新修正
    t.insertFix(node)
    return node
}

func (t *RBTree) insertFix(node *RBNode) {
    // case1: 插入节点即树根节点
    if node.parent == nil {
        t.setColor(node, BLACK)
        return
    }
    
    // case2: 插入节点的父亲节点为黑色节点, 没有破坏红黑树的性质
    // case3: 插入节点的父节点为红色节点, 违反了红黑树的性质4
    for node.parent != nil && t.colorOf(node.parent) == RED {
        if node.parent == node.parent.left {
            uncle := node.parent.parent.right
            // case1: 插入节点的叔叔节点是红色的
            // 令插入节点的叔叔节点和父亲节点变为黑节点, 祖父节点变为红节点
            if t.colorOf(uncle) == RED {
                t.setColor(node.parent, BLACK)
                t.setColor(uncle, BLACK)
                t.setColor(node.parent.parent, RED)
                node = node.parent.parent
            } else {
                if node == node.parent.right {
                    // case2: 插入节点是其父亲节点的右孩子(插入节点,父亲节点,祖父节点不在一条直线上)
                    // 以父亲节点作为失衡点,左旋, 变成case3, 继续处理
                    t.leftRotate(node.parent)
                    node = node.left
                }
                // case3: 插入节点是其父亲节点的左孩子(插入节点,父亲节点,祖父节点在一条直线上)
                // 1)node的父亲设置为黑色
                // 2)node的祖父设置为红色
                // 3)以祖父节点为失衡点, 右旋
                t.setColor(node.parent, BLACK)
                t.setColor(node.parent.parent, RED)
                t.rightRotate(node.parent.parent)
            }
        } else {
            uncle := node.parent.parent.left
            if t.colorOf(uncle) == RED {
                t.setColor(node.parent, BLACK)
                t.setColor(uncle, BLACK)
                t.setColor(node.parent.parent, RED)
                node = node.parent.parent
            } else {
                if node == node.parent.left {
                    t.rightRotate(node.parent)
                    node = node.right
                }
                t.setColor(node.parent, BLACK)
                t.setColor(node.parent.parent, RED)
                t.leftRotate(node.parent.parent)
            }
        }
    }
    t.setColor(t.root, BLACK)
}
```

