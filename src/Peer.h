#ifndef PEER_
#define PEER_
#include <string>
#include <iostream>
#include <stdio.h>
#include "BitTorrentMessage.h"
#include "PieceManager.h"
#include "utils.h"
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
#define BufferSize 100000
#define INFO_HASH_STARTING_POS 28
#define PEER_ID_STARTING_POS 48
#define PEER_ID_LEN 20
#define HASH_LEN 20

void printf_error()
{
    #ifdef _WIN32
        int retval = WSAGetLastError();
        fprintf(stderr, "socket error: %d\n", retval);
    #elif __linux__
        fprintf(stderr, "socket error: %s(errno: %d)\n",strerror(errno),errno);
    #endif
}

class Peer // Peer创建完成后应首先验证是否connected.
{
private:
    bool connected = false;
    bool choked = true;
    bool done = false;
    int sock;
    std::string client_id;
    std::string info_hash;
    uint16_t local_port;
    std::string peer_id;
    std::string peer_ip;
    unsigned int peer_port;
    char recvbuf[BufferSize];
    std::string bit_field;
    // uint32_t now_downloading;

    PieceManager *pm;
    bool handShake();
    bool Send(std::string message);
    BitTorrentMessage RecievePacket();
    bool sendInterested();
    bool recieveBitfiled();  
    void setBitfield(uint32_t idx);

public:
    Peer(std::string client_id_, std::string info_hash_,std::string peer_id_,
    std::string peer_ip_, uint16_t peer_port_, PieceManager* pm_, uint16_t local_port_ = 0);
    bool handleMessage();
    bool Connected();
    bool Done() const;
    int Sock() const;
    bool sendRequest();
    // uint32_t nowDownloading();
    // void setNowDownloading(uint32_t next_download);
    bool havePiece(uint32_t idx);
    ~Peer();
};

Peer::Peer(std::string client_id_, std::string info_hash_,std::string peer_id_,
    std::string peer_ip_, uint16_t peer_port_, PieceManager* pm_, uint16_t local_port_)
{
    client_id = client_id_;
    info_hash = info_hash_;
    local_port = local_port_;
    // if(local_port == 0)
    //     local_port = peer_port_;
    peer_id = peer_id_;
    peer_ip = peer_ip_;
    peer_port = peer_port_;
    pm = pm_;
    // now_downloading = 0;

    int retval;
    struct sockaddr_in client_addr, remote_addr;

    int s = socket(AF_INET, SOCK_STREAM, 0);
    if(s<0)
    {
        printf_error();
    }

    //bind
    client_addr.sin_family = AF_INET;
    #ifdef _WIN32
        client_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    #elif __linux__
        client_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    #endif
    client_addr.sin_port = htons(local_port);
    retval = bind(s, (struct sockaddr*)&client_addr, sizeof(client_addr));
    if(retval < 0){
        printf_error();
        #ifdef _WIN32
            closesocket(s);
        #elif __linux__
            close(s);
        #endif
    }

    //connect
    remote_addr.sin_family = AF_INET;
    //remote_addr.sin_addr.S_un.S_addr = htonl(INADDR_LOOPBACK);
    #ifdef _WIN32
        remote_addr.sin_addr.S_un.S_addr = inet_addr(peer_ip.c_str());
    #elif __linux__
        remote_addr.sin_addr.s_addr = inet_addr(peer_ip.c_str());
    #endif
    remote_addr.sin_port = htons(peer_port);
    retval = connect(s, (struct sockaddr*)&remote_addr, sizeof(remote_addr));
    if(retval < 0){
        printf_error();
        #ifdef _WIN32
            closesocket(s);
        #elif __linux__
            close(s);
        #endif
        return;
    }
    sock = s;

    // hand shake and recieve bit field.
    if(handShake() && recieveBitfiled())
    {
        sendInterested();
        handleMessage();
        // sendRequest();
        // while(1)
        //     handleMessage();
        connected = true;
    }
    else
        connected = false;
}

bool Peer::Send(std::string message)
{
    int retval;
    retval = send(sock,message.c_str(),message.length(),0);
    if(retval < 0){
        printf_error();
        return false;
    }
    return true;
}

BitTorrentMessage Peer::RecievePacket()
{
    int retval;
    uint32_t recvlen;
    uint8_t recvtype;
    memset(recvbuf, 0, sizeof(recvbuf));
    retval = recv(sock, recvbuf,4, MSG_WAITALL);
    if(retval == 0)
    {
        std::cout<< "Didn't recieve message." <<std::endl;
        return BitTorrentMessage (10);
    }
    if(retval < 0){
        printf_error();
        std::cout<<"Recieve Error!"<<std::endl;
        return BitTorrentMessage (10);
    }
    // memcpy(&recvlen,recvbuf,4);
    // recvlen = ntohl(recvlen);
    recvlen = Cstring2Uint32(recvbuf);

    if(recvlen == 0)
    {
        std::cout<< "Recieve a keep alive message."<<std::endl;
        return BitTorrentMessage (10);
    }

    retval = recv(sock, recvbuf, recvlen, MSG_WAITALL);
    if(retval < 0){
        printf_error();
        std::cout<<"Recieve Error!"<<std::endl;
        return BitTorrentMessage (10);
    }

    memcpy(&recvtype, recvbuf, 1);
    std::string buffer;
    for(int i = 1;i<retval;i++)
        buffer+=recvbuf[i];
    
    return BitTorrentMessage (recvtype, buffer);
}

