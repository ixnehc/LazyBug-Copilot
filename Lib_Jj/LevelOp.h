#pragma once

#include "class/class.h"

#include "anim/KeySet.h"

#include "LevelDefines.h"
#include "MagicBoardDefines.h"

#include "LevelBuff.h"

#include "LevelStrike.h"

class CLevelOp;
inline void lop_verify(CLevelOp*c) {}


#define DEFINE_LEVELOP_CLASS(clss,uid)													\
	_DEFINE_CLASS_BEGIN(CClass,CClassPool,clss,void)								\
		instance._flag|=ClassF_LevelOp;															\
		instance._uid=uid;																					\
		{clss *p=NULL;lop_verify(p);}																	\
	_DEFINE_CLASS_END(clss)																			\
	typedef clss ThisType;


class CLevelOp
{
public:
	virtual CClass *GetClass()=0;

	virtual void Save(CBitPacket *bp)	{	}
	virtual void Load(CBitPacket *bp)	{	}

	virtual BOOL CanDelayProcess()	{		return TRUE;	}

	LevelOpDesc &GetDesc()	{		return _desc;	}

	template<typename T>
	T *ToPtr()
	{
		if (GetClass()->IsSameWith(Class_Ptr2(T)))
			return (T *)this;
		return NULL;
	}

protected:
	LevelOpDesc _desc; 
};

//一些基本的Op

//启动技能的Op(播放施放动作)
struct LevelOp_StartSkill:public CLevelOp
{
public:
	DEFINE_LEVELOP_CLASS(LevelOp_StartSkill,1);

	virtual void Save(CBitPacket *bp);
	virtual void Load(CBitPacket *bp);

	virtual BOOL CanDelayProcess()	{		return _desc.link.id!=LevelOpLinkID_Invalid;	}

	RecordSimpleID idRec;
	LevelSkillTarget target;
	LevelSkillGrade grd;
	LevelSkillArg arg;
	ClientSkillID idClient;
};

//通知Skill变为Casted状态
struct LevelOp_SkillCasted:public CLevelOp
{
public:
	DEFINE_LEVELOP_CLASS(LevelOp_SkillCasted,6);

	virtual BOOL CanDelayProcess()	{		return FALSE;	}

};

class CBitPacket;
struct LavMod
{
	void Save(CBitPacket *bp);
	void Load(CBitPacket *bp);
	WORD ver;//修改前的版本号(修改后的版本号是这个值+1)
	BYTE bInstant:4;//表示这次修改的值要立即体现出来,而不是慢慢的过渡到这个值(比如伤害就是立即体现,回血是慢慢过渡)
	BYTE bModMax:4;//表示修改最大值,并且修改当前值(如果delta>0,当前值也要加delta,如果delta<0,则当前值不减,但会clamp到max
	float delta;
};


//HP Mod
struct LevelOp_HPMod:public CLevelOp
{
public:
	DEFINE_LEVELOP_CLASS(LevelOp_HPMod,2);

	virtual void Save(CBitPacket *bp);
	virtual void Load(CBitPacket *bp);

	LavMod mod;

	LevelStrike strike;
};

//Buff时间到
struct LevelOp_BuffTimeUp:public CLevelOp
{
	DEFINE_LEVELOP_CLASS(LevelOp_BuffTimeUp,3);

	virtual BOOL CanDelayProcess()	{		return FALSE;	}

};


struct LevelOp_AddBuff:public CLevelOp
{
	DEFINE_LEVELOP_CLASS(LevelOp_AddBuff,4);

	LevelOp_AddBuff()
	{
	}

	virtual void Save(CBitPacket *bp);
	virtual void Load(CBitPacket *bp);

	LevelBuffData data;
};

struct LevelOp_ModBuff:public CLevelOp
{
	DEFINE_LEVELOP_CLASS(LevelOp_ModBuff,5);

	virtual void Save(CBitPacket *bp);
	virtual void Load(CBitPacket *bp);

	BOOL IsEmpty()
	{
		if (removes.size()>0)
			return FALSE;
		return data.IsEmpty();
	}

