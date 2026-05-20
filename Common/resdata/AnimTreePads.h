#pragma once

#include <string>
#include <vector>
#include "../math/vector3d.h"
#include "../math/range.h"

#include "../class/class.h"

#include "../gds/GObj.h"
#include "../gds/GObjUID.h"


#include "../linkpad/LinkPad.h"
#include "../strlib/strlib.h"

#include "anim/animdefines.h"
#include "avtrstates/avtrstates_defines.h"

#define ANIMTREEROOT_GRPNAME "动画树根节点名称"

class CAnimTreePads:public CLinkPads
{
public:
	CAnimTreePads()
	{
		_idDefRoot=PadID_Null;
	}

	PadID GetDefRoot()	{		return _idDefRoot;	}
	void SetDefRoot(PadID id)	{		_idDefRoot=id;	}

	virtual void Save(CDataPacket &dp)
	{
		CLinkPads::Save(dp);
		DP_WriteVar(dp,_idDefRoot);
	}
	virtual void Load(CDataPacket &dp)
	{
		BOOL bLongPadID;
		CLinkPads::Load(dp,bLongPadID);
		if (bLongPadID)
		{
			DP_ReadVar(dp,_idDefRoot);
		}
		else
			_idDefRoot=_ReadShortPadID(dp);
	}

protected:
	virtual BYTE _CalcClassCode();
	PadID _idDefRoot;
	
};



//注意,如果修改了某个Flag的值,请检查所有AnimTreePad的"Flags"的GSem的contraint
#define ATPF_ALWAYS_TICK_obsolete 1
#define ATPF_LOOP 2
#define ATPF_RESET_WHEN_ACTIVATED_obsolete 4
#define ATPF_DONOT_SEND_EVENT 8
#define ATPF_REVERSE 16

struct AtpChild
{
   BEGIN_GOBJ_PURE(AtpChild,1);
		GELEM_STRING(name);

    END_GOBJ();    

	std::string name;
};

struct AtpTunerInfo
{
	StringID idNm;
	std::string nm;

	BOOL IsEmpty()
	{
		return idNm==StringID_Invalid&&nm.empty();
	}

	BEGIN_GOBJ_PURE(AtpTunerInfo,1);
		GELEM_VAR_INIT(StringID,idNm,StringID_Invalid);
			GELEM_EDITVAR("名称id",GVT_U,GSem(GSem_StringID,"动画树Tuner名称"),"Tuner的名称ID");
		GELEM_STRING(nm);
			GELEM_EDITVAR("名称",GVT_String,GSem_Name,"Tuner的名称");
	END_GOBJ();    
};


class CAnimTreePad:public CLinkPad
{
public:
	enum DbgType
	{
		Dbg_None,
		Dbg_Name,
		Dbg_1D,
		Dbg_2D,
	};
	enum DbgFlag
	{
		DbgF_None=0,
		DbgF_AutoReset=1,
	};

	enum Category
	{
		None,
		Operator,
		Sequence,
		FloatValue,
		Path,
		BoneCtrl,
		IKCtrl,
	};

	virtual Category GetCategory()	{		return Operator;	}

	//注意,所有CAnimTreePad只能有一个PadStub_Out的stub
	//并且,所有CAnimTreePad的PadStub_In的 stub(也就是Child)必须放在最前面
	virtual DWORD GetStubCount()	{		return _childs.size()+1;	}
	virtual PadStub GetStub(DWORD idx);

	virtual DWORD GetChildCount()	{		return _childs.size();	}
	virtual BOOL CanModifyChild()	{		return FALSE;	}//是否能对child的名字进行修改,并且增加/删除child
	virtual const char *GetChildName(DWORD idx)	{		return _childs[idx].name.c_str();	}
	virtual BOOL SetChildName(DWORD idx,const char *name,CLinkPads *owner);
	virtual BOOL AddChild(CLinkPads *owner);
	virtual void RemoveChild(DWORD idx,CLinkPads *owner);


	virtual DbgFlag GetDbgFlag()	{		return DbgF_None;	}
	virtual DbgType GetDbgType()	{		return Dbg_None;	}
	virtual StringID *GetDbgNames(DWORD &count)	{		count=0;	return NULL;}//这个函数在GetDbgType()返回Dbg_Name时需要重载
	virtual const char *GetDbgGroup()	{		return "";	}
	virtual void ConvertDV(i_math::vector2df&cv){};

	virtual AtpTunerInfo *GetTunerInfo()	{		return NULL;	}

	virtual void CollectRefs(std::vector<std::string>&buf)	{	}

public://take it as protected
	DWORD _flags;
	std::vector<AtpChild> _childs;

};



class CAtpRoot:public CAnimTreePad
{
public:
	DECLARE_CLASS(CAtpRoot);

	virtual const char *GetTypeName()	{		return "根";	};
	virtual DWORD GetStubCount();
	virtual PadStub GetStub(DWORD idx);
	virtual const char *GetShowName()	{		return _nm.c_str();	}
	virtual DWORD GetChildCount()	{		return 3;	}

	const char *GetName()	{		return _nm.c_str();	}

    BEGIN_GOBJ_PURE(CAtpRoot,1);
		GELEM_VAR_INIT(DWORD,_flags,0);
		GELEM_STRING(_nm);
			GELEM_VERSION(2);
			GELEM_EDITVAR("名称",GVT_String,GSem_Name,"根结点名称");
    END_GOBJ();    

protected:
	std::string _nm;


};

class CAtpSequence:public CAnimTreePad
{
public:
	DECLARE_CLASS(CAtpSequence);

	virtual const char *GetTypeName()	{		return "动画序列";	};
	virtual DWORD GetStubCount();
	virtual PadStub GetStub(DWORD idx);
	virtual const char *GetShowName()	{		return StrLib_GetStr(_ap);	}

	virtual DWORD GetChildCount()	{		return 0;	}

	virtual Category GetCategory()	{		return Sequence;	}

	virtual DbgType GetDbgType()	{		return Dbg_None;	}


	BEGIN_GOBJ_PURE(CAtpSequence,1);
		GELEM_VAR_INIT(DWORD,_flags,0);
			GELEM_EDITVAR("标志",GVT_U,GSem(GSem_Flags,"循环播放:2,不发送事件:8,反向播放:16"),"标志");
		GELEM_VAR_INIT(StringID,_ap,StringID_Invalid);
			GELEM_EDITVAR("动画",GVT_U,GSem(GSem_StringID,"骨骼动画AnimPiece"),"动画名称");
		GELEM_VAR_INIT(float,_scale,1.0f);
			GELEM_EDITVAR("速度调整",GVT_F,GSem(GSem_Float,"0.00f,100.0f,0.05f"),"播放速度调整");
		GELEM_VAR_INIT(float,_off,0.0f);
			GELEM_EDITVAR("偏移调整",GVT_F,GSem(GSem_Float,"0.00f,1.0f,0.05f"),"取值为0..1之间");
		GELEM_VAR_INIT(float,_varOff,0.0f);
			GELEM_EDITVAR("偏移调整随机范围i",GVT_F,GSem(GSem_Float,"0.00f,1.0f,0.05f"),"取值为0..1之间");
		GELEM_OBJ(AtpTunerInfo,_tuiOff);
			GELEM_EDITOBJ("偏移Tuner信息","偏移Tuner信息");
		GELEM_VAR_INIT(float,_minwt,0.1f);
			GELEM_EDITVAR("最小发送事件的权重",GVT_F,GSem(GSem_Float,"0.00f,1.0f,0.01f"),"当权重小于这个值时,将不发送事件");
		GELEM_VAR_INIT(float,_blendLoop,0.0f);
			GELEM_EDITVAR("循环播放混合",GVT_F,GSem(GSem_Float,"0.00f,0.5f,0.05f"),"取值为0..1之间");
		GELEM_VAR_INIT(StringID,_grp,StringID_Invalid);
			GELEM_EDITVAR("Sync Group",GVT_U,GSem(GSem_StringID,"SyncGroup"),"同步组的名称");
	END_GOBJ();    

public://take it as protected

	StringID _ap;//
	StringID _grp;//sync group
	float _scale;
	float _off;
	AtpTunerInfo _tuiOff;
	float _varOff;
	float _blendLoop;
	float _minwt;//发送事件的最小权重


};

//WH 代表With Head,动画前面有一段头动画
class CAtpSequenceWH:public CAtpSequence
{
public:
	DECLARE_CLASS(CAtpSequenceWH);

	virtual const char *GetTypeName()	{		return "WH动画序列";	};

