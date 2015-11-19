#include "stdafx.h"  

#include "FFmpegImage.h"


FfmpegManager::FfmpegManager()

{

	m_aviFormatContext = NULL;

	m_avoFormatContext = NULL;

	m_viCodecContext = NULL;

	m_viCodec = NULL;

	m_aiCodecContext = NULL;

	m_aiCodec = NULL;

	m_aoCodecContext = NULL;

	m_voCodecContext = NULL;

	m_aoCodec = NULL;

	m_voCodec = NULL;

	m_outputFormat = NULL;

	m_vedioStream = NULL;

	m_audioStream = NULL;

	m_width = 1024;

	m_height = 768;

	m_innerData = (unsigned char *)malloc(m_width*m_height*3);

	memset(m_innerData,0,m_width*m_height*3);

	m_inputFilename = "What Goes Around.avi";

	m_outputFilename = "./testout.avi";

	m_videoIndex = -1;

	m_audioIndex = -1;

}

FfmpegManager::~FfmpegManager()

{

	FinishOutputFfmpeg();

}

void FfmpegManager::initFfmpeg()

{

	av_register_all();

	initInputFfmpeg(m_inputFilename);

	initOutputFfmpeg(m_outputFilename);

}

bool FfmpegManager::readFileStream(std::string filename)

{

	return true;

}

bool FfmpegManager::writeFileStream(std::string filename)

{

	return true;

}

bool FfmpegManager::initInputFfmpeg(std::string inputfile)

{    
	m_aviFormatContext = avformat_alloc_context();

if(m_aviFormatContext == NULL)

{

	printf("alloc input AVFormatContext error.\n");

	return false;

}

if(avformat_open_input(&m_aviFormatContext,m_inputFilename.c_str(),NULL,NULL) != 0)

{

	printf("open input file error .\n");

	return false;

}

//get stream info;

if(avformat_find_stream_info(m_aviFormatContext, NULL) < 0)

{

	printf("not find stream any infomation .\n");

	return false;

}

av_dump_format(m_aviFormatContext,0,m_inputFilename.c_str(),0);

for(int i=0; i<m_aviFormatContext->nb_streams; i++)

{

	if(m_aviFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)

	{

		m_videoIndex = i;

	}else if(m_aviFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)

	{

		m_audioIndex = i;

	}

}

if(m_videoIndex == -1)

{

	printf("not exist video stream.\n");

	return false;

}

m_viCodecContext = m_aviFormatContext->streams[m_videoIndex]->codec;

m_width = m_viCodecContext->width;

m_height = m_viCodecContext->height;

m_viCodec = avcodec_find_decoder(m_viCodecContext->codec_id);

if(m_viCodec ==  NULL)

{

	printf("not find input file's video decoder. \n");

	return false;

}

//open vedio codec;

if(avcodec_open2(m_viCodecContext,m_viCodec, NULL) < 0)

{

	printf("not open vedio codec.\n");

	return false;

}

if(m_audioIndex == -1)

{

	printf("not exist audio stream.\n");

	return false;

}

m_aiCodecContext = m_aviFormatContext->streams[m_audioIndex]->codec;

m_aiCodec = avcodec_find_decoder(m_aiCodecContext->codec_id);

if(m_aiCodec == NULL)

{

	printf("not find input file's audio decoder.\n");

	return false;

}

if(avcodec_open2(m_aiCodecContext,m_aiCodec, NULL) < 0)

{

	printf("not open audio codec. \n");

	return false;

}

return true;

}

void FfmpegManager::readVedioStream()

{            AVPacket pkt;

av_init_packet(&pkt);

m_vedioInBuffSize = 200000;

m_vedioInBuff = (uint8_t *)av_malloc(m_vedioInBuffSize);

int frameFinished = 0;

while(av_read_frame(m_aviFormatContext,&pkt) >=0 )

{

	if(pkt.stream_index==m_videoIndex)

	{

		avcodec_decode_video2(m_viCodecContext, m_vedioFrame, &frameFinished,&pkt);              

		if(frameFinished)

		{    writeVideoFrame(m_avoFormatContext,m_vedioStream); }

	}

}

av_free_packet(&pkt);

av_free(m_vedioInBuff);

}

bool FfmpegManager::initOutputFfmpeg(std::string outputfile)