	std::vector<LevelBuffID> removes;//需要删除的Buff
	LevelBuffData data;
};

struct LevelOp_SkillTeleport:public CLevelOp
{
	DEFINE_LEVELOP_CLASS(LevelOp_SkillTeleport,7);

	LevelOp_SkillTeleport()
	{
		id=LevelTeleportID_Invalid;
		dur=0;
		flag=0;
	}

	virtual BOOL CanDelayProcess()	{		return TRUE;	}

	virtual void Save(CBitPacket *bp);
	virtual void Load(CBitPacket *bp);

	LevelTeleportID id;
	LevelPos target;
	LevelFace face;
	AnimTick dur;//Teleport的过程持续多长时间
	DWORD flag;

};

//SP Modify
struct LevelOp_SPMod:public CLevelOp
{
public:
	DEFINE_LEVELOP_CLASS(LevelOp_SPMod,8);

	virtual void Save(CBitPacket *bp);
	virtual void Load(CBitPacket *bp);

	LavMod mod;
};

//FullSP Modify
struct LevelOp_FullSPMod:public CLevelOp
{
public:
	DEFINE_LEVELOP_CLASS(LevelOp_FullSPMod,32);

	virtual void Save(CBitPacket *bp);
	virtual void Load(CBitPacket *bp);

	LavMod mod;
};


class CLevelRecords;
struct ExprEquips
{
	DEFINE_CLASS(ExprEquips);
	ExprEquips()
	{
		memset(items,0,sizeof(items));
		wpnActive=EquipPart_Invalid;
	}
	void Write(CBitPacket *bp);
	void Read(CBitPacket *bp);

	void CopyFrom(ExprEquips &src)
	{
		memcpy(items,src.items,sizeof(items));
		wpnActive=src.wpnActive;
	}

	void SetItem(EquipPart part,RecordID id)
	{
		items[part]=id;

		//更新wpnActive
		wpnActive=EquipPart_Invalid;
		if (items[EquipPart_Weapon]!=RecordID_Invalid)
			wpnActive=EquipPart_Weapon;
		if (items[EquipPart_Weapon2nd]!=RecordID_Invalid)
			wpnActive=EquipPart_Weapon2nd;
	}

	RecordID items[EquipPart_MaxExpress];
	EquipPart wpnActive;

	BOOL CheckShieldActive(CLevelRecords *records);
	BOOL CheckMagicItemActive();

};


//Expressed Equip 的变化
struct LevelOp_ExprEquip:public CLevelOp
{
	DEFINE_LEVELOP_CLASS(LevelOp_ExprEquip,9);

	LevelOp_ExprEquip()
	{
	}

	virtual void Save(CBitPacket *bp);
	virtual void Load(CBitPacket *bp);

	ExprEquips equips;

};

//到达一个固定位置(包括旋转)
struct LevelOp_FixPosEuler:public CLevelOp
{
	DEFINE_LEVELOP_CLASS(LevelOp_FixPosEuler,10);

	virtual void Save(CBitPacket *bp);
	virtual void Load(CBitPacket *bp);

	LevelPos pos;
	float euler;
};

struct LevelOp_CancelReside:public CLevelOp
{
	DEFINE_LEVELOP_CLASS(LevelOp_CancelReside,11);

	virtual void Save(CBitPacket *bp);
	virtual void Load(CBitPacket *bp);

	LevelPos pos;
	LevelTeleportID idTeleport;
	LevelSkillID idBroken;

};

struct LevelOp_ModBuffDur:public CLevelOp
{
	DEFINE_LEVELOP_CLASS(LevelOp_ModBuffDur,12);

	virtual void Save(CBitPacket *bp);
	virtual void Load(CBitPacket *bp);

	virtual BOOL CanDelayProcess()	{		return FALSE;	}

	AnimTick durNew;
};

struct LevelOp_ItemBirth:public CLevelOp
{
	DEFINE_LEVELOP_CLASS(LevelOp_ItemBirth,13);

	virtual void Save(CBitPacket *bp);
	virtual void Load(CBitPacket *bp);