	BEGIN_GOBJ_PURE(CAtpSequenceWH,1);
		GELEM_VAR_INIT(DWORD,_flags,ATPF_LOOP);
			GELEM_EDITVAR("标志",GVT_U,GSem(GSem_Flags,"循环播放:2,不发送事件:8"),"标志");
		GELEM_VAR_INIT(StringID,_apH,StringID_Invalid);
			GELEM_EDITVAR("Head动画",GVT_U,GSem(GSem_StringID,"骨骼动画AnimPiece"),"Head动画名称");
		GELEM_VAR_INIT(float,_scaleH,1.0f);
			GELEM_EDITVAR("Head动画速度调整",GVT_F,GSem(GSem_Float,"0.00f,100.0f,0.05f"),"Head动画播放速度调整");
		GELEM_VAR_INIT(StringID,_ap,StringID_Invalid);
			GELEM_EDITVAR("动画",GVT_U,GSem(GSem_StringID,"骨骼动画AnimPiece"),"动画名称");
		GELEM_VAR_INIT(float,_scale,1.0f);
			GELEM_EDITVAR("速度调整",GVT_F,GSem(GSem_Float,"0.00f,100.0f,0.05f"),"播放速度调整");
		GELEM_VAR_INIT(float,_off,0.0f);
			GELEM_EDITVAR("偏移调整",GVT_F,GSem(GSem_Float,"0.00f,1.0f,0.05f"),"取值为0..1之间");
		GELEM_VAR_INIT(float,_varOff,0.0f);
			GELEM_EDITVAR("偏移调整随机范围i",GVT_F,GSem(GSem_Float,"0.00f,1.0f,0.05f"),"取值为0..1之间");
		GELEM_VAR_INIT(float,_blendLoop,0.0f);
			GELEM_EDITVAR("循环播放混合",GVT_F,GSem(GSem_Float,"0.00f,0.5f,0.05f"),"取值为0..1之间");
		GELEM_VAR_INIT(float,_minwt,0.1f);
			GELEM_EDITVAR("最小发送事件的权重",GVT_F,GSem(GSem_Float,"0.00f,1.0f,0.01f"),"当权重小于这个值时,将不发送事件");
		GELEM_VAR_INIT(float,_dur,0.2f);
			GELEM_EDITVAR("过渡时间",GVT_F,GSem(GSem_Float,"0.01f,2.0f,0.02f"),"两个动画切换时的过渡时间");
		GELEM_VAR_INIT(StringID,_grp,StringID_Invalid);
			GELEM_EDITVAR("Sync Group",GVT_U,GSem(GSem_StringID,"SyncGroup"),"同步组的名称");
	END_GOBJ();    

public://take it as protected
	StringID _apH;//头动画
	float _scaleH;
	float _dur;	
};


//SD 代表Speed Driven,由移动速度驱动的动画序列
class CAtpSequenceSD:public CAtpSequence
{
public:
	DECLARE_CLASS(CAtpSequenceSD);

	virtual const char *GetTypeName()	{		return "SD动画序列";	};

	virtual DbgType GetDbgType()	{		return Dbg_1D;	}
	virtual const char *GetDbgGroup()	{		return "MoveSpeed";	}
	virtual void ConvertDV(i_math::vector2df&dv)	{		dv.x=i_math::lerp(0.0f,20.0f,dv.x);	}
	AtpTunerInfo *GetTunerInfo() override	{		return &_tui;	}

	BEGIN_GOBJ_PURE(CAtpSequenceSD,1);
		GELEM_VAR_INIT(DWORD,_flags,ATPF_LOOP);
			GELEM_EDITVAR("标志",GVT_U,GSem(GSem_Flags,"循环播放:2,不发送事件:8"),"标志");
		GELEM_VAR_INIT(StringID,_ap,StringID_Invalid);
			GELEM_EDITVAR("动画",GVT_U,GSem(GSem_StringID,"骨骼动画AnimPiece"),"动画名称");
		GELEM_VAR_INIT(float,_minwt,0.1f);
			GELEM_EDITVAR("最小发送事件的权重",GVT_F,GSem(GSem_Float,"0.00f,1.0f,0.01f"),"当权重小于这个值时,将不发送事件");
		GELEM_VAR_INIT(StringID,_grp,StringID_Invalid);
			GELEM_EDITVAR("Sync Group",GVT_U,GSem(GSem_StringID,"SyncGroup"),"同步组的名称");
		GELEM_VAR_INIT(float,_min,1.0f);
			GELEM_EDITVAR("最小速度",GVT_F,GSem(GSem_Float,"0.00f,100.0f,0.05f"),"最小速度限制");
		GELEM_VAR_INIT(float,_max,10.0f);
			GELEM_EDITVAR("最大速度",GVT_F,GSem(GSem_Float,"0.00f,100.0f,0.05f"),"最大速度限制");
		GELEM_VAR_INIT(float,_cycleoff,6.0f);
			GELEM_EDITVAR("周期移动距离",GVT_F,GSem(GSem_Float,"0.01f,100.0f,0.02f"),"每个动画周期移动的距离");
		GELEM_VAR_INIT(float,_off,0.0f);
			GELEM_EDITVAR("偏移调整",GVT_F,GSem(GSem_Float,"0.00f,100.0f,0.05f"),"取值为0..1之间");
		GELEM_VAR_INIT(float,_varOff,0.0f);
			GELEM_EDITVAR("偏移调整随机范围i",GVT_F,GSem(GSem_Float,"0.00f,1.0f,0.05f"),"取值为0..1之间");
		GELEM_VAR_INIT(float,_blendLoop,0.0f);
			GELEM_EDITVAR("循环播放混合",GVT_F,GSem(GSem_Float,"0.00f,0.5f,0.05f"),"取值为0..1之间");
		GELEM_OBJ(AtpTunerInfo,_tui);
			GELEM_EDITOBJ("Tuner信息","Tuner信息");
	END_GOBJ();    

public://take it as protected
	float _min;
	float _max;
	float _cycleoff;//每个循环移动的距离
	AtpTunerInfo _tui;
};

//TSD 代表Turn Speed Driven,由移动速度驱动的动画序列
class CAtpSequenceTSD:public CAtpSequence
{
public:
	DECLARE_CLASS(CAtpSequenceTSD);

	virtual const char *GetTypeName()	{		return "TSD动画序列";	};

	virtual DbgType GetDbgType()	{		return Dbg_1D;	}
	virtual const char *GetDbgGroup()	{		return "TurnSpeed";	}
	virtual void ConvertDV(i_math::vector2df&dv)	{		dv.x=i_math::lerp(0.1f,6.28f,dv.x);	}

	BEGIN_GOBJ_PURE(CAtpSequenceTSD,1);
		GELEM_VAR_INIT(DWORD,_flags,ATPF_LOOP);
			GELEM_EDITVAR("标志",GVT_U,GSem(GSem_Flags,"循环播放:2,不发送事件:8"),"标志");
		GELEM_VAR_INIT(StringID,_ap,StringID_Invalid);
			GELEM_EDITVAR("动画",GVT_U,GSem(GSem_StringID,"骨骼动画AnimPiece"),"动画名称");
		GELEM_VAR_INIT(float,_minwt,0.1f);
			GELEM_EDITVAR("最小发送事件的权重",GVT_F,GSem(GSem_Float,"0.00f,1.0f,0.01f"),"当权重小于这个值时,将不发送事件");
		GELEM_VAR_INIT(StringID,_grp,StringID_Invalid);
			GELEM_EDITVAR("Sync Group",GVT_U,GSem(GSem_StringID,"SyncGroup"),"同步组的名称");
		GELEM_VAR_INIT(float,_min,0.1f);
			GELEM_EDITVAR("最小转身速度",GVT_F,GSem(GSem_Float,"0.00f,100.0f,0.05f"),"最小转身速度限制");
		GELEM_VAR_INIT(float,_max,6.28f);
			GELEM_EDITVAR("最大转身速度",GVT_F,GSem(GSem_Float,"0.00f,100.0f,0.05f"),"最大转身速度限制");
		GELEM_VAR_INIT(float,_cycleoff,1.0f);
			GELEM_EDITVAR("周期转动弧度",GVT_F,GSem(GSem_Float,"0.01f,6.28f,0.01f"),"每个动画周期转动的弧度");
		GELEM_VAR_INIT(float,_off,0.0f);
			GELEM_EDITVAR("偏移调整",GVT_F,GSem(GSem_Float,"0.00f,100.0f,0.05f"),"取值为0..1之间");
		GELEM_VAR_INIT(float,_varOff,0.0f);
			GELEM_EDITVAR("偏移调整随机范围i",GVT_F,GSem(GSem_Float,"0.00f,1.0f,0.05f"),"取值为0..1之间");
		GELEM_VAR_INIT(float,_blendLoop,0.0f);
			GELEM_EDITVAR("循环播放混合",GVT_F,GSem(GSem_Float,"0.00f,0.5f,0.05f"),"取值为0..1之间");
	END_GOBJ();    

public://take it as protected
	float _min;
	float _max;
	float _cycleoff;//每个循环移动的距离
};


//ST 代表Static
class CAtpSequenceST:public CAtpSequence
{
public:
	DECLARE_CLASS(CAtpSequenceST);

	virtual const char *GetTypeName()	{		return "Static动画序列";	};

	virtual DbgType GetDbgType()	{		return Dbg_1D;	}
	virtual const char *GetDbgGroup()	{		return "";	}

	virtual AtpTunerInfo *GetTunerInfo()	{		return &_tui;	}

