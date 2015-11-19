// TestDrawPicDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TestDrawPicDlg.h"
#include "opencv/cv.h"
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
#include "CvvImage.h"  

// CTestDrawPicDlg dialog

IMPLEMENT_DYNAMIC(CTestDrawPicDlg, CDialog)

CTestDrawPicDlg::CTestDrawPicDlg(CWnd* pParent /*=NULL*/)
: CDialog(CTestDrawPicDlg::IDD, pParent)
{
	pStc=NULL;
	pDC=NULL;
	capture=NULL;
	m_bDrawCamer = false;
	m_pTimer = NULL;
	m_bSaveVideo = false;
	//SetTimer(1,25,NULL); //定时器，定时时间和帧率一致
}

CTestDrawPicDlg::~CTestDrawPicDlg()
{
}

void CTestDrawPicDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CTestDrawPicDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_OPENAVI, &CTestDrawPicDlg::OnBnClickedButtonOpenavi)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON1, &CTestDrawPicDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CTestDrawPicDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CTestDrawPicDlg::OnBnClickedButton3)
END_MESSAGE_MAP()


// CTestDrawPicDlg message handlers
BOOL CTestDrawPicDlg::OnInitDialog()
{
	//框架生成代码.......
	pStc=(CStatic *)GetDlgItem(IDC_VIEW);//IDC_VIEW为Picture控件ID
	pStc->GetClientRect(&rect);//将CWind类客户区的坐标点传给矩形
	pDC=pStc->GetDC(); //得到Picture控件设备上下文
	hDC=pDC->GetSafeHdc(); //得到控件设备上下文的句柄 

	return   TRUE;
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

void  DrawToHDC(Mat &cvImg, HDC hDC, CRect &rect, int from_x=0, int from_y=0)
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
		delete m_pImageBuffer;
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
void CTestDrawPicDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	if(1==nIDEvent)
	{
		if (m_bDrawCamer && m_videoCapture.isOpened())
		{
			m_videoCapture >> image;
			if (m_bSaveVideo)
				outputVideo << image;
			//imshow("Sample", image);
			DrawToHDC(image, hDC, rect);

		}
		else if (capture)
		{
			IplImage* img=0;  
			img=cvQueryFrame(capture); //从摄像头或者文件中抓取并返回一帧
			//cvGrabFrame( capture ); //从摄像头或者视频文件中抓取帧
			//img=cvRetrieveFrame(capture); //取回由函数cvGrabFrame抓取的图像
			CvvImage m_CvvImage;  
			m_CvvImage.CopyOf(img,1); //复制该帧图像    
			m_CvvImage.DrawToHDC(hDC, &rect); //显示到设备的矩形框内
		}

	}
	CDialog::OnTimer(nIDEvent);
}

void CTestDrawPicDlg::OnBnClickedButtonOpenavi()
{
	// TODO: Add your control notification handler code here
	if (capture)
		return;
	if (!capture)
	{        
		capture=cvCaptureFromAVI("E:\\E\\OpenCV\\opencv\\sources\\samples\\data\\tree.avi"); //初始化一个视频文件捕捉器
	}
	if(!capture)
	{
		AfxMessageBox("无法获得摄像头");
		return;
	}
	if (!m_pTimer)	
		m_pTimer = SetTimer(1,25,NULL); //定时器，定时时间和帧率一致
}

void CTestDrawPicDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here
	m_bDrawCamer = !m_bDrawCamer;
	if(m_bDrawCamer && !m_videoCapture.isOpened())
	{
		m_videoCapture.open(0);
	}
	if (!m_pTimer)
		m_pTimer = SetTimer(1,25,NULL); //定时器，定时时间和帧率一致
}

void CTestDrawPicDlg::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here
	Mat imagecopy = image.clone();
	cv::imwrite("d:\\testsave.jpg", imagecopy);
	cv::imwrite("d:\\testsave.bmp", imagecopy);
}

void CTestDrawPicDlg::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here
	if (!m_videoCapture.isOpened())
		return;
	if (outputVideo.isOpened())
		return;
	const bool askOutputType = true;  // If false it will use the inputs codec type
	// Transform from int to char via Bitwise operators
	int ex = static_cast<int>(m_videoCapture.get(CAP_PROP_FOURCC));     // Get Codec Type- Int form
	char EXT[] = {(char)(ex & 0XFF) , (char)((ex & 0XFF00) >> 8),(char)((ex & 0XFF0000) >> 16),(char)((ex & 0XFF000000) >> 24), 0};

	Size S = Size((int) m_videoCapture.get(CAP_PROP_FRAME_WIDTH),    // Acquire input size
		(int) m_videoCapture.get(CAP_PROP_FRAME_HEIGHT));

	const std::string strOutPut = "d:\\TestSave.avi";
	if (askOutputType)
		m_bSaveVideo = outputVideo.open(strOutPut, ex=-1, m_videoCapture.get(CAP_PROP_FPS), S, true);
	else
		m_bSaveVideo = outputVideo.open(strOutPut, ex, m_videoCapture.get(CAP_PROP_FPS), S, true);
}
