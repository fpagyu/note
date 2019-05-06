## 红黑树(Red-Black-Tree) PART II

接上篇

在之前我们已经了解了红黑树的查找, 旋转和插入等操作, 接下来继续实现红黑树的删除操作. 红黑树的删除操作较为复杂.

### 查找树的最小/大值节点

```go
func (t *RBTree) FindMin(node *RBNode) *RBNode {
    for node != nil {
        if node.left == nil {
            break
        }
        node = node.left
    }
    return node
}

func (t *RBTree) FindMax(node *RBNode) *RBNode {
    for node != nil {
        if node.right == nil {
            break
        }
        node = node.right
    }
    return node
}
```

### 中序遍历的前序和后序



### 删除

同插入一样, 人人分两步走

> **step 1: **同普通二叉搜索树的删除一样, 找到待删除的点直接删除(参见二叉搜索树的删除)
>
> **step 2:** 删除节点之后, 修正二叉树, 以维持红黑树的性质

```go
func (t *RBTree) Remove(key int) *RBNode {
    node := t.root
    for node != nil {
        if key == node.key {
            // 找到目标节点
            break
        }
        if key < node.key {
            node = node.left
        } else {
            node = node.right
        }
    }
    
    if node == nil {
        // 目标节点不存在
        return nil
    }
    // 对目标节点node, 我们分两种情况进行讨论
    // case1: node没有孩子节点, 或仅有一个孩子节点, 将仅有的孩子节点作为node的替换节点, 直接删除node
    // case2: node的左右孩子都不为NIL节点, 我们找到node节点的后继节点, 将后继节点的值复制到node中
    //        删除后继节点, 删除后继节点的过程就转换为case1
    var delNode *RBNode
    if node.left == nil || node.right == nil {
        delNode = node
    } else {
        delNode = t.FindMin(node.right)
        node.key = delNode.key
    }
    
    // 确定delNode的替代节点
    if delNode.left != nil {
        node = delNode.left
    } else {
        node = delNode.right
    }
    
    if node != nil {
        node.parent = delNode.parent
        if delNode == delNode.parent.left {
            delNode.parent.left = node
        } else {
            delNode.parent.right = node
        }
        // 如果被删除节点是黑色的, 那么这条支路上黑节点数量减1, 
        // 必定会破坏红黑树的性质5, 因此需要调整红黑树的结构
        if t.colorOf(delNode) == BLACK {
            t.removeFix(node)
        }
    } else {
        // 此时node节点为空, 因为我们没有设置NIL节点为哨兵节点
        // 因此需要以delNode暂代哨兵节点, 完成修复工作以后
        // 再将delNode删除
        if delNode.parent == nil {
            // 这种情况下, 整棵树只有delNode这么一个节点, 因此直接删除即可
            t.root = nil
        } else {
            if t.colorOf(delNode) == BLACK {
                t.removeFix(delNode)
            }
            if delNode == delNode.parent.left {
                delNode.parent.left = nil
            } else {
                delNode.parent.right = nil
            }
        }
    }
    delNode.parent = nil
    delNode.left = nil
    delNode.right = nil
    
    return delNode
}

func (t *RBTree) removeFix(node *RBNode) {
    // case 1: node节点就是新的树根节点
    if node.parent == nil {
        t.setColor(node, BLACK)
        return
    }
    
    for node.parent != nil && t.colorOf(node) == BLACK {
        if node == node.parent.left {
            sib := node.parent.right
            if t.colorOf(sib) == RED {
                t.setColor(sib, BLACK)
                t.setColor(node.parent, RED)
                t.leftRotate(node.parent)
                sib = node.parent.right
            }
            
            if t.colorOf(sib.left) == BLACK && t.colorOf(sib.right) == BLACK {
                t.setColor(sib, RED)
                node = node.parent
            } else {
                if t.colorOf(sib.right) == BLACK {
                    t.setColor(sib, RED)
                    t.setColor(sib.left, BLACK)
                    t.rightRotate(sib)
                    sib = node.parent.right
                }
                t.setColor(sib, node.parent.color)
                t.setColor(node.parent, BLACK)
                t.setColor(sib.left, BLACK)
                t.leftRotate(node.parent)
                node = t.root
            }
        } else {
            sib := node.parent.left
            if t.colorOf(sib) == RED {
                t.setColor(sib, BLACK)
                t.setColor(sib.parent, RED)
                t.rightRotate(node.parent)
                sib = node.parent.left
            }
            
            if t.colorOf(sib.left) == BLACK && t.colorOf(sib.right) == BLACK {
                t.setColor(sib, RED)
                node = node.parent
            } else {
                if t.colorOf(sib.left) == BLACK {
                    t.setColor(sib, RED)
                    t.setColor(sib.right, BLACK)
                    t.leftRotate(sib)
                    sib = node.parent.left
                }
                t.setColor(sib, node.parent.color)
                t.setColor(node.parent, BLACK)
                t.setColor(sib.right, BLACK)
                t.rightRotate(node.parent)
                node = t.root
            }
        }
    }
    t.setColor(node, BLACK)
}
```

