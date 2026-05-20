#pragma once
#include "GuiLib.h"
#include "GuiActor_OverallMap.h"


class GraphicsGraph;
class GuiLib_Api CMapToolDlg : public CDialog
{
public:
	CMapToolDlg(CWnd * pParent= NULL);
	~CMapToolDlg(void);
	BOOL Create(CWnd * pParent,CGuiMgr * mgr);

protected:
	class CMapView :public CWnd
	{
	public:
		CMapView(CMapToolDlg * owner){ _owner = owner;}	
	protected:	
		CMapToolDlg * _owner;
	public:
		DECLARE_MESSAGE_MAP()
		afx_msg BOOL OnEraseBkgnd(CDC* pDC){return TRUE;}
		void OnMouseMove(UINT nFlags, CPoint point){ CWnd::OnMouseMove(nFlags,point); SetFocus();}
		virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	};

	DECLARE_MESSAGE_MAP()
	BOOL OnInitDialog();
	afx_msg void OnPaint();
	LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	
	afx_msg void OnSize(UINT nType, int cx, int cy);

	void _Init();
private:
	CGuiMgr * _mgrGui;
	CGuiActor_OverallMap _actorMap;
	CMapView  _viewDraw;

	BOOL _bInit;
};
