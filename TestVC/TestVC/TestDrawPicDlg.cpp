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

//method���ŵķ�ʽ��0��ʾ����ͼƬԭ����ȥ��Ӧrect,1��ʾ����ͼƬȥ��Ӧrect
void ResizeImage(Mat &img,CRect rect,Mat &dst_img,int method)
{
	// ��ȡͼƬ�Ŀ�͸�
	int h = img.rows;
	int w = img.cols;
	int nw,nh;

	if (0==method)
	{
		// ���㽫ͼƬ���ŵ�dst_drawing��������ı�������
		float scale1 = (float) ( (float)w /(float)rect.Width() );
		float scale2 = (float) ( (float)h /(float)rect.Height() );

		float scale=(scale1>scale2)?scale1:scale2;

		// ���ź�ͼƬ�Ŀ�͸�
		nw = ((float)w)/scale;
		nh = ((float)h)/scale;

		//���ڸ���Ҳ�������ݽضϣ�Ϊ��ֹnw����Ŀ����С�������߽籣��
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
		// ���ź�ͼƬ�Ŀ�͸�
		nw = rect.Width();
		nh = rect.Height();
	}

	// Ϊ�˽����ź��ͼƬ���� dst_drawing �����в�λ�������ͼƬ�� dst_drawing ���Ͻǵ���������ֵ
	int tlx = ( nw < rect.Width())? (((float)(rect.Width()-nw))/2.0f+0.5f):   0;
	int tly = ( nh < rect.Height())? (((float)(rect.Height()-nh))/2.0f+0.5f): 0;

	//���þ����С��ͼ�Ƚϴ�ʱҲ�ܷ�ʱ�䣬�Ӹ��жϿɱ����ظ������þ����С
	if ( dst_img.rows!=rect.Height() || dst_img.cols!=rect.Width() || dst_img.type()!=img.type() )
	{
		dst_img=Mat(rect.Height(),rect.Width(),img.type());
	}
	if (0==method)
	{
		dst_img=0;
	}
	// ���� dst_img �� ROI ��������������ĺ��ͼƬ img
	Rect rect_roi=Rect(tlx,tly,nw,nh);

	// ��ͼƬ img �������ţ������뵽 src_drawing ��
	Mat dst_img_roi=dst_img(rect_roi);

	//��img�ߴ�������dst_img_roi�ߴ�,��ֵ����ѡ��INTER_NEAREST���ŵ������޽�����ʾ�ܽ�ÿ�����ؿ���
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
	m_bmih->biHeight = -m_cvImg.rows;           // �����¶��ϵ�λͼ�� �߶�Ϊ��
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
		int nLineWidth = (sw * m_cvImg.channels() + 3)/4*4; //�ճɴ��ڵ���nColumn����С��4��������
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
