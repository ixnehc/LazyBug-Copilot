#pragma once

#include "GuiLib.h"
#include "GuiData_Pathes.h"

#include "FileSystem/IMapFileDefines.h"
#include "WorldSystem/IEntitySystemDefines.h"

#include "GuiEditor.h"

#include "ResTree.h"

#include "resdata/AnimData.h"

#include "spline/CubicSpline.h"

#include "GuiAgent_MatrixEdit.h"

#include "GObjGrid.h"

class CMod_ReplacePath:public CModBase
{
public:
	CMod_ReplacePath()
	{
		_dataPathes=NULL;
	}
	virtual BOOL TestUndo()	{		return TRUE;	}
	virtual BOOL TestRedo()	{		return TRUE;	}

	virtual BOOL Undo()
	{
		return Redo();
	}
	virtual BOOL Redo();
	virtual BOOL IsEmpty()	{		return _dataPathes?TRUE:FALSE;	}

protected:

	XFormData _backup;
	std::string _pathRes;

	GuiData_Pathes *_dataPathes;

	friend class CGuiPanel_Pathes;

};


//绘制path上各个节点的Agent
class CGuiAgent_DrawPath:public CGuiAgent
{
public:
	CGuiAgent_DrawPath()
	{
		_meshPlane=NULL;
		_mtrlPlane=NULL;
		_mtrlPlane2=NULL;
	}
	virtual BOOL OnDraw();

	virtual void OnAttachView(CGeView *view,DWORD iLevel);
	virtual void OnDetachView(CGeView *view,DWORD iLevel);

protected:
	IMesh *_meshPlane;
	IMtrl *_mtrlPlane;
	IMtrl *_mtrlPlane2;
	ILight *_lgt;

	CCubicSpline _spln;
	std::vector<i_math::vector3df> _lines;
};

class CGuiAgent_RunPath:public CGuiAgent
{
public:
	CGuiAgent_RunPath()
	{
		_player=NULL;
		_tickStart=0;
		_tOff=0;
	}

	virtual void OnAttachView(CGeView *view,DWORD iLevel);
	virtual void OnDetachView(CGeView *view,DWORD iLevel);

	virtual BOOL Respond(CtrlOp &co);//return whether the other agents need further processing

	virtual BOOL OnRButtonClick(int x,int y,DWORD flag);
	virtual BOOL OnCommand(DWORD idCmd);
	virtual BOOL OnKeyDown(char c,DWORD flag);


	virtual BOOL OnTimer(int dt,DWORD flag);

protected:
	ICamera *_cam;//用来备份
	IAnimPlayer *_player;//这个指针非空,表示在巡航模式下
	DWORD _tickStart;
	AnimTick _tOff;
	i_math::vector3df _eye;
};

class CGuiAgent_LocateCP:public CGuiAgent
{
public:
	virtual BOOL OnDraw();

protected:
	virtual BOOL Respond(CtrlOp &co);//return whether the other agents need further processing

	virtual BOOL OnMouseMove(int x,int y,DWORD flag);
	virtual BOOL OnMouseWheel(int delta,DWORD flag);
	virtual BOOL OnLButtonDown(int x,int y,DWORD flag);
	virtual BOOL OnRButtonClick(int x,int y,DWORD flag);

protected:
	void _UpdateCP(int x,int y);
};

class CGuiAgent_OperateCP:public CGuiAgent
{
public:
protected:
	virtual BOOL OnRButtonClick(int x,int y,DWORD flag);
	virtual BOOL OnCommand(DWORD idCmd);

	virtual BOOL OnKeyDown(char c,DWORD flag);

};

class CGuiAgent_SelCP:public CGuiAgent
{
public:
protected:
	virtual BOOL OnLButtonDown(int x,int y,DWORD flag);


};

class CGuiAgent_CPMatrixEdit:public CGuiAgent_MatrixEdit
{
public:
	CGuiAgent_CPMatrixEdit();
	void UpdateBind();
	virtual BOOL OnRButtonClick(int x,int y,DWORD flag);
	virtual BOOL OnCommand(DWORD idCmd);
protected:
	void _BeginMatrixEdit(i_math::matrix43f *mat);
	void _MatrixEdit(i_math::matrix43f *mat);
	void _EndMatrixEdit(i_math::matrix43f *mat);
	i_math::matrix43f _matWork;

	BOOL _bChanging;
};


class CPathesTree:public CResTree
{

};

struct CPInfo
{
	BOOL bVelocityAlign;
	float vel;

	BEGIN_GOBJ_PURE(CPInfo,1);
		GELEM_VAR_INIT(BOOL,bVelocityAlign,0);
			GELEM_EDITVAR("旋转依赖于速度",GVT_S,GSem_Boolean,"旋转依赖于速度");
		GELEM_VAR_INIT(float,vel,-1.0f);
			GELEM_EDITVAR("速率",GVT_F,GSem(GSem_Float,"0.1f,100.0f,0.05f"),"控制点附近的移动速率");
	END_GOBJ();    

};

class CCPPage:public CGObjGrid
{
public:
	CCPPage()
	{
		_bModified=FALSE;
	}

	void Bind(XFormData::CtrlPoint *cp);
	void Fill(XFormData::CtrlPoint &cp);
	virtual void OnEndItemChange(CXTPPropertyGridItem *item);
	void ClearModified()	{		_bModified=FALSE;	}
	BOOL IsModified()	{		return _bModified;	};

protected:
	CPInfo _info;
	BOOL _bModified;

};


class IAssetClassLib;
class GuiLib_Api CGuiPanel_Pathes:public CGuiPanel
{
public:
	CGuiPanel_Pathes(CWnd* pParent = NULL);
	virtual const char *GetName()	{		return "pathes";	}

	BOOL Create(CWnd *pParent);

	virtual void Reset();
	virtual void UpdateUI();

	virtual void OnDetachView(CGeView *view,DWORD iLevel);

	virtual void OnEnterActivity();
	virtual void OnLeaveActivity();


protected:
	virtual const char *_GetModMgrName()	{		return "world";	}

	void _OccupyActor();

	void _SetRootPath(const char *path);

	void _SaveModified();
	void _UpdateCPPage();
	void _UpdateTreeSel();
	void _UpdateDesc();

	CGuiAgent_CPMatrixEdit *_matedit;

	CPathesTree _tree;

	CCPPage _page;

	BOOL _bInUpdateTreeSel;

public:
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();

	afx_msg void OnTvnSelchangedTree(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBrowse();
};
