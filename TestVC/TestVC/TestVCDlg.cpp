// TestVCDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TestVC.h"
#include "TestVCDlg.h"
#include "TestDrawPicDlg.h"
#include "FFMpegTestDlg.h"
#include "CTableDefine.h"
#include "commonfunc.h"
#include "CameraDS.h"
#include "IniFile.h"
#include "ImageManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CAboutDlg dialog used for App About
class CAboutDlg : public CDialog
{
public:
    CAboutDlg();

// Dialog Data
    enum { IDD = IDD_ABOUTBOX };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CTestVCDlg dialog
CTestVCDlg::CTestVCDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CTestVCDlg::IDD, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_iStartY = 0;
	m_imgManager = new CImageManager();
}
CTestVCDlg::~CTestVCDlg()
{
	delete m_imgManager;
}

void CTestVCDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_SCROLLBAR_IMAGE, m_sclBar);
}

BEGIN_MESSAGE_MAP(CTestVCDlg, CDialog)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    //}}AFX_MSG_MAP
    //ON_EN_CHANGE(IDC_RICHEDIT21, &CTestVCDlg::OnEnChangeRichedit21)
    ON_BN_CLICKED(IDC_BUTTON1, &CTestVCDlg::OnBnClickedButton1)
    //ON_NOTIFY(NM_THEMECHANGED, IDC_SCROLLBAR_IMAGE, &CTestVCDlg::OnNMThemeChangedScrollbarImage)
    ON_WM_HSCROLL()
    ON_WM_VSCROLL()
    ON_BN_CLICKED(IDC_BUTTON2, &CTestVCDlg::OnBnClickedButton2)
    ON_BN_CLICKED(IDC_BUTTON_FFMPEG, &CTestVCDlg::OnBnClickedButtonFfmpeg)
    ON_BN_CLICKED(IDC_BUTTON3, &CTestVCDlg::OnBnClickedButton3)
    ON_BN_CLICKED(IDC_BUTTON4, &CTestVCDlg::OnBnClickedButton4)
    ON_BN_CLICKED(IDC_BUTTON5, &CTestVCDlg::OnBnClickedButton5)
END_MESSAGE_MAP()


// CTestVCDlg message handlers

BOOL CTestVCDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    // Add "About..." menu item to system menu.
    // IDM_ABOUTBOX must be in the system command range.
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);
    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != NULL)
    {
        CString strAboutMenu;
        strAboutMenu.LoadString(IDS_ABOUTBOX);
        if (!strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }
    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);			// Set big icon
    SetIcon(m_hIcon, FALSE);		// Set small icon
    // TODO: Add extra initialization here
    std::string strFile = "E:\\HJF\\TestVC\\0.bmp";
    CWnd *pWnd = GetDlgItem(IDC_STATIC_IMAGE);
    CRect rect;
    pWnd->GetClientRect(&rect);
    pWnd->ClientToScreen(&rect);
    ScreenToClient(&rect);
    m_imgManager->SetDrawRect(rect);
    //m_image.LoadImage(strFile.c_str(), m_degree, TRUE);
    //HBITMAP hbmp = (HBITMAP)::LoadImage(AfxGetInstanceHandle(), strFile.c_str(), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
    //if (hbmp)
    //{
    //	m_bitMap.Attach(hbmp);
    //}
    m_sclBar.SetScrollRange(0, 1000);
	m_sclBar.EnableWindow(false);
    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTestVCDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    }
    else
    {
        CDialog::OnSysCommand(nID, lParam);
    }
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTestVCDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // device context for painting
        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);
        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;
        // Draw the icon
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        m_imgManager->DrawImage(this, m_iStartY / 1000.0);
        CDialog::OnPaint();
        //DrawImage();
    }
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CTestVCDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}
void CTestVCDlg::ReDrawImage()
{
	CWnd *pWnd = GetDlgItem(IDC_STATIC_IMAGE);
	//pWnd->Invalidate();
	CRect rect;
	pWnd->GetClientRect(&rect);
	pWnd->ClientToScreen(&rect);
    ScreenToClient(&rect);
	
	InvalidateRect(&rect);
	UpdateWindow();
	m_sclBar.EnableWindow(m_imgManager->ScrollEnable());
}
void CTestVCDlg::OnBnClickedButton1()
{
    // TODO: Add your control notification handler code here
    int 	m_degree=1;
    std::string strFile = "E:\\HJF\\TestVC\\0.bmp";
    m_imgManager->AddBmpFile(strFile);
    //m_image.LoadImage(strFile.c_str(), m_degree, TRUE);
    //HBITMAP hbmp = (HBITMAP)::LoadImage(AfxGetInstanceHandle(), strFile.c_str(), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
    //if (hbmp)
    //{
    //	m_bitMap.Attach(hbmp);
    //}
    //m_bitMap.LoadBitmap()
	ReDrawImage();
    //Invalidate(TRUE);
}

