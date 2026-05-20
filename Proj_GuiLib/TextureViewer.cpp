// TextureViewer.cpp : implementation file
//

#include "stdh.h"

#include "RenderSystem/ITexture.h"


#include "TextureViewer.h"
#include "stringparser/stringparser.h"
#include "resdata/TexData.h"

#include "ResSelectorDlg.h"

CTextureViewer::CTextureViewer(IRenderSystem* rs) : _pRender(rs), _selectMode(RSM_PARTIAL), _opMode(OP_NONE)
{
	_ptRef.x = _ptRef.y = 0;

	_rcSelection.left = _rcSelection.right = 0;
	_rcSelection.top = _rcSelection.bottom = 0;

	//_font.CreatePointFont(90, _T("Tahoma"));
	
	LOGBRUSH br;
	br.lbStyle = BS_HOLLOW;
	br.lbColor = 0;
	br.lbHatch = 0;
	_brSelection.CreateBrushIndirect(&br);

	_penSelection.CreatePen(PS_SOLID, 2, RGB(0, 255, 0));
	_penImage.CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
}

CTextureViewer::~CTextureViewer()
{
}

BEGIN_MESSAGE_MAP(CTextureViewer, CWnd)
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()


// CTextureViewer message handlers
void CTextureViewer::SetSelectMode(int mode)
{
	_selectMode = mode;
}

void CTextureViewer::SetResource(const char* res)
{
	_ptRef.x = _ptRef.y = 0;

	_rcSelection.left = _rcSelection.right = 0;
	_rcSelection.top = _rcSelection.bottom = 0;	

	if (_image.IsValid())
	{
		_image.Destroy();
	}

	if (!res || !_pRender)
	{
		InvalidateRect(NULL, FALSE);
		return;
	}

	//计算资源的相对路径
	const char* pszName = NULL;
	const char* pszTexPath = _pRender->GetPath(Path_Res);
	if (CheckPathContaining(pszTexPath,res))
	{
		pszName = res + strlen(pszTexPath);
		if (*pszName == '\\')
			pszName++;
	}

	// Invalid resource path
	if (!pszName)
		return;

	// Obtain texture resource
	ITexture* pTex = (ITexture*) _pRender->GetTexMgr()->ObtainRes(pszName);
	if (pTex&&pTex->ForceTouch())
	{
		extern CxImage *ImageFromTex(ITexture*tex,CxImage *container);
		ImageFromTex(pTex,&_image);
		pTex->Release(); 
	}
	InvalidateRect(NULL, FALSE);
}

BOOL CTextureViewer::IsCanView(const char* res) const
{
	BOOL bOK = FALSE;

	static const char* TEXTURE_FILTER[] = 
	{
		".tex", ".dds", ".jpg", ".jpeg", ".bmp", ".png", ".tif", ".tga"
	};

	std::string path = res;
	strlwr(const_cast<char*>(path.c_str()));
	
	for (int i = 0; i < sizeof(TEXTURE_FILTER)/sizeof(char*); i++)
	{
		if (path.find(TEXTURE_FILTER[i]) != std::string::npos)
		{
			bOK = TRUE;
			break;
		}
	}
	return bOK;
}

const RECT& CTextureViewer::GetSelectedRect() const
{
	return _rcSelection;
}

void CTextureViewer::SetSelectedRect(RECT &rc)
{
	_rcSelection=rc;
}


void CTextureViewer::Update()
{
}

void CTextureViewer::Draw(CDC* pDC)
{
	RECT rcView;
	GetClientRect(&rcView);
	int width = rcView.right - rcView.left;
	int height = rcView.bottom - rcView.top;
	
	pDC->FillSolidRect(&rcView, GetSysColor(COLOR_BACKGROUND));

	if (!_image.IsValid())	// Fail to load the resource
		return;

	if (_selectMode == RSM_ENTIRE)
	{
		float xScale;
		float yScale;
		if (_image.GetWidth() > 0 && _image.GetHeight() > 0)
		{
			xScale = float(width) / float(_image.GetWidth());
			yScale = float(height) / float(_image.GetHeight());
			if (xScale > yScale) xScale = yScale;
			if (xScale > 1.0f) xScale = 1.0f;

			rcView.right = rcView.left + int(_image.GetWidth() * xScale);
			rcView.bottom = rcView.top + int(_image.GetHeight() * xScale);
			_image.Draw(pDC->GetSafeHdc(), rcView, NULL);
		}

		DrawImageEdge(pDC);
	}
	else	// partial
	{
		rcView.left = _ptRef.x;
		rcView.top = _ptRef.y;
		rcView.right = rcView.left + _image.GetWidth();
		rcView.bottom = rcView.top + _image.GetHeight();

		RECT rcClip;
		rcClip.left = 0;
		rcClip.top = 0;
		rcClip.right = width;
		rcClip.bottom = height;
	
		_image.Draw(pDC->GetSafeHdc(), rcView, &rcClip);

		DrawImageEdge(pDC);

		if ((_rcSelection.right > _rcSelection.left) && 
			(_rcSelection.bottom > _rcSelection.top))
		{
			DrawSelection(pDC);
		}

		//DrawInformation(pDC);
	}	
}

