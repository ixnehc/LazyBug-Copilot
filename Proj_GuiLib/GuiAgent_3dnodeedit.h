/********************************************************************
	created:	2008/2/3   12:46
	file path:	d:\IxEngine\Proj_GuiLib
	author:		cxi
	
	purpose:	the general-usage GuiAgents
*********************************************************************/

#pragma once

#include "GuiLib.h"

#include "GuiEditor.h"

#include "GuiAgent_MatrixEdit.h"



typedef unsigned __int64 H3DNode;
#define H3DNode_Invalid 0xffffffffffffffff

struct Envelope;
//目前支持3D Node的选择/绘制/删除
class CGuiAgent_3DNodeOperate:public CGuiAgent
{
public:
	virtual BOOL OnLButtonDown(int x,int y,DWORD flag);
	virtual BOOL OnRButtonDown(int x,int y,DWORD flag);

	virtual BOOL OnRButtonClick(int x,int y,DWORD flag);
	virtual BOOL OnCommand(DWORD idCmd);
	virtual BOOL OnDraw();
	virtual BOOL OnKeyDown(char c,DWORD flag);
protected:

	//注意,返回值必须为一个可以强转为std::vector<H3DNode>*的指针
	virtual  void*_GetSelBuf()=0;
	virtual DWORD *_GetVer()	{		return NULL;	}
	virtual i_math::pos2di *_GetBlock(H3DNode node)	{		return NULL;	}

	virtual H3DNode _HitTest(i_math::line3df &ray)	{		return H3DNode_Invalid;	}

	virtual BOOL _NeedBackup()	{		return TRUE;	}

	virtual BOOL _NeedRemove()	{		return TRUE;	}
	virtual BOOL _Remove(H3DNode node)	{		return FALSE;	}

	virtual BOOL _NeedClone()	{		return FALSE;	}
	virtual H3DNode _Clone(H3DNode node)	{		return H3DNode_Invalid;	}

	virtual void _CollectEnvelope(H3DNode *node,DWORD nNodes,Envelope &evlp)	{	}

	BOOL CGuiAgent_3DNodeOperate::_Select(int x,int y,DWORD flag,int mode);

	void _DrawEnvelope(Envelope &evlp);
	
	void _IncVer()
	{
		DWORD *p=_GetVer();
		if (p)
			(*p)++;
	}
};

class CGuiAgent_3DNodeMatEdit:public CGuiAgent_MatrixEdit
{
public:
	CGuiAgent_3DNodeMatEdit(Matrix_EditMode mode = EditMode_All);
	virtual void UpdateBind();
protected:
	//注意,返回值必须为一个可以强转为std::vector<H3DNode>*的指针
	virtual  void*_GetSelBuf()=0;
	virtual DWORD *_GetVer()	{		return NULL;	}
	virtual i_math::matrix43f *_GetMat(H3DNode node)=0;
	virtual i_math::matrix43f *_GetLocalMat(H3DNode node,i_math::matrix43f &matBase)	{		return NULL;	}
	virtual i_math::pos2di *_GetBlock(H3DNode node)=0;//返回这个node位于那个block内

	virtual void _Move(H3DNode &node,i_math::matrix43f &mat)=0;
	virtual void _MoveLocal(H3DNode &node,i_math::matrix43f &mat)	{	}

	virtual BOOL _NeedBackup()	{		return TRUE;	}

	void _IncVer()
	{
		DWORD *p=_GetVer();
		if (p)
			(*p)++;
	}

	virtual BOOL OnRButtonClick(int x,int y,DWORD flag);
	virtual BOOL OnCommand(DWORD idCmd);	


	virtual void _BeginMatrixEdit(i_math::matrix43f *mat);
	virtual void _MatrixEdit(i_math::matrix43f *mat);
	virtual void _EndMatrixEdit(i_math::matrix43f *mat);
	std::vector<H3DNode> _editings;//正在编辑的Node
	std::vector<H3DNode> _backups;//备份的selections
	BOOL _bEditing;
	i_math::matrix43f _matWork;
	BOOL _bParentMat;
	std::vector<i_math::matrix43f> _matsLocal;
	std::vector<i_math::pos2di> _affected;

};