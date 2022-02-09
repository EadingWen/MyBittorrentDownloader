#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>

#include "bencoding/bencoding.h"
#include "OneFileTorrentPaser.h"


// bencoding Document:
// https://projects.petrzemek.net/cpp-bencoding/doc/latest/

int main() {
    std::string file_path = "./inputs/ubuntu-16.04-desktop-amd64.iso.torrent";
    // std::cin >> file_path;
    OneFileTorrentPaser torrent(file_path);
    std::cout<<"announce: "<< torrent.getAnnounce() <<std::endl;
    std::cout<<"file name: "<< torrent.getFilename() << std::endl;
    std::cout<<"hash: "<< torrent.getInfoHash() << std::endl;
    std::cout<<"file length: "<< torrent.getFileLength() << std::endl;
    std::cout<<"piece length: "<< torrent.getPieceLength() << std::endl;
    std::cout<<"pieces hash: " <<std::endl;
    auto pieces_hash = torrent.getPiecesHash();
    for(auto each: pieces_hash)
        std::cout<< each << std::endl;
    std::cout<< std::endl;

    system("pause");
    return 0;
}