void CTextureViewer::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	RECT rcView;
	GetClientRect(&rcView);
	DWORD width = rcView.right - rcView.left;
	DWORD height = rcView.bottom - rcView.top;
	if ((_selectMode == RSM_PARTIAL) && 
		(width < _image.GetWidth() || height < _image.GetHeight()))
	{
		_opMode = OP_MOVE;
		SetCapture();
		SetCursor(LoadCursor(NULL, IDC_SIZEALL));
		GetCursorPos(&_ptMove);		
	}	

	//__super::OnRButtonDown(nFlags, point);
}

void CTextureViewer::OnRButtonUp(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if (_opMode == OP_MOVE)
	{
		ReleaseCapture();
		SetCursor(LoadCursor(NULL, IDC_ARROW));		
		_opMode = OP_NONE;
	}

	//__super::OnRButtonUp(nFlags, point);
}

void CTextureViewer::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	RECT rcView;
	GetClientRect(&rcView);
	if (_opMode == OP_MOVE)
	{
		POINT pt;
		GetCursorPos(&pt);

		int width = rcView.right - rcView.left;
		int height = rcView.bottom - rcView.top;

		if (width < _image.GetWidth())
		{
			_ptRef.x += (pt.x - _ptMove.x);
			if (_ptRef.x > 0) _ptRef.x = 0;
			if (DWORD(width -_ptRef.x) > _image.GetWidth())
				_ptRef.x = width - _image.GetWidth();
		}
		if (height < _image.GetHeight())
		{
			_ptRef.y += (pt.y - _ptMove.y);
			if (_ptRef.y > 0)_ptRef.y = 0;
			if (DWORD(height -_ptRef.y) > _image.GetHeight())
				_ptRef.y = height - _image.GetHeight();
		}	

		_ptMove = pt;

		InvalidateRect(NULL, FALSE);
	}

	if (_selectMode == RSM_PARTIAL)
	{
		point -= _ptRef;
		GetParent()->SendMessage(WM_TV_UPDATEINFO, WPARAM(&point), LPARAM(&_rcSelection));
	}

	//__super::OnMouseMove(nFlags, point);
}

void CTextureViewer::OnLButtonDown(UINT nFlags, CPoint point)
{
	// Point select
	if ((_selectMode == RSM_PARTIAL) && _image.IsValid())
	{
		point -= _ptRef;

		RECT rcView;
		GetClientRect(&rcView);

		RECT rcInfo;
		rcInfo.left = 0;
		rcInfo.right = rcView.right;
		rcInfo.top = 0;
		rcInfo.bottom = TV_INFO_HEIGHT;
		InvalidateRect(&rcInfo, FALSE);

		RECT rcImage;
		rcImage.left = 0;
		rcImage.top = 0;
		rcImage.right = _image.GetWidth();
		rcImage.bottom = _image.GetHeight();
		if (!PtInRect(&rcImage, point))
		{
			InvalidateRect(&_rcSelection, FALSE);

			_rcSelection.left = 0;
			_rcSelection.top = 0;
			_rcSelection.right = 0;
			_rcSelection.bottom = 0;

			goto LastHandle;
		}

		_opMode = OP_SELECT;		

		if ((nFlags & MK_CONTROL) && (_rcSelection.right > _rcSelection.left))
		{			
			if (!(PtInRect(&_rcSelection, point) && IsTransparent(point)))
			{
				RECT rcSelection;
				if (PointSelect(rcSelection, point))
				{
					if (_rcSelection.left > rcSelection.left)
						_rcSelection.left = rcSelection.left;
					if (_rcSelection.right < rcSelection.right)
						_rcSelection.right = rcSelection.right;

					if (_rcSelection.top > rcSelection.top)
						_rcSelection.top = rcSelection.top;
					if (_rcSelection.bottom < rcSelection.bottom)
						_rcSelection.bottom = rcSelection.bottom;
				}
			}			
		}
		else if (!PtInRect(&_rcSelection, point))
		{
			PointSelect(_rcSelection, point);			
		}
		InvalidateRect(NULL, FALSE);

		if (_rcSelection.right > _image.GetWidth())
			_rcSelection.right = _image.GetWidth();

		if (_rcSelection.bottom > _image.GetHeight())
			_rcSelection.bottom = _image.GetHeight();
		
LastHandle:
		if (_selectMode == RSM_PARTIAL)
			GetParent()->SendMessage(WM_TV_UPDATEINFO, WPARAM(&point), LPARAM(&_rcSelection));
	}

	//__super::OnLButtonDown(nFlags, point);
}

