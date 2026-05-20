#pragma once

#include <string>
#include <vector>
#include "../math/vector3d.h"

#include "../class/class.h"

#include "../gds/GObj.h"

#include "../linkpad/LinkPad.h"
#include "../strlib/strlib.h"

#include "../records/recordsdefine.h"

#include "../gds/GObjUID.h"


#include "FillDescAssist.h"

#include "BehaviorValue.h"

enum BgpFamily
{
	BgpFamily_Common=0,
	BgpFamily_Level,
	BgpFamily_Game,
	BgpFamily_MagicBoard,

	BgpFamily_ForceDword=0xffffffff,
};


enum BgpCategory
{
	BgpCtgr_None,
	BgpCtgr_State,
	BgpCtgr_Controller,
	BgpCtgr_Action,
	BgpCtgr_Condition,
	BgpCtgr_Helper,
	BgpCtgr_Talk,
	BgpCtgr_GA,
	BgpCtgr_Math,
	BgpCtgr_Troop,
	BgpCtgr_Threat,
	BgpCtgr_Var,
	BgpCtgr_AGA,
	BgpCtgr_Func,
	BgpCtgr_SlatesA,
	BgpCtgr_Rtnu,
	BgpCtgr_AI,
	BgpCtgr_Skill,
	BgpCtgr_Buff,
	BgpCtgr_Misc,
	BgpCtgr_Service,
	BgpCtgr_Env,
	BgpCtgr_SlatesB,

	BgpCtgr_Max,
};

inline const char *GetBgpCategoryName(BgpCategory ctgr)
{
	switch(ctgr)
	{
		case BgpCtgr_State:			return "状态控制";
		case BgpCtgr_Func:			return "函数";
		case BgpCtgr_Controller:		return "流程控制";
		case BgpCtgr_Action:				return "单位行动";
		case BgpCtgr_Condition:		return "条件判断";
		case BgpCtgr_Helper:		return "辅助";
		case BgpCtgr_Talk:		return "对话";
		case BgpCtgr_GA:		return "通用Agent";
		case BgpCtgr_Math:		return "计算";
		case BgpCtgr_Troop:		return "Troop";
		case BgpCtgr_Threat:		return "Threat";
		case BgpCtgr_Var:		return "变量";
		case BgpCtgr_AGA:		return "通用Agent访问";
		case BgpCtgr_SlatesA:		return "石板迷宫A";
		case BgpCtgr_SlatesB:		return "石板迷宫B";
		case BgpCtgr_Rtnu:		return "随从";
		case BgpCtgr_AI:		return "AI";
		case BgpCtgr_Skill:		return "技能";
		case BgpCtgr_Buff:		return "Buff";
		case BgpCtgr_Misc:		return "杂项";
		case BgpCtgr_Service:		return "服务";
		case BgpCtgr_Env:		return "战斗环境";
	}
	return "n/a";
}




#define BEGIN_STUB() 										\
switch(idx)															\
{

#define STUB_IN(__idx,__name)							\
		case __idx:													\
			return PadStub(__name,PadStub_In,1)

#define STUB_OUT(__idx,__name)						\
		case __idx:													\
			return PadStub(__name,PadStub_Out,1)

#define STUB_C_IN(__idx,__name)							\
		case __idx:													\
		return PadStub(__name,PadStub_CIn,1)

#define STUB_C_OUT(__idx,__name)						\
		case __idx:													\
		return PadStub(__name,PadStub_COut,1)

		
#define END_STUB()											\
}																			\
return PadStub();

#define GELEM_BGP_BASE()											\
		GELEM_VAR_INIT(BOOL,_bEnabled,TRUE);			\
			GELEM_EDITVAR("可用",GVT_S,GSem_Boolean,"是否可用");\
		GELEM_VAR_INIT(ReturnStyle,_styleRet,Default);\
			GELEM_EDITVAR("返回方式",GVT_U,GSem(GSem_Interger,"缺省,Not,AlwaysTrue,AlwaysFalse"),"返回方式");\
		GELEM_STRING(_comment);\
			GELEM_EDITVAR("注释",GVT_String,GSem_Name,"注释");

extern void FormatString(std::string &s,const char *formatstring,...);
extern void AppendFmtString(std::string &s,const char *formatstring,...);

typedef  PadID ( *UniquePadIDGenFunc)();


class CBehaviorGraphPad;
class CBgp_Func;
class CBgp_Consts;
class CBgp_Vars;
class CBehaviorGraphPads:public CLinkPads
{
public:
	DEFINE_CLASS(CBehaviorGraphPads);
	CBehaviorGraphPads()
	{
		Zero();
	}
	~CBehaviorGraphPads()
	{
		Clear();
	}


	struct Mod_IncludeFolder
	{
		PadID idPad;
	};

	struct Mod_ExcludePad
	{
		PadID idPad;
	};

	struct Mod_RemoveLink
	{
		PadID idPad1;
		PadID idPad2;
	};

	struct Mod_MovePad
	{
		PadID idPad;
		i_math::pos2d_sh pos;
	};

