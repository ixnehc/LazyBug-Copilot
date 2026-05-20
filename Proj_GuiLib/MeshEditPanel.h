
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
struct Reps_Mesh:public ResEditPanelState
{
	Reps_Mesh()
	{
		Zero();
	}
	void Zero()
	{
		iSelFrame=0;
		iLod = 0;
	}

	int iSelFrame;
	int iLod;
	virtual void CleanAndDelete();
	virtual void Copy(ResEditPanelState &src);
	virtual void SetData(ResData *data);

};




// CMeshEditPanel 对话框
class GuiLib_Api CMeshEditPanel : public CResEditPanel
{
// 构造
public:
	CMeshEditPanel();

	virtual void OnResDataChange(ResData *data);
	virtual void Draw(IRenderPort *rp);

	virtual void Init3d();
	virtual void Clear3d();
	virtual void OnSelect();

	virtual BOOL StateToControl(ResEditPanelState *state);//Update the controls in the panel to reflect the state
protected:
	virtual ResEditPanelState *_NewState();



protected:
	virtual UINT GetIDD()	{		return IDD_MESHMAIN;	}
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
protected:

	CMeshContent _meshcontent;

	ILight *_lgt;


};

