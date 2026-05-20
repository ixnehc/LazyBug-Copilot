#pragma once
#include "matrixedit_base.h"
#include "GuiEditor.h"
#include "fastdelegate/FastDelegate.h"
#include "GuiAgent_MatrixRot.h"
#include "GuiAgent_MatrixMove.h"
#include "GuiAgent_MatrixSimpleScale.h"



class CTransformInputDlg;
class CGuiAgent_MatrixEdit :public CGuiAgent , public CMatrixEditBase 
{
public:
	typedef fastdelegate::FastDelegate1<i_math::matrix43f *> EventEdit;

	CGuiAgent_MatrixEdit(DWORD flag); //indicate EditMode supports 
	~CGuiAgent_MatrixEdit(void);

	void SetEventListener(EventEdit e0,EventEdit e1,EventEdit e3);
	BOOL Bind(MatrixEditData &data);
	i_math::matrix43f *GetBindMat()	{		return _data.matrix;	}
	i_math::matrix43f *GetBindParentMat()	{		return &_data.matParent;	}
	Matrix_EditMode GetEditMode();
	virtual BOOL IsSelected();
	void SetWorkable(Matrix_EditMode flag,BOOL bWorkable);
	void ShowSpaceMenu(BOOL bShow)	{		_bShowSpaceMenu=bShow;	}
	void ShowMoveToCamera(BOOL bShow)	{		_bShowMoveToCamera=bShow;	}
	void ShowResetPRS(BOOL bShow)	{		_bShowResetPRS=bShow;	}
	virtual void Enable(BOOL bEnable);//Added by Chenxi

	template <class T >
	void SetCallBack(T *pThis,
					void(T:: *fnBegin)(i_math::matrix43f *),
					void(T:: *fnOn)(i_math::matrix43f *),
					void(T:: *fnEnd)(i_math::matrix43f *))
	{
		if(fnBegin)
			_funPreEdit.bind(pThis,fnBegin);
		if(fnOn)
			_funOnEdit.bind(pThis,fnOn);
		if(fnEnd)
			_funEndEdit.bind(pThis,fnEnd);
	}

protected:

	virtual BOOL OnRButtonClick(int x,int y,DWORD flag);
	virtual BOOL PreEditCallBack(CMatrixEditBase * editAgent);
	virtual BOOL OnEditCallBack(CMatrixEditBase * editAgent);
	virtual BOOL EndEditCallBack(CMatrixEditBase * editAgent);
	virtual BOOL OnCommand(DWORD idCmd);	
	virtual void OnAttachView(CGeView *view,DWORD iLevel);
	virtual const char * getClassName() {return "CGuiAgent_MatrixEdit";}
	virtual BOOL OnKeyDown(char c,DWORD flag);
	
protected:
	void _AddAgent();
	void _InstanceAgent();
	void _DoInputDlg();
protected:
	BOOL _bShowSpaceMenu;//是否选择space的菜单
	BOOL _bShowMoveToCamera;//是否显示[移至camera位置]的菜单选项
	BOOL _bShowResetPRS;//是否显示[归零]的菜单选项
	Matrix_EditMode _mode;
	Matrix_EditSpace _space;
	EventEdit _funPreEdit;
	EventEdit  _funOnEdit;
	EventEdit _funEndEdit;

	CGuiAgent_MatrixRot * _pAgentRot;
	CAgent_MatrixMove  *  _pAgentMove;
	CAgent_MatrixSimpleScale *  _pAgentScale;
private:
	DWORD _modeSupports;
};

//////////////////////////////////////////////////////////////////////////
/*
	向量 在任何空间上的的平移,旋转或缩放都是等价的 ,平移旋转缩放相对于向量
	向量在不同空间上的表现形式不同
	计算某个空间上的 平移 缩放 旋转 首先要得到 变换相对的向量 在该空间的表示形式
*/


