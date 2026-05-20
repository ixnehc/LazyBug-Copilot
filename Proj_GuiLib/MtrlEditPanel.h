
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
struct Reps_Mtrl:public ResEditPanelState
{
	Reps_Mtrl()
	{
		Zero();
	}
	virtual ~Reps_Mtrl()
	{
	}
	void Zero()
	{
		stateRG.clear();
	}
	virtual void CleanAndDelete();
	virtual void Copy(ResEditPanelState &src);

	RGState stateRG;
};

struct SampleMeshInfo
{
	SampleMeshInfo()
	{
		mesh=NULL;
	}
	SampleMeshInfo(const char *path0,const char *showname0)
	{
		path=path0;
		showname=showname0;
		mesh=NULL;
	}
	~SampleMeshInfo()
	{
		SAFE_RELEASE(mesh);
	}

	std::string path;
	std::string showname;
	IMesh *mesh;

	SampleMeshInfo& operator=(const SampleMeshInfo&src)
	{
		path=src.path;
		showname=src.showname;
		mesh=src.mesh;
		if (mesh)
			mesh->AddRef();
		return *this;
	}
};



// CMtrlEditPanel 对话框
class GuiLib_Api CMtrlEditPanel : public CResEditPanel
{
// 构造
public:
	CMtrlEditPanel();

	virtual void OnResDataChange(ResData *data);
	virtual void Draw(IRenderPort *rp);

	virtual void Init3d();
	virtual void Clear3d();
	virtual void OnSelect();
	virtual BOOL StateToControl(ResEditPanelState *state);//Update the controls in the panel to reflect the state
	virtual void UpdateUI();

protected:
	virtual ResEditPanelState *_NewState();

	virtual BOOL _SupportUndo()	{		return TRUE;	}
protected:
	virtual UINT GetIDD()	{		return IDD_MTRLMAIN;	}
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

	afx_msg void OnSampleSelChange();
	afx_msg void OnSampleClick();
	afx_msg void OnLod1();
	afx_msg void OnLod2();
	afx_msg void OnLod3();
	afx_msg void OnLod4();
protected:

	void _RecalcLayout();
	DWORD _curSample;
	std::vector<SampleMeshInfo>_samples;
	IMtrl *_mtrl;
	ILight *_lgts[2];

	CMtrlGrid _gridMtrl;
	CValueSetDialog _vsdlg;

	CResAnchor _sampleanchor;

	int _iLod;


public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
};