void CTestVCDlg::DrawImage()
{
    CPaintDC dc(this);
    CDC dcImage;
    if (!dcImage.CreateCompatibleDC(&dc))
        return;
    CWnd *pWnd = GetDlgItem(IDC_STATIC_IMAGE);
    CRect rect;
    pWnd->GetClientRect(&rect);
    pWnd->ClientToScreen(&rect);
    ScreenToClient(&rect);
    CBitmap *pOld = dcImage.SelectObject(&m_bitMap);
    dc.BitBlt(rect.left, rect.top, rect.Width(), rect.Height(), &dcImage, 0, m_iStartY, SRCCOPY);
    dcImage.SelectObject(pOld);
    //m_image.Draw(dc.GetSafeHdc(), rect.left, rect.top, rect.Width(), rect.Height(), 0, 0, m_image.GetWidth(), m_image.GetHeight());
}
void CTestVCDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    // TODO: 在此添加消息处理程序代码和/或调用默认值
    SCROLLINFO si;
    si.cbSize = sizeof(si);
    m_sclBar.GetScrollInfo(&si, SIF_POS|SIF_PAGE|SIF_RANGE);
    int nVscroll = si.nPos;
    switch(nSBCode)
    {
    case SB_LINEDOWN:
        nVscroll += 10;
        if (nVscroll > (si.nMax - si.nMin - si.nPage ))
        {
            nVscroll = si.nMax - si.nMin - si.nPage;
        }
        break;
    case SB_LINEUP:
        nVscroll -= 10;
        if (nVscroll < si.nMin)
        {
            nVscroll = 0;
        }
        break;
    case SB_PAGEDOWN:
        nVscroll += si.nPage;
        if (nVscroll > (si.nMax - si.nMin - si.nPage))
        {
            nVscroll = si.nMax - si.nMin - si.nPage;
        }
        break;
    case SB_PAGEUP:
        nVscroll -= si.nPage;
        if (nVscroll < si.nMin)
        {
            nVscroll = 0;
        }
        break;
    case  SB_THUMBTRACK:
        nVscroll = nPos;
        break;
    }
    if (nVscroll != m_iStartY)
    {
        m_iStartY = nVscroll;
        //Invalidate();
		ReDrawImage();
    }
    //ScrollWindow(0, -(nVscroll -si.nPos) , NULL ,NULL);
    //m_sclBar.MoveWindow(rt_mvScroll, TRUE);
    si.fMask = SIF_POS;
    si.nPos = nVscroll;
    m_sclBar.SetScrollInfo(&si, TRUE);
    //CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CTestVCDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    // TODO: Add your message handler code here and/or call default
    CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CTestVCDlg::OnBnClickedButton2()
{
    // TODO: Add your control notification handler code here
    CTestDrawPicDlg dlg;
    dlg.DoModal();
}

void CTestVCDlg::OnBnClickedButtonFfmpeg()
{
    // TODO: Add your control notification handler code here
    CFFMpegTestDlg dlg;
    dlg.DoModal();
}

void CTestVCDlg::OnBnClickedButton3()
{
    // TODO: Add your control notification handler code here
    const std::string strDB = "TestScope.db3";
    bool bRes = OpenProject(strDB);
    std::vector<SharRecBase> vecValues, vecValues2;
    SharRecBase sValue;
    ReadAllInfo(DB_ID_BEXAMINE, vecValues);
    ReadAllInfoCon(DB_ID_BEXAMINE, vecValues2, "id > 2");
    ReadInfo(DB_ID_BEXAMINE, 1, sValue);
    if (sValue.get())
    {
        CRecData *pData = (CRecData *)(sValue.get());
        pData->m_strColumn2 = "strColumn2";
        WriteInfo(DB_ID_BEXAMINE, pData);
    }
    CRecData item;
    item.m_strColumn1 = GetRandString();
    int id = AddInfo(DB_ID_BEXAMINE, &item);
    bRes = Delete(DB_ID_BEXAMINE, id);
}

void CTestVCDlg::OnBnClickedButton4()
{
    // TODO: Add your control notification handler code here
	std::string strFile = GetConfigFile("Config.ini");
	CIniFile iniFile(strFile.c_str());
	iniFile.SetProfileInt("Section", "Age", 100);
	int iVal = iniFile.GetProfileInt("Section", "Age");
	::WritePrivateProfileString("Section", "Name", "HJF", strFile.c_str());
	//WritePrivateProfileFloat("Section", "Age", "HJF", 10);
	int iRes = ::GetPrivateProfileInt("Section", "Age", 0, strFile.c_str());
	//float fRes = ::GetPrivateProfileFloat("Section", "Age", 0, strFile.c_str());

    SetOpitionValue("Section", "Name", "HJF");
    std::string strName;
    GetOpitionValue("Section", "Name", strName);
}

void CTestVCDlg::OnBnClickedButton5()
{
    // TODO: Add your control notification handler code here
	TestDSDraw();
}