	BEGIN_GOBJ_PURE(CAtpSequenceST,1);
		GELEM_VAR_INIT(DWORD,_flags,ATPF_LOOP);
			GELEM_EDITVAR("标志",GVT_U,GSem(GSem_Flags,"循环播放:2"),"标志");
		GELEM_VAR_INIT(StringID,_grp,StringID_Invalid);
		GELEM_VAR_INIT(StringID,_ap,StringID_Invalid);
			GELEM_EDITVAR("动画",GVT_U,GSem(GSem_StringID,"骨骼动画AnimPiece"),"动画名称");
		GELEM_OBJ(AtpTunerInfo,_tui);
			GELEM_EDITOBJ("Tuner信息","Tuner信息");
	END_GOBJ();    

public://take it as protected
	AtpTunerInfo _tui;
};

class CAtpBlend:public CAnimTreePad
{
public:
	DECLARE_CLASS(CAtpBlend);

	virtual const char *GetTypeName()	{		return "混合";	};
	virtual DWORD GetStubCount();
	virtual PadStub GetStub(DWORD idx);

	virtual DWORD GetChildCount()	{		return 2;	}

	virtual DbgType GetDbgType()	{		return Dbg_1D;	}

	virtual AtpTunerInfo *GetTunerInfo()	{		return &_tui;	}

	BEGIN_GOBJ_PURE(CAtpBlend,1);
		GELEM_VAR_INIT(DWORD,_flags,0);
		GELEM_OBJ(AtpTunerInfo,_tui);
			GELEM_EDITOBJ("Tuner信息","Tuner信息");
	END_GOBJ();    

public://take it as protected
	AtpTunerInfo _tui;

};

class CAtpBlendX:public CAnimTreePad
{
public:
	DECLARE_CLASS(CAtpBlendX);

	virtual const char *GetTypeName()	{		return "混合X";	};
	virtual DWORD GetStubCount();
	virtual PadStub GetStub(DWORD idx);

	virtual DWORD GetChildCount();

	virtual DbgType GetDbgType()	{		return Dbg_1D;	}

	virtual AtpTunerInfo *GetTunerInfo()	{		return &_tui;	}

	BEGIN_GOBJ_PURE(CAtpBlendX,1);
		GELEM_VAR_INIT(DWORD,_flags,0);
		GELEM_OBJ(AtpTunerInfo,_tui);
			GELEM_EDITOBJ("Tuner信息","Tuner信息");
		GELEM_VAR_INIT(DWORD,_nChilds,3);
			GELEM_EDITVAR("分支个数",GVT_U,GSem(GSem_Interger,"0,1,2,3,4,5,6,7,8,9"),"有几个分支");
		GELEM_VAR_INIT(float,_min,0.0f);
			GELEM_EDITVAR("最小值",GVT_F,GSem(GSem_Float,"-100000,100000,0.05"),"最小值");
		GELEM_VAR_INIT(float,_max,1.0f);
			GELEM_EDITVAR("最大值",GVT_F,GSem(GSem_Float,"-100000,100000,0.05"),"最大值");
	END_GOBJ();    

public://take it as protected
	AtpTunerInfo _tui;
	DWORD _nChilds;
	float _min;
	float _max;

};


#define SPEEDBLEND_MAXSPEED 30.0f
#define SPEEDBLEND_MAXSPEED_S "30.0f"

class CAtpSpeedBlend:public CAnimTreePad
{
public:
	DECLARE_CLASS(CAtpSpeedBlend);

	virtual const char *GetTypeName()	{		return "速度混合";	};
	virtual DWORD GetStubCount();
	virtual PadStub GetStub(DWORD idx);

	virtual DWORD GetChildCount()	{		return 4;	}

	virtual DbgType GetDbgType()	{		return Dbg_1D;	}
	virtual const char *GetDbgGroup()	{		return "MoveSpeed";	}
	virtual void ConvertDV(i_math::vector2df&dv)	{		dv.x=i_math::lerp(0.5f,20.0f,dv.x);	}

	BEGIN_GOBJ_PURE(CAtpSpeedBlend,1);
		GELEM_VAR_INIT(DWORD,_flags,0);
		GELEM_VAR_INIT(float,_min,1.0f);
			GELEM_EDITVAR("最小速度",GVT_F,GSem(GSem_Float,"0.00f,"SPEEDBLEND_MAXSPEED_S",0.05f"),"最小速度限制");
		GELEM_VAR_INIT(float,_max,10.0f);
			GELEM_EDITVAR("最大速度",GVT_F,GSem(GSem_Float,"0.00f,"SPEEDBLEND_MAXSPEED_S",0.05f"),"最大速度限制");

	END_GOBJ();    

public://take it as protected
	float _min;
	float _max;

};

class CAtpShiftBlend:public CAnimTreePad
{
public:
	DECLARE_CLASS(CAtpShiftBlend);

	virtual const char *GetTypeName()	{		return "方向混合";	};
	virtual DWORD GetStubCount();
	virtual PadStub GetStub(DWORD idx);

	virtual DWORD GetChildCount()	{		return 4;	}

	virtual DbgType GetDbgType()	{		return Dbg_1D;	}
	virtual const char *GetDbgGroup()	{		return "MoveDir";	}
	virtual void ConvertDV(i_math::vector2df&dv)	{		dv.x=dv.x*i_math::Pi*2.0f;	}

	BEGIN_GOBJ_PURE(CAtpShiftBlend,1);
		GELEM_VAR_INIT(DWORD,_flags,0);

	END_GOBJ();    

public://take it as protected

};

class CAtpMoveRotBlend:public CAnimTreePad
{
public:
	DECLARE_CLASS(CAtpMoveRotBlend);

	virtual const char *GetTypeName()	{		return "转身混合(移动)";	};
	virtual DWORD GetStubCount();
	virtual PadStub GetStub(DWORD idx);

	virtual DWORD GetChildCount()	{		return 3;	}

	virtual DbgType GetDbgType()	{		return Dbg_1D;	}
	virtual const char *GetDbgGroup()	{		return "MoveRotOff";	}
	virtual void ConvertDV(i_math::vector2df&dv)	{		dv.x=i_math::lerp(-180.0f,180.0f,dv.x);	}

	BEGIN_GOBJ_PURE(CAtpMoveRotBlend,1);
		GELEM_VAR_INIT(DWORD,_flags,0);
		GELEM_VAR_INIT(float,_max,60.0f);
			GELEM_EDITVAR("最大转身角度",GVT_F,GSem(GSem_Float,"0.00f,180.0,0.05f"),"最大转身角度");

	END_GOBJ();    

public://take it as protected
	float _max;

};

class CAtpMoveStartRotBlend:public CAnimTreePad
{
public:
	DECLARE_CLASS(CAtpMoveStartRotBlend);

	virtual const char *GetTypeName()	{		return "转身混合(起步)";	};
	virtual DWORD GetStubCount();
	virtual PadStub GetStub(DWORD idx);

	virtual DWORD GetChildCount()	{		return 3;	}

	virtual DbgType GetDbgType()	{		return Dbg_1D;	}
	virtual const char *GetDbgGroup()	{		return "MoveStartRotOff";	}
	virtual void ConvertDV(i_math::vector2df&dv)	{		dv.x=i_math::lerp(-180.0f,180.0f,dv.x);	}

	BEGIN_GOBJ_PURE(CAtpMoveStartRotBlend,1);
		GELEM_VAR_INIT(DWORD,_flags,0);
		GELEM_VAR_INIT(float,_max,60.0f);
			GELEM_EDITVAR("最大转身角度",GVT_F,GSem(GSem_Float,"0.00f,180.0,0.05f"),"最大转身角度");

	END_GOBJ();    

public://take it as protected
	float _max;

};

class CAtpRotOnSpotBlend:public CAnimTreePad
{
public:
	DECLARE_CLASS(CAtpRotOnSpotBlend);

	virtual const char *GetTypeName()	{		return "转身混合(原地转身)";	};
	virtual DWORD GetStubCount();
	virtual PadStub GetStub(DWORD idx);

	virtual DWORD GetChildCount()	{		return 6;	}

	virtual DbgType GetDbgType()	{		return Dbg_1D;	}
	virtual const char *GetDbgGroup()	{		return "RotOnSpotOff";	}
	virtual void ConvertDV(i_math::vector2df&dv)	{		dv.x=i_math::lerp(-180.0f,180.0f,dv.x);	}

	BEGIN_GOBJ_PURE(CAtpRotOnSpotBlend,1);
		GELEM_VAR_INIT(DWORD,_flags,0);
		GELEM_VAR_INIT(float,_angleX,90.0f);
			GELEM_EDITVAR("转身角度X",GVT_F,GSem(GSem_Float,"0.00f,180.0,0.05f"),"转身角度中间值");
	END_GOBJ();    

public://take it as protected
	float _angleX;

};