{



	avformat_alloc_output_context2(&m_avoFormatContext, NULL, NULL, outputfile.c_str());  

	if (!m_avoFormatContext)  

	{  

		printf("Could not deduce output format from file extension: using MPEG.\n");  

		avformat_alloc_output_context2(&m_avoFormatContext, NULL,"flv", outputfile.c_str());  

	}  



	if (!m_avoFormatContext)  

	{  

		return false;  

	}  



	// 强制指定 MPEG4 编码;  

	m_avoFormatContext->oformat->video_codec = AV_CODEC_ID_MPEG4;  

	m_avoFormatContext->oformat->audio_codec = AV_CODEC_ID_WMV1;  



	m_outputFormat = m_avoFormatContext->oformat;  

	m_vedioStream = NULL;  

	m_audioStream = NULL;  

	if (m_outputFormat->video_codec != AV_CODEC_ID_NONE) 

	{  

		m_vedioStream = addVideoStream(m_avoFormatContext, m_outputFormat->video_codec);  

	}  

	if (m_outputFormat->audio_codec != AV_CODEC_ID_NONE) 

	{  

		m_audioStream = addAudioStream(m_avoFormatContext, m_outputFormat->audio_codec);  

	}  



	av_dump_format(m_avoFormatContext, 0, outputfile.c_str(), 1);  

	if (m_vedioStream)  

		openVideoStream(m_avoFormatContext, m_vedioStream);  

	if (m_audioStream)  

		openAudioStream(m_avoFormatContext, m_audioStream);  





	if (!(m_outputFormat->flags & AVFMT_NOFILE)) 

	{  

		if (avio_open(&m_avoFormatContext->pb, outputfile.c_str(), AVIO_FLAG_WRITE) < 0) 

		{  

			fprintf(stderr, "Could not open '%s'\n", outputfile.c_str());  

			return false;  

		}  

	}  

	avformat_write_header(m_avoFormatContext, NULL);  

	m_vedioFrame->pts = 0;

	return true;

}

bool FfmpegManager::FinishOutputFfmpeg()

{    av_write_trailer(m_avoFormatContext);  

if (m_vedioStream)  

closeVideoStream(m_avoFormatContext, m_vedioStream);  

if (m_audioStream)  

closeAudioStream(m_avoFormatContext, m_audioStream);  

for(int i = 0; i < m_avoFormatContext->nb_streams; i++) 

{  

	av_freep(&m_avoFormatContext->streams[i]->codec);  

	av_freep(&m_avoFormatContext->streams[i]);  

}  

if (!(m_outputFormat->flags & AVFMT_NOFILE)) 

{  

	avio_close(m_avoFormatContext->pb);  

}  

av_free(m_avoFormatContext);  

return true;

}

AVStream *FfmpegManager::addVideoStream(AVFormatContext *ofmtCxt, enum AVCodecID codec_id)

{

	AVCodecContext *codecCxt;  

	AVStream *st;  

	AVCodec *codec;  

	st = avformat_new_stream(ofmtCxt, NULL);  

	if (!st) 

	{  

		fprintf(stderr, "Could not alloc stream.\n");  

		return NULL;  

	}  

	codecCxt = st->codec; 

	codec = avcodec_find_encoder(codec_id);  

	if (!codec) 

	{  

		fprintf(stderr, "encoder not found.\n");  

		return NULL;  

	}  

	avcodec_get_context_defaults3(codecCxt, codec);  

	codecCxt->codec_id = codec_id;  

	codecCxt->bit_rate = 400000;  

	codecCxt->width = m_width;  

	codecCxt->height = m_height;  

	codecCxt->time_base.den = 25;  

	codecCxt->time_base.num = 1;  

	codecCxt->gop_size = 12;  

	codecCxt->max_b_frames  = 1;

	codecCxt->pix_fmt = (PixelFormat)AV_PIX_FMT_YUV420P;

	if (codecCxt->codec_id == AV_CODEC_ID_MPEG2VIDEO) 

	{  

		codecCxt->max_b_frames = 2;  

	}  

	if (codecCxt->codec_id == AV_CODEC_ID_MPEG1VIDEO)

	{  

		codecCxt->mb_decision=2;  

	}  

	if (ofmtCxt->oformat->flags & AVFMT_GLOBALHEADER)  

	{

		codecCxt->flags |= CODEC_FLAG_GLOBAL_HEADER;  

	}

	return st;  

};

