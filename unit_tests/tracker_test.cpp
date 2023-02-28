#include "Tracker.h"

int main()
{
    OneFileTorrentPaser tp("./inputs/MoralPsychHandbook.torrent");
    std::cout<<"announce: "<< tp.getAnnounce() <<std::endl;
    std::cout<<"file name: "<< tp.getFilename() << std::endl;
    std::cout<<"hash: "<< tp.getInfoHash() << std::endl;
    std::cout<<"file length: "<< tp.getFileLength() << std::endl;
    std::cout<<"piece length: "<< tp.getPieceLength() << std::endl;
    Tracker tracker(tp.getAnnounce(),tp.getInfoHash(),tp.getFileLength(),6897);

    std::string responce = tracker.getResponce();
    // std::cout<<"responce: " << responce <<std::endl;

    std::shared_ptr<bencoding::BItem> decodeResponse = bencoding::decode(responce);
    std::cout << bencoding::getPrettyRepr(decodeResponse)<<std::endl;
    return 0;
}