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
#include "commonfunc.h"
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
