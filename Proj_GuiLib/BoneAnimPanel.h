
#pragma once
#include "GuiLib.h"

#include "resource.h"
#include "afxwin.h"

#include "ResAnchor.h"
#include "ResEditPanel.h"

#include "BoneAnimPieceList.h"
#include "BoneAnimPieceRange.h"
#include "BoneAnimCtrlWnd.h"
#include "SKeletonModel.h"
#include "../Interfaces/RenderSystem/IRenderSystem.h"
#include "AnimStateDef.h"

class IBoneAnim;
class ILight;
class IMatrice43;
// CBoneAnimPanel 对话框
class GuiLib_Api CBoneAnimPanel : public CResEditPanel
{
// 构造
public:
	CBoneAnimPanel();

	virtual UINT GetIDD()	{		return IDD_BONEANIM;	}
	virtual void OnResDataChange(ResData *data);

	virtual BOOL StateToControl(ResEditPanelState *state);//Update the controls in the panel to reflect the state
	virtual BOOL StateToFile(ResEditPanelState *state)	{		return TRUE;}//永远不存盘	

	virtual void Init3d();
	virtual void Clear3d();
	virtual void OnSelect();
	virtual void Draw(IRenderPort *rp);
	
	protected:

	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	void _DrawSkeleton(IRenderPort *rp,i_math::matrix43f * key,DWORD *col = NULL);
// 实现
protected:
	virtual ResEditPanelState *_NewState();

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
protected:

	//Controls
	CBoneAnimPieceList _listAP;
	CXTCaption _animinfo;

	CBoneAnimPieceRange _rangeAP;
	//
	CBoneAnimCtrlWnd	_animCtrlBar;
	IMatrice43 *_matrice;
	ILight   *_light;
	IBoneAnim	*_anim ,*_animStd;
	std::string _pathAnim,_pathAnimStd;
	//States
	Reps_Anim _state;
	CSkeletonModel _skeletonModel;
};

