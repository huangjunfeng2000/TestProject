#pragma once


// CFFMpegTestDlg dialog

class CFFMpegTestDlg : public CDialog
{
	DECLARE_DYNAMIC(CFFMpegTestDlg)

public:
	CFFMpegTestDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFFMpegTestDlg();
	BOOL OnInitDialog();
// Dialog Data
	enum { IDD = IDD_DIALOG_FFMPEG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonSavebmp();
};