bool Peer::sendRequest()
{
    if(choked) 
    {
        std::cout<< "Peer is chocked. Can't send request." <<std::endl;
        sendInterested();
        return false;
    }
    std::string s = pm->GetNextRequestString(peer_id, bit_field);
    if(s == "File Dowloaded.") return false;
    BitTorrentMessage btmassage (request,s);
    s = btmassage.toString();
    if(Send(s)) return true;
    return false;
}

bool Peer::sendInterested()
{
    BitTorrentMessage btmassage (interested);
    std::string s = btmassage.toString();
    if(Send(s)) return true;
    return false;
}

bool Peer::handShake()
{
    const std::string protocol = "BitTorrent protocol";
    std::string send_buffer; 
    send_buffer += (char) protocol.length();
    send_buffer += protocol;
    for(int i=0;i<8;i++)
        send_buffer += '\0';
    send_buffer += Hex2Ascii(info_hash);
    send_buffer += client_id;
    int retval;
    if(Send(send_buffer))
    {
        std::cout<< "Hand shake message sended."<< std::endl;

        memset(recvbuf,0,sizeof(recvbuf));
        retval = recv(sock, recvbuf, send_buffer.length(), MSG_WAITALL);
        if(retval < 0){
            printf_error();
            std::cout<<"Recieve Error!"<<std::endl;
            return false;
        }
        std::string recieved_peer_id = Cstring2String(recvbuf, PEER_ID_LEN, PEER_ID_STARTING_POS);
        std::string recieved_info_hash = Cstring2String(recvbuf,HASH_LEN,INFO_HASH_STARTING_POS);
        
        // std::cout<< "Remote peer id is:"<< recieved_peer_id << std::endl;
        if(peer_id != recieved_peer_id)
        {
            std::cout<< "Remote peer id is wrong." << std::endl; // It should be: " << peer_id <<std::endl; 
            // return false;
        }
        // std::cout<< "Remote peer info hash is:" << recieved_info_hash <<std::endl;
        if(Hex2Ascii(info_hash) != recieved_info_hash)
        {
            std::cout<< "Remote peer info hash is wrong. "<<std::endl; 
            return false;
        }
        return true;
    }
    return false;
}

bool Peer::handleMessage()
{
    auto btmessage = RecievePacket();
    if(btmessage.getMessageType() == 10) return false;
    std::string s = btmessage.getPayload();
    switch (btmessage.getMessageType())
    {
    case choke:
    {
        choked = true;
        std::cout<< "This peer has been chocked."<<std::endl;
        break;
    }
    case unchoke:
    {
        choked = false;
        std::cout<< "Peer has been unchocked."<<std::endl;
        // std::cout<< "Peer "<<peer_id<<" has been unchocked."<<std::endl;
        // if(!sendRequest())
        //     std::cout << "Send Request Error."<<std::endl;
        break;
    }
    case interested:
    {
        std::cout<< "Recieve a interested message. Pass."<<std::endl;
        break;
    }
    case notInterested:
    {
        std::cout<< "Recieve a not interested message. Pass."<<std::endl;
        break;
    }
    case have:
    {
        uint32_t idx = String2Uint32(s);
        setBitfield(idx);
        std::cout<< "Peer:" << peer_id <<" have "<< idx <<std::endl;
        break;
    }
    case bitField:
    {
        std::cout << "Recieve a bit field message again. Pass." <<std::endl;
        break;
    }
    case request:
    {
        std::cout<< "Recieve a request message. Pass.";
        break;
    }
    case piece:
    {
        pm->FillIn(String2Uint32(s.substr(0,4)),String2Uint32(s.substr(4,4)), s.substr(8));
        // if(!sendRequest() && pm->Done())
        if(pm->Done())
        {
            done = true;
            std::cout<< "File is downloaded." <<std::endl;
            return true;
        }
        break;
    }
    case cancel:
    {
        std::cout<< "Recieve a port message. Pass.";
        break;
    }
    case port:
    {
        std::cout<< "Recieve a port message. Pass.";
        break;
    }
    
    default:
        std::cout<< "Recieve a non-BitTorrentMessage. What happended?"<<std::endl;
        break;
    }
    return false;
}

bool Peer::recieveBitfiled()
{
    BitTorrentMessage btmessage = RecievePacket();
    if(btmessage.getMessageType() != bitField)
    {
        std::cout<< "Wrong: Recieve a wrong bit field message. Message id: " << btmessage.getMessageType() <<std::endl;
        return false;
    }
    bit_field = btmessage.getPayload();
    return true;
}

void Peer::setBitfield(uint32_t idx)
{
    bit_field[idx / 8] |= (1 << (7 - (idx%8)));
}

bool Peer::havePiece(uint32_t idx)
{
    return bit_field[idx / 8] & (1 << (7 - (idx%8)));
}

bool Peer::Connected()
{
    return connected;
}

bool Peer::Done() const
{
    return done;
}

int Peer::Sock() const
{
    return sock;
}

// uint32_t Peer::nowDownloading()
// {
//     return now_downloading;
// }

// void Peer::setNowDownloading(uint32_t next_download)
// {
//     now_downloading = next_download;
// }

Peer::~Peer()
{
}

#endif