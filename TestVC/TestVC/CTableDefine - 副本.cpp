#include "stdafx.h"

#include "CTableDefine.h"
#define  WIN32_LEAN_AND_MEAN
//#include <msgpack.hpp>
#include <time.h>
 
//#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <iostream>

#pragma warning (disable: 4996)
#define bExport true

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
CPowerTableBase::CPowerTableBase()
{ 
	m_bDelete = false;
	m_ID = -1;
}
CPowerTableBase::~CPowerTableBase()
{

}
std::string CPowerTableBase::GetDeleteSql()
{
	return strFormat("DELETE FROM %s WHERE %s=%d", m_strTableName.c_str(), "id", m_ID);
}
std::string CPowerTableBase::GetMaxIDSql()
{
	return strFormat("SELECT MAX(%s) AS MaxID FROM %s", "id", m_strTableName.c_str());
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
#define shift(x, n) (((x) << (n)) | ((x) >> (32-(n))))//右移的时候，高位一定要补零，而不是补充符号位
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))    
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))
#define A 0x67452301
#define B 0xefcdab89
#define C 0x98badcfe
#define D 0x10325476
//strBaye的长度
unsigned int strlength;
//A,B,C,D的临时变量
unsigned int atemp;
unsigned int btemp;
unsigned int ctemp;
unsigned int dtemp;
//常量ti unsigned int(abs(sin(i+1))*(2pow32))
const unsigned int k[]={
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
        0x4e0811a1,0xf7537e82,0xbd3af235,0x2ad7d2bb,0xeb86d391};
