#include "stdafx.h"
#include "ImageManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CImageManager::CImageManager()
{
	m_iOffset = 10;
	m_bitMap = NULL;
}
CImageManager::~CImageManager()
{
	if (m_bitMap)
	{
		m_bitMap->DeleteObject();
		delete m_bitMap;
		m_bitMap = NULL;
	}
}
void CImageManager::SetDrawRect(CRect &rect)
{
	m_drawRect = rect;
	InflateRect(&m_drawRect, -3, -3);
}
bool CImageManager::ScrollEnable()
{
	return GetBmpHeightTotal(GetBmpCount()) > m_drawRect.Height();
}
int CImageManager::GetBmpCount()
{
	return m_vecFiles.size();
}
int CImageManager::GetBmpWidth()
{
	return min(m_drawRect.Width(), m_drawRect.Height());
}
int CImageManager::GetBmpHeight()
{
	return min(m_drawRect.Width(), m_drawRect.Height());
}
int CImageManager::GetBmpHeightTotal(int iCount)
{
	int iRes =  (GetBmpHeight() + m_iOffset) * iCount;
	return iCount ? iRes - m_iOffset : 0;
}
void CImageManager::AddBmpFile(const std::string &strFile)
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
CBitmap * CImageManager::InitBitMap()
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
void CImageManager::CopyBitmap(CBitmap *pMap, int index)
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

void CImageManager::DrawImage(CWnd *pWnd, float fScale)
{
	if (GetBmpCount() <=0 )
		return;
	CPaintDC dc(pWnd);
	CDC dcImage;
	if (!dcImage.CreateCompatibleDC(&dc))
		return;
	CRect rect = m_drawRect;
	int m_iStartY = fScale * (GetBmpHeightTotal(GetBmpCount()) - m_drawRect.Height());
	CBitmap *pOld = dcImage.SelectObject(m_bitMap);
	dc.BitBlt(rect.left, rect.top, rect.Width(), rect.Height(), &dcImage, 0, m_iStartY, SRCCOPY);
	dcImage.SelectObject(pOld);
}