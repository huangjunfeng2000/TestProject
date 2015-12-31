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
	//SetTimer(1,25,NULL); //��ʱ������ʱʱ���֡��һ��
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
	//������ɴ���.......
	pStc=(CStatic *)GetDlgItem(IDC_VIEW);//IDC_VIEWΪPicture�ؼ�ID
	pStc->GetClientRect(&rect);//��CWind��ͻ���������㴫������
	pDC=pStc->GetDC(); //�õ�Picture�ؼ��豸������
	hDC=pDC->GetSafeHdc(); //�õ��ؼ��豸�����ĵľ�� 

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
			img=cvQueryFrame(capture); //������ͷ�����ļ���ץȡ������һ֡
			//cvGrabFrame( capture ); //������ͷ������Ƶ�ļ���ץȡ֡
			//img=cvRetrieveFrame(capture); //ȡ���ɺ���cvGrabFrameץȡ��ͼ��
			CvvImage m_CvvImage;  
			m_CvvImage.CopyOf(img,1); //���Ƹ�֡ͼ��    
			m_CvvImage.DrawToHDC(hDC, &rect); //��ʾ���豸�ľ��ο���
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
		capture=cvCaptureFromAVI("E:\\E\\OpenCV\\opencv\\sources\\samples\\data\\tree.avi"); //��ʼ��һ����Ƶ�ļ���׽��
	}
	if(!capture)
	{
		AfxMessageBox("�޷��������ͷ");
		return;
	}
	if (!m_pTimer)	
		m_pTimer = SetTimer(1,25,NULL); //��ʱ������ʱʱ���֡��һ��
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
		m_pTimer = SetTimer(1,25,NULL); //��ʱ������ʱʱ���֡��һ��
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
