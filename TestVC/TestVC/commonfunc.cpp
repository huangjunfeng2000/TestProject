#include "stdafx.h"
#include "commonfunc.h"
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <vector>

#pragma warning (disable: 4996)

using namespace std;
//////////////

std::string strFormat (const char *format, ...)
{
    char strLine[1025];
    memset(strLine, 0, 1025);
    va_list argptr;
    va_start (argptr, format);
    vsnprintf (strLine, 1024, format, argptr);
    va_end (argptr);
    return strLine;
}

std::string ValueToStr(const time_t &val)
{
    static char strTime[40];
    if (-1 != val)
    {
        tm ptr;
        localtime_s(&ptr, &val);
        strftime(strTime, 40, "'%Y-%m-%d'", &ptr);
    }
    return val!=-1 ? strTime : "''";
}
#define shift(x, n) (((x) << (n)) | ((x) >> (32-(n))))//���Ƶ�ʱ�򣬸�λһ��Ҫ���㣬�����ǲ������λ
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))
#define A 0x67452301
#define B 0xefcdab89
#define C 0x98badcfe
#define D 0x10325476
//strBaye�ĳ���
unsigned int strlength;
//A,B,C,D����ʱ����
unsigned int atemp;
unsigned int btemp;
unsigned int ctemp;
unsigned int dtemp;
//����ti unsigned int(abs(sin(i+1))*(2pow32))
const unsigned int k[]=
{
    0xd76aa478,0xe8c7b756,0x242070db,0xc1bdceee,
    0xf57c0faf,0x4787c62a,0xa8304613,0xfd469501,0x698098d8,
    0x8b44f7af,0xffff5bb1,0x895cd7be,0x6b901122,0xfd987193,
    0xa679438e,0x49b40821,0xf61e2562,0xc040b340,0x265e5a51,
    0xe9b6c7aa,0xd62f105d,0x02441453,0xd8a1e681,0xe7d3fbc8,
    0x21e1cde6,0xc33707d6,0xf4d50d87,0x455a14ed,0xa9e3e905,
    0xfcefa3f8,0x676f02d9,0x8d2a4c8a,0xfffa3942,0x8771f681,
    0x6d9d6122,0xfde5380c,0xa4beea44,0x4bdecfa9,0xf6bb4b60,
    0xbebfbc70,0x289b7ec6,0xeaa127fa,0xd4ef3085,0x04881d05,
    0xd9d4d039,0xe6db99e5,0x1fa27cf8,0xc4ac5665,0xf4292244,
    0x432aff97,0xab9423a7,0xfc93a039,0x655b59c3,0x8f0ccc92,
    0xffeff47d,0x85845dd1,0x6fa87e4f,0xfe2ce6e0,0xa3014314,
    0x4e0811a1,0xf7537e82,0xbd3af235,0x2ad7d2bb,0xeb86d391
};
//����λ����
const unsigned int s[]= {7,12,17,22,7,12,17,22,7,12,17,22,7,
                         12,17,22,5,9,14,20,5,9,14,20,5,9,14,20,5,9,14,20,
                         4,11,16,23,4,11,16,23,4,11,16,23,4,11,16,23,6,10,
                         15,21,6,10,15,21,6,10,15,21,6,10,15,21
                        };
const char str16[]="0123456789abcdef";
void mainLoop(unsigned int M[])
{
    unsigned int f,g;
    unsigned int a=atemp;
    unsigned int b=btemp;
    unsigned int c=ctemp;
    unsigned int d=dtemp;
    for (unsigned int i = 0; i < 64; i++)
    {
        if(i<16)
        {
            f=F(b,c,d);
            g=i;
        }
        else if (i<32)
        {
            f=G(b,c,d);
            g=(5*i+1)%16;
        }
        else if(i<48)
        {
            f=H(b,c,d);
            g=(3*i+5)%16;
        }
        else
        {
            f=I(b,c,d);
            g=(7*i)%16;
        }
        unsigned int tmp=d;
        d=c;
        c=b;
        b=b+shift((a+f+k[i]+M[g]),s[i]);
        a=tmp;
    }
    atemp=a+atemp;
    btemp=b+btemp;
    ctemp=c+ctemp;
    dtemp=d+dtemp;
}
/*
*��亯��
*������Ӧ����bits��448(mod512),�ֽھ���bytes��56��mode64)
*��䷽ʽΪ�ȼ�һ��1,����λ����
*������64λ��ԭ������
*/
unsigned int* add(string str)
{
    unsigned int num=((str.length()+8)/64)+1;//��512λ,64���ֽ�Ϊһ��
    unsigned int *strByte=new unsigned int[num*16];    //64/4=16,������16������
    strlength=num*16;
    for (unsigned int i = 0; i < num*16; i++)
        strByte[i]=0;
    for (unsigned int i=0; i <str.length(); i++)
    {
        strByte[i>>2]|=(str[i])<<((i%4)*8);//һ�������洢�ĸ��ֽڣ�i>>2��ʾi/4 һ��unsigned int��Ӧ4���ֽڣ�����4���ַ���Ϣ
    }
    strByte[str.length()>>2]|=0x80<<(((str.length()%4))*8);//β������1 һ��unsigned int����4���ַ���Ϣ,������128����
    /*
    *����ԭ���ȣ�����ָλ�ĳ��ȣ�����Ҫ��8��Ȼ����С�������Է��ڵ����ڶ���,���ﳤ��ֻ����32λ
    */
    strByte[num*16-2]=str.length()*8;
    return strByte;
}
string changeHex(int a)
{
    int b;
    string str1;
    string str="";
    for(int i=0; i<4; i++)
    {
        str1="";
        b=((a>>i*8)%(1<<8))&0xff;   //������ÿ���ֽ�
        for (int j = 0; j < 2; j++)
        {
            str1.insert(0,1,str16[b%16]);
            b=b/16;
        }
        str+=str1;
    }
    return str;
}
string getMD5(string source)
{
    atemp=A;    //��ʼ��
    btemp=B;
    ctemp=C;
    dtemp=D;
    unsigned int *strByte=add(source);
    for(unsigned int i=0; i<strlength/16; i+=16)
    {
        unsigned int num[16];
        for(unsigned int j=0; j<16; j++)
            num[j]=strByte[i*16+j];
        mainLoop(num);
    }
    return changeHex(atemp).append(changeHex(btemp)).append(changeHex(ctemp)).append(changeHex(dtemp));
}
std::string CryptString(const std::string &str)
{
    std::string str1 = getMD5(str);
    std::string str2 = getMD5(str1+str);
    return str2 + str1;
}

