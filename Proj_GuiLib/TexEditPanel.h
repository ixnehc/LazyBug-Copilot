
#pragma once
#include "GuiLib.h"

#include "resource.h"

#include "ResEditPanel.h"

#include "ResAnchor.h"

#include "mod/ModBase.h"

#include "MeshContent.h"

#include "RenderSystem/IRenderSystem.h"

struct MeshData;
class IMesh;


//Reps for ResEditPanelState
struct Reps_Tex:public ResEditPanelState
{
	Reps_Tex()
	{
		Zero();
	}
	void Zero()
	{
	}

};




// CTexEditPanel 对话框
class GuiLib_Api CTexEditPanel : public CResEditPanel
{
// 构造
public:
	CTexEditPanel();

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
	virtual UINT GetIDD()	{		return IDD_TEXPANEL;	}
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
protected:


};