	virtual BOOL CanDelayProcess()	{		return TRUE;	}

};

struct LevelOp_ResouceMod:public CLevelOp
{
	DEFINE_LEVELOP_CLASS(LevelOp_ResouceMod,14);

	virtual void Save(CBitPacket *bp);
	virtual void Load(CBitPacket *bp);

	virtual BOOL CanDelayProcess()	{		return FALSE;	}

	LevelResourceType tpRes;
	LavMod mod;

};

//苏醒
struct LevelOp_Revive:public CLevelOp
{
	DEFINE_LEVELOP_CLASS(LevelOp_Revive,15);
	LevelOp_Revive()
	{
		idTeleport=LevelTeleportID_Invalid;
	}

	virtual void Save(CBitPacket *bp);
	virtual void Load(CBitPacket *bp);

	virtual BOOL CanDelayProcess()	{		return FALSE;	}

	LevelPos posRevive;
	LevelTeleportID idTeleport;//如果为LevelTeleportID_Invalid,表示位置没有改变(在原地苏醒)
};

//未击中
struct LevelOp_Miss:public CLevelOp
{
	DEFINE_LEVELOP_CLASS(LevelOp_Miss,16);
	LevelOp_Miss()
	{
		reason=(BYTE)None;
	}

	enum Reason
	{
		None,
		Miss,
		Block,
	};

	virtual void Save(CBitPacket *bp);
	virtual void Load(CBitPacket *bp);

	virtual BOOL CanDelayProcess()	{		return TRUE;	}

	BYTE reason;//什么原因导致未命中(比如闪避,格挡..)
};

//通知发射
struct LevelOp_StartFire:public CLevelOp
{
	DEFINE_LEVELOP_CLASS(LevelOp_StartFire,17);
	LevelOp_StartFire()
	{

	}

	virtual void Save(CBitPacket *bp)
	{

	}
	virtual void Load(CBitPacket *bp)
	{

	}

	virtual BOOL CanDelayProcess()	{		return FALSE;	}

};

struct LevelOp_EoBirth:public CLevelOp
{
	DEFINE_LEVELOP_CLASS(LevelOp_EoBirth,18);

	LevelOp_EoBirth()
	{
		tOwnerSkillCastTime=ANIMTICK_INFINITE;
	}

	virtual void Save(CBitPacket *bp);
	virtual void Load(CBitPacket *bp);

	virtual BOOL CanDelayProcess()	{		return TRUE;	}

	AnimTick tOwnerSkillCastTime;

};

struct LevelOp_GradeMod:public CLevelOp
{
public:
	DEFINE_LEVELOP_CLASS(LevelOp_GradeMod,19);

	virtual void Save(CBitPacket *bp);
	virtual void Load(CBitPacket *bp);

	LevelGrade grd;
};


struct LevelOp_SyncBuffData:public CLevelOp
{
public:
	DEFINE_LEVELOP_CLASS(LevelOp_SyncBuffData,21);

	virtual BOOL CanDelayProcess()	{		return FALSE;	}

	virtual void Save(CBitPacket *bp);
	virtual void Load(CBitPacket *bp);

	BYTE szData;
	BYTE szBitsData;//in Byte
	BYTE data[MAX_BUFF_DATA];//最多MAX_BUFF_DATA个字节
	BYTE bits[MAX_BUFF_DATA];//最多MAX_BUFF_DATA个字节
};

struct LevelOp_CombineSkill:public CLevelOp
{
public:
	DEFINE_LEVELOP_CLASS(LevelOp_CombineSkill,22);

	virtual void Save(CBitPacket *bp);
	virtual void Load(CBitPacket *bp);

	virtual BOOL CanDelayProcess()	{		return FALSE;	}

	LevelSkillTarget target;
};

struct LevelOp_SyncSkillData:public CLevelOp
{
	DEFINE_LEVELOP_CLASS(LevelOp_SyncSkillData,23);

	virtual BOOL CanDelayProcess()	{		return TRUE;	}

