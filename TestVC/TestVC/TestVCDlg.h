// TestVCDlg.h : header file
//

#pragma once

#include <string>
#include "afxwin.h"
#include <vector>

class CImageManager
{
public:
	CImageManager()
	{
		m_iOffset = 10;
		m_bitMap = NULL;
	}
	~CImageManager()
	{
		if (m_bitMap)
		{
			m_bitMap->DeleteObject();
			delete m_bitMap;
			m_bitMap = NULL;
		}
	}
	void SetDrawRect(CRect &rect)
	{
		m_drawRect = rect;
		;
	}
	bool ScrollEnable()
	{
		return GetBmpHeightTotal(GetBmpCount()) > m_drawRect.Height();
	}
	int GetBmpCount()
	{
		return m_vecFiles.size();
	}
	int GetBmpWidth()
	{
		return min(m_drawRect.Width(), m_drawRect.Height());
	}
	int GetBmpHeight()
	{
		return min(m_drawRect.Width(), m_drawRect.Height());
	}
	int GetBmpHeightTotal(int iCount)
	{
		int iRes =  (GetBmpHeight() + m_iOffset) * iCount;
		return iCount ? iRes - m_iOffset : 0;
	}
	void AddBmpFile(const std::string &strFile)
	{
		m_vecFiles.push_back(strFile);
		CBitmap *pbitMap = InitBitMap();
		for(int i=0; i<GetBmpCount(); ++i)
			CopyBitmap(pbitMap, i);
		CBitmap *pOldMap = m_bitMap;
		m_bitMap = pbitMap;
		if (pOldMap)
		{
			pOldMap->DeleteObject();
			delete pOldMap;
		}
	}
	CBitmap * InitBitMap()
	{
		CBitmap *m_bitMapTemp = new CBitmap;
		CClientDC dc(NULL);
		m_bitMapTemp->CreateCompatibleBitmap(&dc, GetBmpWidth(), GetBmpHeightTotal(GetBmpCount()));
		HBITMAP hbmResult = (HBITMAP)m_bitMapTemp->m_hObject;
		CDC destDC;
		destDC.CreateCompatibleDC(NULL);
		HBITMAP hbmOldDest = (HBITMAP)::SelectObject(destDC.m_hDC, hbmResult);
		destDC.FillSolidRect(0, 0, GetBmpWidth(), GetBmpHeightTotal(GetBmpCount()), RGB(255, 255, 255));
		::SelectObject(destDC.m_hDC, hbmOldDest);
		::DeleteObject(destDC.m_hDC);
		return m_bitMapTemp;
	}
	void CopyBitmap(CBitmap *pMap, int index)
	{
		CDC sourcedDC, destDC;
		sourcedDC.CreateCompatibleDC(NULL);
		destDC.CreateCompatibleDC(NULL);
		HBITMAP hbmResult = (HBITMAP)pMap->m_hObject;
		HBITMAP hbmOldDest = (HBITMAP)::SelectObject(destDC.m_hDC, hbmResult);
		{
			CBitmap pbitmap;
			HBITMAP hbmp = (HBITMAP)::LoadImage(AfxGetInstanceHandle(), m_vecFiles[index].c_str(), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_LOADFROMFILE);
			if (!hbmp)
				return;
			pbitmap.Attach(hbmp);
			BITMAP bm;
			pbitmap.GetBitmap(&bm);
			HBITMAP hbmOldSource = (HBITMAP)::SelectObject(sourcedDC.m_hDC, pbitmap.m_hObject);
			int width = GetBmpWidth();
			int height = GetBmpHeight();
			if (bm.bmWidth < bm.bmHeight)
				width = (bm.bmWidth / (float)bm.bmHeight * height);
			else
				height = (bm.bmHeight / (float)bm.bmWidth * width);
			destDC.StretchBlt(0, GetBmpHeightTotal(index)+m_iOffset, width, height, &sourcedDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
			::SelectObject(sourcedDC.m_hDC, hbmOldSource);
		}
		SelectObject(destDC.m_hDC, hbmOldDest);
		DeleteObject(sourcedDC.m_hDC);
		DeleteObject(destDC.m_hDC);
	}

	void DrawImage(CWnd *pWnd, float fScale)
	{
		if (GetBmpCount() <=0 )
			return;
		CPaintDC dc(pWnd);
		CDC dcImage;
		if (!dcImage.CreateCompatibleDC(&dc))
			return;
		//CWnd *pWnd = GetDlgItem(IDC_STATIC_IMAGE);
		CRect rect = m_drawRect;
		//pWnd->GetClientRect(&rect);
		//pWnd->ClientToScreen(&rect);
		//ScreenToClient(&rect);
		int m_iStartY = fScale * (GetBmpHeightTotal(GetBmpCount()) - m_drawRect.Height());
		CBitmap *pOld = dcImage.SelectObject(m_bitMap);
		dc.BitBlt(rect.left, rect.top, rect.Width(), rect.Height(), &dcImage, 0, m_iStartY, SRCCOPY);
		dcImage.SelectObject(pOld);
		//m_image.Draw(dc.GetSafeHdc(), rect.left, rect.top, rect.Width(), rect.Height(), 0, 0, m_image.GetWidth(), m_image.GetHeight());
	}
public:
	CBitmap *m_bitMap;
	CRect m_drawRect;
	int m_iOffset;
	std::vector<std::string> m_vecFiles;


};
// CTestVCDlg dialog
class CTestVCDlg : public CDialog
{
// Construction
public:
	CTestVCDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_TESTVC_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	//afx_msg void OnEnChangeRichedit21();
	afx_msg void OnBnClickedButton1();
	//virtual void OnDraw(CDC* pDC);  // overridden to draw this view
public:
	//LanImage m_image;
	CBitmap m_bitMap;
	CImageManager m_imgManager;
	int m_iStartY;

	void DrawImage();
	CScrollBar m_sclBar;
	//afx_msg void OnNMThemeChangedScrollbarImage(NMHDR *pNMHDR, LRESULT *pResult);
	void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);  
	BOOL CTestVCDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)  ;
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButtonFfmpeg();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButton5();
};

