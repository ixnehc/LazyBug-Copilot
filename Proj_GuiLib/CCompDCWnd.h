#pragma once

template <class T>
class CCompDCWnd:public T
{
public:
	CCompDCWnd()
	{
	}
public:

	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
			case WM_PAINT:
			{
				CPaintDC dc(this);
				CRect rc;
				GetClientRect(&rc);
				if (_bmpCompDC.GetSafeHandle()==NULL)
				{
					CDC *pDC=GetDC();
					_bmpCompDC.CreateCompatibleBitmap(pDC,((CRect&)rc).Width(),((CRect&)rc).Height());
					ReleaseDC(pDC);
				}

				CDC dcCompatible;
				CBitmap *pBmpOld;
				dcCompatible.CreateCompatibleDC(&dc);
				pBmpOld=dcCompatible.SelectObject(&_bmpCompDC);

				Draw(&dcCompatible);

				dc.BitBlt(0,0,rc.Width(),rc.Height(),&dcCompatible,0,0,SRCCOPY);
				dcCompatible.SelectObject(pBmpOld);
				return TRUE;
			}
		}

		return T::WindowProc(message, wParam, lParam);
	}

	virtual void Draw(CDC *pDC)	{	}


protected:
	CBitmap _bmpCompDC;

};


