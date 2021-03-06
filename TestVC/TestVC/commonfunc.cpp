#include "stdafx.h"
#include "commonfunc.h"
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <WinCrypt.h>

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
//向左位移数
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
    for(int i=0; i<4; i++)
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
    for(unsigned int i=0; i<strlength/16; i+=16)
    {
        unsigned int num[16];
        for(unsigned int j=0; j<16; j++)
            num[j]=strByte[i*16+j];
        mainLoop(num);
    }
    return changeHex(atemp).append(changeHex(btemp)).append(changeHex(ctemp)).append(changeHex(dtemp));
}

CString getMD5API( CString & csBuffer)
{
    CString csDigest;
    HCRYPTPROV hCryptProv;
    HCRYPTHASH hHash;
    BYTE bHash [0x7f];
    DWORD dwHashLen = 16; // The MD5 algorithm always returns 16 bytes.
    DWORD cbContent = csBuffer.GetLength ();
    BYTE * pbContent = ( BYTE *) csBuffer.GetBuffer ( cbContent );
    if ( CryptAcquireContext (& hCryptProv , NULL , NULL , PROV_RSA_FULL ,CRYPT_VERIFYCONTEXT | CRYPT_MACHINE_KEYSET ))
    {
        if ( CryptCreateHash (hCryptProv, CALG_MD5 , 0,0, & hHash ))
        {
            if ( CryptHashData ( hHash , pbContent , cbContent , 0))
            {
                if (CryptGetHashParam ( hHash , HP_HASHVAL , bHash , & dwHashLen , 0))   // Make a   string version of the numeric digest value csDigest.Empty ();
                {
                    CString tmp ;
                    for ( int i = 0; i <16; i ++)
                    {
                        tmp.Format ( "%02x" , bHash [ i ]);
                        csDigest  += tmp ;
                    }
                }
                else csDigest = _T ( "Error getting hash param" );
            }
            else csDigest= _T ( "Error hashing data" );
        }
        else csDigest = _T ( "Error creating hash" );
    }
    else csDigest = _T ( "Error acquiring context" );
    CryptDestroyHash ( hHash );
    CryptReleaseContext ( hCryptProv , 0);
    csBuffer.ReleaseBuffer ();
    return  csDigest ;
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
std::string g_strExePath;
std::string GetExePath()
{
	if (!g_strExePath.empty())
		return g_strExePath;
	TCHAR   szPath[MAX_PATH];
	::GetModuleFileName(NULL,szPath,MAX_PATH);
	CString   strPath   =   szPath; 
	int   index   =   strPath.ReverseFind(_T( '\\')); 
	g_strExePath = strPath.Left(index   +   1);
	return g_strExePath;
}
std::string GetConfigFile(const std::string &strFileName)
{
	std::string strFile = GetExePath() + "\\" + strFileName;
	return strFile;
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
//unicode 转为 ascii
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

//utf-8 转 ascii
string UTF_82ASCII(string& strUtf8Code)
{
    string strRet("");
    //先把 utf8 转为 unicode
    wstring wstr = Utf82Unicode(strUtf8Code);
    //最后把 unicode 转为 ascii
    strRet = WideByte2Acsi(wstr);
    return strRet;
}

///////////////////////////////////////////////////////////////////////

//ascii 转 Unicode
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

//Unicode 转 Utf8
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

//ascii 转 Utf8
string ASCII2UTF_8(string& strAsciiCode)
{
    string strRet("");
    //先把 ascii 转为 unicode
    wstring wstr = Acsi2WideByte(strAsciiCode);
    //最后把 unicode 转为 utf8
    strRet = Unicode2Utf8(wstr);
    return strRet;
}

const std::string strConfigFile = "Config.ini";
bool SetOpitionValue(const std::string &strSection, const std::string &strAttr, const std::string &strValue)
{
	return ::WritePrivateProfileString(strSection.c_str(), strAttr.c_str(), strValue.c_str(), strConfigFile.c_str());
}
bool GetOpitionValue(const std::string &strSection, const std::string &strAttr, std::string &strValue)
{
	char strTemp[1024];
	memset(strTemp, 0, 1024);
	int iCount = ::GetPrivateProfileString(strSection.c_str(), strAttr.c_str(), "", strTemp, 1024, strConfigFile.c_str());
	if (iCount)
		strValue = strTemp;
	return iCount > 0;
}

//method缩放的方式：0表示保持图片原比例去适应rect,1表示拉伸图片去适应rect
void ResizeImage(Mat &img,CRect rect,Mat &dst_img,int method)
{
	// 读取图片的宽和高
	int h = img.rows;
	int w = img.cols;
	int nw,nh;

	if (0==method)
	{
		// 计算将图片缩放到dst_drawing区域所需的比例因子
		float scale1 = (float) ( (float)w /(float)rect.Width() );
		float scale2 = (float) ( (float)h /(float)rect.Height() );

		float scale=(scale1>scale2)?scale1:scale2;

		// 缩放后图片的宽和高
		nw = ((float)w)/scale;
		nh = ((float)h)/scale;

		//由于浮点也存在数据截断，为防止nw大于目标框大小，做个边界保护
		if (nw>rect.Width())
		{
			nw=rect.Width();
		}
		if (nh>rect.Height())
		{
			nh=rect.Height();
		}
	}else if (1==method)
	{
		// 缩放后图片的宽和高
		nw = rect.Width();
		nh = rect.Height();
	}

	// 为了将缩放后的图片存入 dst_drawing 的正中部位，需计算图片在 dst_drawing 左上角的期望坐标值
	int tlx = ( nw < rect.Width())? (((float)(rect.Width()-nw))/2.0f+0.5f):   0;
	int tly = ( nh < rect.Height())? (((float)(rect.Height()-nh))/2.0f+0.5f): 0;

	//重置矩阵大小在图比较大时也很费时间，加个判断可避免重复的重置矩阵大小
	if ( dst_img.rows!=rect.Height() || dst_img.cols!=rect.Width() || dst_img.type()!=img.type() )
	{
		dst_img=Mat(rect.Height(),rect.Width(),img.type());
	}
	if (0==method)
	{
		dst_img=0;
	}
	// 设置 dst_img 的 ROI 区域，用来存入更改后的图片 img
	Rect rect_roi=Rect(tlx,tly,nw,nh);

	// 对图片 img 进行缩放，并存入到 src_drawing 中
	Mat dst_img_roi=dst_img(rect_roi);

	//将img尺寸缩放至dst_img_roi尺寸,插值方法选用INTER_NEAREST，放到最大后无近似显示能将每个像素看清
	resize(img,dst_img_roi,cv::Size(nw,nh),0.0,0.0,INTER_NEAREST);
}

void  DrawToHDC(Mat &cvImg, HDC hDC, CRect &rect, int from_x, int from_y)
{
	Mat m_cvImg;
	ResizeImage(cvImg, rect, m_cvImg, 1);
	//Size sz2Sh(rect.Width(),rect.Height());
	//resize(cvImg, m_cvImg, sz2Sh);

	unsigned int              m_buffer[sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*256];

	BITMAPINFO* m_bmi = (BITMAPINFO*) m_buffer;
	BITMAPINFOHEADER* m_bmih = &(m_bmi->bmiHeader);
	memset(m_bmih, 0, sizeof(*m_bmih));
	m_bmih->biSize = sizeof(BITMAPINFOHEADER);

	m_bmih->biWidth = m_cvImg.cols;
	m_bmih->biHeight = -m_cvImg.rows;           // 在自下而上的位图中 高度为负
	m_bmih->biPlanes = 1;
	m_bmih->biCompression = BI_RGB;
	m_bmih->biBitCount = 8 * m_cvImg.channels();

	if( m_cvImg.channels() == 1 )
	{
		RGBQUAD* palette = m_bmi->bmiColors;
		int i;
		for( i = 0; i < 256; i++ )
		{
			palette[i].rgbBlue = palette[i].rgbGreen = palette[i].rgbRed = (BYTE)i;
			palette[i].rgbReserved = 0;
		}
	}

	from_x = MIN( MAX( from_x, 0 ), m_cvImg.cols - 1 );
	from_y = MIN( MAX( from_y, 0 ), m_cvImg.rows - 1 );

	int sw = MAX( MIN( m_cvImg.cols - from_x, rect.Width() ), 0 );
	int sh = MAX( MIN( m_cvImg.rows - from_y, rect.Height() ), 0 );

	if(m_cvImg.cols % 4 != 0 && m_cvImg.isContinuous())
	{
		int nLineWidth = (sw * m_cvImg.channels() + 3)/4*4; //凑成大于等于nColumn的最小的4的整倍数
		BYTE  *m_pImageBuffer = new BYTE[sh * nLineWidth];
		for(int i = 0; i < sh; i++)
		{
			memcpy(m_pImageBuffer + i * nLineWidth, m_cvImg.ptr<uchar>(i), m_cvImg.step);
		}
		SetDIBitsToDevice(
			hDC, 
			rect.left, 
			rect.top, 
			rect.Width(), 
			rect.Height(), 
			from_x, 
			from_y,  
			from_y, 
			sh,
			m_pImageBuffer + from_y * nLineWidth,
			m_bmi, 
			DIB_RGB_COLORS
			);
		delete[] m_pImageBuffer;
	}
	else
	{
		SetDIBitsToDevice(
			hDC, 
			rect.left, 
			rect.top, 
			rect.Width(), 
			rect.Height(), 
			from_x, 
			from_y,  
			from_y, 
			sh,
			m_cvImg.data + from_y * m_cvImg.step,
			m_bmi, 
			DIB_RGB_COLORS
			);
	}
	//StretchDIBits(      
	//	hDC,
	//	0, 0, rect.Width(), rect.Height(),
	//	0, 0, m_cvImg.cols, m_cvImg.rows,
	//	m_cvImg.data,
	//	(BITMAPINFO*) m_bmi,
	//	DIB_RGB_COLORS,
	//	SRCCOPY);
}