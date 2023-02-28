// g++ E:\MyBittorrent\lib\bencoding\*.o -I E:\MyBittorrent\lib\ -c *Paser.cpp

#include"OneFileTorrentPaser.h"

OneFileTorrentPaser::OneFileTorrentPaser(const std::string &file_path)
{
    try
    {
        std::ifstream file_stream(file_path, std::ifstream::binary);
        std::shared_ptr<bencoding::BItem> decodedTorrent = bencoding::decode(file_stream);
        root_dict = decodedTorrent->as<BDictionary>();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        std::cerr << "Can't open the torrent file or the torrent file is Invalid."<<std::endl;
    }
}

void OneFileTorrentPaser::printError(const std::exception& e, const std::string &no_item) const
{
    std::cerr << "exception caught: " << e.what() << '\n';
    std::cerr << "Invalid torrent file. Can't find " << no_item << "or " << no_item << "data is wrong.\n";
    exit(-1);
}

std::shared_ptr<BDictionary> OneFileTorrentPaser::getInfoDict() const
{
    std::shared_ptr<BDictionary> info_dict;
    try
    {
        auto info_item = root_dict->getValue("info");
        info_dict = info_item->as<BDictionary>();
    }
    catch(const std::exception& e)
    {
        printError(e, "info");
    }
    return info_dict;
}

std::string OneFileTorrentPaser::getAnnounce() const
{
    auto announce_ptr = root_dict->getValue("announce");
    std::string announce;
    try
    {
        announce = announce_ptr->as<BString>()->value();
    }
    catch(const std::exception& e)
    {
        printError(e, "announce");
    }
    return announce;
}
std::string OneFileTorrentPaser::getFilename() const
{
    auto info = getInfoDict();
    std::string file_name; 
    try
    {
        file_name = info->getValue("name")->as<BString>()->value();
    }
    catch(const std::exception& e)
    {
        printError(e, "file_name");
    }
    return file_name;
}
std::string OneFileTorrentPaser::getInfoHash() const
{
    auto info_dict = getInfoDict();
    const std::string info = encode(info_dict);
    SHA1 checksum;
    checksum.update(info);
    const std::string info_hash = checksum.final();
    // std::cout << "The SHA1 of Info is: " << info_hash << std::endl;
    return info_hash;
}
uint32_t OneFileTorrentPaser::getFileLength() const
{
    auto info = getInfoDict();
    uint32_t file_length;
    try
    {
        file_length = info->getValue("length")->as<BInteger>()->value();
    }
    catch(const std::exception& e)
    {
        printError(e, "file length");
    }
    return file_length;
}
uint32_t OneFileTorrentPaser::getPieceLength() const
{
    auto info = getInfoDict();
    uint32_t piece_length;
    try
    {
        piece_length = info->getValue("piece length")->as<BInteger>()->value();
    }
    catch(const std::exception& e)
    {
        printError(e, "piece length");
    }
    return piece_length;
}
std::vector<std::string> OneFileTorrentPaser::getPiecesHash() const
{
    auto info = getInfoDict();
    std::vector<std::string> pieces_hash;
    try
    {
        std::string pieces = info->getValue("pieces")->as<BString>()->value();
        assert(pieces.length() % 10 == 0);
        for(int i=0; i*HASH_LENGTH < pieces.length(); i++)
            pieces_hash.push_back(pieces.substr(i*HASH_LENGTH, HASH_LENGTH));
    }
    catch(const std::exception& e)
    {
        printError(e, "pieces hash");
    }
    return pieces_hash;
}