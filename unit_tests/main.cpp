#include<iostream>
#include"PieceManager.h"
#include"Peer.h"
#include "OneFileTorrentPaser.h"
#include "Tracker.h"
#include "SockList.h"
#ifdef _WIN32
    #include <winsock.h>
    #pragma comment (lib,"wsock32.lib")
    #pragma warning(disable:4996)
#elif __linux__
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <sys/select.h>
#endif

#pragma comment (lib,"wsock32.lib")
#pragma warning(disable:4996)

#define LOCAL_ID "-YD2333-706887310628"
#define LOCAL_START_PORT 13555

void init()
{
    #ifdef _WIN32
        struct WSAData wsa;
        WSAStartup(0x101, &wsa);
    #endif
}

void end()
{
    #ifdef _WIN32
        WSACleanup();
    #endif
}

int Download(std::string file_path, int max_peer_num = 1)
{
    init();

    // std::string file_path = "./inputs/ComputerNetworks.torrent";
    // std::cin >> file_path;
    OneFileTorrentPaser torrent(file_path);
    std::cout<<"announce: "<< torrent.getAnnounce() <<std::endl;
    std::cout<<"file name: "<< torrent.getFilename() << std::endl;
    std::cout<<"hash: "<< torrent.getInfoHash() << std::endl;
    std::cout<<"file length: "<< torrent.getFileLength() << std::endl;
    std::cout<<"piece length: "<< torrent.getPieceLength() << std::endl;
    // std::cout<<"pieces hash: " <<std::endl;
    // auto pieces_hash = torrent.getPiecesHash();
    // for(auto each: pieces_hash)
    //     std::cout<< each << std::endl;
    std::cout<< std::endl;

    PieceManager pm(torrent.getFileLength(),torrent.getPieceLength(),torrent.getPiecesHash(),torrent.getFilename());
    
    Tracker tracker(torrent.getAnnounce(),torrent.getInfoHash(),torrent.getFileLength(),LOCAL_START_PORT);

    std::string responce = tracker.getResponce();
    // std::cout<<"responce: " << responce <<std::endl;

    std::shared_ptr<bencoding::BItem> decodeResponse = bencoding::decode(responce);
    // std::shared_ptr<bencoding::BDictionary> responseDict = decodeResponse->as<BDictionary>();
    // std::cout << bencoding::getPrettyRepr(decodeResponse)<<std::endl;

    
    
    uint32_t interval = decodeResponse->as<BDictionary>()->getValue("interval")->as<BInteger>()->value();

    // int max_peer_num = 2;
    std::vector<Peer> peer_list;
    socket_list sock_list;
    init_list(&sock_list);
    fd_set readfds, writefds, exceptfds;
	struct timeval timeout;
    int retval;

    timeout.tv_sec = 1;
	timeout.tv_usec = 0;
    
    for(const auto&item : *decodeResponse->as<BDictionary>()->getValue("peers")->as<BList>())
    {
        if(max_peer_num == peer_list.size()) break;
        std::string peer_ip = item->as<BDictionary>()->getValue("ip")->as<BString>()->value();
        std::string peer_id = item->as<BDictionary>()->getValue("peer id")->as<BString>()->value();
        uint16_t peer_port = item->as<BDictionary>()->getValue("port")->as<BInteger>()->value();
        Peer p (LOCAL_ID,torrent.getInfoHash(),peer_id,peer_ip,peer_port,&pm);
        if(!p.Connected()) continue;
        peer_list.push_back(p);
        insert_list(p.Sock(),&sock_list);
    }

	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	FD_ZERO(&exceptfds);
    if(peer_list.empty()) return 0;
    while(1)
    {
        // if(peer_list[0].handleMessage()) return 0;
        make_fdlist(&sock_list, &readfds);
        make_fdlist(&sock_list, &writefds);
        retval = select(FD_SETSIZE, &readfds, &writefds, &exceptfds, &timeout);
        if(retval < 0){
            printf_error();
            goto exit;
        }
        for(auto peer : peer_list){
            int newsock = peer.Sock();

            if(FD_ISSET(newsock, &readfds)){
                //recv data
                if(peer.handleMessage())
                {
                    #ifdef _WIN32
                        closesocket(peer.Sock());
                    #elif __linux__
                        close(peer.Sock());
                    #endif
                    return 0;
                }
            }
            if(FD_ISSET(newsock, &writefds)){
                peer.sendRequest();
            }
            
    }
    }
    exit:{
        end();
        return 0;
        }
    return 0;
}

int main(int argc,char *argv[])
{
    if(argc == 3)
        return Download(argv[1], atoi(argv[2]));
    if(argc == 2)
        return Download(argv[1]);
    if(argc == 1)
    {
        std::cout<< "Please input torrent file path:"<<std::endl;
        std::string path;
        std::cin>> path;
        return Download(path);
    }
    std::cout<< "Input arg wrong."<<std::endl;
    // Download("./inputs/MoralPsychHandbook.torrent");
    return -1;
}
//g++ -c .\*.cpp
//g++ .\lib\bencoding\*.o -I .\lib\ -c *Paser.cpp Tracker.cpp utils.cpp