#ifndef BIT_TORRENT_MESSAGE
#define BIT_TORRENT_MESSAGE
#include<string>

enum MessageType
{
    // keepAlive is a message with 4(lenth) all zero Bytes.
    choke = 0,
    unchoke = 1,
    interested = 2,
    notInterested = 3,
    have = 4,
    bitField = 5,
    request = 6,
    piece = 7,
    cancel = 8,
    port = 9
};

class BitTorrentMessage
{
private:
    uint8_t type;
    std::string payload;
    uint32_t message_len;

public:
    explicit BitTorrentMessage(uint8_t type_, const std::string& payload_="");
    std::string toString();
    uint8_t getMessageType() const;
    std::string getPayload() const;
};

BitTorrentMessage::BitTorrentMessage(uint8_t type_, const std::string& payload_)
  : type(type_),
    payload(payload_),
    message_len(payload_.length()+1)
{}

std::string BitTorrentMessage::toString()
{
    std::string str;
    char* message_len_bytes = (char*) &message_len;
    for(int i=0;i<4;i++)
    {
        #if __BYTE_ORDER__==__ORDER_LITTLE_ENDIAN__
        str += message_len_bytes[3 - i];
        #elif
        buffer += message_len_bytes[i];
        #endif
    }
    str += (char) type;
    str += payload;
    return str;
}

uint8_t BitTorrentMessage::getMessageType() const
{
    return type;
}

std::string BitTorrentMessage::getPayload() const
{
    return payload;
}


#endif