BOOL CTextureViewer::PointSelect(RECT& rcSelection, const POINT& pt)
{
	if(TRUE)
	{
		CResSelectorDlg *dlg=(CResSelectorDlg *)GetParent();
		if (dlg)
		{
			DWORD w,h;
			dlg->GetUnitSize(w,h);
			if ((w>0)&&(h>0))
			{
				rcSelection.left=pt.x/w*w;
				rcSelection.top=pt.y/h*h;
				rcSelection.right=rcSelection.left+w;
				rcSelection.bottom=rcSelection.top+h;
				return TRUE;
			}
		}
	}
	POINT ptCur;
	POINT ptTest;

	int sz = _image.GetWidth() * _image.GetHeight();
	if (sz <= 0)
		return FALSE;

	std::vector<POINT> ptVec;
	ptVec.push_back(pt);

	char* pVisit = new char[sz];
	if (!pVisit)
		return FALSE;

	memset(pVisit, 0, sz * sizeof(char));

	rcSelection.left = pt.x;
	rcSelection.top = pt.y;
	rcSelection.right = pt.x;
	rcSelection.bottom = pt.y;

	int hy = _image.GetHeight() - 1;

	int index;
	int count = 1;
	int i = 0;
	for (int i = 0; i < count; i++)
	{
		ptCur = ptVec[i];

		// itself
		index = ptCur.x + ptCur.y * _image.GetWidth();
		if ((index > 0 && index < sz) && pVisit[index] == 0)
		{
			if (!_image.AlphaIsValid() || 
				(TV_TRANSPAENT_ALPHA  < _image.AlphaGet(ptCur.x, hy - ptCur.y)))
			{
				if (rcSelection.left > ptCur.x)
					rcSelection.left = ptCur.x;
				else if (rcSelection.right < ptCur.x)
					rcSelection.right = ptCur.x;
				if (rcSelection.top > ptCur.y)
					rcSelection.top = ptCur.y;
				else if (rcSelection.bottom < ptCur.y)
					rcSelection.bottom = ptCur.y;
			}

			pVisit[index] = 1;
		}

		// Left
		ptTest.x = ptCur.x - 1;
		ptTest.y = ptCur.y; 
		if (ptTest.x >= 0)
		{
			index = ptTest.x + ptTest.y * _image.GetWidth();
			if ((index > 0 && index < sz) && pVisit[index] == 0)
			{
				if (!_image.AlphaIsValid() || 
					(TV_TRANSPAENT_ALPHA  < _image.AlphaGet(ptCur.x, hy - ptCur.y)))
				{
					if (rcSelection.left > ptTest.x)
						rcSelection.left = ptTest.x;
					ptVec.push_back(ptTest);
				}

				pVisit[index] = 1;
			}			
		}	

		// Top
		ptTest.x = ptCur.x;
		ptTest.y = ptCur.y - 1;
		if (ptTest.y >= 0)
		{
			index = ptTest.x + ptTest.y * _image.GetWidth();
			if ((index > 0 && index < sz) && pVisit[index] == 0)
			{
				if (!_image.AlphaIsValid() || 
					(TV_TRANSPAENT_ALPHA  < _image.AlphaGet(ptCur.x, hy - ptCur.y)))
				{
					if (rcSelection.top > ptTest.y)
						rcSelection.top = ptTest.y;
					ptVec.push_back(ptTest);
				}

				pVisit[index] = 1;
			}
		}

		// Right
		ptTest.x = ptCur.x + 1;
		ptTest.y = ptCur.y;
		if (ptTest.x <= _image.GetWidth())
		{
			index = ptTest.x + ptTest.y * _image.GetWidth();
			if ((index > 0 && index < sz) && pVisit[index] == 0)
			{
				if (!_image.AlphaIsValid() || 
					(TV_TRANSPAENT_ALPHA  < _image.AlphaGet(ptCur.x, hy - ptCur.y)))
				{
					if (rcSelection.right < ptTest.x)
						rcSelection.right = ptTest.x;
					ptVec.push_back(ptTest);
				}

				pVisit[index] = 1;
			}
		}

		// Bottom
		ptTest.x = ptCur.x;
		ptTest.y = ptCur.y + 1;
		if (ptTest.y <= _image.GetHeight())
		{
			index = ptTest.x + ptTest.y * _image.GetWidth();
			if ((index > 0 && index < sz) && pVisit[index] == 0)
			{
				if (!_image.AlphaIsValid() || 
					(TV_TRANSPAENT_ALPHA  < _image.AlphaGet(ptCur.x, hy - ptCur.y)))
				{
					if (rcSelection.bottom < ptTest.y)
						rcSelection.bottom = ptTest.y;
					ptVec.push_back(ptTest);
				}

				pVisit[index] = 1;
			}
		}

		count = static_cast<int>(ptVec.size());
	}

	delete []pVisit;

	rcSelection.right++;
	rcSelection.bottom++;
	return TRUE;
}

