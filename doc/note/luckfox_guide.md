# 一、官方资料

官网可以收集到很多有效的资料：[Luckfox Pico RV1103 | LUCKFOX WIKI](https://wiki.luckfox.com/zh/Luckfox-Pico-RV1103)

rk sdk 的文档资料：https://files.luckfox.com/wiki/Luckfox-Pico/PDF/doc.7z

我当前的目标是完成一些视频多媒体相关的开发，所以需要收集这部分的资料：
- Rockchip_Developer_Guide_MPI.pdf：描述了MPI的模块和使用方法
- https://github.com/LuckfoxTECH/luckfox_pico_rkmpi_example.git：一些使用MPI的例程，比较简单可入门参考


# 二、开发环境

我主要使用的是ubuntu 虚拟机进行开发，我希望以ubuntu为中心进行研发，希望所有动作都在虚拟机内产生，这样我可以通过ssh 登录到ubuntu，并且利用tmux 会话的特性，在片段的时间持续进行开发调试。
- luckfox-pico mini开发板，以下简称pico
- winows宿主机，以下简称windows
- windows上运行的ubuntu虚拟机，以下简称ubuntu



## 2.1、网络连接

网络用于在pico和ubuntu之间传输数据，是后面一切开发的基石，主要参考[网络共享 | LUCKFOX WIKI](https://wiki.luckfox.com/zh/Luckfox-Pico/Luckfox-Pico-Network-Sharing-1)

pico 的usb 接口可以实现RNDIS协议，模拟成一个usb 网络适配器，所以windows 会将其识别为一个usb 适配器，这样pico 就与windows建立了网络连接（172.32网段）。（然后通过网络共享的方式，也可以将pico 连接到互联网）

接下来需要将ubuntu也接入到172.32网段。原理是在vmware 里新增一个网络适配器VMNet，桥接到windows的RNDIS 适配器，然后编辑ubuntu 虚拟机配置，添加网络适配器VMNet。

进入ubuntu后，使用`ifconfig -a`查看新增的VMNet，名称可能是ens37，但是没有ip地址。所以我们随便给其设置一个地址，例如`sudo ifconfig ens37 172.32.0.101`

大功告成，这样ubuntu和pico就建立了基本的连接，如ssh。


## 2.2、网络共享

pico 开发应用需要与ubuntu进行数据交互，应用调试。ubuntu 开启nfs server，将文件、应用放到nfs server，pico 作为nfs client，mount 共享目录并进行访问。

ubuntu 端的设置脚本：
```bash
# 设置目录
mkdir ~/nfs
chmod 777 ~/nfs

# 安装nfs
sudo apt update
sudo apt install nfs-kernel-server

# 配置nfs server
sudo vi /etc/exports

# 增加以下配置：
# 将/home/prx/nfs 目录共享给172.32.0.0/16 整个网段的设备
# rw：允许读写
# sync：同步写到磁盘
# no_subtree_check：不检查子树
# no_all_squash：不将所有的客户端用户映射到服务器上的匿名用户
# no_root_squash：允许客户端的root用户在服务器上保持root权限
# 以下配置的权限风险比较大，但考虑到我们在内网执行调试，这样的设置可以避免很多权限问题
/home/prx/nfs 172.32.0.0/16(rw,sync,no_subtree_check,no_all_squash,no_root_squash)
/home/prx/nfs 192.168.1.0/24(rw,sync,no_subtree_check,no_all_squash,no_root_squash)

# 应用/etc/exports
sudo exportfs -a
# 启动nfs server
sudo systemctl restart nfs-server
```

在pico 端，挂载nfs 命令如下：
```bash
mount -t nfs -o nolock 172.32.0.101:/home/prx/nfs /mnt/nfs
```

如果windows 也想挂载该nfs 目录，也可以按以下步骤：
1. 确保windows  **控制面板** → **程序** → **程序和功能**->**启用或关闭 Windows 功能**中开启nfs 客户端
2. 映射网络驱动器：\\192.168.1.142\home\prx\nfs
目前测试下来只有只读权限