	struct Mod_OverridePad
	{
		void Save(CDataPacket &dp)
		{
			dp.Data_WriteSimple(idPad);
			DP_WriteVector(dp,data);
		}
		void Load(CDataPacket &dp)
		{
			dp.Data_ReadSimple(idPad);
			DP_ReadVector(dp,data);
		}
		PadID idPad;
		std::vector<BYTE> data;
	};

	struct Mod_AddLink
	{
		CLinkPads::LinkPersist link;
	};

	void Zero()
	{
		_bResolved=FALSE;
		_bLookUpConsts=FALSE;
		_callbackGenPadID=NULL;
	}

	void Clear()
	{
		CLinkPads::Clear();

		ClearMods();

		_bases.clear();

		_lookupConsts.clear();

		_lookupDeltePtrs.clear();

		Zero();
	}

	void ClearMods()
	{
		_foldersInclude.clear();
		_folderExclude.clear();
		_padsMove.clear();
		_padsOverride.clear();
		_linksAdd.clear();
	}

	virtual void Save(CDataPacket &dp);
	virtual void Load(CDataPacket &dp);

	BgpFamily GetFamily();

	StringID GetName();
	const char *GetPadName(PadID idPad);

	void EnumConstsDeclare(std::vector<BhvConstDeclare*>&declares,BOOL bSource,BOOL bParam);

	CBgp_Consts*FindConsts(CBehaviorGraphPad *pad);
	CBgp_Vars*FindVars();
	BehaviorMemType GetVarMemType(StringID nm);

	BhvConstDeclare *FindConstDeclare(StringID nm);

	CBgp_Func *FindFunc(StringID nm);

	CBgp_Func *FindOwnerFunc(CBehaviorGraphPad *pad);

	PadID GetMinPadID()
	{
		PadID idMin=0x7fffffff;
		for (int i=0;i<_pads.size();i++)
		{
			if (_pads[i]->GetID()<idMin)
				idMin=_pads[i]->GetID();
		}
		return idMin;
	}

	virtual PadID GenPadID()
	{
		if (_callbackGenPadID)
			return (*_callbackGenPadID)();
		MessageBoxA(NULL,(LPCSTR)(LPCWSTR)"Fuck",(LPCSTR)(LPCWSTR)"Fuck",MB_OK);
		return CLinkPads::GenPadID();
	}

	void SetPadIDGenCallBack(UniquePadIDGenFunc func);

	BOOL IsResolved()	{		return _bResolved;	}

	BOOL IsPadIncluded(CBehaviorGraphPads &padsBase,PadID idPad);//检测一个base pads的一个pad是不是可以include

	void AddBase(StringID nm);
	BOOL IsBase(StringID nm);
	void RemoveBase(StringID nm);

	StringID *GetBases(DWORD &c);

	void EnumTopStates(std::vector<PadID> &ids);
	void EnumFuncs(std::vector<PadID> &ids);

	std::vector<void*> *FindDeltaPtrs(PadID idPad);

	BOOL ResolveBhvValType(BhvValType &tp,CClass *&clss,GElemBase *&elem);

public://take it as protected
	virtual BYTE _CalcClassCode();

	virtual BOOL _FoldLinkSrc()	{		return FALSE;	}

	BOOL _bResolved;

	//基于哪些其它的BGPads
	std::vector<StringID> _bases;

	//Mods
	std::vector<Mod_IncludeFolder> _foldersInclude;
	std::vector<Mod_ExcludePad> _folderExclude;
	std::vector<Mod_MovePad> _padsMove;
	std::deque<Mod_OverridePad> _padsOverride;
	std::vector<Mod_AddLink> _linksAdd;


	//Consts
	BOOL _bLookUpConsts;
	std::unordered_map<StringID,BhvConstDeclare*> _lookupConsts;

	//GenPadID回调
	UniquePadIDGenFunc _callbackGenPadID;

	//所有被Delta的数据指针,此信息仅用于编辑器
	std::unordered_map<PadID,std::vector<void*> >_lookupDeltePtrs;

	friend class CBehaviorGraphUtil;
	
};

class CBehaviorGraphPad:public CLinkPad
{
public:
	CBehaviorGraphPad()
	{
		_bOverriden=0;
		_nmBase=StringID_Invalid;
		_bEnabled=TRUE;
	}

	enum ReturnStyle
	{
		Default,
		Not,
		AlwaysTrue,
		AlwaysFalse,

		ForceDword=0xffffffff,
	};

	virtual BgpFamily GetFamily()=0;
	virtual BgpCategory GetCategory()	{		return BgpCtgr_None;	}
	virtual const char *GetShowName()		{		return GetTypeName();	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)=0;

	BOOL IsEnabled()	{		return _bEnabled;	}
	ReturnStyle GetReturnStyle()	{		return _styleRet;	}
	const char *GetComment()	{		return _comment.c_str();	}

public://take it as protected
	DWORD _bOverriden:1;
	StringID _nmBase;//这个pad是从哪个base直接继承过来的(注意间接继承的不算)
	ReturnStyle _styleRet;
	BOOL _bEnabled;
	std::string _comment;//注释
};


