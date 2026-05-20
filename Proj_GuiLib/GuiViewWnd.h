#pragma once

#include "GuiEditor.h"

class CGuiView;
template <typename T_Base>
class CGuiViewWnd: public T_Base
{
public: // create from serialization only
	CGuiViewWnd()
	{
		_view=NULL;
	}

	void SetView(CGuiView *view)
	{
		_view=view;
		_RecalcLayout();
		if (_view)
			_view->Invalidate();
	}

protected:
	virtual void _RecalcLayout()
	{
		if (_view)
		{
			i_math::recti rcView;
			T_Base::GetClientRect((CRect*)&rcView);
			_view->SetWnd(this,rcView);
		}
	}

	//embedded
	CGuiView *_view;


// Generated message map functions
protected:
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
	{
		LRESULT ret;
		if (message==WM_SIZE)
		{
			ret=T_Base::WindowProc(message,wParam,lParam);
			_RecalcLayout();
			if (_view)
			{
				_view->Draw();
				//		_view->Invalidate();
			}
			return ret;
		}

		if (WM_PAINT==message)
		{
			CPaintDC dc(this); // device context for painting

			if (_view)
				_view->Invalidate();

			return 0;
		}
		if (WM_ERASEBKGND==message)
		{
			if (_view)
				return 1;
			return T_Base::WindowProc(message,wParam,lParam);
		}

		if (_view)
		{
			CPoint ptCursor;
			GetCursorPos(&ptCursor);
			ScreenToClient(&ptCursor);

			LRESULT ret=0;
			if (TRUE==_view->RespondMsg(ptCursor,message,wParam,lParam,ret))
				return ret;
		}

		return T_Base::WindowProc(message, wParam, lParam);
	}
};


typedef CGuiViewWnd<CView> CGuiView_View;