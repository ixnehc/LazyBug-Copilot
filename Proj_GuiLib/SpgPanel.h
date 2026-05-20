#pragma once

#include "ResAnchor.h"

struct SpeedGlassPanelSate :public ResEditPanelState
{
	SpeedGlassPanelSate()
	{
	}
	virtual void Copy(ResEditPanelState &src)
	{
		ResEditPanelState::Copy(src);
	}
};


class ISpg;
class IMtrl;
class GuiLib_Api CSpgPanel :public CResEditPanel
{
public:
	CSpgPanel(void);
	~CSpgPanel(void);
public:
	virtual UINT GetIDD()	{		return IDD_RESPANEL_SPG;	}
	virtual void OnResDataChange(ResData *data);

	virtual BOOL StateToControl(ResEditPanelState *state);//Update the controls in the panel to reflect the state

	virtual void Init3d();
	virtual void Clear3d();
	virtual void Draw(IRenderPort *rp);
	
	virtual void OnSelect();

	virtual ResEditPanelState *_NewState(){return new SpeedGlassPanelSate();}

protected:

	DECLARE_MESSAGE_MAP()
	afx_msg virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 
	afx_msg virtual BOOL OnInitDialog();

protected:

	IMtrl  * _mtl;
	ISpg * _pSpg;

};


