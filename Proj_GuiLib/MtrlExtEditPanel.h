
#pragma once
#include "GuiLib.h"

#include "resource.h"

#include "ResEditPanel.h"

#include "MtrlEditPanel.h"

#include "ResAnchor.h"

#include "MtrlGrid.h"

#include "ScintillaView.h"

#include "mod/ModBase.h"

struct MtrlExtData;
class IMtrlExt;


//Reps for ResEditPanelState
struct Reps_MtrlExt:public ResEditPanelState
{
	Reps_MtrlExt()
	{
		Zero();
	}
	virtual ~Reps_MtrlExt()
	{
	}
	void Zero()
	{
	}
	virtual void CleanAndDelete();
	virtual void Copy(ResEditPanelState &src);

	RGState stateRG;

};

class CMteGrid:public CRichGrid,public CResEditCtrl
{
public:
	CMteGrid()
	{
	}

	virtual BOOL Create(const RECT& rect, CWnd* pParentWnd, UINT nID, DWORD dwListStyle = LBS_OWNERDRAWFIXED| LBS_NOINTEGRALHEIGHT);
	virtual void Bind(ResEditPanelState *state,BOOL bUpdateCtrl);
	Reps_MtrlExt *GetState()	{		return (Reps_MtrlExt*)_state;	}

	virtual void OnBeginItemChange(CXTPPropertyGridItem *item);
	virtual void OnItemChange(CXTPPropertyGridItem *item);
	virtual void OnEndItemChange(CXTPPropertyGridItem *item);

	virtual void EnableCtrl(BOOL bActive=TRUE);

protected:
	DECLARE_MESSAGE_MAP()

	virtual MtrlExtData *_GetMtrlExtData()	{		return (MtrlExtData*)_GetResData();	}
	virtual RGState &_GetRGState();
	virtual const char *_GetGridName()	{		return "Material Extension Data";	}
};


class CMtrlGrid_Mte:public CMtrlGrid
{
public:
protected:
	virtual MtrlData *_GetMtrlData();
	virtual RGState &_GetRGState()	{		return ((Reps_MtrlExt*)_state)->stateRG;	}
	virtual const char *_GetGridName()	{		return "测试材质数据";	}
	virtual BOOL _NeedUniFeature()	{		return FALSE;}
	virtual BOOL _NeedMtrlExt()	{		return FALSE;}
	virtual BOOL _NeedRefPath()	{		return FALSE;}

};

class CMtrlExtEditPanel;
class CMtrlExtEditor:public CScintillaView
{
public:
	CMtrlExtEditor()
	{
		_owner=NULL;
	}

	virtual void _OnModified();
protected:
	CMtrlExtEditPanel *_owner;


	friend class CMtrlExtEditPanel;
};


// CMtrlExtEditPanel 对话框
class GuiLib_Api CMtrlExtEditPanel : public CResEditPanel
{
// 构造
public:
	CMtrlExtEditPanel();

	virtual void OnResDataChange(ResData *data);
	virtual void Draw(IRenderPort *rp);

	virtual void Init3d();
	virtual void Clear3d();
	virtual void OnSelect();
	virtual BOOL StateToControl(ResEditPanelState *state);//Update the controls in the panel to reflect the state
	virtual BOOL StateToFile(ResEditPanelState *state) override;//Save all the need-saving part of the state(generally it's the resdata the panel is editing)
	virtual void UpdateUI();


	void OnSrcModified();
protected:
	virtual ResEditPanelState *_NewState();

	void _Compile(BOOL bCheckErr);//如果bCheckErr为TRUE,我们只会检查错误,不会修改data
	void _ParseEPs();
	void _UpdateEffectParamFormat(CScintillaWnd &wnd);

	const char *_GetTemplate();

	BOOL _bRepaired;

	CMtrlExtEditor *_editor;
	CMtrlExtEditor *_editor2;


	CMtrlGrid_Mte _gridMtrl;
	CMteGrid _gridMte;

	DWORD _curSample;
	std::vector<SampleMeshInfo>_samples;
	ILight *_lgts[2];

	CResAnchor _sampleanchor;

	std::vector<FeatureParamA> _fpsCache;

	BOOL _bError;//最近一次是否绘制成功
	std::string _sFX;
	std::string _err;
	int _iErrLine;

	BOOL _bShowError;
	BOOL _bShowEdit2;

	i_math::recti _rcEdit;//不显示error string时的大小
	i_math::recti _rcEdit2;//显示error string时的大小


protected:
	virtual UINT GetIDD()	{		return IDD_MTRLEXTMAIN;	}
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	virtual BOOL PreTranslateMessage(MSG* pMsg);


// 实现
protected:
	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSampleSelChange();
	afx_msg void OnSampleClick();
	afx_msg void OnBrowse();
	afx_msg void OnApply();
	afx_msg void OnEditEP();
	afx_msg void OnLocateErr();

protected:

	void _RecalcLayout();

public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
};


