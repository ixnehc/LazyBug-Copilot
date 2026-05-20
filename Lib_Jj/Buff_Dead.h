#pragma once

#include "LevelBuff.h"

#include "Buff_Stun.h"

#include "LevelObjPauser.h"
#include "LevelStrike.h"

#include "LevelObliterateArg.h"

struct DyingDesc
{
	BEGIN_GOBJ_PURE(DyingDesc,1);

		GELEM_VAR_INIT(unsigned __int64,effect,0);
			GELEM_EDITVAR("效果",GVT_Bx8,GSem_ProtoPath,"死亡倒下效果");
		GELEM_VAR_INIT(float,distOff,0.0f);
			GELEM_EDITVAR("偏移距离",GVT_F,GSem(GSem_Float,"0.0,10.0,0.05"),"在偏移朝向上的距离,单位为米");
		GELEM_VAR_INIT(LevelFaceYaw,yaw,0.0f);
			GELEM_EDITVAR("偏移朝向",GVT_F,GSem(GSem_Float,"-180.0,180.0,0.5"),"朝向偏移,单位为度,向左为负,向右为正");
		GELEM_VAR_INIT(BOOL,bRagdoll,FALSE);
			GELEM_EDITVAR("是否切换为Ragdoll",GVT_S,GSem_Boolean,"是否切换为Ragdoll");
		GELEM_VAR_INIT(BOOL,bPetrified,FALSE);
			GELEM_EDITVAR("是否为石化死亡",GVT_S,GSem_Boolean,"是否为石化死亡");
		GELEM_VAR_INIT(float,ht,0.0f);
			GELEM_EDITVAR("高度",GVT_F,GSem(GSem_Float,"0.0,10.0,0.05"),"高度,单位为米");
		GELEM_STRING_INIT(path,"");
			GELEM_EDITVAR("路径动画",GVT_String,GSem_XformAnimPath,"路径动画的资源");
		GELEM_VAR_INIT(RecordID,idSkill,RecordID_Invalid);
			GELEM_EDITVAR("Skill",GVT_U,GSem(GSem_RecordID,"skills"),"Skill");
	END_GOBJ();

	unsigned __int64 effect;
	float distOff;
	LevelFaceYaw yaw;
	BOOL bRagdoll;
	BOOL bPetrified;
	float ht;
	std::string path;
	RecordID idSkill;
};

struct ObliterateDealEntry
{
	LevelObliterateType tp;
	unsigned __int64 effect;
	CLevelDeal *deal;

	BEGIN_GOBJ_PURE(ObliterateDealEntry,1);

		GELEM_VAR_INIT(LevelObliterateType,tp,LevelObliterate_None);
			GELEM_EDITVAR("死亡爆裂类型",GVT_U,GSem(GSem_Interger,LevelObliterateType_SemConstraint),"死亡爆裂类型");

		GELEM_VAR_INIT(unsigned __int64,effect,0);
			GELEM_EDITVAR("表现效果",GVT_Bx8,GSem_ProtoPath,"爆裂表现效果");
		GELEM_DYNOBJPTR_DEAL(CLevelDeal,deal,Deal_Null, "命中效果", "选择不同的命中效果" );
			GELEMS_LEVELDEAL_CANDIDATES();
	END_GOBJ();

};