AVStream *FfmpegManager::addAudioStream(AVFormatContext *ofmtCxt, enum AVCodecID codec_id)

{

	AVCodecContext *codecCxt;  

	AVStream *st;  

	st = avformat_new_stream(ofmtCxt, NULL);  

	if (!st) 

	{  

		fprintf(stderr, "Could not alloc stream.\n");  

		return NULL;  

	}  

	st->id = 1;  

	codecCxt = st->codec;  

	codecCxt->codec_id = codec_id;  

	codecCxt->codec_type = AVMEDIA_TYPE_AUDIO;  

	codecCxt->sample_fmt = AV_SAMPLE_FMT_S16;  

	codecCxt->bit_rate = 64000;  

	codecCxt->sample_rate = 44100;  

	codecCxt->channels = 2;  

	if (ofmtCxt->oformat->flags & AVFMT_GLOBALHEADER)  

		codecCxt->flags |= CODEC_FLAG_GLOBAL_HEADER;  

	return st;

};

bool FfmpegManager::openVideoStream(AVFormatContext *ofmtCxt, AVStream *st)

{

	AVCodec *codec;  

	AVCodecContext *codecCxt;  



	codecCxt = st->codec;     

	codec = avcodec_find_encoder(codecCxt->codec_id);  

	if (!codec) 

	{  

		fprintf(stderr, "codec not found\n");  

		return false;  

	}  

	if (avcodec_open2(codecCxt, codec, NULL) < 0) 

	{  

		fprintf(stderr, "could not open codec\n");  

		return false;  

	}  



	m_vedioOutBuff = NULL;

	if (!(ofmtCxt->oformat->flags & AVFMT_RAWPICTURE)) 

	{          m_vedioOutBuffSize = 200000;  

	m_vedioOutBuff = (AVPacket *)av_malloc(m_vedioOutBuffSize);  

	}  



	m_vedioFrame = allocFrameMemmory(codecCxt->pix_fmt, codecCxt->width, codecCxt->height);  

	if (!m_vedioFrame) 

	{  

		fprintf(stderr, "Could not allocate frame.\n");  

		return false;  

	}  

	m_rgbFrame = av_frame_alloc();//PixelFormat24bppRGB, codecCxt->width, codecCxt->height);

	if(!m_rgbFrame)

	{

		printf("could not allocate frame.\n");

		return false;

	}

	m_tempVedioFrame = NULL;  

	if (codecCxt->pix_fmt != AV_PIX_FMT_YUV420P) 

	{  

		m_tempVedioFrame = allocFrameMemmory(codecCxt->pix_fmt, codecCxt->width, codecCxt->height);  

		if (!m_tempVedioFrame) 

		{  

			fprintf(stderr, "Could not allocate temporary picture\n");  

			return false;  

		}  

	}      return true;

}

bool FfmpegManager::openAudioStream(AVFormatContext *ofmtCxt, AVStream *st)

{

	AVCodecContext *codecCxt;  

	AVCodec *codec;  

	codecCxt = st->codec;  

	codec = avcodec_find_encoder(codecCxt->codec_id);  

	if (!codec) 

	{  

		fprintf(stderr, "codec not found\n");  

		return false;  

	}  

	if (avcodec_open2(codecCxt, codec, NULL) < 0) 

	{  

		fprintf(stderr, "could not open codec\n");  

		return false;  

	}  

	t = 0;  

	tincr = 2 * M_PI * 110.0 / codecCxt->sample_rate;  

	tincr2 = 2 * M_PI * 110.0 / codecCxt->sample_rate / codecCxt->sample_rate;  



	m_audioOutBufSize = 100000;  

	m_audioOutBuf = av_packet_alloc();//(m_audioOutBufSize);  



	if (codecCxt->frame_size <= 1) 

	{  

		audio_input_frame_size = m_audioOutBufSize / codecCxt->channels;  

		switch(st->codec->codec_id) 

		{  

		case AV_CODEC_ID_PCM_S16LE:  

		case AV_CODEC_ID_PCM_S16BE:  

		case AV_CODEC_ID_PCM_U16LE:  

		case AV_CODEC_ID_PCM_U16BE:  

			audio_input_frame_size >>= 1;  

			break;  

		default:  

			break;  

		}  

	} else {  

		audio_input_frame_size = codecCxt->frame_size;  

	}  

	m_samples = (int16_t *)av_malloc(audio_input_frame_size * 2 * codecCxt->channels); 

	return true;

}