struct SwitchDur
{
	BEGIN_GOBJ_PURE(SwitchDur,1);
		GELEM_VAR_INIT(DWORD,iFrom,0);
			GELEM_EDITVAR("切出",GVT_U,GSem(GSem_Interger,"任意:-1,0:0,1:1,2:2,3:3,4:4,5:5,6:6,7:7,8:8,9:9"),"从第几个child切换");
		GELEM_VAR_INIT(DWORD,iTo,1);
			GELEM_EDITVAR("切入",GVT_U,GSem(GSem_Interger,"任意:-1,0:0,1:1,2:2,3:3,4:4,5:5,6:6,7:7,8:8,9:9"),"切到第几个child");
		GELEM_VAR_INIT(float,dur,0.1f);
			GELEM_EDITVAR("过渡时间",GVT_F,GSem(GSem_Float,"0.0f,2.0f,0.02f"),"两个动画切换时的过渡时间");
	END_GOBJ();    

	int iFrom;
	int iTo;
	float dur;
};



#define GELEM_SWITCH_BASE()																																		\
		GELEM_VAR_INIT(DWORD,_flags,0);																															\
		GELEM_VAR_INIT(float,_dur,0.1f);																																\
			GELEM_EDITVAR("过渡时间",GVT_F,GSem(GSem_Float,"0.0f,2.0f,0.02f"),"两个动画切换时的过渡时间");			\
		GELEM_OBJVECTOR(SwitchDur,_durs);																														\
			GELEM_EDITOBJ("过渡时间(额外)","额外声明的过渡时间");


class CAtpSwitchBase:public CAnimTreePad
{
public:

	virtual BOOL CanModifyChild()	{		return FALSE;	}//是否能对child的名字进行修改,并且增加/删除child

	virtual DbgType GetDbgType()	{		return Dbg_1D;	}

public://take it as protected
	float _dur;
	std::vector<SwitchDur> _durs;
};


class CAtpSwitch:public CAtpSwitchBase
{
public:
	DECLARE_CLASS(CAtpSwitch);

	CAtpSwitch()
	{
		GConstructor();
		_childs.resize(4);
	}
	~CAtpSwitch()
	{
		GDestructor();
	}

	virtual const char *GetTypeName()	{		return "切换";	};

	virtual BOOL CanModifyChild()	{		return TRUE;	}//是否能对child的名字进行修改,并且增加/删除child

	virtual AtpTunerInfo *GetTunerInfo()	{		return &_tui;	}

	BEGIN_GOBJ(CAtpSwitch,1);
		GELEM_SWITCH_BASE();
		GELEM_OBJ(AtpTunerInfo,_tui);
			GELEM_EDITOBJ("Tuner信息","Tuner信息");
	END_GOBJ();    

protected:
	AtpTunerInfo _tui;
};

class CAtpSwitch_Move:public CAtpSwitchBase
{
public:
	DECLARE_CLASS(CAtpSwitch_Move);

	CAtpSwitch_Move()
	{
		GConstructor();
		_childs.resize(2);
		_childs[0].name="NotMove";
		_childs[1].name="Move";
	}
	~CAtpSwitch_Move()
	{
		GDestructor();
	}


	virtual const char *GetTypeName()	{		return "切换-Move";	};
	virtual const char *GetDbgGroup()	{		return "Move";	};

	BEGIN_GOBJ(CAtpSwitch_Move,1);
		GELEM_SWITCH_BASE();
	END_GOBJ();    

};

class CAtpSwitch_Fly:public CAtpSwitchBase
{
public:
	DECLARE_CLASS(CAtpSwitch_Fly);

	CAtpSwitch_Fly()
	{
		GConstructor();
		_childs.resize(2);
		_childs[0].name="NotFly";
		_childs[1].name="Fly";
	}
	~CAtpSwitch_Fly()
	{
		GDestructor();
	}


	virtual const char *GetTypeName()	{		return "切换-Fly";	};
	virtual const char *GetDbgGroup()	{		return "Fly";	};

	BEGIN_GOBJ(CAtpSwitch_Fly,1);
		GELEM_SWITCH_BASE();
	END_GOBJ();    

};


class CAtpSwitch_Jump:public CAtpSwitchBase
{
public:
	DECLARE_CLASS(CAtpSwitch_Jump);

	CAtpSwitch_Jump()
	{
		GConstructor();
		_childs.resize(3);
		_childs[0].name="NotJump";
		_childs[1].name="Jump";
		_childs[2].name="JumpLand";
	}
	~CAtpSwitch_Jump()
	{
		GDestructor();
	}


	virtual const char *GetTypeName()	{		return "切换-Jump";	};
	virtual const char *GetDbgGroup()	{		return "Jump";	};

	BEGIN_GOBJ(CAtpSwitch_Jump,1);
		GELEM_SWITCH_BASE();
	END_GOBJ();    
};

class CAtpSwitch_Turn:public CAtpSwitchBase
{
public:
	DECLARE_CLASS(CAtpSwitch_Turn);

	CAtpSwitch_Turn()
	{
		GConstructor();
		_childs.resize(3);
		_childs[0].name="NotTurn";
		_childs[1].name="TurnLeft";
		_childs[2].name="TurnRight";
	}
	~CAtpSwitch_Turn()
	{
		GDestructor();
	}

	virtual const char *GetTypeName()	{		return "切换-Turn";	};
	virtual const char *GetDbgGroup()	{		return "Turn";	};

	BEGIN_GOBJ(CAtpSwitch_Turn,1);
		GELEM_SWITCH_BASE();
	END_GOBJ();    
};

class CAtpSwitch_Slide:public CAtpSwitchBase
{
public:
	DECLARE_CLASS(CAtpSwitch_Slide);

	CAtpSwitch_Slide()
	{
		GConstructor();
		_childs.resize(3);
		_childs[0].name="NotSlide";
		_childs[1].name="SlideLeft";
		_childs[2].name="SlideRight";
	}
	~CAtpSwitch_Slide()
	{
		GDestructor();
	}

	virtual const char *GetTypeName()	{		return "切换-Slide";	};
	virtual const char *GetDbgGroup()	{		return "Slide";	};

	BEGIN_GOBJ(CAtpSwitch_Slide,1);
	GELEM_SWITCH_BASE();
	END_GOBJ();    
};



struct NameCases
{
	DWORD GetStubCount();
	PadStub GetStub(DWORD idx);
	DWORD GetChildCount();
	StringID *GetDbgNames(DWORD &count);

	int DecideSwitch(StringID nm)
	{
		if (nm!=StringID_Invalid)
		{
			for(int i=0;i<nms.size();i++)
			{
// 				const char *s=StrLib_GetStr(nm);
// 				const char *s2=StrLib_GetStr(nms[i]);
				if (nms[i]==nm)
					return i;
			}
		}
		return -1;
	}
	std::vector<StringID> nms;
};

struct NameCasesPosture:public NameCases
{
	BEGIN_GOBJ_PURE(NameCasesPosture,1);
		GELEM_VARVECTOR_INIT(StringID,nms,StringID_Invalid);
			GELEM_EDITVAR("名称",GVT_U,GSem(GSem_StringID,"PostureType"),"名称");
	END_GOBJ();    
};


class CAtpSwitch_Posture:public CAtpSwitchBase,public NameCases
{
public:
	DECLARE_CLASS(CAtpSwitch_Posture);

	virtual DWORD GetStubCount()	{		return _cases.GetStubCount();	}
	virtual PadStub GetStub(DWORD idx)	{		return _cases.GetStub(idx);	}
	virtual DWORD GetChildCount()	{		return _cases.GetChildCount();	}

	virtual const char *GetTypeName()	{		return "切换-Posture";	};
	virtual const char *GetDbgGroup()	{		return "Posture";	};
	virtual DbgType GetDbgType()	{		return Dbg_Name;	}
	StringID *GetDbgNames(DWORD &count)	{		return _cases.GetDbgNames(count);	}

	BEGIN_GOBJ_PURE(CAtpSwitch_Posture,1);
		GELEM_SWITCH_BASE();
		GELEM_OBJ(NameCasesPosture,_cases);
			GELEM_EDITOBJ("切换条件","切换条件");
	END_GOBJ();    
	NameCasesPosture _cases;
};

struct NameCasesTunerString:public NameCases
{
	BEGIN_GOBJ_PURE(NameCasesTunerString,1);
		GELEM_VARVECTOR_INIT(StringID,nms,StringID_Invalid);
			GELEM_EDITVAR("名称",GVT_U,GSem(GSem_StringID,"TunerString"),"名称");
	END_GOBJ();    
};


class CAtpSwitch_TunerString:public CAtpSwitchBase,public NameCases
{
public:
	DECLARE_CLASS(CAtpSwitch_TunerString);

	virtual DWORD GetStubCount()	{		return _cases.GetStubCount();	}
	virtual PadStub GetStub(DWORD idx)	{		return _cases.GetStub(idx);	}
	virtual DWORD GetChildCount()	{		return _cases.GetChildCount();	}

	virtual const char *GetTypeName()	{		return "切换-Tuner字符串";	};
	virtual const char *GetDbgGroup()	{		return "Tuner字符串";	};
	virtual DbgType GetDbgType()	{		return Dbg_Name;	}
	StringID *GetDbgNames(DWORD &count)	{		return _cases.GetDbgNames(count);	}
	virtual AtpTunerInfo *GetTunerInfo()	{		return &_tui;	}

