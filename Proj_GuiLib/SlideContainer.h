#include "GuiLib.h"
#include <map>
#include "math/imath_all.h"

LRESULT CALLBACK MyWndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);

typedef  LRESULT ( CALLBACK *FUN_PTR)(HWND,UINT,WPARAM,LPARAM);
class GuiLib_Api CSlideContainer :public CWnd
{
	struct ControlInfo
	{
		CRect  rc;
		LONG_PTR  wndProc;
	};
	
public:
	CSlideContainer()
	{
		_curPosX=0,_curPosY=0;
		_pControl=NULL;
		_flagSize=FALSE;
	}
	~CSlideContainer()
	{
	}
	enum 
	{
		State_Scrolling=100,
		State_EndScroll,
	};
	enum
	{
		Scroll_Horizon,
		Scroll_Vertical,
		//Scroll_Both,
	};
	enum
	{	
		ScrollBar_Width=5,
	};
public :
	BOOL SetControl(CWnd * pControl,CRect &rc);
	CWnd *GetControl()	{		return _pControl;	}
	virtual BOOL Create(RECT & rc,CWnd *pParent,UINT templateId,DWORD flag=Scroll_Vertical,
						DWORD wsStyle=WS_CHILD|WS_VISIBLE|WS_BORDER);
	

protected:
	
	DECLARE_MESSAGE_MAP();

	ControlInfo _controlInfo;
	CWnd  * _pControl;	
	DWORD _w ,_h;
	DWORD _curPosX,_curPosY;
	DWORD  _state;
	HCURSOR  _cursor;
	CPoint  _point;
	DWORD  _flag;
	BOOL   _flagSize;

	virtual void afx_msg _OnRecalculate(int offsetX,int offsetY);
	virtual void afx_msg _OnScrolling();
private:
	void _SetCursor();
	friend 	LRESULT CALLBACK MyWndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);
	void _DrawScrollBar();
	void _EraseBlank();
	void _OnWheeling(short zDelta);
	void _OffsetRect(CRect &distSrc,const CRect &src,int offsetX,int offsetY);
public:
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnCaptureChanged(CWnd *pWnd);
	afx_msg void OnSize(UINT nType, int cx, int cy);
};