BOOL CTextureViewer::IsTransparent(const POINT& pt)
{
	int dy = _image.GetHeight() - pt.y - 1;
	return (_image.AlphaIsValid() && 
		(TV_TRANSPAENT_ALPHA  >= _image.AlphaGet(pt.x, dy)));
}

void CTextureViewer::DrawInformation(CDC* pDC)
{	
	CString strTemp;
	RECT rcText;
	RECT rcClient;
	GetClientRect(&rcClient);

	CDC dcMem;
	dcMem.CreateCompatibleDC(pDC);
	CBitmap bmpMem;
	bmpMem.CreateCompatibleBitmap(pDC, rcClient.right, TV_INFO_HEIGHT);
	dcMem.SelectObject(&bmpMem);

	dcMem.SelectObject(&_font);
	dcMem.SetBkMode(TRANSPARENT);
	//dcMem.SetTextColor(RGB(255, 0, 0));

	rcText.left = 0;
	rcText.top = 0;
	rcText.right = rcClient.right;
	rcText.bottom = TV_INFO_HEIGHT;
	dcMem.FillSolidRect(&rcText, RGB(20, 200, 20));

	// Mouse position
	POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(&pt);

	pt.x -= _ptRef.x;
	pt.y -= _ptRef.y;
	strTemp.Format(_T("Mouse: x=%d, y=%d"), pt.x, pt.y);

	rcText.left = 0;
	rcText.right = 160;
	dcMem.DrawText(strTemp, &rcText, 0);

	// Selection rect & rect size
	int width = _rcSelection.right - _rcSelection.left;
	int height = _rcSelection.bottom - _rcSelection.top;

	strTemp.Format(_T("Selection: %d,%d,%d,%d %dx%d"), 
		_rcSelection.left, _rcSelection.top, _rcSelection.right, _rcSelection.bottom, 
		width, height);

	rcText.left = 160;	
	rcText.right = rcClient.right;	
	dcMem.DrawText(strTemp, &rcText, DT_RIGHT);

	BLENDFUNCTION bf;
	bf.BlendOp = AC_SRC_OVER;
	bf.SourceConstantAlpha = 200;
	bf.AlphaFormat = 0;
	bf.BlendFlags = 0;
	pDC->AlphaBlend(0, 0, rcClient.right, TV_INFO_HEIGHT, &dcMem, 0, 0, rcClient.right, 
		TV_INFO_HEIGHT, bf);
}

void CTextureViewer::DrawImageEdge(CDC* pDC)
{
	CBrush* oldBrush = pDC->SelectObject(&_brSelection);
	CPen* oldPen = pDC->SelectObject(&_penImage);

	RECT rcClient;
	GetClientRect(&rcClient);

	RECT rc;
	rc.left = _ptRef.x;
	rc.top = _ptRef.y;
	rc.right = _image.GetWidth() + _ptRef.x;
	rc.bottom = _image.GetHeight() + _ptRef.y;

	// Clip
	if (rc.left < 0)
		rc.left = -1;
	if (rc.top < 0)
		rc.top = - 1;
	if (rc.right > rcClient.right)
		rc.right = rcClient.right + 1;
	if (rc.bottom > rcClient.bottom)
		rc.bottom = rcClient.bottom + 1;

	pDC->Rectangle(&rc);

	pDC->SelectObject(oldBrush);
	pDC->SelectObject(oldPen);
}

void CTextureViewer::DrawSelection(CDC* pDC)
{
	CBrush* oldBrush = pDC->SelectObject(&_brSelection);
	CPen* oldPen = pDC->SelectObject(&_penSelection);

	RECT rcClient;
	GetClientRect(&rcClient);

	RECT rc = _rcSelection;
	rc.left += _ptRef.x;
	rc.top += _ptRef.y;
	rc.right += _ptRef.x;
	rc.bottom += _ptRef.y;

	// Clip
	if (rc.left < 0)
		rc.left = -1;
	if (rc.top < 0)
		rc.top = - 1;
	if (rc.right > rcClient.right)
		rc.right = rcClient.right + 1;
	if (rc.bottom > rcClient.bottom)
		rc.bottom = rcClient.bottom + 1;
	
	pDC->Rectangle(&rc);

	pDC->SelectObject(oldBrush);
	pDC->SelectObject(oldPen);
}