	BEGIN_GOBJ_PURE(CAtpSwitch_TunerString,1);
		GELEM_SWITCH_BASE();
		GELEM_OBJ(NameCasesTunerString,_cases);
			GELEM_EDITOBJ("切换条件","切换条件");
		GELEM_OBJ(AtpTunerInfo,_tui);
			GELEM_EDITOBJ("Tuner信息","Tuner信息");
	END_GOBJ();    
	NameCasesTunerString _cases;
	AtpTunerInfo _tui;
};


struct NameCasesAct:public NameCases
{
	BEGIN_GOBJ_PURE(NameCasesAct,1);
		GELEM_VARVECTOR_INIT(StringID,nms,StringID_Invalid);
			GELEM_EDITVAR("名称",GVT_U,GSem(GSem_StringID,"ActType"),"名称");
	END_GOBJ();    
};

struct SwitchDurAct
{
	BEGIN_GOBJ_PURE(SwitchDurAct,1);
		GELEM_VAR_INIT(StringID,nmFrom,StringID_Invalid);
			GELEM_EDITVAR("切出",GVT_U,GSem(GSem_StringID,"ActType"),"名称");
		GELEM_VAR_INIT(StringID,nmTo,StringID_Invalid);
			GELEM_EDITVAR("切入",GVT_U,GSem(GSem_StringID,"ActType"),"名称");
		GELEM_VAR_INIT(float,dur,0.1f);
			GELEM_EDITVAR("过渡时间",GVT_F,GSem(GSem_Float,"0.0f,2.0f,0.02f"),"两个动画切换时的过渡时间");
	END_GOBJ();    

	StringID nmFrom;
	StringID nmTo;
	float dur;
};

class CAtpSwitch_Act:public CAtpSwitchBase
{
public:
	DECLARE_CLASS(CAtpSwitch_Act);


	virtual DWORD GetStubCount()	{		return _cases.GetStubCount();	}
	virtual PadStub GetStub(DWORD idx)	{		return _cases.GetStub(idx);	}
	virtual DWORD GetChildCount()	{		return _cases.GetChildCount();	}

	virtual const char *GetTypeName()	{		return "切换-Act";	};
	virtual const char *GetDbgGroup()	{		return "Act";	};
	virtual DbgFlag GetDbgFlag()	{		return DbgF_AutoReset;	}
	virtual DbgType GetDbgType()	{		return Dbg_Name;	}
	StringID *GetDbgNames(DWORD &count)	{		return _cases.GetDbgNames(count);	}

	BEGIN_GOBJ_PURE(CAtpSwitch_Act,1);
		GELEM_SWITCH_BASE();
		GELEM_OBJVECTOR(SwitchDurAct,_durs2);
			GELEM_EDITOBJ("过渡时间(Act)","Act额外声明的过渡时间");
		GELEM_OBJ(NameCasesAct,_cases);
			GELEM_VERSION(2)
			GELEM_EDITOBJ("切换条件","切换条件");
	END_GOBJ();    

public://take it as protected
	NameCasesAct _cases;
	std::vector<SwitchDurAct> _durs2;
};

class CAtpSwitch_ActSub:public CAtpSwitchBase
{
public:
	DECLARE_CLASS(CAtpSwitch_ActSub);

	virtual DWORD GetStubCount();
	virtual PadStub GetStub(DWORD idx);
	virtual DWORD GetChildCount();


	virtual const char *GetTypeName()	{		return "切换-ActSub";	};
	virtual const char *GetDbgGroup()	{		return "ActSub";	};

	BEGIN_GOBJ_PURE(CAtpSwitch_ActSub,1);
		GELEM_SWITCH_BASE();
		GELEM_VAR_INIT(DWORD,_count,3);
			GELEM_EDITVAR("Sub个数",GVT_U,GSem(GSem_Interger,"0,1,2,3,4,5,6,7,8,9"),"有几个Sub");
	END_GOBJ();    

	DWORD _count;
};


class CAtpSwitch_Auto:public CAtpSwitchBase
{
public:
	DECLARE_CLASS(CAtpSwitch_Auto);

	CAtpSwitch_Auto()
	{
		GConstructor();
		_childs.resize(3);
		_childs[0].name="Lvl0";
		_childs[1].name="Lvl1";
		_childs[2].name="Lvl2";
	}
	~CAtpSwitch_Auto()
	{
		GDestructor();
	}

	virtual const char *GetTypeName()	{		return "切换-Auto";	};
	virtual DbgType GetDbgType()	{		return Dbg_None;	}
	virtual const char *GetDbgGroup()	{		return "";	};

	BEGIN_GOBJ(CAtpSwitch_Auto,1);
		GELEM_SWITCH_BASE();
	END_GOBJ();    
};

class CAtpSwitch_AutoX:public CAtpSwitchBase
{
public:
	DECLARE_CLASS(CAtpSwitch_AutoX);

	virtual DWORD GetStubCount();
	virtual PadStub GetStub(DWORD idx);
	virtual DWORD GetChildCount();


	virtual const char *GetTypeName()	{		return "切换-AutoX";	};
	virtual DbgType GetDbgType()	{		return Dbg_None;	}
	virtual const char *GetDbgGroup()	{		return "";	};

	BEGIN_GOBJ_PURE(CAtpSwitch_AutoX,1);
		GELEM_SWITCH_BASE();
		GELEM_VAR_INIT(DWORD,_nLevels,3);
			GELEM_EDITVAR("Level个数",GVT_U,GSem(GSem_Interger,"0,1,2,3,4,5,6,7,8,9"),"有几个Level");
	END_GOBJ();    
	DWORD _nLevels;

};

//移动动画(起步,行走,停步等)
#define AvtrLocoChild_NotMove 0
#define AvtrLocoChild_StartFw 1
#define AvtrLocoChild_StartL 2
#define AvtrLocoChild_StartR 3
#define AvtrLocoChild_MoveL 4
#define AvtrLocoChild_MoveR 5
#define AvtrLocoChild_StopLPass 6
#define AvtrLocoChild_StopL 7
#define AvtrLocoChild_StopRPass 8
#define AvtrLocoChild_StopR 9
#define AvtrLocoChild_RotateOnSpot 10
#define AvtrLocoChild_Count 11 

#define AvtrLocoChild_IsStop(iChild) ((iChildFrom>=AvtrLocoChild_StopLPass)&&(iChildFrom<=AvtrLocoChild_StopR))
class CAtpSwitch_AvtrLoco:public CAtpSwitchBase
{
public:
	DECLARE_CLASS(CAtpSwitch_AvtrLoco);

	virtual const char *GetTypeName()	{		return "切换-Avtr移动";	};

