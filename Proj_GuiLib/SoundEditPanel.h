
#pragma once
#include "GuiLib.h"

#include "resource.h"
#include "RenderSystem/IMesh.h"


#include "ResEditPanel.h"

#include "ResAnchor.h"

#include "MtrlGrid.h"

#include "ValueSetDialog.h"

#include "mod/ModBase.h"

#include "RenderSystem/IRenderSystem.h"
struct MtrlData;
class IMtrl;


//Reps for ResEditPanelState
struct Reps_Sound:public ResEditPanelState
{
	Reps_Sound()
	{
		Zero();
	}
	void Zero()
	{
	}

};

// CSoundEditPanel 对话框
class GuiLib_Api CSoundEditPanel : public CResEditPanel
{
// 构造
public:
	CSoundEditPanel();
	virtual void OnResDataChange(ResData *data);
	virtual void Draw(IRenderPort *rp);

	virtual void Init3d();
	virtual void Clear3d();
	virtual void OnSelect();

	virtual BOOL StateToControl(ResEditPanelState *state);//Update the controls in the panel to reflect the state
	virtual BOOL StateToFile(ResEditPanelState *state);//Save all the need-saving part of the state(generally it's the resdata the panel is editing)
protected:
	virtual ResEditPanelState *_NewState();



protected:
	virtual UINT GetIDD()	{		return IDD_SOUNDPANEL;	}
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


	// 实现
protected:
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPlay();
protected:

};


