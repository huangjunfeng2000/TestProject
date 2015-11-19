// FFMpegTestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TestVC.h"
#include "FFMpegTestDlg.h"

#include <stdlib.h>  
#include <stdio.h>  
#include <string.h>  

#include <windows.h>  
#include <time.h>  
extern "C"
{
#include "libavformat/avformat.h" 
#include "libavcodec/avcodec.h" 
#include "libavdevice/avdevice.h"  
#include "libswscale/swscale.h" 

};

#define MAX_INPUT_DEVICE_NUM 10  

// CFFMpegTestDlg dialog

IMPLEMENT_DYNAMIC(CFFMpegTestDlg, CDialog)

CFFMpegTestDlg::CFFMpegTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CFFMpegTestDlg::IDD, pParent)
{

}

CFFMpegTestDlg::~CFFMpegTestDlg()
{
}

void CFFMpegTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CFFMpegTestDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_SAVEBMP, &CFFMpegTestDlg::OnBnClickedButtonSavebmp)
END_MESSAGE_MAP()


void save_bmp(unsigned char * data,int data_size,int w,int h,FILE * out)  
{  
	// 位图文件头  
	BITMAPFILEHEADER bmpheader;   
	BITMAPINFO bmpinfo;   
	int bit = 24;  

	bmpheader.bfType = ('M' <<8)|'B';   
	bmpheader.bfReserved1 = 0;   
	bmpheader.bfReserved2 = 0;   
	bmpheader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);   
	bmpheader.bfSize = bmpheader.bfOffBits + w*h*bit/8;  

	bmpinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);   
	bmpinfo.bmiHeader.biWidth = w;   
	bmpinfo.bmiHeader.biHeight = 0-h;   
	bmpinfo.bmiHeader.biPlanes = 1;   
	bmpinfo.bmiHeader.biBitCount = bit;   
	bmpinfo.bmiHeader.biCompression = BI_RGB;   
	bmpinfo.bmiHeader.biSizeImage = 0;   
	bmpinfo.bmiHeader.biXPelsPerMeter = 100;   
	bmpinfo.bmiHeader.biYPelsPerMeter = 100;   
	bmpinfo.bmiHeader.biClrUsed = 0;   
	bmpinfo.bmiHeader.biClrImportant = 0;  

	fwrite(&bmpheader,sizeof(BITMAPFILEHEADER),1,out);   
	fwrite(&bmpinfo.bmiHeader,sizeof(BITMAPINFOHEADER),1,out);  
	fwrite(data,data_size,1,out);  
}  
int CaptureFromLocalCamera()  
{  
	AVFormatContext *ic = NULL;  
	///*AVFormatParameters*/AVDictionary in_fmt_para={0};  
	AVPacket packet;  
	char buffer[MAX_PATH]={0};  
	int width = 0,height = 0;  
	int ret,video_stream = -1,i=0;  

	//查找输入（vfwcap）格式  
	AVInputFormat *in_fmt = av_find_input_format ("vfwcap");   
	if (in_fmt == NULL)   
	{  
		printf("not support input device vfwcap.\n");  
		return -1;  
	}  
	//memset (&in_fmt_para, 0, sizeof(in_fmt_para));  
	//指定需要采集图像的高度  
	//in_fmt_para.height = height;  
	//指定需要采集图像的宽度  
	//in_fmt_para.width  = width;  

	//设置帧率  
	//av_parse_video_frame_rate(&in_fmt_para.time_base,"20");  

	//打开摄像头设备,从"0"到MAX_INPUT_DEVICE_NUM依次尝试打开  
	for( i=0 ; i < MAX_INPUT_DEVICE_NUM + 1; i++ )  
	{  
		sprintf(buffer,"%d",i);  
		ret = avformat_open_input ( &ic, buffer, in_fmt,NULL);//sizeof(in_fmt_para),&in_fmt_para);  
		if ( ret == 0 && ic)  
		{  
			break;  
		}  
	}  
	//open success?  
	if(!ic || ret != 0)  
	{  
		if(ic)  
			avformat_close_input(&ic);  
		printf("can not open input file.\n");  
		return -2;  
	}  
	printf("input device no. is %d\n",i);  
	//find the video stream  
	for(i=0;i<ic ->nb_streams;i++)  
	{  
		if ( AVMEDIA_TYPE_VIDEO == ic ->streams[i] ->codec ->codec_type )  
		{  
			video_stream = i;  
			break;  
		}  
	}  
	if(video_stream < 0)  
	{  
		avformat_close_input(&ic);  
		printf("can not find a video stream.\n");  
		return -3;  
	}  
	//获取视频时间宽度和高度  
	width  = ic ->streams[video_stream] ->codec ->width;  
	height = ic ->streams[video_stream] ->codec ->height;  
	printf("video size: %dx%d\n",width,height);  
	//从摄像头获取图像数据  
	if( 0 == av_read_frame(ic,&packet))  
	{  
		//find the decode codec  
		AVCodec * decodec =  avcodec_find_decoder(ic ->streams[video_stream] ->codec ->codec_id);  
		if(decodec)  
		{  
			//open the decode codec  
			if( 0 == avcodec_open2(ic ->streams[video_stream] ->codec,decodec, NULL) )  
			{  
				int got_picture = 0;  
				AVFrame * frame = av_frame_alloc();  
				avcodec_decode_video2(ic ->streams[video_stream] ->codec,frame,&got_picture,&packet);  
				//decode success  
				if(got_picture)  
				{  
					char * buffer = NULL;  
					size_t buffer_size = 0;  
					struct SwsContext *pSwsCtx=NULL;  
					AVFrame * rgb_frame = av_frame_alloc();  
					buffer_size = avpicture_get_size(AV_PIX_FMT_BGR24,width,height);  
					buffer = (char *)av_malloc(buffer_size);  
					avpicture_fill((AVPicture*)rgb_frame,(uint8_t *)buffer,AV_PIX_FMT_BGR24,width,height);  
					//get swscale ctx  
					pSwsCtx = sws_getContext(  
						ic ->streams[video_stream] ->codec ->width,  
						ic ->streams[video_stream] ->codec ->height,  
						ic ->streams[video_stream] ->codec ->pix_fmt,  
						width,  
						height,  
						AV_PIX_FMT_BGR24,  
						SWS_BILINEAR,  
						NULL,  
						NULL,  
						NULL);  
					if(pSwsCtx)  
					{  
						FILE *fp = NULL;  
						SYSTEMTIME dt={0};  
						//图像格式转换  
						sws_scale(  
							pSwsCtx,  
							frame ->data,  
							frame ->linesize,  
							0,  
							ic ->streams[video_stream] ->codec ->height,  
							rgb_frame ->data,  
							rgb_frame ->linesize);  
						//create the image file name  
						GetLocalTime(&dt);  
						srand(0);  
						sprintf(buffer,"d:\\%04d_%02d_%02d %02d_%02d_%02d %02d.bmp",dt.wYear,dt.wMonth,dt.wDay,dt.wHour,dt.wMinute,dt.wSecond,rand()%30);  
						//  
						//CreateDirectoryA("imgs",NULL);  
						//open file  
						fp = fopen(buffer, "wb");  
						if(fp)  
						{  
							save_bmp(rgb_frame ->data[0],rgb_frame ->linesize[0]*height,width,height,fp);  
							fclose(fp);  
						}  
						//free sws ctx  
						sws_freeContext(pSwsCtx);  
					}  
					//free buffer  
					av_free(rgb_frame);  
					av_free(buffer);  
				}  
				//free buffer  
				av_free(frame);  
				//close the decode codec  
				avcodec_close(ic ->streams[video_stream] ->codec);  
			}  
		}  
	}  

	//close the input device  
	avformat_close_input(&ic);  

	return 0;  
} 

// CFFMpegTestDlg message handlers
BOOL CFFMpegTestDlg::OnInitDialog()
{
	//框架生成代码.......
	//pStc=(CStatic *)GetDlgItem(IDC_VIEW);//IDC_VIEW为Picture控件ID
	//pStc->GetClientRect(&rect);//将CWind类客户区的坐标点传给矩形
	//pDC=pStc->GetDC(); //得到Picture控件设备上下文
	//hDC=pDC->GetSafeHdc(); //得到控件设备上下文的句柄 

	avcodec_register_all();  
	avdevice_register_all();  

	return   TRUE;
}
void CFFMpegTestDlg::OnBnClickedButtonSavebmp()
{
	// TODO: Add your control notification handler code here
	CaptureFromLocalCamera();  
}
