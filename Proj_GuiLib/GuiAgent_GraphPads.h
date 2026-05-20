
#pragma once

#include "GuiLib.h"

#include "GuiEditor.h"

#include "GuiAgent_2DTransform.h"

#include "linkpad/LinkPad.h"


class CDataSrc_GraphPads
{
public:
	virtual CGraphPads *GetGraph(CGuiAgent *agent)=0;
	virtual std::vector<PadID>*GetSelBuf(CGuiAgent *agent)=0;
	virtual void NotifyChange(CGuiAgent *agent,BOOL bSave)=0;
};


class CGuiAgent_GraphPadScroll:public CGuiAgent_2DTransform
{
public:
	CGuiAgent_GraphPadScroll(CDataSrc_GraphPads *src)
	{
		_src=src;
	}
	virtual void OnUpdateTransform(const i_math::pos2df &pos, const i_math::pos2df &scale);
protected:
	virtual CGraphPads *_GetGraph()		{		return _src?_src->GetGraph(this):NULL;	}
	CDataSrc_GraphPads *_src;

};

class CGraphPads;
class CGuiAgent_GraphPadSel:public CGuiAgent_Dragger<TRUE,0>
{
public:
	CGuiAgent_GraphPadSel(CDataSrc_GraphPads *src,BOOL bReadOnly)
	{
		_src=src;
		_bReadOnly=bReadOnly;
	}

	virtual BOOL OnBeginDrag(int x,int y,DWORD flag);
	virtual void OnEndDrag(int x,int y,DWORD flag);
	virtual void OnDrag(int x,int y,DWORD flag);

	virtual BOOL OnTimer(int dt,DWORD flag);

	virtual BOOL OnRButtonDown(int x,int y,DWORD flag);
	virtual BOOL OnRButtonClick(int x,int y,DWORD flag);

protected:

	void _SelectPad(PadID id);

	virtual CGraphPads *_GetGraph()		{		return _src?_src->GetGraph(this):NULL;	}
	virtual std::vector<PadID>*_GetSelBuf(){		return _src?_src->GetSelBuf(this):NULL;	}
	virtual void _NotifyChange(BOOL bSave)	{		if (_src)			_src->NotifyChange(this,bSave);	}
	CDataSrc_GraphPads *_src;

	//这个函数会在开始拖拽时调用,子类可以根据需要修改需要拖拽的pad
	virtual void _ModDrags(std::vector<PadID>&sel)	{	}

	std::vector<PadID>_sels;
	std::vector<pos2di>_starts;

	i_math::pos2di _pt;

	BOOL _bReadOnly;

};

class CGuiAgent_GraphPadRectSel:public CGuiAgent_Dragger<TRUE,0>
{
public:
	CGuiAgent_GraphPadRectSel(CDataSrc_GraphPads *src)
	{
		_src=src;
	}

	virtual BOOL OnBeginDrag(int x,int y,DWORD flag);
	virtual void OnEndDrag(int x,int y,DWORD flag);
	virtual void OnDrag(int x,int y,DWORD flag);
	virtual BOOL OnDraw();

protected:
	void _Sel(PadID *inrects,DWORD c);

	virtual CGraphPads *_GetGraph()		{		return _src?_src->GetGraph(this):NULL;	}
	virtual std::vector<PadID>*_GetSelBuf(){		return _src?_src->GetSelBuf(this):NULL;	}
	virtual void _NotifyChange(BOOL bSave)	{		if (_src)			_src->NotifyChange(this,bSave);	}
	CDataSrc_GraphPads *_src;

	std::vector<PadID>_initials;
	i_math::pos2di _start;
	i_math::recti _rcDraw;


};

class CGuiAgent_GraphPadConnect:public CGuiAgent_Dragger<TRUE,0>
{
public:
	CGuiAgent_GraphPadConnect(CDataSrc_GraphPads *src)
	{
		_src=src;
	}
	virtual BOOL OnBeginDrag(int x,int y,DWORD flag);
	virtual void OnEndDrag(int x,int y,DWORD flag);
	virtual void OnDrag(int x,int y,DWORD flag);

protected:

	virtual CGraphPads *_GetGraph()		{		return _src?_src->GetGraph(this):NULL;	}
	virtual std::vector<PadID>*_GetSelBuf(){		return _src?_src->GetSelBuf(this):NULL;	}
	virtual void _NotifyChange(BOOL bSave)	{		if (_src)			_src->NotifyChange(this,bSave);	}
	CDataSrc_GraphPads *_src;


	ConnectDyn _conn;


};

struct PadsCB
{
	BOOL IsEmpty()
	{
		return (pads.size()<=0);
	}

	void Clear()
	{
		pads.clear();
		links.clear();
	}

	struct Pad
	{
		PadID id;
		std::string classname;
		StringID name;

		BOOL bSub;
		
		i_math::pos2di ptOff;//bInFolder为FALSE才有效
		std::string nameFolder;
		BOOL bFolder;
		i_math::pos2di ptFolder;

		PadID idFolder;
		i_math::pos2di pt;
		std::vector<BYTE> data;
	};


	int Find(PadID id);
	BOOL Exist(PadID id);

	std::vector<Pad> pads;
	std::vector<CLinkPads::LinkPersist> links;
};

class CGuiAgent_GraphPadCommand:public CGuiAgent
{
public:
	CGuiAgent_GraphPadCommand(CDataSrc_GraphPads *src,PadsCB *cb)
	{
		_src=src;
		_cb=cb;

		_iSelFolderLabel=-1;
	}

	virtual BOOL OnRButtonClick(int x,int y,DWORD flag);
	virtual BOOL OnLButtonDblClk(int x,int y,DWORD flag);
	virtual BOOL OnMouseMove(int x,int y,DWORD flag);
	virtual BOOL OnLButtonClick(int x,int y,DWORD flag);

	virtual BOOL OnCommand(DWORD idCmd);

	virtual BOOL OnDraw();

protected:

	virtual CGraphPads *_GetGraph()		{		return _src?_src->GetGraph(this):NULL;	}
	virtual std::vector<PadID>*_GetSelBuf(){		return _src?_src->GetSelBuf(this):NULL;	}
	virtual void _NotifyChange(BOOL bSave)	{		if (_src)			_src->NotifyChange(this,bSave);	}

	void _TransformGGtoPads(CLinkPads *pads);
	void _TransformGGfromPads(CLinkPads *pads);
	CDataSrc_GraphPads *_src;

	void _Copy(std::vector<PadID>*sels,CLinkPads *pads);
	void _Paste(CLinkPads *pads);

	PadsCB *_cb;

	i_math::pos2di _pt;

	struct FolderLabel
	{
		FolderLabel()
		{
			idPad=0;
		}
		i_math::recti rc;
		PadID idPad;
	};

	std::vector<FolderLabel> _labelsFolder;
	int _iSelFolderLabel;
	FolderLabel _labelSelFolder;


};

