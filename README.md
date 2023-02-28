# MyBittorrentDownloader
这是一个用C++编写的具有简单功能的种子下载器。代码主要通过学习以下两个博客资料编写：
* [A BitTorrent client in Python 3.5](https://markuseliasson.se/article/bittorrent-in-python/)
* [重复造轮子的喜悦：从零开始用C++写一个最基础的BitTorrent客户端](https://zhuanlan.zhihu.com/p/386437665)


代码风格可能也不规范，欢迎指出。

***
## 使用到的三方库
[SHA-1 implementation in C++](https://github.com/vog/sha1)

[cpp-bencoding](https://github.com/s3rvac/cpp-bencoding)  稍作了修改

[curl](https://github.com/curl/curl)

***
## 系统环境
Ubuntu 20.04

安装curl: `sudo apt-get install libcurl4-openssl-dev`

***
## 安装编译

```
mkdir build
cd build
cmake ..
make
```
项目根目录下将生成一个可执行文件`BTDownloader`

***
## 测试
BTDownloader可通过命令行传入参数启动，也可直接启动后，根据提示填入种子文件路径。
参数格式如下：
`./BTDownloader torrent_file_path (max_connection_num)`
注意：该项目目前只支持下载包含单个文件的种子，包含多个文件的种子将解析失败。文件将被下载至本程序所在目录下的Download文件夹内，若不存在Download文件夹，将只执行下载流程，而不会写入硬盘。

inputs中提供了几个测试用种子文件，使用了如下测试命令，均下载成功：
```
./BTDownloader ./inputs/Dataset_BUSI.zip.torrent 2
./BTDownloader ./inputs/DukeMTMC-reID.zip.torrent
./BTDownloader ./inputs/LC25000.zip.torrent 2
./BTDownloader ./inputs/Market-1501-v15.09.15.zip.torrent 2
./BTDownloader ./inputs/MoralPsychHandbook.torrent 2
```

***
## 项目总体结构与程序执行流程
### 种子文件的解析——OneFileTorrentPaser
基于cpp-bencoding的解析，从中提取出了torrent文件的关键信息。
### 与Tracker的通信——Tracker
BitTorrent协议要求，与Tracker的通信使用HTTP协议，此处借助curl实现通信。获取Peers的信息。
### 与Peers的通信——Peer
创建了一个Peer类，来针对每个建立连接的Peer进行管理。在Peer初始化时就执行HandShack以及SendIntersted的操作。后续即是持续发送Request与接受处理即可。
### 下载内容的管理——PieceManager
Peer发送的Request都是通过PieceManager生成，在Peer收到数据后，都将传入PieceManager进行处理。若某Piece下载完成，PieceManager就将把该Piece写入文件。  
Peer与PieceManager的逻辑为本人自己设计，与上述参考资料并不一样。可能会有一些漏洞。设计逻辑如下：  
> Peer会持续不断发送Request，每次发送从PieceManager获取请求报文。  
> PieceManager会记录每个Peer上一次发送的PieceID，以及每一个Piece上一次请求的BlockID（可能会有多个Peer同时请求同一个Piece，防止各Peer重复请求同一个Block），并生成该Piece中下一个未下载Block对应的报文，若为最后一个Block则重新计数BlockID。直至该Piece下载完成，Peer进入对支持下载的下一个未下载Piece请求。
### 对下载过程中套接字的管理——main.cpp中Download()
本程序虽支持同时与多个Peer通信，但并非是多线程。而是通过维护一个socket list，用select()实现了多个套接字的通信。

***
## 一些问题
* 由于本程序下载为单线程，用select管理套接字，尽最大努力收发报文，tcp进行流式传输时一个包未完全到达时就会转入处理该peer数据，可能出现某一个peer很慢，进而影响下载速度的情况。