//向左位移数
const unsigned int s[]={7,12,17,22,7,12,17,22,7,12,17,22,7,
        12,17,22,5,9,14,20,5,9,14,20,5,9,14,20,5,9,14,20,
        4,11,16,23,4,11,16,23,4,11,16,23,4,11,16,23,6,10,
        15,21,6,10,15,21,6,10,15,21,6,10,15,21};
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
        if(i<16){
            f=F(b,c,d);
            g=i;
        }else if (i<32)
        {
            f=G(b,c,d);
            g=(5*i+1)%16;
        }else if(i<48){
            f=H(b,c,d);
            g=(3*i+5)%16;
        }else{
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
*填充函数
*处理后应满足bits≡448(mod512),字节就是bytes≡56（mode64)
*填充方式为先加一个1,其它位补零
*最后加上64位的原来长度
*/
unsigned int* add(string str)
{
    unsigned int num=((str.length()+8)/64)+1;//以512位,64个字节为一组
    unsigned int *strByte=new unsigned int[num*16];    //64/4=16,所以有16个整数
    strlength=num*16;
    for (unsigned int i = 0; i < num*16; i++)
        strByte[i]=0;
    for (unsigned int i=0; i <str.length(); i++)
    {
        strByte[i>>2]|=(str[i])<<((i%4)*8);//一个整数存储四个字节，i>>2表示i/4 一个unsigned int对应4个字节，保存4个字符信息
    }
    strByte[str.length()>>2]|=0x80<<(((str.length()%4))*8);//尾部添加1 一个unsigned int保存4个字符信息,所以用128左移
    /*
    *添加原长度，长度指位的长度，所以要乘8，然后是小端序，所以放在倒数第二个,这里长度只用了32位
    */
    strByte[num*16-2]=str.length()*8;
    return strByte;
}
string changeHex(int a)
{
    int b;
    string str1;
    string str="";
    for(int i=0;i<4;i++)
    {
        str1="";
        b=((a>>i*8)%(1<<8))&0xff;   //逆序处理每个字节
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
    atemp=A;    //初始化
    btemp=B;
    ctemp=C;
    dtemp=D;
    unsigned int *strByte=add(source);
    for(unsigned int i=0;i<strlength/16;i+=16)
    {
        unsigned int num[16];
        for(unsigned int j=0;j<16;j++)
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
//CUserTable
CUserTable::CUserTable()
{ 
	m_strTableName = "auth_user";
	date_joined = time(0);
}
CUserTable::~CUserTable()
{

}
bool CUserTable::ChangePassword(const std::string &strOld, const std::string &strNew)
{
	if (CryptString(strOld) == password)
	{
		password = CryptString(strNew);
		return true;
	}
	return false;
}
void CUserTable::SetPassword(const std::string &strNew)
{
	if (password.length() == 0)
		password = CryptString(strNew);
}
std::string CUserTable::GetAddSql()
{
	std::string strSql = strFormat("INSERT INTO %s (password, is_superuser, username, first_name,\
		last_name, email, is_staff, is_active, date_joined) VALUES \
		('%s', %d, '%s','%s','%s','%s', %d, %d, %s)", \
		m_strTableName.c_str(), password.c_str(), is_superuser, username.c_str(), first_name.c_str(), last_name.c_str(), email.c_str(),\
		is_staff, is_active, ValueToStr(date_joined).c_str());
	return strSql;
}
std::string CUserTable::GetModifySql()
{
	std::string strSql = strFormat("UPDATE %s SET password = '%s', is_superuser=%d, username='%s', first_name='%s',\
								   last_name='%s', email='%s', is_staff=%d, is_active=%d, date_joined=%s WHERE ID = %d", \
								   m_strTableName.c_str(), password.c_str(), is_superuser, username.c_str(), first_name.c_str(), 
								   last_name.c_str(), email.c_str(),is_staff, is_active, ValueToStr(date_joined).c_str(), m_ID);
	return strSql;
}
void CUserTable::Init()
{
	username = GetRandString();//"y142a";
	SetPassword(GetRandString());
}
//CPermissionTable
CPermissionTable::CPermissionTable()
{ 
	m_strTableName = "auth_permission";
}
CPermissionTable::~CPermissionTable()
{

}
std::string CPermissionTable::GetAddSql()
{
	std::string strSql = strFormat("INSERT INTO %s (content_type_id, codename, name) VALUES \
								   (%d, '%s','%s')", m_strTableName.c_str(), content_type_id, codename.c_str(), name.c_str());
	return strSql;
}
std::string CPermissionTable::GetModifySql()
{
	std::string strSql = strFormat("UPDATE %s SET content_type_id=%d, codename='%s', name='%s' WHERE ID = %d", \
								   m_strTableName.c_str(), content_type_id, codename.c_str(), name.c_str(), m_ID);
	return strSql;
}
void CPermissionTable::Init()
{
	codename = GetRandString();//"y142a";
	name = GetRandString();
}

//auth_group
CGroupTable::CGroupTable()
{ 
	m_strTableName = "auth_group";
}
CGroupTable::~CGroupTable()
{

}
std::string CGroupTable::GetAddSql()
{
	std::string strSql = strFormat("INSERT INTO %s (name) VALUES \
								   ('%s')", m_strTableName.c_str(), name.c_str());
	return strSql;
}
std::string CGroupTable::GetModifySql()
{
	std::string strSql = strFormat("UPDATE %s SET name='%s' WHERE ID = %d", \
		m_strTableName.c_str(), name.c_str(), m_ID);
	return strSql;
}
void CGroupTable::Init()
{
	name = GetRandString();
}
//auth_user_groups
CUserGroupTable::CUserGroupTable()
{ 
	m_strTableName = "auth_user_groups";
}
CUserGroupTable::~CUserGroupTable()
{

}
std::string CUserGroupTable::GetAddSql()
{
	std::string strSql = strFormat("INSERT INTO %s (user_id, group_id) VALUES \
								   (%d, %d)", m_strTableName.c_str(),user_id, group_id);
	return strSql;
}
std::string CUserGroupTable::GetModifySql()
{
	std::string strSql = strFormat("UPDATE %s SET user_id=user_id, group_id=group_id WHERE ID = %d", \
		m_strTableName.c_str(), user_id, group_id, m_ID);
	return strSql;
}
void CUserGroupTable::Init()
{
	user_id = rand();
	group_id = rand();
}
//auth_group_permissions
CGroupPermissionsTable::CGroupPermissionsTable()
{ 
	m_strTableName = "auth_group_permissions";
}
CGroupPermissionsTable::~CGroupPermissionsTable()
{

}
std::string CGroupPermissionsTable::GetAddSql()
{
	std::string strSql = strFormat("INSERT INTO %s (group_id, permission_id) VALUES \
								   (%d, %d)", m_strTableName.c_str(), group_id, permission_id);
	return strSql;
}
std::string CGroupPermissionsTable::GetModifySql()
{
	std::string strSql = strFormat("UPDATE %s SET group_id=%d, permission_id=%d WHERE ID = %d", \
		m_strTableName.c_str(), group_id, permission_id, m_ID);
	return strSql;
}
void CGroupPermissionsTable::Init()
{
	permission_id = rand();//"y142a";
	group_id = rand();
}

//auth_user_user_permissions
CUserUserPermissionsTable::CUserUserPermissionsTable()
{ 
	m_strTableName = "auth_user_user_permissions";
}
CUserUserPermissionsTable::~CUserUserPermissionsTable()
{

}
std::string CUserUserPermissionsTable::GetAddSql()
{
	std::string strSql = strFormat("INSERT INTO %s (user_id, permission_id) VALUES \
								   (%d, %d)", m_strTableName.c_str(), user_id, permission_id);
	return strSql;
}
std::string CUserUserPermissionsTable::GetModifySql()
{
	std::string strSql = strFormat("UPDATE %s SET user_id=%d, permission_id=%d WHERE ID = %d", \
		m_strTableName.c_str(), user_id, permission_id, m_ID);
	return strSql;
}
void CUserUserPermissionsTable::Init()
{
	permission_id = rand();//"y142a";
	user_id = rand();
}

//django_content_type
CContentTypeTable::CContentTypeTable()
{ 
	m_strTableName = "django_content_type";
}
CContentTypeTable::~CContentTypeTable()
{

}
std::string CContentTypeTable::GetAddSql()
{
	std::string strSql = strFormat("INSERT INTO %s ( app_label, model) VALUES \
								   ('%s','%s')", m_strTableName.c_str(),  app_label.c_str(), model.c_str());
	return strSql;
}
std::string CContentTypeTable::GetModifySql()
{
	std::string strSql = strFormat("UPDATE %s SET app_label='%s', model='%s' WHERE ID = %d", \
		m_strTableName.c_str(), app_label.c_str(), model.c_str(), m_ID);
	return strSql;
}
void CContentTypeTable::Init()
{
	app_label = GetRandString();//"y142a";
	model = GetRandString();
}
