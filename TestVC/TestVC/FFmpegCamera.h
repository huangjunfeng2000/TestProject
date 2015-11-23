#pragma once

//#include "inc.h"
#include <windows.h>
#include <time.h>
extern "C"
{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavdevice/avdevice.h"
#include "libswscale/swscale.h"
};
#include <string>

using namespace std;
class CFfmpegCamera
{
public:
    CFfmpegCamera();
    ~CFfmpegCamera();

    void initCamera();
	AVFrame * CaptureFromLocalCamera() ;
	bool SaveBmp(const std::string &strFile = "");

private:
	int FindCamera();
	int width,	height; 
	int video_stream;
	AVFormatContext *ic;
	AVPacket packet;
	AVFrame * rgb_frame;
	AVFrame * frame;
	char * buffer;
};