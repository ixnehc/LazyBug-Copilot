#pragma once

#include "RenderSystem/IRenderSystem.h"
#include "AxisArrow.h"
#include "ResEditPanel.h"
#include "AnimPieceList.h"
#include "AnimPieceRange.h"
#include "AnimCtrlWnd.h"

class GuiLib_Api CTransformAnimPanel :public CResEditPanel
{
public:
	CTransformAnimPanel(void);
	~CTransformAnimPanel(void);
public:
	virtual UINT GetIDD()	{		return IDD_BONEANIM;	}
	virtual void OnResDataChange(ResData *data);

	virtual BOOL StateToControl(ResEditPanelState *state);//Update the controls in the panel to reflect the state

	virtual void Init3d();
	virtual void Clear3d();
	virtual void OnSelect();
	virtual void Draw(IRenderPort *rp);

	ResEditPanelState *_NewState(void) ;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
private:
	void _drawPathLine(IRenderPort * rp,KeySet * keys,DWORD color,DWORD iStart,DWORD iEnd);
	void _showEventLabel(IRenderPort *rp,AnimPiece * aP,IAnimPlayer *IAnimPlayer,KeySet * keys,int offset);
	void _showKeyframe(IRenderPort *rp,AnimPiece * aP,KeySet * keys,IAnimPlayer *IAnimPlayer);
	void _showAnim(IRenderPort *rp,AnimPiece * aP,DWORD wTick,IAnimPlayer *IAnimPlayer);
	//void _drawUnActive(TimeValue tStar,TimeValue tEnd,IRenderPort * rp,IAnimPlayer * IAnimPlayer,KeySet_xform * keys,DWORD color);
private:
	IMesh * _mesh;
	IMtrl * _mtrl;
	IMtrl *_mtrlEvent;
	IMtrlMgr *_mtrlMgr;
	IMeshMgr *_meshMgr;
	ILight * _light;
	IAnimPlayer* _player;
	CAxisArrow  _axisArrow;
	
	CAnimPieceList _listAP;
	CXTCaption _animinfo;
	CAnimPieceRange _rangeAP;
	CAnimCtrlWnd	_animCtrlBar;

	ICamera * _newCamer;

};
