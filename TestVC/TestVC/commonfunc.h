#pragma once
#include <string>
//#include <wstring>
using namespace std;

std::string strFormat (const char *format, ...);
std::string getMD5(std::string source);
std::string CryptString(const std::string &str);
std::string GetRandString();
std::string ConvertSqliteString(const char *pValue);

std::wstring Utf82Unicode(const std::string& utf8string);	//UTF-8תUnicode
string WideByte2Acsi(wstring& wstrcode);					//unicode תΪ ascii
string UTF_82ASCII(string& strUtf8Code);					//utf-8 ת ascii
wstring Acsi2WideByte(string& strascii);					//ascii ת Unicode
std::string Unicode2Utf8(const std::wstring& widestring);	//Unicode ת Utf8
string ASCII2UTF_8(string& strAsciiCode);					//ascii ת Utf8