	virtual void Save(CBitPacket *bp);
	virtual void Load(CBitPacket *bp);

	BYTE szData;
	BYTE szBitsData;//in Byte
	BYTE data[MAX_SKILL_DATA];//最多MAX_SKILL_DATA个字节
	BYTE bits[MAX_SKILL_DATA/4];//最多MAX_SKILL_DATA个字节
};

struct LevelOp_CancelMount:public CLevelOp
{
	DEFINE_LEVELOP_CLASS(LevelOp_CancelMount,24);

	virtual void Save(CBitPacket *bp);
	virtual void Load(CBitPacket *bp);

	LevelPos pos;
	LevelTeleportID idTeleport;
	LevelSkillID idBroken;
};

struct LevelOp_CancelSkill:public CLevelOp
{
	DEFINE_LEVELOP_CLASS(LevelOp_CancelSkill,25);

	virtual void Save(CBitPacket *bp);
	virtual void Load(CBitPacket *bp);

	LevelSkillID idSkill;
	BOOL bStopAct;
};

struct LevelOp_MBResouceMod:public CLevelOp
{
	DEFINE_LEVELOP_CLASS(LevelOp_MBResouceMod,26);

	virtual void Save(CBitPacket *bp);
	virtual void Load(CBitPacket *bp);

	virtual BOOL CanDelayProcess()	{		return TRUE;	}

	MBResourceType tpRes;
	LavMod mod;
};

struct LevelOp_Path:public CLevelOp
{
	DEFINE_LEVELOP_CLASS(LevelOp_Path,27);

	virtual void Save(CBitPacket *bp);
	virtual void Load(CBitPacket *bp);

	KeySet ksPos;
	KeySet ksFace;
	AnimTick dur;
};

struct LevelOp_SpeedMod:public CLevelOp
{
public:
	DEFINE_LEVELOP_CLASS(LevelOp_SpeedMod,29);

	virtual void Save(CBitPacket *bp);
	virtual void Load(CBitPacket *bp);

	short ims;
	short ias;
};

struct LevelOp_ShapeMod:public CLevelOp
{
public:
	DEFINE_LEVELOP_CLASS(LevelOp_ShapeMod,30);

	void Save(CBitPacket *bp) override;
	void Load(CBitPacket *bp)override;

	int op;
	StringID nm;
};

struct LevelDmgAbort
{
	enum Type
	{
		None,
		ShieldMask,
		ShieldAmulet,
	};

	LevelDmgAbort()
	{
		tp=None;
		idEo=LevelObjID_Invalid;
	}

	void Save(CBitPacket *bp);
	void Load(CBitPacket *bp);


	DWORD tp;
	LevelStrike strike;
	LevelObjID idEo;
};

struct LevelOp_DmgAbort:public CLevelOp
{
	DEFINE_LEVELOP_CLASS(LevelOp_DmgAbort,31);


	LevelOp_DmgAbort()
	{
	}

	virtual void Save(CBitPacket *bp)	{		v.Save(bp);	}
	virtual void Load(CBitPacket *bp)	{		v.Load(bp);	}

	LevelDmgAbort v;
};

struct LevelOp_HonorMod:public CLevelOp
{
public:
	DEFINE_LEVELOP_CLASS(LevelOp_HonorMod,33);

	virtual void Save(CBitPacket *bp);
	virtual void Load(CBitPacket *bp);

	DWORD hnr;
};

struct LevelOp_VitaMod:public CLevelOp
{
public:
	DEFINE_LEVELOP_CLASS(LevelOp_VitaMod,34);
	LevelOp_VitaMod()
	{
		delta=0;
		idSrcOwner=LevelObjID_Invalid;
	}

	virtual void Save(CBitPacket *bp);
	virtual void Load(CBitPacket *bp);

	char delta;
	LevelObjID idSrcOwner;
};

struct LevelOp_WormMod:public CLevelOp
{
public:
	DEFINE_LEVELOP_CLASS(LevelOp_WormMod,36);
	LevelOp_WormMod()
	{
		delta=0;
	}

	virtual void Save(CBitPacket *bp);
	virtual void Load(CBitPacket *bp);

