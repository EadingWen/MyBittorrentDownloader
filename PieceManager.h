#ifndef PIECE_MANAGER
#define PIECE_MANAGER

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <winsock.h>
#include <bitset>
#include "sha1.hpp"
#include "utils.h"

#define Default_Block_len (2<<13)

class Block
{
private:
    uint32_t index;
    uint32_t begin;
    uint32_t length;
    bool done = false;
    std::string buff;
public:
    explicit Block(uint32_t index_, uint32_t begin_, uint32_t length_);
    bool Done() const;
    std::string RequestString() const;
    void FillIn(const std::string &buf);
    std::string GetBuff() const;
    void Clear();
};


class Piece
{
private:
    uint32_t length;
    uint32_t index;
    std::string hash;
    std::vector<Block> blocks;
    uint32_t downloaded;
    uint32_t nx_begin;
public:
    explicit Piece(uint32_t length_, uint32_t index_, std::string hash_, uint32_t block_len = Default_Block_len);
    bool CheckHash() const;
    void Clear();
    std::string GetNextRequestString();
    uint32_t GetIndex();
    std::string GetData() const;
    std::string FillIn(uint32_t begin,const std::string &buf);
    bool Done() const;
};

class PieceManager
{
private:
    uint32_t file_length;
    uint32_t piece_length;
    std::vector<Piece> pieces;
    std::string file_name;
    std::string download_path;
    std::ofstream download_file;

    struct PieceInfo
    {
        bool downloaded;
        uint32_t index;
        uint32_t length;
        std::string hash;
        PieceInfo(uint32_t index_, uint32_t length_, std::string hash_)
         : index(index_), length(length_), hash(hash_), downloaded(false){};
    };
    std::vector<PieceInfo> pieces_info;
    
    std::map<std::string, uint32_t> peers_downloading;

    inline bool havePiece(const std::string &bit_field, uint32_t idx)
    {
        return bit_field[idx / 8] & (1 << (7 - (idx%8)));
    }

    inline uint32_t findPiece(uint32_t idx)
    {
        for(uint32_t i = 0;i<pieces.size();i++)
        {
            if(pieces[i].GetIndex() == idx)
                return i;
        }
        return MAXUINT32;
    }

public:
    explicit PieceManager(uint32_t file_length_, uint32_t piece_length_, std::vector<std::string> pieces_hash, std::string file_name_, std::string download_path_ = "./Download/");
    std::string GetNextRequestString(std::string peer_id,const std::string &bitfield); //若为空，说明下载完成。若多次返回的piece对方peer均无资源，可放弃该peer。
    std::string FillIn(uint32_t index, uint32_t begin, const std::string &buf);
    bool Done() const;
    void write(Piece& piece);
    ~PieceManager();
};

Block::Block(uint32_t index_, uint32_t begin_, uint32_t length_)
  : index(index_), begin(begin_), length(length_)
{
}
std::string Block::RequestString() const
{
    std::string s;
    char st[12];
    uint32_t var = htonl(index);
    memcpy(st,&var,4);
    var = htonl(begin);
    memcpy(st+4,&var,4);
    var = htonl(length);
    memcpy(st+8,&var,4);
    for(int i =0;i<12;i++)
        s+=st[i];
    std::cout<< "Send request: Index:"<< index <<" Begin:"<<begin<<" Length:"<<length<<std::endl;
    return s;
}
bool Block::Done() const
{
    return done;
}
void Block::FillIn(const std::string &buf)
{
    buff = buf;
    done = true;
    std::cout<< "Recieve piece: Index:"<< index <<" Begin:"<<begin<<" Length:"<<length<<std::endl;

}
std::string Block::GetBuff() const
{
    if(done)
        return buff;
    return std::string ();
}
void Block::Clear()
{
    done = false;
    buff.clear();
}

Piece::Piece(uint32_t length_, uint32_t index_, std::string hash_, uint32_t block_len)
{
    length = length_;
    hash = hash_;
    index = index_;
    downloaded = 0;
    nx_begin=0;

    uint32_t begin = 0;
    while(begin < length)
    {
        if(begin + block_len >= length)
        {
            blocks.push_back(Block(index, begin, length - begin));
            begin = length;
        }
        else
        {
            blocks.push_back(Block(index, begin, block_len));
            begin += block_len;
        }
    }
}