struct BuffParam_Dead:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_Dead);

	BEGIN_GOBJ_PURE(BuffParam_Dead,1);

		GELEM_OBJVECTOR(DyingDesc,dyings);
			GELEM_EDITOBJ("死亡效果列表","死亡效果列表");
		GELEM_OBJVECTOR(ObliterateDealEntry,dealsObliterate);
			GELEM_EDITOBJ("死亡爆裂结算列表","死亡爆裂结算列表");
		GELEM_VARVECTOR_INIT(unsigned __int64,corpse,0);
			GELEM_EDITVAR("尸体效果",GVT_Bx8,GSem_ProtoPath,"各种尸体效果");
		GELEM_VAR_INIT(BOOL,bVanish,FALSE);
			GELEM_EDITVAR("是否消失",GVT_S,GSem_Boolean,"死亡后尸体是否消失");
		GELEM_VAR_INIT(BOOL,bAllowFalling,TRUE);
			GELEM_EDITVAR("是否允许坠落",GVT_S,GSem_Boolean,"在空中死亡后是否允许坠落到地面");
		GELEM_DYNOBJPTR_DEAL(CLevelDeal,deal,Deal_Null, "单个结算", "选择不同的结算" );
			GELEMS_LEVELDEAL_CANDIDATES();
		GELEM_OBJVECTOR(DealEntry,deals);
			GELEM_EDITOBJ("结算列表","多个结算");
		GELEM_VAR_INIT(AnimTick,delayDeal,ANIMTICK_FROM_SECOND(0.0f));
			GELEM_EDITVAR("结算延迟时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"延迟多久结算");

		GELEM_VARVECTOR_INIT(unsigned __int64,dying,0);
			GELEM_EDITVAR("死亡倒下效果--obsolete",GVT_Bx8,GSem_ProtoPath,"各种死亡倒下效果");
		GELEM_VARVECTOR_INIT(unsigned __int64,dyingRgd,0);
			GELEM_EDITVAR("死亡倒下效果(Ragdoll)--obsolete",GVT_Bx8,GSem_ProtoPath,"各种死亡倒下效果(Ragdoll)");
		GELEM_VAR_INIT(unsigned __int64,petrified,0);GELEM_VERSION(2);
			GELEM_EDITVAR("石化碎裂效果--obsolete",GVT_Bx8,GSem_ProtoPath,"石化碎裂效果");
	END_GOBJ();

	int FindDyingDesc(float htMe,LevelFace faceMe,LevelFace faceStrike);

	std::vector<DyingDesc> dyings;
	std::vector<ObliterateDealEntry> dealsObliterate;

	CLevelDeal *deal;
	std::vector<DealEntry> deals;
	AnimTick delayDeal;

	std::vector<unsigned __int64> dying;
	std::vector<unsigned __int64> dyingRgd;
	std::vector<unsigned __int64> corpse;
	unsigned __int64 petrified;
	BOOL bVanish;
	BOOL bAllowFalling;


};


struct BuffArg_Dead:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_Dead);
	BuffArg_Dead()
	{
		osbSrc=NULL;
		strike=NULL;
		argObliterate=NULL;
	}
	LevelOSB *osbSrc;
	LevelStrike *strike;
	LevelObliterateArg *argObliterate;
	LevelOpLink link;
};

struct BuffData_Dead
{
	BuffData_Dead()
	{
		memset(this,0,sizeof(*this));
		idxDesc=-1;
	}
	void Save(CBitPacket *bp);
	void Load(CBitPacket *bp);
	LevelSkillID idBroken;//死亡导致那个Skill被中断了
	LevelStrike strike;
	LevelObliterateType tpObliterate;
	char idxDesc;

};

class Buff_Dead:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_Dead,6)

	Buff_Dead()
	{
		_bNeedDeal=FALSE;
		_bInSkill=FALSE;
	}

	virtual BOOL NeedSync()	{		return TRUE;	}//是否需要同步给客户端
	virtual BOOL NeedSyncTimeUp()	{		return TRUE;	}//结束时要同步给客户端,以避免客户端出现多个Dead的Buff并存的情况


	ConflictResult Buff_Dead::CheckConflict(CLevelBuff *buffExist);
	virtual LevelBuffMask GetReplaceBuffs();

	virtual void _OnCreate(LevelBuffArg *param) override;
	virtual void _OnDestroy() override;
	virtual void _OnUpdate(AnimTick dt) override;

	//Factor Overriding
	BuffFlag GetFlags()	
	{		
		if (!_bInSkill)
			return BuffFlag_NotAttackable|BuffFlag_GhostCollide|BuffFlag_Dead|BuffFlag_LayDown|BuffFlag_Pausing;	
		return BuffFlag_NotAttackable|BuffFlag_GhostCollide|BuffFlag_DamageImmune;	
	}

protected:

	virtual void _WriteData(CBitPacket *dp);

	void _UpdateCreateEo();

	BuffData_Dead _data;

	CLevelObjPauser _pauser;
	BOOL _bNeedDeal;

	BOOL _bInSkill;


};

