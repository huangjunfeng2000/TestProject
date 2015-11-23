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

class FfmpegManager

{

public:

    FfmpegManager();

    ~FfmpegManager();

    void initFfmpeg();//1.

    bool readFileStream (std::string filename);

    bool writeFileStream (std::string filename);

    bool initInputFfmpeg (std::string inputfile);

    bool initOutputFfmpeg (std::string outputfile); //2.

    bool writeFfmpegData (unsigned char *data);  //3.

    bool FinishOutputFfmpeg();//4.

    void readVedioStream();
    AVStream *addVideoStream (AVFormatContext *oc, enum AVCodecID codec_id);

    AVStream *addAudioStream (AVFormatContext *oc, enum AVCodecID codec_id);

    bool openVideoStream (AVFormatContext *oc, AVStream *st);

    bool openAudioStream (AVFormatContext *oc, AVStream *st);

    bool writeVideoFrame (AVFormatContext *oc, AVStream *st); //3.

    bool writeAudioFrame (AVFormatContext *oc, AVStream *st);

    bool closeVideoStream (AVFormatContext *oc, AVStream *st);

    bool closeAudioStream (AVFormatContext *oc, AVStream *st);

    AVFrame *allocFrameMemmory (enum PixelFormat pix_fmt, int width, int height);

    void getAudioFrame (int16_t *samples, int frame_size, int nb_channels);

    void transformFormat (unsigned char *rgbSrcData, int srcWidth, int srcHeight, int pixelBits,AVFrame *yuvDstPic);

private:

    //in

    AVFormatContext       *m_aviFormatContext;

    AVCodecContext         *m_viCodecContext;//视频codec上下文;

    AVCodecContext         *m_aiCodecContext;//音频codec上下文;

    AVCodec                                                          *m_viCodec;

    AVCodec                                                          *m_aiCodec;

    //out

    AVFormatContext       *m_avoFormatContext;

    AVOutputFormat                    *m_outputFormat;

    AVCodecContext         *m_voCodecContext;

    AVCodecContext         *m_aoCodecContext;

    AVCodec                                                          *m_voCodec;

    AVCodec                                                          *m_aoCodec;

    AVStream                                            *m_vedioStream;

    AVStream                                            *m_audioStream;

    AVFrame                                                          *m_vedioFrame;

    AVFrame                                                          *m_tempVedioFrame;

    AVFrame                                                          *m_rgbFrame;

    std::string                     m_inputFilename;

    std::string                     m_outputFilename;

    int                                             m_videoIndex;

    int                                             m_audioIndex;

    int                                             m_width;

    int                                             m_height;

    unsigned int     m_frameCount;

    uint8_t             *m_vedioInBuff;

    int                                m_vedioInBuffSize;

    AVPacket                         *m_vedioOutBuff;

    int                                             m_vedioOutBuffSize;

    int16_t                         *m_samples;

    AVPacket                         *m_audioOutBuf;

    int                                             m_audioOutBufSize;
    float t, tincr, tincr2;

    int audio_input_frame_size;

    unsigned char   *m_innerData;

    double audio_pts, video_pts;

};