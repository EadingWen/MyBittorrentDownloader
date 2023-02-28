#include"utils.h"

std::string Cstring2String(char* cs, int len, int start_pos)
{
    std::string s;
    for(int i=start_pos;i<start_pos + len;i++)
        s+=cs[i];
    return s;
}

uint32_t Cstring2Uint32(const char* s)
{
    uint32_t ans;
    memcpy(&ans,s,4);
    ans = ntohl(ans);
    return ans;
}

uint32_t String2Uint32(const std::string& s)
{
    const char* ss = s.c_str();
    return Cstring2Uint32(ss);
}

unsigned char ToHex(unsigned char x) 
{ 
    return  x > 9 ? x + 55 : x + 48; 
}

unsigned char FromHex(unsigned char x) 
{ 
    unsigned char y;
    if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
    else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
    else if (x >= '0' && x <= '9') y = x - '0';
    else assert(0);
    return y;
}

//This function is from: https://stackoverflow.com/questions/3790613/how-to-convert-a-string-of-hex-values-to-a-string
std::string Hex2Ascii(const std::string& s)
{
    std::string ans;
    ans.reserve(s.length() / 2);
    for (std::string::const_iterator p = s.begin(); p != s.end(); p++)
    {
       unsigned char c = FromHex(*p);
       p++;
       if (p == s.end()) break; // incomplete last digit - should report error
       c = (c << 4) + FromHex(*p); // + takes precedence over <<
       ans.push_back(c);
    }
    return ans;
}

std::string Ascii2Hex(const std::string& s)
{
    std::string ans;
    for (std::string::const_iterator p = s.begin(); p != s.end(); p++)
    {
        ans.push_back(ToHex(*p>>4));
        ans.push_back(ToHex(*p&(unsigned char) 15));
    }
    return ans;
}

std::string UrlEncode(const std::string& str)
{
    std::string strTemp = "";
    size_t length = str.length();
    for (size_t i = 0; i < length; i++)
    {
        if (isalnum((unsigned char)str[i]) || 
            (str[i] == '-') ||
            (str[i] == '_') || 
            (str[i] == '.') || 
            (str[i] == '~'))
            strTemp += str[i];
        else if (str[i] == ' ')
            strTemp += "+";
        else
        {
            strTemp += '%';
            strTemp += ToHex((unsigned char)str[i] >> 4);
            strTemp += ToHex((unsigned char)str[i] % 16);
        }
    }
    return strTemp;
}

std::string UrlDecode(const std::string& str)
{
    std::string strTemp = "";
    size_t length = str.length();
    for (size_t i = 0; i < length; i++)
    {
        if (str[i] == '+') strTemp += ' ';
        else if (str[i] == '%')
        {
            assert(i + 2 < length);
            unsigned char high = FromHex((unsigned char)str[++i]);
            unsigned char low = FromHex((unsigned char)str[++i]);
            strTemp += high*16 + low;
        }
        else strTemp += str[i];
    }
    return strTemp;
}

std::string UrlEncodeHexString(const std::string& s)
{
    return UrlEncode(Hex2Ascii(s));
}