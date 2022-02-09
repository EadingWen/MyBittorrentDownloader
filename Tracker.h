#ifndef TRACKER_
#define TRACKER_

#include <iostream>
#include <string>
#include <stdlib.h>
#include <map>
#include <vector>
#include <curl/curl.h>
#include "OneFileTorrentPaser.h"
#include "bencoding/bencoding.h"
#include "utils.h"

class Tracker
{
private:
    std::map<std::string,std::string> params;
    std::string URL;
    std::string local_id;
    std::string info_hash;
    unsigned long uploaded;
    unsigned long downloaded;
    unsigned long left;
    unsigned long file_size;
    int port;
    int compact;
    std::string getURL() const;
    static size_t WriteCallback(char *contents, size_t size, size_t nmemb, void *userp);
public:
    Tracker(std::string URL_, std::string info_hash_, unsigned long file_size, int port_ = 12365, std::string local_id_= "-YD2333-706887310628");
    void updateDownloaded(unsigned long downloaded_);
    std::string getResponce();

};

// uint32_t GetInterval(std::shared_ptr<BDictionary> response);

// std::vector<std::map<std::string,std::string>> GetPeerInfoList(std::shared_ptr<BDictionary> response);

#endif