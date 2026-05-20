#pragma once

#include "AnimStateDef.h"
#include "ResAnchor.h"
#include "AnimPieceList.h"
#include "AnimPieceRange.h"
#include "AnimCtrlWnd.h"

class GuiLib_Api CUVAnimPanel : public CResEditPanel
{
public:
	CUVAnimPanel(void);
	~CUVAnimPanel(void);
public:
	virtual UINT GetIDD()	{		return IDD_BONEANIM;	}
	virtual void OnResDataChange(ResData *data);

	virtual BOOL StateToControl(ResEditPanelState *state);//Update the controls in the panel to reflect the state
	virtual void Init3d();
	virtual void Clear3d();
	virtual void Draw(IRenderPort *rp);
	 
	virtual ResEditPanelState *_NewState();
protected:

	DECLARE_MESSAGE_MAP()
	afx_msg virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 
	afx_msg virtual BOOL OnInitDialog();
	afx_msg void OnChooseMesh();
	afx_msg void OnChooseMtrl();
	void OnSelect();

protected:

	CAnimPieceList _listAP;
	CXTCaption _animinfo;
	CAnimPieceRange _rangeAP;
	CAnimCtrlWnd	_animCtrlBar;

	CResAnchor _meshSelector;
	CResAnchor _mtlSelector;
	ILight   *_light;
};