bool FfmpegManager::writeVideoFrame(AVFormatContext *ofmtCxt, AVStream *st)

{

	int out_size, ret;  

	AVCodecContext *codecCxt;  

	static struct SwsContext *img_convert_ctx;  

	codecCxt = st->codec;  



	if (codecCxt->pix_fmt != AV_PIX_FMT_YUV420P) 

	{          if (img_convert_ctx == NULL) 

	{  

		img_convert_ctx = sws_getContext(codecCxt->width, codecCxt->height, AV_PIX_FMT_RGB24, codecCxt->coded_width, codecCxt->coded_height, AV_PIX_FMT_RGB24, SWS_FAST_BILINEAR, NULL, NULL, NULL);  

		if (img_convert_ctx == NULL) 

		{  

			fprintf(stderr, "Cannot initialize the conversion context.\n");  

			return false;  

		}  

	}   

	sws_scale(img_convert_ctx, m_tempVedioFrame->data, m_tempVedioFrame->linesize,  

		0, codecCxt->height, m_vedioFrame->data, m_vedioFrame->linesize);  

	}

	if (ofmtCxt->oformat->flags & AVFMT_RAWPICTURE) 

	{  

		AVPacket pkt;  

		av_init_packet(&pkt);  

		pkt.flags |= AV_PKT_FLAG_KEY;  

		pkt.stream_index = st->index;  

		pkt.data = (uint8_t *)m_vedioFrame;  

		pkt.size = sizeof(AVPicture);  

		ret = av_interleaved_write_frame(ofmtCxt, &pkt);  

	} else 

	{  

		out_size = avcodec_encode_video2(codecCxt, m_vedioOutBuff, m_vedioFrame, NULL);  

		if (out_size > 0) 

		{  

			AVPacket pkt;  

			av_init_packet(&pkt);  

			if(codecCxt->coded_frame->pts != AV_NOPTS_VALUE)  

				pkt.pts= av_rescale_q(codecCxt->coded_frame->pts, codecCxt->time_base, st->time_base);  

			if(codecCxt->coded_frame->key_frame )  

			{

				pkt.flags  |= AV_PKT_FLAG_KEY;  

			}

			pkt.stream_index = st->index;  

			pkt.data = m_vedioOutBuff->data;  

			pkt.size = out_size;  

			ret = av_interleaved_write_frame(ofmtCxt, &pkt);  

			//ret = av_write_frame(ofmtCxt,&pkt);

		} else 

		{  

			ret = 0;  

		} 

	}  

	if (ret != 0) 

	{  

		fprintf(stderr, "Error while writing video frame\n");  

		return false;  

	}  

	m_frameCount++;  

	return true;

}

bool FfmpegManager::writeAudioFrame(AVFormatContext *ofmtCxt, AVStream *st)

{

	AVCodecContext *codecCxt;  

	AVPacket pkt;  

	av_init_packet(&pkt);  

	codecCxt = st->codec;  

	getAudioFrame(m_samples, audio_input_frame_size, codecCxt->channels);  

	int got_packet_ptr;

	AVFrame *frame = av_frame_alloc();  
	frame->nb_samples = codecCxt->frame_size;  
	frame->format = codecCxt->sample_fmt;  
	frame->channel_layout = codecCxt->channel_layout;  
	pkt.size = avcodec_encode_audio2(codecCxt, m_audioOutBuf, frame, &got_packet_ptr);  
	//pkt.size = avcodec_encode_audio2(codecCxt, m_audioOutBuf, m_audioOutBufSize, m_samples);  

	if (codecCxt->coded_frame && codecCxt->coded_frame->pts != AV_NOPTS_VALUE)  

		pkt.pts= av_rescale_q(codecCxt->coded_frame->pts, codecCxt->time_base, st->time_base);  

	pkt.flags |= AV_PKT_FLAG_KEY;  

	pkt.stream_index = st->index;  

	pkt.data = m_audioOutBuf->data;              
	if (av_interleaved_write_frame(m_avoFormatContext, &pkt) != 0) 

	{  

		fprintf(stderr, "Error while writing audio frame\n");  

		return false;  

	}  

	return true;

} 