std::string GetRandString()
{
    char str[20];
    sprintf(str, "str_%d_%d", rand(), rand());
    return str;
}

std::string ConvertSqliteString(const char *pValue)
{
	if (size_t iLen = strlen(pValue))
	{
		wstring wstr = Utf82Unicode(std::string (pValue, iLen));
		string strRes = WideByte2Acsi(wstr);
		return strRes;
	}
	return "";
}

std::wstring Utf82Unicode(const std::string& utf8string)
{
    int widesize = ::MultiByteToWideChar(CP_UTF8, 0, utf8string.c_str(), -1, NULL, 0);
    if (widesize == ERROR_NO_UNICODE_TRANSLATION)
    {
        throw std::exception("Invalid UTF-8 sequence.");
    }
    if (widesize == 0)
    {
        throw std::exception("Error in conversion.");
    }
    std::vector<wchar_t> resultstring(widesize);
    int convresult = ::MultiByteToWideChar(CP_UTF8, 0, utf8string.c_str(), -1, &resultstring[0], widesize);
    if (convresult != widesize)
    {
        throw std::exception("La falla!");
    }
    return std::wstring(&resultstring[0]);
}
//unicode תΪ ascii
string WideByte2Acsi(wstring& wstrcode)
{
    int asciisize = ::WideCharToMultiByte(CP_OEMCP, 0, wstrcode.c_str(), -1, NULL, 0, NULL, NULL);
    if (asciisize == ERROR_NO_UNICODE_TRANSLATION)
    {
        throw std::exception("Invalid UTF-8 sequence.");
    }
    if (asciisize == 0)
    {
        throw std::exception("Error in conversion.");
    }
    std::vector<char> resultstring(asciisize);
    int convresult =::WideCharToMultiByte(CP_OEMCP, 0, wstrcode.c_str(), -1, &resultstring[0], asciisize, NULL, NULL);
    if (convresult != asciisize)
    {
        throw std::exception("La falla!");
    }
    return std::string(&resultstring[0]);
}

//utf-8 ת ascii
string UTF_82ASCII(string& strUtf8Code)
{
    string strRet("");
    //�Ȱ� utf8 תΪ unicode
    wstring wstr = Utf82Unicode(strUtf8Code);
    //���� unicode תΪ ascii
    strRet = WideByte2Acsi(wstr);
    return strRet;
}

///////////////////////////////////////////////////////////////////////

//ascii ת Unicode
wstring Acsi2WideByte(string& strascii)
{
    int widesize = MultiByteToWideChar (CP_ACP, 0, (char*)strascii.c_str(), -1, NULL, 0);
    if (widesize == ERROR_NO_UNICODE_TRANSLATION)
    {
        throw std::exception("Invalid UTF-8 sequence.");
    }
    if (widesize == 0)
    {
        throw std::exception("Error in conversion.");
    }
    std::vector<wchar_t> resultstring(widesize);
    int convresult = MultiByteToWideChar (CP_ACP, 0, (char*)strascii.c_str(), -1, &resultstring[0], widesize);
    if (convresult != widesize)
    {
        throw std::exception("La falla!");
    }
    return std::wstring(&resultstring[0]);
}

//Unicode ת Utf8
std::string Unicode2Utf8(const std::wstring& widestring)
{
    int utf8size = ::WideCharToMultiByte(CP_UTF8, 0, widestring.c_str(), -1, NULL, 0, NULL, NULL);
    if (utf8size == 0)
    {
        throw std::exception("Error in conversion.");
    }
    std::vector<char> resultstring(utf8size);
    int convresult = ::WideCharToMultiByte(CP_UTF8, 0, widestring.c_str(), -1, &resultstring[0], utf8size, NULL, NULL);
    if (convresult != utf8size)
    {
        throw std::exception("La falla!");
    }
    return std::string(&resultstring[0]);
}

//ascii ת Utf8
string ASCII2UTF_8(string& strAsciiCode)
{
    string strRet("");
    //�Ȱ� ascii תΪ unicode
    wstring wstr = Acsi2WideByte(strAsciiCode);
    //���� unicode תΪ utf8
    strRet = Unicode2Utf8(wstr);
    return strRet;
}