	BEGIN_GOBJ_PURE(CAtpSwitch_AvtrLoco,1);
		GELEM_SWITCH_BASE();
		GELEM_VAR_INIT(AnimTick,durStartFw,ANIMTICK_FROM_SECOND(0.5f));
			GELEM_EDITVAR("起步(前方)持续时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"起步(前方)持续时间");
		GELEM_VAR_INIT(AnimTick,durStartRot,ANIMTICK_FROM_SECOND(1.0f));
			GELEM_EDITVAR("起步(转身)持续时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"起步(前方)持续时间");
		GELEM_VAR_INIT(AnimTick,durStop,ANIMTICK_FROM_SECOND(1.0f));
			GELEM_EDITVAR("停步持续时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"停步持续时间");
		GELEM_VAR_INIT(float,durStopSwitch,0.5f);
			GELEM_EDITVAR("过渡时间(停步到站立)",GVT_F,GSem(GSem_Float,"0,10,0.1"),"停步到站立的过渡时间");
		GELEM_VAR_INIT(float,durRotOnSpot,1.0f);
			GELEM_EDITVAR("原地转身持续时间",GVT_F,GSem(GSem_Float,"0,100,0.1"),"原地转身持续时间");

		GELEM_VAR_INIT(AvtrFootStep,fsInitial,AvtrFootStep_LeftPass);
			GELEM_EDITVAR("初始脚步",GVT_U,GSem(GSem_Interger,GSemConstraint_AvtrFootStep),"移动开始时的脚步");
		GELEM_VAR_INIT(AvtrFootStep,fsStartFw,AvtrFootStep_LeftPass);
			GELEM_EDITVAR("起步脚步(前方)",GVT_U,GSem(GSem_Interger,GSemConstraint_AvtrFootStep),"前方起步时的脚步");
		GELEM_VAR_INIT(AvtrFootStep,fsStartL,AvtrFootStep_LeftPass);
			GELEM_EDITVAR("起步脚步(左转)",GVT_U,GSem(GSem_Interger,GSemConstraint_AvtrFootStep),"左转起步时的脚步");
		GELEM_VAR_INIT(AvtrFootStep,fsStartR,AvtrFootStep_RightPass);
			GELEM_EDITVAR("起步脚步(右转)",GVT_U,GSem(GSem_Interger,GSemConstraint_AvtrFootStep),"右转起步时的脚步");

		GELEM_VAR_INIT(DWORD,dbgStartType,0);
			GELEM_EDITVAR("(调试)起步方向",GVT_U,GSem(GSem_Interger,"前方:0,左转:1,右转:2"),"起步方向");
	END_GOBJ();    

public://take it as protected
	virtual DWORD GetStubCount();
	virtual PadStub GetStub(DWORD idx);
	virtual DWORD GetChildCount()	{		return AvtrLocoChild_Count;	}

	virtual const char *GetDbgGroup()	{		return "AvtrMove";	};
	virtual DbgFlag GetDbgFlag()	{		return DbgF_AutoReset;	}
	virtual DbgType GetDbgType()	{		return Dbg_Name;	}
	StringID *GetDbgNames(DWORD &count);
	StringID GetDbgName_NotMove();
	StringID GetDbgName_Start();
	StringID GetDbgName_Move();
	StringID GetDbgName_Stop();
	StringID GetDbgName_RotOnSpot();

	AnimTick durStartFw;
	AnimTick durStartRot;
	AnimTick durStop;
	float durStopSwitch;//停步动画到站立动画的切换时间
	float durRotOnSpot;

	int dbgStartType;

	AvtrFootStep fsStartFw;
	AvtrFootStep fsStartL;
	AvtrFootStep fsStartR;
	AvtrFootStep fsInitial;

};


#define GELEM_PARTIALSWITCH_BASE()																															\
	GELEM_VAR_INIT(DWORD,_flags,0);																																\
	GELEM_STRING_INIT(_root0,"");																																	\
		GELEM_EDITVAR("起始骨骼#1",GVT_String,GSem(GSem_Name,"BoneName"),"起始骨骼以下的骨骼参与混合");		\
	GELEM_STRING_INIT(_root1,"");																																	\
		GELEM_EDITVAR("起始骨骼#2",GVT_String,GSem(GSem_Name,"BoneName"),"起始骨骼以下的骨骼参与混合");		\
	GELEM_STRING_INIT(_root2,"");																																	\
		GELEM_EDITVAR("起始骨骼#3",GVT_String,GSem(GSem_Name,"BoneName"),"起始骨骼以下的骨骼参与混合");		\
	GELEM_STRING_INIT(_root3,"");																																	\
		GELEM_EDITVAR("起始骨骼#4",GVT_String,GSem(GSem_Name,"BoneName"),"起始骨骼以下的骨骼参与混合");		\
	GELEM_VAR_INIT(BOOL,_bNoChildren,FALSE);																											\
		GELEM_EDITVAR("不计算子骨骼",GVT_S,GSem_Boolean,"是否不计算子骨骼");																\
	GELEM_VAR_INIT(float,_dur,0.2f);																																	\
		GELEM_EDITVAR("过渡时间",GVT_F,GSem(GSem_Float,"0.01f,2.0f,0.02f"),"两个动画切换时的过渡时间");

class CAtpPartialSwitchBase:public CAnimTreePad
{
public:
	virtual DbgType GetDbgType()	{		return Dbg_1D;	}

public://take it as protected

	std::string _root0;//
	std::string _root1;//
	std::string _root2;//
	std::string _root3;//
	BOOL _bNoChildren;
	float _dur;
};


class CAtpPartialSwitch:public CAtpPartialSwitchBase
{
public:
	DECLARE_CLASS(CAtpPartialSwitch);

	virtual const char *GetTypeName()	{		return "部分骨骼切换";	};
	virtual DWORD GetStubCount();
	virtual PadStub GetStub(DWORD idx);

	virtual DWORD GetChildCount()	{		return 5;	}

	virtual AtpTunerInfo *GetTunerInfo()	{		return &_tui;	}

	BEGIN_GOBJ_PURE(CAtpPartialSwitch,1);
		GELEM_PARTIALSWITCH_BASE()
		GELEM_OBJ(AtpTunerInfo,_tui);
			GELEM_EDITOBJ("Tuner信息","Tuner信息");
	END_GOBJ();    

protected:
	AtpTunerInfo _tui;
};



class CAtpPartialSwitch_Auto:public CAtpPartialSwitchBase
{
public:
	DECLARE_CLASS(CAtpPartialSwitch_Auto);

	CAtpPartialSwitch_Auto()
	{
		GConstructor();
		_childs.resize(3);
		_childs[0].name="Default";
		_childs[1].name="Lvl0";
		_childs[2].name="Lvl1";
	}
	~CAtpPartialSwitch_Auto()
	{
		GDestructor();
	}
	virtual const char *GetTypeName()	{		return "部分骨骼切换-Auto";	};
	virtual const char *GetDbgGroup()	{		return "";	};
	virtual DbgType GetDbgType()	{		return Dbg_None;	}

	BEGIN_GOBJ(CAtpPartialSwitch_Auto,1);
		GELEM_PARTIALSWITCH_BASE()
	END_GOBJ();    

public://take it as protected
};

class CAtpPartialBlend:public CAnimTreePad
{
public:
	DECLARE_CLASS(CAtpPartialBlend);

	virtual const char *GetTypeName()	{		return "部分骨骼混合";	};
	virtual DWORD GetStubCount();
	virtual PadStub GetStub(DWORD idx);

	virtual DWORD GetChildCount()	{		return 2;	}

	virtual DbgType GetDbgType()	{		return Dbg_1D;	}

	virtual AtpTunerInfo *GetTunerInfo()	{		return &_tui;	}

	BEGIN_GOBJ_PURE(CAtpPartialBlend,1);
		GELEM_VAR_INIT(DWORD,_flags,0);
		GELEM_STRING_INIT(_root,"");
			GELEM_EDITVAR("起始骨骼",GVT_String,GSem(GSem_Name,"BoneName"),"起始骨骼以下的骨骼参与混合");
		GELEM_VAR_INIT(BOOL,_bNoChildren,FALSE);
			GELEM_EDITVAR("不计算子骨骼",GVT_S,GSem_Boolean,"是否不计算子骨骼");
		GELEM_OBJ(AtpTunerInfo,_tui);
			GELEM_EDITOBJ("Tuner信息","Tuner信息");
		GELEM_VAR_INIT(BOOL,_bForceFullBlend,0);
			GELEM_EDITVAR("强制FullBlend",GVT_S,GSem_Boolean,"设为TRUE表示永远使用最大Weight(1.0)混合")
	END_GOBJ();    

public://take it as protected
	std::string _root;//
	BOOL _bNoChildren;
	AtpTunerInfo _tui;
	BOOL _bForceFullBlend;
};

struct AtpBoneCtrlInfo
{
	std::string nmBone;//
	float xOff;
	float yOff;
	float zOff;
	float scale;

	BEGIN_GOBJ_PURE(AtpBoneCtrlInfo,1);
		GELEM_STRING_INIT(nmBone,"");
			GELEM_EDITVAR("骨骼",GVT_String,GSem(GSem_Name,"BoneName"),"骨骼名称");
		GELEM_VAR_INIT(float,xOff,0.0f);
			GELEM_EDITVAR("x方向位移偏移",GVT_F,GSem(GSem_Float,"-10.00f,10.0f,0.01f"),"x方向位移偏移调整");
		GELEM_VAR_INIT(float,yOff,0.0f);
			GELEM_EDITVAR("y方向位移偏移",GVT_F,GSem(GSem_Float,"-10.00f,10.0f,0.01f"),"y方向位移偏移调整");
		GELEM_VAR_INIT(float,zOff,0.0f);
			GELEM_EDITVAR("z方向位移偏移",GVT_F,GSem(GSem_Float,"-10.00f,10.0f,0.01f"),"z方向位移偏移调整");

		GELEM_VAR_INIT(float,scale,1.0f);
			GELEM_EDITVAR("缩放调整",GVT_F,GSem(GSem_Float,"-0.05f,5.0f,0.01f"),"缩放调整");
	END_GOBJ();    

};

class CAtpBoneCtrl:public CAnimTreePad
{
public:
	DECLARE_CLASS(CAtpBoneCtrl);

	virtual const char *GetTypeName()	{		return "骨骼控制";	};
	virtual DWORD GetStubCount();
	virtual PadStub GetStub(DWORD idx);

	virtual DWORD GetChildCount()	{		return 1;	}

	virtual Category GetCategory()	{		return BoneCtrl;	}
	virtual DbgType GetDbgType()	{		return Dbg_1D;	}

	BEGIN_GOBJ_PURE(CAtpBoneCtrl,1);
		GELEM_VAR_INIT(DWORD,_flags,0);
		GELEM_OBJVECTOR(AtpBoneCtrlInfo,_infos);
			GELEM_EDITOBJ("调整骨骼列表","各个骨骼的调整信息");
	END_GOBJ();    

public://take it as protected
	std::vector<AtpBoneCtrlInfo> _infos;
};

class CAtpBoneCtrlMerge:public CAnimTreePad
{
public:
	DECLARE_CLASS(CAtpBoneCtrlMerge);

	virtual const char *GetTypeName()	{		return "骨骼控制叠加";	};
	virtual DWORD GetStubCount();
	virtual PadStub GetStub(DWORD idx);

	virtual DWORD GetChildCount();

	virtual Category GetCategory()	{		return BoneCtrl;	}
	virtual DbgType GetDbgType()	{		return Dbg_1D;	}

	BEGIN_GOBJ_PURE(CAtpBoneCtrlMerge,1);
		GELEM_VAR_INIT(DWORD,_flags,0);
		GELEM_VAR_INIT(DWORD,_nChild,3);
			GELEM_EDITVAR("Child个数",GVT_U,GSem(GSem_Interger,"0,1,2,3,4,5,6,7,8,9"),"有几个Child");
	END_GOBJ();    

public://take it as protected
	DWORD _nChild;

};

class CAtpBoneCtrlBlend:public CAnimTreePad
{
public:
	DECLARE_CLASS(CAtpBoneCtrlBlend);

	virtual const char *GetTypeName()	{		return "骨骼控制混合";	};
	virtual DWORD GetStubCount();
	virtual PadStub GetStub(DWORD idx);

	virtual DWORD GetChildCount()	{		return 2;	}

	virtual Category GetCategory()	{		return BoneCtrl;	}
	virtual DbgType GetDbgType()	{		return Dbg_1D;	}

	virtual AtpTunerInfo *GetTunerInfo()	{		return &_tui;	}

	BEGIN_GOBJ_PURE(CAtpBoneCtrlBlend,1);
		GELEM_VAR_INIT(DWORD,_flags,0);
		GELEM_OBJ(AtpTunerInfo,_tui);
			GELEM_EDITOBJ("Tuner信息","Tuner信息");
	END_GOBJ();    

public://take it as protected
	AtpTunerInfo _tui;
};

class CAtpBoneCtrlChainStretch:public CAnimTreePad
{
public:
	DECLARE_CLASS(CAtpBoneCtrlChainStretch);

	virtual const char *GetTypeName()	{		return "骨骼控制锁链伸展";	};
	virtual DWORD GetStubCount();
	virtual PadStub GetStub(DWORD idx);

	virtual DWORD GetChildCount()	{		return 2;	}

	virtual Category GetCategory()	{		return BoneCtrl;	}
	virtual DbgType GetDbgType()	{		return Dbg_1D;	}

	virtual AtpTunerInfo *GetTunerInfo()	{		return &_tui;	}

	BEGIN_GOBJ_PURE(CAtpBoneCtrlChainStretch,1);
		GELEM_VAR_INIT(DWORD,_flags,0);
		GELEM_STRING_INIT(_start,"");
			GELEM_EDITVAR("开始骨骼",GVT_String,GSem(GSem_Name,"BoneName"),"从哪根骨骼开始");
		GELEM_STRING_INIT(_end,"");
			GELEM_EDITVAR("结束骨骼",GVT_String,GSem(GSem_Name,"BoneName"),"到哪根骨骼结束");
		GELEM_OBJ(AtpTunerInfo,_tui);
			GELEM_EDITOBJ("Tuner信息","Tuner信息");
	END_GOBJ();    

public://take it as protected
	AtpTunerInfo _tui;
	std::string _start;
	std::string _end;

};

class CAtpBoneCtrlEel:public CAnimTreePad
{
public:
	DECLARE_CLASS(CAtpBoneCtrlEel);

	virtual const char *GetTypeName()	{		return "骨骼控制Eel";	};
	virtual DWORD GetStubCount();
	virtual PadStub GetStub(DWORD idx);

	virtual DWORD GetChildCount()	{		return 0;	}

	virtual Category GetCategory()	{		return BoneCtrl;	}
	virtual DbgType GetDbgType()	{		return Dbg_1D;	}

	virtual AtpTunerInfo *GetTunerInfo()	{		return &_tui;	}

	BEGIN_GOBJ_PURE_UID2(CAtpBoneCtrlEel,497,1);
		GELEM_VAR_INIT(DWORD,_flags,0);
		GELEM_STRING_INIT(_end,"");
			GELEM_EDITVAR("结束骨骼",GVT_String,GSem(GSem_Name,"BoneName"),"到哪根骨骼结束");
		GELEM_OBJ(AtpTunerInfo,_tui);
			GELEM_EDITOBJ("Tuner信息","Tuner信息");
	END_GOBJ();    

public://take it as protected
	AtpTunerInfo _tui;
	std::string _end;

};


class CAtpIKCtrl_Base:public CAnimTreePad
{
public:
	virtual DWORD GetStubCount();
	virtual PadStub GetStub(DWORD idx);

	virtual DWORD GetChildCount()	{		return 0;	}

	virtual Category GetCategory()	{		return IKCtrl;	}
	virtual DbgType GetDbgType()	{		return Dbg_1D;	}

	virtual AtpTunerInfo *GetTunerInfo()
	{
		_tuiDbg.idNm=_nmEffector;
		return &_tuiDbg;
	}

public:
	StringID _nmEffector;
	AnimTick _durBlendIn;
	AnimTick _durBlendOut;
	AtpTunerInfo _tuiDbg;

};

class CAtpIKCtrl_Chain:public CAtpIKCtrl_Base
{
public:
	DECLARE_CLASS(CAtpIKCtrl_Chain);

	virtual const char *GetTypeName()	{		return "ChainIK";	};

	BEGIN_GOBJ_PURE(CAtpIKCtrl_Chain,1);
		GELEM_VAR_INIT(StringID,_nmEffector,StringID_Invalid);
			GELEM_EDITVAR("Effector名称id",GVT_U,GSem(GSem_StringID,"动画树IKEffector名称"),"Effector名称id");
		GELEM_VAR_INIT(AnimTick,_durBlendIn,ANIMTICK_FROM_SECOND(0.5f));
			GELEM_EDITVAR("BlendIn时间",GVT_U,GSem(GSem_AnimTick,"0.01,100,0.1"),"BlendIn时间");
		GELEM_VAR_INIT(AnimTick,_durBlendOut,ANIMTICK_FROM_SECOND(0.5f));
			GELEM_EDITVAR("BlendOut时间",GVT_U,GSem(GSem_AnimTick,"0.01,100,0.1"),"BlendOut时间");
	END_GOBJ();    

public://take it as protected

};

class CAtpIKCtrl_Simple:public CAtpIKCtrl_Base
{
public:
	DECLARE_CLASS(CAtpIKCtrl_Simple);

	virtual const char *GetTypeName()	{		return "SimpleIK";	};

	BEGIN_GOBJ_PURE(CAtpIKCtrl_Simple,1);
		GELEM_STRING_INIT(_root,"");
			GELEM_EDITVAR("起始骨骼",GVT_String,GSem(GSem_Name,"BoneName"),"起始骨骼以下的骨骼参与IK计算");
		GELEM_STRING_INIT(_nmEffectorRefBone,"");
			GELEM_EDITVAR("Effector参考骨骼",GVT_String,GSem(GSem_Name,"BoneName"),"Effector参考骨骼");
		GELEM_VAR_INIT(StringID,_nmEffector,StringID_Invalid);
			GELEM_EDITVAR("Effector名称id",GVT_U,GSem(GSem_StringID,"动画树IKEffector名称"),"Effector名称id");
		GELEM_VAR_INIT(AnimTick,_durBlendIn,ANIMTICK_FROM_SECOND(0.5f));
			GELEM_EDITVAR("BlendIn时间",GVT_U,GSem(GSem_AnimTick,"0.01,100,0.1"),"BlendIn时间");
		GELEM_VAR_INIT(AnimTick,_durBlendOut,ANIMTICK_FROM_SECOND(0.5f));
			GELEM_EDITVAR("BlendOut时间",GVT_U,GSem(GSem_AnimTick,"0.01,100,0.1"),"BlendOut时间");
	END_GOBJ();    

public://take it as protected
	std::string _root;//
	std::string _nmEffectorRefBone;

};

class CAtpIKCtrl_Custom:public CAtpIKCtrl_Base
{
public:
	DECLARE_CLASS(CAtpIKCtrl_Custom);

	virtual const char *GetTypeName()	{		return "CustomIK";	};

	BEGIN_GOBJ_PURE(CAtpIKCtrl_Custom,1);
		GELEM_VAR_INIT(StringID,_nmEffector,StringID_Invalid);
			GELEM_EDITVAR("Effector名称id",GVT_U,GSem(GSem_StringID,"动画树IKEffector名称"),"Effector名称id");
// 		GELEM_VAR_INIT(AnimTick,_durBlendIn,ANIMTICK_FROM_SECOND(0.5f));
// 			GELEM_EDITVAR("BlendIn时间",GVT_U,GSem(GSem_AnimTick,"0.01,100,0.1"),"BlendIn时间");
// 		GELEM_VAR_INIT(AnimTick,_durBlendOut,ANIMTICK_FROM_SECOND(0.5f));
// 			GELEM_EDITVAR("BlendOut时间",GVT_U,GSem(GSem_AnimTick,"0.01,100,0.1"),"BlendOut时间");
	END_GOBJ();    

public://take it as protected

};


class CAtpFloatST:public CAnimTreePad
{
public:
	DECLARE_CLASS(CAtpFloatST);

	virtual const char *GetTypeName()	{		return "Static数值";	};
	virtual DWORD GetStubCount();
	virtual PadStub GetStub(DWORD idx);

	virtual DbgType GetDbgType()	{		return Dbg_1D;	}
	virtual const char *GetDbgGroup()	{		return "";	}
	virtual void ConvertDV(i_math::vector2df&dv)	
	{		
		dv.x=i_math::lerp(_rng.low,_rng.hi,dv.x);	
	}

	virtual AtpTunerInfo *GetTunerInfo()	{		return &_tui;	}

	BEGIN_GOBJ_PURE(CAtpFloatST,1);
		GELEM_VAR_INIT( i_math::rangef,_rng,i_math::rangef(0.0f,1.0f));
			GELEM_EDITVAR( "取值范围", GVT_Fx2,GSem_Range,"取值范围");
		GELEM_OBJ(AtpTunerInfo,_tui);
			GELEM_EDITOBJ("Tuner信息","Tuner信息");
	END_GOBJ();    

public://take it as protected
	i_math::rangef _rng;
	AtpTunerInfo _tui;

};

class CAtpPath:public CAnimTreePad
{
public:
	DECLARE_CLASS(CAtpPath);

	virtual const char *GetTypeName()	{		return "路径动画";	};
	virtual DWORD GetStubCount();
	virtual PadStub GetStub(DWORD idx);
	virtual const char *GetShowName()	{		return _path.c_str();	}

	virtual DWORD GetChildCount()	{		return 0;	}

	virtual Category GetCategory()	{		return Path;	}

	virtual DbgType GetDbgType()	{		return Dbg_None;	}

	virtual void CollectRefs(std::vector<std::string>&buf);


	BEGIN_GOBJ_PURE(CAtpPath,1);
		GELEM_VAR_INIT(DWORD,_flags,0);
			GELEM_EDITVAR("标志",GVT_U,GSem(GSem_Flags,"循环播放:2,不发送事件:8"),"标志");
		GELEM_STRING_INIT(_path,"");
			GELEM_EDITVAR("路径动画",GVT_String,GSem_XformAnimPath,"路径动画的资源");
		GELEM_VAR_INIT(float,_scale,1.0f);
			GELEM_EDITVAR("速度调整",GVT_F,GSem(GSem_Float,"0.00f,100.0f,0.05f"),"播放速度调整");
		GELEM_VAR_INIT(float,_minwt,0.1f);
			GELEM_EDITVAR("最小发送事件的权重",GVT_F,GSem(GSem_Float,"0.00f,1.0f,0.01f"),"当权重小于这个值时,将不发送事件");
		GELEM_VAR_INIT(StringID,_grp,StringID_Invalid);
			GELEM_EDITVAR("Sync Group",GVT_U,GSem(GSem_StringID,"SyncGroup"),"同步组的名称");
	END_GOBJ();    

public://take it as protected

	std::string _path;//资源路径名
	StringID _grp;//sync group
	float _scale;
	float _minwt;//发送事件的最小权重
};

//ST 代表Static
class CAtpPathST:public CAtpPath
{
public:
	DECLARE_CLASS(CAtpPathST);

	virtual const char *GetTypeName()	{		return "Static路径动画";	};

	virtual DbgType GetDbgType()	{		return Dbg_1D;	}
	virtual const char *GetDbgGroup()	{		return "";	}

	virtual AtpTunerInfo *GetTunerInfo()	{		return &_tui;	}

	BEGIN_GOBJ_PURE(CAtpPathST,1);
		GELEM_VAR_INIT(DWORD,_flags,ATPF_LOOP);
			GELEM_EDITVAR("标志",GVT_U,GSem(GSem_Flags,"循环播放:2"),"标志");
		GELEM_STRING_INIT(_path,"");
			GELEM_EDITVAR("路径动画",GVT_String,GSem_XformAnimPath,"路径动画的资源");
		GELEM_VAR_INIT(StringID,_grp,StringID_Invalid);
		GELEM_OBJ(AtpTunerInfo,_tui);
			GELEM_EDITOBJ("Tuner信息","Tuner信息");
	END_GOBJ();    

public://take it as protected
	AtpTunerInfo _tui;
};

struct ActSeqInfo
{
	DWORD flags;
	StringID ap;//
	StringID act;
	float scale;
	float minwt;//发送事件的最小权重

	BEGIN_GOBJ_PURE(ActSeqInfo,1);
		GELEM_VAR_INIT(DWORD,flags,0);
			GELEM_EDITVAR("标志",GVT_U,GSem(GSem_Flags,"循环播放:2,不发送事件:8,反向播放:16"),"标志");
		GELEM_VAR_INIT(StringID,act,StringID_Invalid);
			GELEM_EDITVAR("Act名称",GVT_U,GSem(GSem_StringID,"ActType"),"Act名称");
		GELEM_VAR_INIT(StringID,ap,StringID_Invalid);
			GELEM_EDITVAR("动画",GVT_U,GSem(GSem_StringID,"骨骼动画AnimPiece"),"动画名称");
		GELEM_VAR_INIT(float,scale,1.0f);
			GELEM_EDITVAR("速度调整",GVT_F,GSem(GSem_Float,"0.00f,100.0f,0.05f"),"播放速度调整");
		GELEM_VAR_INIT(float,minwt,0.1f);
			GELEM_EDITVAR("最小发送事件的权重",GVT_F,GSem(GSem_Float,"0.00f,1.0f,0.01f"),"当权重小于这个值时,将不发送事件");
	END_GOBJ();    

};

class CAtpCombo_Act:public CAnimTreePad
{
public:
	DECLARE_CLASS(CAtpCombo_Act);

	virtual const char *GetTypeName()	{		return "动画序列组合-Act";	};
	virtual DWORD GetStubCount();
	virtual PadStub GetStub(DWORD idx);
	virtual const char *GetShowName()	{		return "";	}

	virtual DWORD GetChildCount()	{		return 0;	}

	virtual Category GetCategory()	{		return Sequence;	}

	virtual const char *GetDbgGroup()	{		return "Act";	};
	virtual DbgFlag GetDbgFlag()	{		return DbgF_AutoReset;	}
	virtual DbgType GetDbgType()	{		return Dbg_Name;	}
	StringID *GetDbgNames(DWORD &count);


	BEGIN_GOBJ_PURE(CAtpCombo_Act,1);
		GELEM_OBJVECTOR(ActSeqInfo,_acts);
			GELEM_EDITOBJ("动画列表","各个Act的动画");
	END_GOBJ();    

	std::vector<ActSeqInfo>_acts;


};

struct PostureTrans
{
	std::string nm;
	StringID from;
	StringID to;
	float dur;
	BEGIN_GOBJ_PURE(PostureTrans,1);
		GELEM_STRING_INIT(nm,"");
			GELEM_EDITVAR("过渡名称",GVT_String,GSem_Name,"过渡的名称(小于15个字节)");
		GELEM_VAR_INIT(StringID,from,StringID_Invalid);
			GELEM_EDITVAR("原始Posture",GVT_U,GSem(GSem_StringID,"PostureType"),"从哪个Posture开始过渡");
		GELEM_VAR_INIT(StringID,to,StringID_Invalid);
			GELEM_EDITVAR("目的Posture",GVT_U,GSem(GSem_StringID,"PostureType"),"过渡到哪个Posture");
		GELEM_VAR_INIT(float,dur,1.0f);
			GELEM_EDITVAR("过渡时间",GVT_F,GSem(GSem_Float,"0.01f,20.0f,0.02f"),"过渡时间");
	END_GOBJ();    

};


class CAtpSwitch_PostureTrans:public CAtpSwitchBase,public NameCases
{
public:
	DECLARE_CLASS(CAtpSwitch_PostureTrans);

	virtual DWORD GetStubCount()	{		return _cases.GetStubCount()+_transes.size();	}
	virtual PadStub GetStub(DWORD idx);
	virtual DWORD GetChildCount()	{		return _cases.GetChildCount()+_transes.size();	}

	virtual const char *GetTypeName()	{		return "切换-Posture(带过渡)";	};
	virtual const char *GetDbgGroup()	{		return "Posture";	};
	virtual DbgType GetDbgType()	{		return Dbg_Name;	}
	StringID *GetDbgNames(DWORD &count)	{		return _cases.GetDbgNames(count);	}

	BEGIN_GOBJ_PURE(CAtpSwitch_PostureTrans,1);
		GELEM_SWITCH_BASE();
		GELEM_OBJ(NameCasesPosture,_cases);
			GELEM_EDITOBJ("切换条件","切换条件");
		GELEM_OBJVECTOR(PostureTrans,_transes)
			GELEM_EDITOBJ("过渡条件","设置哪些条件下需要过渡");
	END_GOBJ();    
	NameCasesPosture _cases;
	std::vector<PostureTrans> _transes;
};


//XXXXX:more AnimTreePad
