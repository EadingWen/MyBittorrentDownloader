#ifndef ONE_FILE_TORRENT_PASER
#define ONE_FILE_TORRENT_PASER

#include "bencoding/bencoding.h"
#include<exception>
#include<iostream>
#include<assert.h>
#include"sha1.hpp"
using namespace bencoding;

class OneFileTorrentPaser
{
private:
    const static int HASH_LENGTH = 20;
    std::shared_ptr<BDictionary> root_dict;
    std::shared_ptr<BDictionary> getInfoDict() const;
    void printError(const std::exception& e, const std::string &no_item) const;
public:
    OneFileTorrentPaser(const std::string &file_path);
    std::string getAnnounce() const;
    std::string getFilename() const;
    std::string getInfoHash() const;
    uint32_t getFileLength() const;
    uint32_t getPieceLength() const;
    std::vector<std::string> getPiecesHash() const;
};

#endif