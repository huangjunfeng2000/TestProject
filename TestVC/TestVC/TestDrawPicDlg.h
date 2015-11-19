#pragma once
#include "resource.h"
#include "opencv2/highgui.hpp"
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/videoio/videoio.hpp"  // Video write

#include <iostream>

using namespace cv;
using namespace std;
// CTestDrawPicDlg dialog

class CTestDrawPicDlg : public CDialog
{
	DECLARE_DYNAMIC(CTestDrawPicDlg)

public:
	CTestDrawPicDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTestDrawPicDlg();
	BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_DIALOG_CAPTURE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	//void OnTimer(UINT nIDEvent);

private:
	CRect rect;
	CStatic* pStc; //标识图像显示的Picture控件
	CDC* pDC; //视频显示控件设备上下文
	HDC hDC; //视频显示控件设备句柄
	CvCapture* capture; //视频获取结构
	VideoCapture m_videoCapture;
	VideoWriter outputVideo;
	bool m_bSaveVideo;
	Mat image;
	bool m_bDrawCamer;
	UINT_PTR m_pTimer;

public:
	afx_msg void OnBnClickedButtonOpenavi();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
};
