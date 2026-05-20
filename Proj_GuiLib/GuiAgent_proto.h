
#pragma once

#include "GuiLib.h"

#include "GuiEditor.h"

#include "WorldSystem/IEntitySystemDefines.h"
#include "protograph.h"

#include "GuiAgent_2DTransform.h"

#include "GuiAgent_3dnodeedit.h"

class CGuiAgent_PNMatrixEdit:public CGuiAgent_3DNodeMatEdit
{
public:
	virtual BOOL Respond(CtrlOp &co);
	virtual BOOL OnDraw();
protected:
	virtual BOOL OnCommand(DWORD idCmd);	

	virtual void _BeginMatrixEdit(i_math::matrix43f *mat);
	virtual void _EndMatrixEdit(i_math::matrix43f *mat);

	virtual  void*_GetSelBuf();
	virtual i_math::matrix43f *_GetMat(H3DNode node);
	virtual i_math::matrix43f *_GetLocalMat(H3DNode node,i_math::matrix43f &matParent);
	virtual i_math::pos2di *_GetBlock(H3DNode node)	{		return NULL;	}

	virtual void _Move(H3DNode &node,i_math::matrix43f &mat);
	virtual void _MoveLocal(H3DNode &node,i_math::matrix43f &mat);

	virtual BOOL _NeedBackup()	{		return FALSE;	}

	i_math::matrix43f _matTemp;

};


class CGuiAgent_OperatePN:public CGuiAgent_3DNodeOperate
{
public:
protected:
	virtual BOOL OnRButtonClick(int x,int y,DWORD flag);
	virtual BOOL OnCommand(DWORD idCmd);
	virtual BOOL OnDraw();

	virtual  void*_GetSelBuf();
	virtual i_math::pos2di *_GetBlock(H3DNode node)
	{
		return NULL;
	}
	virtual H3DNode _HitTest(i_math::line3df &ray);
	virtual BOOL _NeedBackup()	{		return FALSE;	}
	virtual BOOL _NeedRemove()	{		return FALSE;	}
	virtual BOOL _Remove(H3DNode node);
	virtual void _CollectEnvelope(H3DNode *nodes,DWORD nNodes,Envelope &evlp);

	BOOL _CanOp();

};


class CGuiAgent_TestProto:public CGuiAgent
{
public:
	CGuiAgent_TestProto()
	{
	}
	~CGuiAgent_TestProto()
	{
	}


	virtual BOOL Respond(CtrlOp &op);
	virtual BOOL OnRButtonClick(int x,int y,DWORD flag);
	virtual BOOL OnCommand(DWORD idCmd);
	virtual void OnAttachView(CGeView *view,DWORD iLevel);
	virtual void OnDetachView(CGeView *view,DWORD iLevel);
	virtual BOOL OnKeyDown(char c,DWORD flag);

	virtual BOOL OnDraw();

	void Run();
	void Stop();


	void BrowseGlobal(BOOL bGE);

protected:

};

#define ProtoThumbnailWidth 96
#define ProtoThumbnailHeight 96

class CGuiAgent_ThumbnailMake:public CGuiAgent
{
public:
	CGuiAgent_ThumbnailMake()
	{
		_bDumping=FALSE;
	}
	~CGuiAgent_ThumbnailMake()
	{
	}


	virtual BOOL OnRButtonClick(int x,int y,DWORD flag);
	virtual BOOL OnCommand(DWORD idCmd);
	virtual void OnAttachView(CGeView *view,DWORD iLevel);
	virtual void OnDetachView(CGeView *view,DWORD iLevel);
	virtual BOOL OnTimer(int dt,DWORD flag);

protected:
	const char *_GetThumbnailPath();

	std::string _pathThumbnailTemp;

	BOOL _bDumping;


};



class CGuiAgent_ViewTimeCtrl:public CGuiAgent_Dragger<TRUE,0>
{
public:
	CGuiAgent_ViewTimeCtrl()
	{
		_bLoop=FALSE;
	}
	~CGuiAgent_ViewTimeCtrl()
	{
	}

	virtual BOOL OnBeginDrag(int x,int y,DWORD flag);
	virtual void OnEndDrag(int x,int y,DWORD flag);
	virtual void OnDrag(int x,int y,DWORD flag);
	virtual BOOL OnDraw();
	virtual BOOL OnLButtonDblClk(int x,int y,DWORD flag);
	virtual BOOL OnTimer(int dt,DWORD flag);

protected:
	float _TimeFromX(int x);
	void _UpdateViewTime(int x,int y);
	BOOL _bLoop;
	DWORD _tLoopStart;
	float _durLoop;


};



//用于proto logic 的滚屏与缩放

class CGuiAgent_GraphScroll:public CGuiAgent_2DTransform
{
public:
	virtual void OnUpdateTransform(const i_math::pos2df & pos,const i_math::pos2df &scale);

protected:
};

class CGuiAgent_GraphNodeSel:public CGuiAgent_Dragger<TRUE,0>
{
public:
	virtual BOOL OnBeginDrag(int x,int y,DWORD flag);
	virtual void OnEndDrag(int x,int y,DWORD flag);
	virtual void OnDrag(int x,int y,DWORD flag);

	virtual BOOL OnTimer(int dt,DWORD flag);

	virtual BOOL OnRButtonDown(int x,int y,DWORD flag);
	virtual BOOL OnRButtonClick(int x,int y,DWORD flag);


protected:
	void _SelectProtoNode(ProtoNodeID id);

	std::vector<ProtoNodeID>_sels;
	std::vector<pos2di>_starts;

	std::string _nameExpose;
	i_math::pos2di _startExpose;

	i_math::pos2di _pt;

};

class CGuiAgent_GraphNodeRectSel:public CGuiAgent_Dragger<TRUE,0>
{
public:
	virtual BOOL OnBeginDrag(int x,int y,DWORD flag);
	virtual void OnEndDrag(int x,int y,DWORD flag);
	virtual void OnDrag(int x,int y,DWORD flag);
	virtual BOOL OnDraw();

protected:
	void _Sel(ProtoNodeID *inrects,DWORD c);
	std::vector<ProtoNodeID>_initials;
	i_math::pos2di _start;
	i_math::recti _rcDraw;


};

class CGuiAgent_GraphNodeConnect:public CGuiAgent_Dragger<TRUE,0>
{
public:
	CGuiAgent_GraphNodeConnect()
	{
	}
	virtual BOOL OnBeginDrag(int x,int y,DWORD flag);
	virtual void OnEndDrag(int x,int y,DWORD flag);
	virtual void OnDrag(int x,int y,DWORD flag);

	virtual BOOL OnSetCursor(int x,int y,DWORD flag);
	virtual BOOL OnTimer(int dt,DWORD flag);


protected:

	ConnectDynPG _conn;


};

class CGuiAgent_GraphNodeCommand:public CGuiAgent
{
public:
	virtual BOOL OnLButtonDown(int x,int y,DWORD flag);
	virtual BOOL OnRButtonClick(int x,int y,DWORD flag);
	virtual BOOL OnLButtonDblClk(int x,int y,DWORD flag);

	virtual BOOL OnCommand(DWORD idCmd);

protected:
	void _ClearCache();

	void _AddLuaSrcFunc(IProtoNode *node,const char *name,int tp);
	std::vector<std::string> _shrinknames;//用来记录处理SHRINKSTUB_COMMAND时所需的数据
	std::string _nameExpose;
	ProtoNodeID _nodeid;
	std::string _nameStub;
	i_math::pos2di _ptClick;




};