	char delta;
};


struct LevelOp_ChainedHammer:public CLevelOp
{
public:
	DEFINE_LEVELOP_CLASS(LevelOp_ChainedHammer,35);
	LevelOp_ChainedHammer()
	{
		op=Withdraw;
		dur=0;
	}

	enum Op
	{
		Withdraw,
		Pull,
		PullOut,
		Broken,
		Grab,
	};

	virtual void Save(CBitPacket *bp);
	virtual void Load(CBitPacket *bp);

	Op op;
	AnimTick dur;
};

struct LevelOp_Spore:public CLevelOp
{
	DEFINE_LEVELOP_CLASS(LevelOp_Spore,37);
	LevelOp_Spore()
	{
		op=Spawn;
		handle=0;
	}

	enum Op
	{
		Spawn,
		Detonate,
	};

	virtual void Save(CBitPacket *bp);
	virtual void Load(CBitPacket *bp);

	Op op;
	DWORD handle;
	LevelPos pos;
};

struct LevelOp_FireFly:public CLevelOp
{
	DEFINE_LEVELOP_CLASS(LevelOp_FireFly,38);
	LevelOp_FireFly()
	{
		op=StartFlee;
	}

	enum Op
	{
		StartFlee,
		StopFlee,
		EnterTorch,
		LeaveTorch,
	};

	virtual void Save(CBitPacket *bp);
	virtual void Load(CBitPacket *bp);

	LevelPos *PosBuf()
	{
		return (LevelPos*)bufPos;
	}
	LevelPos3D *Pos3DBuf()
	{
		return (LevelPos3D*)bufPos3D;
	}

	Op op;
	union
	{
		float bufPos[16*sizeof(LevelPos)];//big enougth
		float bufPos3D[16*sizeof(LevelPos3D)];//big enougth
	};
	DWORD nPos;
	LevelObjID idTorch;
	LevelObjID idPlayer;

};

struct LevelOp_TempleMod:public CLevelOp
{
public:
	DEFINE_LEVELOP_CLASS(LevelOp_TempleMod,39);
	LevelOp_TempleMod()
	{
		tp=LevelTemple_None;
		iAltar=0;
	}

	virtual void Save(CBitPacket *bp);
	virtual void Load(CBitPacket *bp);

	LevelTempleType tp;
	BYTE iAltar;
};

struct LevelOp_StrengthMod:public CLevelOp
{
public:
	DEFINE_LEVELOP_CLASS(LevelOp_StrengthMod,40);
	LevelOp_StrengthMod()
	{
	}

	virtual void Save(CBitPacket *bp);
	virtual void Load(CBitPacket *bp);

	WORD str;
};

struct LevelOp_EnableBody:public CLevelOp
{
public:
	DEFINE_LEVELOP_CLASS(LevelOp_EnableBody,41);

	virtual void Save(CBitPacket *bp);
	virtual void Load(CBitPacket *bp);

	BOOL bEnable;
};

struct LevelOp_MagicMod:public CLevelOp
{
public:
	DEFINE_LEVELOP_CLASS(LevelOp_MagicMod,42);
	LevelOp_MagicMod()
	{
	}

	virtual void Save(CBitPacket *bp);
	virtual void Load(CBitPacket *bp);

	WORD magic;
};

struct LevelOp_PainMod:public CLevelOp
{
public:
	DEFINE_LEVELOP_CLASS(LevelOp_PainMod,43);
	LevelOp_PainMod()
	{
		pain=0.0f;
		tServer=0;
	}

	void Save(CBitPacket *bp) override;
	void Load(CBitPacket *bp)override;

	float pain;
	AnimTick tServer;
};

struct LevelOp_Dummy:public CLevelOp
{
public:
	DEFINE_LEVELOP_CLASS(LevelOp_Dummy,44);
	LevelOp_Dummy()
	{
		t=0;
	}

	void Save(CBitPacket *bp) override;
	void Load(CBitPacket *bp)override;
	AnimTick t;
};

//XXXXX:More LevelOp