bool Piece::CheckHash() const
{
    std::string s = GetData();
    SHA1 checksum;
    checksum.update(s);
    const std::string get_hash = checksum.final();
    // std::cout<< get_hash <<std::endl<<Ascii2Hex(hash) <<std::endl<< hash<<std::endl;
    if(Hex2Ascii(get_hash) != hash)
        return false;
    return true;
}

std::string Piece::GetData() const
{
    std::string s;
    for(Block block: blocks)
    {
        std::string block_s = block.GetBuff();
        if(block_s.empty())
        {
            std::cout<< "Bad code, block shouldn't return a empity string!";
            exit(-1);
        }
        s+= block_s;
    }
    return s;
}

void Piece::Clear()
{
    downloaded = 0;
    nx_begin = 0;
    for(Block& block: blocks)
        block.Clear();
}

std::string Piece::GetNextRequestString()
{
    nx_begin %= blocks.size();
    return blocks[nx_begin++].RequestString();
}

uint32_t Piece::GetIndex()
{
    return index;
}

std::string Piece::FillIn(uint32_t begin,const std::string &buf)
{
    if(blocks[begin/Default_Block_len].Done()) return std::string("Block downloaded.");
    blocks[begin/Default_Block_len].FillIn(buf);
    downloaded ++;
    if(downloaded == blocks.size())
    {
        if(CheckHash())
        {
            std::cout<< "Piece " << index << " downloaded."<<std::endl;
            return std::string("Piece downloaded.");
        }
        std::cout<< "Warning: Piece "<< index <<" had been downloaded, but it's hash is wrong. Redownloading this piece..." <<std::endl;
        Clear();
        return std::string("Piece hash wrong.");
    }
    return std::string("Wrong.");
}

bool Piece::Done() const
{
    if(downloaded == blocks.size())
        return true;
    return false;
}

PieceManager::PieceManager(uint32_t file_length_, uint32_t piece_length_,std::vector<std::string> pieces_hash, std::string file_name_, std::string download_path_)
{
    file_length = file_length_;
    piece_length = piece_length_;
    file_name = file_name_;
    download_path = download_path_;
    for(uint32_t i = 0; i * piece_length < file_length;i++)
    {
        if((i + 1) * piece_length >= file_length)
            pieces_info.push_back(PieceInfo(i, file_length - piece_length * i, pieces_hash[i]));
        else
            pieces_info.push_back(PieceInfo(i,piece_length,pieces_hash[i]));
    }

    // Creates the destination file with the file size specified in the Torrent file
    download_file.open(download_path + file_name, std::ios::binary | std::ios::out);
    download_file.seekp(file_length - 1);
    download_file.write("", 1);

}
std::string PieceManager::GetNextRequestString(std::string peer_id, const std::string &bitfield)
{
    if(Done()) return std::string("File Dowloaded.");

    uint32_t idx = peers_downloading[peer_id];
    uint32_t i = findPiece(idx);
    if(i==MAXUINT32)
        pieces.push_back(Piece(pieces_info[idx].length, idx, pieces_info[idx].hash));
    while(pieces_info[idx].downloaded || !havePiece(bitfield,idx))
    {
        idx++;
        idx%=pieces_info.size();
    }
    peers_downloading[peer_id] = idx;

    i = findPiece(idx);
    if(i==MAXUINT32)
        pieces.push_back(Piece(pieces_info[idx].length, idx, pieces_info[idx].hash));
    i = findPiece(idx);
    return pieces[i].GetNextRequestString();
}

bool PieceManager::Done() const
{
    for(const auto & each: pieces_info)
    {
        if(!each.downloaded) return false;
    }
    return true;
}

std::string PieceManager::FillIn(uint32_t index, uint32_t begin, const std::string &buf)
{
    if(pieces_info[index].downloaded) return std::string ("Piece Downloaded.");
    uint32_t i = findPiece(index);
    std::string write_info = pieces[i].FillIn(begin, buf);
    if(write_info == "Piece downloaded.")
    {
        // Write this piece into file. Delect this piece from pieces.
        pieces_info[index].downloaded = true;
        write(pieces[i]);
        pieces.erase(pieces.begin() + i);

        if(Done())
            return std::string("File Downloaded.");
    }
    return write_info;
}

void PieceManager::write(Piece & piece)
{
    uint32_t position = piece_length * piece.GetIndex();
    download_file.seekp(position);
    download_file << piece.GetData();
}

PieceManager::~PieceManager()
{
    download_file.close();
}

#endif