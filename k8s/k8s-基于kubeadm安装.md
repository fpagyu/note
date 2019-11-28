# 基于阿里云镜像站安装Kubernetes



> 环境：centos7



## 准备镜像源

**CentOS7镜像源**

[使用aliyun的镜像源](https://developer.aliyun.com/mirror/centos?spm=a2c6h.13651102.0.0.53322f70Ctcqey)

```bash
# step1: 备份
mv /etc/yum.repos.d/CentOS-Base.repo /etc/yum.repos.d/CentOS-Base.repo.backup

# step2: 下载新的CentOS-Base.repo
wget -O /etc/yum.repos.d/CentOS-Base.repo http://mirrors.aliyun.com/repo/Centos-7.repo

# 非aliyun ECS用户
sed -i -e '/mirrors.cloud.aliyuncs.com/d' -e '/mirrors.aliyuncs.com/d' /etc/yum.repos.d/CentOS-Base.repo

# 
yum makecache
```

**docker镜像源**

```bash
wget -O /etc/yum.repos.d/docker-ce.repo https://mirrors.aliyun.com/docker-ce/linux/centos/docker-ce.repo
```

**kubernetes镜像源**

```bash
# cat /etc/yum.repos.d/kubernetes.repo

[kubernetes]
name=Kubernetes Repo
baseurl=https://mirrors.aliyun.com/kubernetes/yum/repos/kubernetes-el7-x86_64/
gpgcheck=1
gpgkey=https://mirrors.aliyun.com/kubernetes/yum/doc/rpm-package-key.gpg
enabled=1
```

**查看源是否生效**

```
# yum clean all
# yum repolist
# yum makecache
```

## 安装前准备工作

```bash
# 安装必要的系统工具
yum install -y yum-utils device-mapper-persistent-data lvm2
yum install -y conntrack ipvsadm ipset jq sysstat curl iptables libseccomp

# 关闭防火墙
systemctl stop firewalld && systemctl disable firewalld

# 重置iptables
iptables -F && iptables -X && iptables -F -t nat && iptables -X -t nat && iptables -P FORWARD ACCEPT

# 关闭swap
sudo swapoff -a
sudo sed -i '/swap/s/^\(.*\)$/#\1/g' /etc/fstab

# 关闭selinux
setenforce 0

# 关闭dnsmasq(否则可能导致docker容器无法解析域名)
service dnsmasq stop && systemctl disable dnsmasq


#
cat <<EOF >  /etc/sysctl.d/k8s.conf
net.bridge.bridge-nf-call-ip6tables = 1
net.bridge.bridge-nf-call-iptables = 1
net.ipv4.ip_forward = 1
EOF
sysctl --system

systemctl daemon-reload
systemctl restart kubelet

```

## 安装docker

```bash
# 选择合适的docker-ce版本, 查看镜像源中有效的docker版本
yum list docker-ce --showduplicates | sort -r

# 安装docker
yum install -y docker-ce-18.03.0.ce

sudo systemctl daemon-reload
sudo systemctl start docker
sudo systemctl enable docker

# 修改driver Cgroup -> systemd
vi /etc/docker/daemon.json

{
  "exec-opts":["native.cgroupdriver=systemd"]
}
```

## 安装 kubelet kubeadm kubectl

```bash
yum install -y kubelet kubeadm kubectl
```

**设置kubelet**

查看kubelet安装生成了哪些文件

```
[root@master ~]# rpm -ql kubelet
/etc/kubernetes/manifests              # 清单目录
/etc/sysconfig/kubelet                 # 配置文件
/etc/systemd/system/kubelet.service    # unit file
/usr/bin/kubelet                       # 主程序
```

**默认**的配置文件

```
[root@master ~]# cat /etc/sysconfig/kubelet
KUBELET_EXTRA_ARGS=
```

修改kubelet的配置文件

```
[root@master ~]# cat /etc/sysconfig/kubelet
KUBELET_EXTRA_ARGS="--fail-swap-on=false"
```

此时还无法正常启动kubelet，先设置kubelet开机自启动，使用如下命令： `systemctl enable kubelet` 。

## 拉取Kubernetes镜像

```bash
for i in `kubeadm config images list`; do 
  imageName=${i#k8s.gcr.io/}
  docker pull registry.aliyuncs.com/google_containers/$imageName
  docker tag registry.aliyuncs.com/google_containers/$imageName k8s.gcr.io/$imageName
  docker rmi registry.aliyuncs.com/google_containers/$imageName
done;
```

## kubeadm init

在master节点上执行

```
kubeadm init --kubernetes-version=v1.16.3 --pod-network-cidr=10.244.0.0/16 --service-cidr=10.96.0.0/12 --ignore-preflight-errors=Swap
```

在worker节点上执行

```bash
kubeadm join 10.0.2.15:6443 --token n2oyku.8n4uvaww5m3mmihw --discovery-token-ca-cert-hash sha256:c08f9abb8c93f21e70b44b6ee11170a386c23ffd822820f084912b1220853b9c
```

