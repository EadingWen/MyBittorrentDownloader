#ifndef MY_BITTORRETN_UTILS
#define MY_BITTORRETN_UTILS

#include <string>
#include<stdio.h>
#include<iostream>
#include<assert.h>
#include<string>
#include <cstring>
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

std::string Cstring2String(char* cs, int len, int start_pos = 0);

uint32_t Cstring2Uint32(const char* s);

uint32_t String2Uint32(const std::string& s);

unsigned char ToHex(unsigned char x);

unsigned char FromHex(unsigned char x);

std::string Hex2Ascii(const std::string& s);

std::string Ascii2Hex(const std::string& s);

std::string UrlEncode(const std::string& str);

std::string UrlDecode(const std::string& str);

std::string UrlEncodeHexString(const std::string& s);

#endif
