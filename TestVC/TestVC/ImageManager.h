// TestVCDlg.h : header file
//

#pragma once

#include <string>
#include "afxwin.h"
#include <vector>

class CImageManager
{
public:
	CImageManager();
	~CImageManager();

	void SetDrawRect(CRect &rect);
	bool ScrollEnable();
	int GetBmpCount();
	int GetBmpWidth();
	int GetBmpHeight();
	int GetBmpHeightTotal(int iCount);
	void AddBmpFile(const std::string &strFile);
	CBitmap * InitBitMap();
	void CopyBitmap(CBitmap *pMap, int index);
	void DrawImage(CWnd *pWnd, float fScale);

public:
	CBitmap *m_bitMap;
	CRect m_drawRect;
	int m_iOffset;
	std::vector<std::string> m_vecFiles;
};
