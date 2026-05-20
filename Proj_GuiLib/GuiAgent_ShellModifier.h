#pragma once

#include "GuiEditor.h"
#include "MultiTree/NodeTree.h"
#include "WorldSystem/IEntitySystemDefines.h"
class IProtoNode;
class IProto;
struct Prop_Sx4;
class CGuiAgent_ShellModifier :public CGuiAgent_Dragger<TRUE,FALSE>
{
public:
	CGuiAgent_ShellModifier(void);
	~CGuiAgent_ShellModifier(void);

	void OnAttachView(CGeView *view,DWORD iLevel);
	void OnDetachView(CGeView *view,DWORD iLevel);
	BOOL OnMouseMove(int x,int y,DWORD flag);
	BOOL OnLButtonDown(int x,int y,DWORD flag);
	BOOL OnLButtonUp(int x,int y,DWORD flag);
	BOOL OnDraw();
	BOOL OnSetCursor(int x,int y,DWORD flag);
	BOOL OnBeginDrag(int x,int y,DWORD flag);
	void OnEndDrag(int x,int y,DWORD flag);
	void OnDrag(int x,int y,DWORD flag);
	BOOL OnTimer(int dt,DWORD flag);

	BOOL OnKeyDown(char c,DWORD flag);

	BOOL OnRButtonClick(int x, int y,DWORD flag);
	
	BOOL OnLButtonClick(int x,int y,DWORD flag);

	BOOL OnCommand(DWORD idCmd);

	BOOL NeedDepthBuffer()	{		return FALSE;	}

protected:
	enum
	{
		Drag_Left,
		Drag_Right,
		Drag_Top,
		Drag_Bottom ,
		Drag_UpLeft,
		Drag_UpRight,
		Drag_LowLeft,
		Drag_LowRight,
		Drag_Move,
		Drag_None,
	};

	void _UpdateGuiList();
	
	void _Update();

	void _KeyMove(int unit,const BOOL * keyState);
	
	int  _GetUnit(int t);

	void _IndicateMove(int x,int y);

	struct _ShellNode
	{
		_ShellNode(){memset(idxs,0,sizeof(idxs)); c = 0;bMain = FALSE;iParent=-1;}
		ProtoNodeID nodeProtoID;
		int idxs[4];
		int c;
		BOOL bMain;
//		i_math::pos2di ptOff;
		i_math::recti rcParent;
		DWORD alignType;
		std::vector<std::string> images;
		int iParent;
	};
	
	struct _Rect
	{
		int iNode;
		int iProp;
		int iParent;
//		BOOL bTop;

		ProtoNodeID protoID;

		bool operator==(const _Rect & rc){return rc.iNode==iNode&&rc.iParent==iParent&&rc.iProp==iProp;/*bTop = FALSE*/;}
	};
	
	void _EnumNode(IProto * proto,CNodeTree * tree,NodeHandle handle,int nodeParent,i_math::recti *rcParent,std::vector<_ShellNode> &nodes);
	
	void _SelNodeByUI(int x,int y,DWORD flag);

	void _SelActiveArea(int x,int y);

	void _ProtoNodes2Rects();

	void _DecideOperateType(int x,int y);

	void _Move(IProto * proto,int offx,int offy,BOOL bAbsoulte);

	void _UpdateRects(IProto * proto);

	void CheckValid(i_math::recti &rc,BOOL bX,BOOL bY,BOOL bLeft,BOOL bTop,i_math::recti & rcSrc,i_math::recti * rcAdj = NULL);
	
	Prop_Sx4 * _CheckNode(IProto * proto,int idxNode,int idx);

	void _Reset();
	
	BOOL _GetPropRect(IProto * proto,int iNode,int iProp,i_math::recti & rc);

	ProtoNodeID _GetNodeProtoID(int idx);

	BOOL _IsReadOnly();

private:
	std::vector<_ShellNode> _nodes;
	std::vector<_ShellNode> _topnodes;

	i_math::recti _rcActive;
	int _iRc;
	int _iRcProp;
	ProtoNodeID _activeprotoID;

	i_math::recti _rcEdit;
	int _iRcEdit;
	int _iProp;
	ProtoNodeID _protoID;

	DWORD _Op;
	DWORD _ver;

	DWORD _verAppear;

	BOOL _bTempSel;
	//
	i_math::recti _rcInit;
	int _x,_y;
	
	std::vector<_Rect> _rects;
	
	std::vector<ProtoNodeID> _idprotos;

	std::vector<i_math::recti> _rectinits;
	
	CMenu  * _newMenu;
	
	// time since key pressed.
	int _time;
};


