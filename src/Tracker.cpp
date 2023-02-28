// gcc -I E:\MyBittorrent\lib\ -c Tracker.cpp

#include "Tracker.h"

size_t Tracker::WriteCallback(char *contents, size_t size, size_t nmemb, void *userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

std::string Tracker::getURL() const
{
    std::string trueURL= URL;
    trueURL += "?";
    trueURL += "info_hash=";
    // std::cout<< "info hash: " << info_hash<< std::endl;
    std::cout<<"url encoded info hash: " << UrlEncodeHexString(info_hash)<< std::endl;
    trueURL += UrlEncodeHexString(info_hash); // The info hash should be a url encoded string, not a hex string.
    trueURL += "&peer_id=";
    trueURL += local_id;
    trueURL += "&uploaded=";
    trueURL += std::to_string(uploaded);
    trueURL += "&downloaded=";
    trueURL += std::to_string(downloaded);
    trueURL += "&left=";
    trueURL += std::to_string(file_size - downloaded);
    trueURL += "&port=";
    trueURL += std::to_string(port);
    trueURL += "&compact=";
    trueURL += std::to_string(compact);
    return trueURL;
}

void Tracker::updateDownloaded(unsigned long downloaded_)
{
    downloaded = downloaded_;
}

Tracker::Tracker(std::string URL_, std::string info_hash_, unsigned long file_size_, int port_, std::string local_id_)
{
    URL = URL_;
    info_hash = info_hash_;
    file_size = file_size_;
    port = port_;
    local_id = local_id_;
    compact = 0;
    uploaded = downloaded = 0;
}

std::string Tracker::getResponce()
{
    CURL *curl;
    CURLcode res;

    std::string trueURL = getURL();
    // FILE *fp;
    // if ((fp = fopen(filename, "w")) == NULL)  // 返回结果用文件存储
    //     return false;
    std::string readBuffer;
    // struct curl_slist *headers = NULL;
    // headers = curl_slist_append(headers, "Accept: Agent-007");
    curl = curl_easy_init();    // 初始化

    if (curl)
    {
        //curl_easy_setopt(curl, CURLOPT_PROXY, "10.99.60.201:8080");// 代理
        // curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);// 改协议头
        curl_easy_setopt(curl, CURLOPT_URL, trueURL.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        // curl_easy_setopt(curl, CURLOPT_HEADERDATA, &readBuffer);
        
        // curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp); //将返回的http头输出到fp指向的文件
        // curl_easy_setopt(curl, CURLOPT_HEADERDATA, fp); //将返回的html主体数据输出到fp指向的文件
        res = curl_easy_perform(curl);   // 执行
        if (res != 0) {
            // curl_slist_free_all(headers);
            // curl_easy_cleanup(curl);
            std::cout<< "Connect to traker error. Error code: "<< res <<std::endl;
        }
        curl_easy_cleanup(curl);
        // std::cout<<readBuffer<<std::endl;
        return readBuffer;
    }
    std::cout<< "Init curl error.";
    return readBuffer;
}