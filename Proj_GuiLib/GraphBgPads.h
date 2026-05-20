#pragma once

#include "GraphPads.h"

#include "behaviorgraph/FillDescAssist.h"


//////////////////////////////////////////////////////////////////////////
//CGraphBgPads
class CBehaviorGraphPads;
struct FillDescAssist_GuiLib:public FillDescAssist
{
	FillDescAssist_GuiLib()
	{
		_gg=NULL;
		_pads=NULL;
	}
	virtual float CalcHeight(const char *str);

	virtual const char *GetUnitName(RecordID idUnit)
	{
		return _GetRecName(idUnit,"units.rcs");
	}

	virtual const char *GetSkillName(RecordID idSkill)
	{
		return _GetRecName(idSkill,"skills.rcs");
	}

	virtual const char *GetItemName(RecordID idItem)
	{
		return _GetRecName(idItem,"items.rcs");
	}

	virtual const char *GetAgentName(RecordID idAgent)
	{
		return _GetRecName(idAgent,"agents.rcs");
	}

	virtual const char *GetGestureName(RecordID idGesture)
	{
		return _GetRecName(idGesture,"gestures.rcs");
	}
	virtual const char *GetBuffName(RecordID idBuff)
	{
		return _GetRecName(idBuff,"buffs.rcs");
	}
	virtual const char *GetMagicTileName(RecordID idMagicTile)
	{
		return _GetRecName(idMagicTile,"magictiles.rcs");
	}

	virtual const char *GetMapName(RecordID idMap)
	{
		return _GetRecName(idMap,"maps.rcs");
	}

	virtual const char *GetResName(RecordID idRes)
	{
		return _GetRecName(idRes,"resources.rcs");
	}

	virtual const char *GetEoName(RecordID idEo)
	{
		return _GetRecName(idEo,"eos.rcs");
	}


	virtual CRecord *GetClonedRec_Buff(RecordID idBuff)
	{
		return _GetClonedRec(idBuff,"buffs.rcs");
	}

	virtual const char *GetStr(StringID nm)
	{
		switch(nm)
		{
		case 1:
			return "Check Day";
		case 2:
			return "BYTE";
		case 3:
			return "WORD";
		case 4:
			return "FLAG0";
		case 5:
			return "FLAG1";
		case 6:
			return "FLAG2";
			//XXXXX:more simple var
		}
		return StrLib_GetStr(nm);
	}

	virtual BehaviorMemType GetMemType(StringID nm);

	void SetGG(GraphicsGraph *gg)	{		_gg=gg;	}
	void SetBehaviorGraphPads(CBehaviorGraphPads *pads)	{	_pads=pads;	}

protected:
	const char *_GetRecName(RecordID idRec,const char *path);
	CRecord *_GetClonedRec(RecordID idRec,const char *path);

	GraphicsGraph *_gg;
	CBehaviorGraphPads *_pads;

};


struct GraphBgPadItem:public GraphPadItem
{
	DEFINE_CLASS(GraphBgPadItem);

	virtual i_math::pos2di GetConnectSpot(BOOL bConnecting);//or connected

};

//GraphBehaviorPad
class CGraphBgPad:public CGraphPad
{
public:
	DEFINE_CLASS(CGraphBgPad);

	CGraphBgPad()
	{
		Zero();
	}
	~CGraphBgPad()
	{
		Clear();
	}

	void Zero();
	void Clear();

	virtual void RecalcLayout(GraphicsGraph *gg);
	virtual void Draw(GraphicsGraph *gg,BOOL bHilight);
	virtual BOOL HitTest(int x,int y,GraphPadHit &hit);

	GraphPadItem *FindItem(const char *name);

protected:

	BOOL _IsState();

	void _DrawTitle(GraphPadItem*item,BOOL bHilight,GraphicsGraph *gg);
	void _DrawOut(GraphPadItem*item,GraphicsGraph *gg);
	void _DrawIn(GraphPadItem*item,GraphicsGraph *gg);
	void _DrawCIn(GraphPadItem*item,GraphicsGraph *gg);
	void _DrawCOut(GraphPadItem*item,GraphicsGraph *gg);
	void _DrawSep(GraphicsGraph *gg,int ySep);
	void _DrawBP(GraphPadItem*item,GraphicsGraph *gg);

	void _LocateOut(GraphPadItem *item,i_math::recti &rcItem);
	void _LocateIn(GraphPadItem *item,i_math::recti &rcItem);

	BOOL _ExistInOutSeg();
	BOOL _ExistCtrlSeg();
	BOOL _NeedSep2();//signal/slot后面的seperator
    BOOL _IsPending(GraphPadItem*item, GraphicsGraph *gg);

	BOOL _ItemsHitTest(int x,int y,GraphPadItem**items,DWORD c,int part,GraphPadHit &hit);
	BOOL _ItemHitTest(int x,int y,GraphPadItem*item,int part,GraphPadHit &hit);


	std::vector<GraphPadItem*>_ins;
	std::vector<GraphPadItem*>_outs;
	std::vector<GraphPadItem*>_cins;
	std::vector<GraphPadItem*>_couts;

	int _ySep2;//in/out与ctrl之间的seperator
	i_math::recti  _rcDesc;

	std::string _desc;

	BOOL _bBase;
	BOOL _bOverriden;


	friend class CGraphBgPads;
};


class GuiLib_Api CGraphBgPads:public CGraphPads
{
public:
	CGraphBgPads()
	{
	}

	void Draw(GraphicsGraph *gg,PadID *sels,DWORD c)
	{
		CGraphPads::Draw(gg,sels,c);
	}

protected:

	virtual CGraphPad *_LoadPad(CLinkPad *pad);
	virtual void _DrawDynConnect(GraphicsGraph *gg,ConnectDyn &conn);
	virtual void _DrawPermConnect(GraphicsGraph *gg);


};
