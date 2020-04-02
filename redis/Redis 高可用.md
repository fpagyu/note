## Redis 高可用

### redis 高可用方案

- Keepalived
- Codis (豌豆荚, 大规模集群)
- Twemproxy(twitter,大规模集群)
- Redis Sentinel (official)
- Redis Cluster



### Redis Sential

**方案1: 单机版Redis Server， 无Sentinel**

![img](https://user-gold-cdn.xitu.io/2018/9/25/1661015eb4f772df?imageView2/0/w/1280/h/960/format/webp/ignore-error/1)

> 缺点： 单点故障



**方案2: 主从同步Redis Server，单实例Sentinel**

![img](https://user-gold-cdn.xitu.io/2018/9/25/166101643c7beb8d?imageView2/0/w/1280/h/960/format/webp/ignore-error/1)

> 缺点: 虽然解决了Redis Server的单点故障，但没有解决Redis Sentinel的单点故障



**方案3: 主从同步Redis Server，双实例Sentinel**

![img](https://user-gold-cdn.xitu.io/2018/9/25/1661016dd6a1ee04?imageView2/0/w/1280/h/960/format/webp/ignore-error/1)

> 缺点：该方案基本没用，在其中一台服务器故障的时候，不会重新选master，因为投票率没有超过50%



**方案4: 主从同步Redis Server， 三实例Sentinel**

![img](https://user-gold-cdn.xitu.io/2018/9/25/16610178cdf882c0?imageView2/0/w/1280/h/960/format/webp/ignore-error/1)



###  阅读资料

**文章**

[这可能是目前最全的Redis高可用技术解决方案总结](https://yq.aliyun.com/articles/626532)

[教你用 3 台机器搞定一个 Redis 高可用架构](https://juejin.im/post/5baa0169f265da0b001f34be)