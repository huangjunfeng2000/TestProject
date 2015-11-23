// FFMpegTestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TestVC.h"
#include "FFMpegTestDlg.h"

#include <stdlib.h>  
#include <stdio.h>  
#include <string.h>  

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

// CFFMpegTestDlg message handlers
BOOL CFFMpegTestDlg::OnInitDialog()
{
	m_Camera.initCamera();
	return   TRUE;
}
void CFFMpegTestDlg::OnBnClickedButtonSavebmp()
{
	// TODO: Add your control notification handler code here
	m_Camera.SaveBmp();
}
