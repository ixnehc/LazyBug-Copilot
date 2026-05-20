
#pragma once
#include "GuiLib.h"
#include "RenderSystem/IRenderSystem.h"
#include "engine/D3DView.h"
#include "ArcBall.h"
#include "WorldSystem/ISpt.h"

class GuiLib_Api CSptViewPanel :public CD3DView<CWnd>
{
	enum  CameraState
	{
		Camera_Still,
		Camera_Move,
		Camera_Rotate,
	};

public:
	CSptViewPanel();
	~CSptViewPanel();
public:
	virtual void OnDraw(IRenderPort *rp);
	void SetSpt(ISpt * pSpt);
	
	virtual void OnInit3D();
	virtual void OnClear3D();

protected:
	
	afx_msg  void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg  void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg  void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg  void OnRButtonUp(UINT nFlags, CPoint point);
	 
	afx_msg  void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg  BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg  void OnCaptureChanged(CWnd* pWnd);
	
	DECLARE_MESSAGE_MAP()
private:
	ISptDrawer * _sptDrawer;
	ISpt  * _pSpt;
	CCameraController _ccler;
	CameraState _cameraState;
};


