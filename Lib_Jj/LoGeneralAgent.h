#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"
#include "behaviorgraph/BehaviorValue.h"


#include "LevelDefines.h"

#include "LoAgent.h"
#include "LevelObjSrc.h"

#include "LevelAttrs.h"

#define CLASSUID_GeneralAgent 20

class CBehaviorPersist;


struct GeneralAgentShapeEntry
{

	BEGIN_GOBJ_PURE(GeneralAgentShapeEntry,1);
		GELEM_VAR_INIT( StringID,nm,StringID_Invalid);	
			GELEM_EDITVAR( "Shape名称", GVT_U, GSem(GSem_StringID,"Shape名称"), "Shape名称" );
		GELEM_VARVECTOR(i_math::spheref,shape)
			GELEM_EDITVAR("外形模拟",GVT_Fx4,GSem(GSem_Unknown,"SphereSetLS"),"外形的模拟");
	END_GOBJ();

	StringID nm;
	std::vector<i_math::spheref> shape;//外形模拟

};


struct LopGeneralAgent:public CLevelObjParam
{
public:
	DEFINE_LEVELOBJPARAM_CLASS(LopGeneralAgent,CLASSUID_GeneralAgent);

	BEGIN_GOBJ_PURE(LopGeneralAgent,1);

		GELEM_ALLOWDISABLE();
		GELEM_ALLOWPLAYERID();

		GELEM_VARVECTOR(i_math::spheref,shape)
			GELEM_EDITVAR("外形模拟",GVT_Fx4,GSem(GSem_Unknown,"SphereSetLS"),"外形的模拟");

		GELEM_OBJVAR(BhvValues,valuesBhv)
			GELEM_VERSION(2)
			GELEM_EDITOBJ("常量数据","常量数据");

	END_GOBJ();


	BhvValues valuesBhv;
	std::vector<i_math::spheref> shape;//外形模拟
// 	BehaviorConstData consts;


};

struct InvokeAgentInfo
{
	enum Type
	{
		InvokeType_Default,
		InvokeType_PlantNameless,

		ForceDword=0xffffffff,
	};

	BEGIN_GOBJ_PURE(InvokeAgentInfo,1);

		GELEM_VAR_INIT(BOOL,bEnable,FALSE)
			GELEM_EDITVAR("Enable",GVT_S,GSem(GSem_Boolean,"类型,Invoke技能,Invoke位置"),"是否Enable");
		GELEM_VAR_INIT(Type,tp,InvokeType_Default);
			GELEM_EDITVAR("类型",GVT_U,GSem(GSem_Interger,"缺省,无名之剑插入"),"类型");
		GELEM_VAR_INIT(RecordID,idSkill,RecordID_Invalid);
			GELEM_EDITVAR("Invoke技能",GVT_U,GSem(GSem_RecordID,"skills"),"Invoke技能");
		GELEM_VARVECTOR(i_math::matrix43f,mats)
			GELEM_EDITVAR("Invoke位置",GVT_Fx12,GSem(GSem_Unknown,"MatSetLS"),"Invoke位点");

	END_GOBJ();

	BOOL bEnable;
	Type tp;
	RecordID idSkill;
	std::vector<i_math::matrix43f> mats;

};

struct LosGeneralAgent:public LosAgent
{
public:
	DEFINE_LEVELOBJSRC_CLASS(LosGeneralAgent,CLASSUID_GeneralAgent);

	BEGIN_GOBJ_PURE(LosGeneralAgent,1);

		GELEM_AGENTRECORD();
		GELEM_AGENTGUID();

		GELEM_VARVECTOR(i_math::spheref,shape)
			GELEM_EDITVAR("外形模拟",GVT_Fx4,GSem(GSem_Unknown,"SphereSetLS"),"外形的模拟");
		GELEM_OBJVECTOR(GeneralAgentShapeEntry,shapesEx);
			GELEM_EDITOBJ("额外的Shape","额外的Shape");
		GELEM_VARVECTOR(i_math::vector3df,centerBrief)
			GELEM_EDITVAR("Icon中心点",GVT_Fx3,GSem(GSem_Unknown,"MatSetLS"),"Icon中心点");
		GELEM_OBJ(InvokeAgentInfo,invoke);
			GELEM_EDITOBJ("Invoke信息","Invoke信息");
		GELEM_OBJVAR(BhvValues,valuesBhv)
			GELEM_VERSION(2)
			GELEM_EDITOBJ("常量数据","常量数据");

	END_GOBJ();

	std::vector<i_math::spheref>*FindShape(StringID nm);

	std::vector<i_math::spheref> shape;//外形模拟
	std::vector<GeneralAgentShapeEntry> shapesEx;
	std::vector<i_math::vector3df> centerBrief;
	InvokeAgentInfo invoke;

	BhvValues valuesBhv;

	virtual BOOL NeedSyncGUID()	{		return TRUE;	}

};

struct LevelAttr_Attackable
{
	DEFINE_CLASS(LevelAttr_Attackable);
	LevelAttr_Attackable()
	{
		attrResists=NULL;
		attrEvade=NULL;
	}
	LevelAttr_Base attrBase;
	LevelAttr_Resists *attrResists;
	LevelAttr_Evade *attrEvade;
};

struct LevelBuffsWrapper
{
	DEFINE_CLASS(LevelBuffsWrapper);

	CLevelBuffs buffs;
};

struct GeneralAgentShape
{
	DEFINE_CLASS(GeneralAgentShape);

	void Clear();

	std::vector<CUnit*>units;
	std::vector<LevelObjCircle> circles;
};


class CBehaviorMem;
class CLevelEventSrc;
class CLoGeneralAgent:public CLoAgent
{
public:
	CLoGeneralAgent()
	{
		_driver=NULL;
		_talks=NULL;
		_troops=NULL;
		_residable=NULL;
		memset(_bhvs,0,sizeof(_bhvs));
		memset(_bhvsLast,0,sizeof(_bhvsLast));
		_tUpdate=0;

		_buffs=NULL;

		_attrAttackable=NULL;
		_attrMagicBoard=NULL;

		_dmgsrc=NULL;

		_shape=NULL;
		_nmShape=StringID_Invalid;
		_bBodyEnabled=TRUE;
	}
	DEFINE_LEVELOBJ_CLASS(CLoGeneralAgent,CLASSUID_GeneralAgent);



	virtual const char *GetShowName()	override{		return "通用Agent";	}

	virtual LevelObjShapeType GetShapeType()	override{		return _shape?LevelObjShape_MultiCircle:LevelObjShape_SingleCircle;	}//外形模拟方式
	virtual LevelObjCircle *GetShapeCircles(DWORD &count)override;

	LevelPos3D GetBriefCenter()	 override;

	virtual BOOL OnActivate()override;
	virtual void OnDestroy()override;

	virtual void HandleHook(LevelHook &hk);

	virtual void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)override;
	virtual void _OnWriteSyncL(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)override;
	virtual void _OnPostWriteSync()override;

	virtual void Update()override;

	virtual CLevelSkillDriver *GetSkillDriver()	override{		return _driver;	}
	virtual CLevelTalks*GetTalks()	override{		return _talks;	}
	virtual CLevelObjResidable*GetResidable()	override{		return _residable;	}
	virtual CLevelBuffs *GetBuffs()	override{		return _buffs?&(_buffs->buffs):NULL;	}
	virtual LevelAttr_Base*GetAttr_Base()	override{		return _attrAttackable?&_attrAttackable->attrBase:NULL;	}
	virtual LevelAttr_Resists*GetAttr_Resists()	override{		return _attrAttackable?_attrAttackable->attrResists:NULL;	}
	virtual LevelAttr_Evade*GetAttr_Evade()	override{		return _attrAttackable?_attrAttackable->attrEvade:NULL;	}
	virtual LevelAttr_MagicBoard*GetAttr_MagicBoard()	override{		return _attrMagicBoard;	}
	virtual CLevelTroops *ObtainTroops()override;
	virtual CLevelTroops *GetTroops()	override{		return _troops;	}
	virtual CLevelEventSrc*GetEventSrc()	override{		return _dmgsrc;	}

	BOOL IsShapeEnabled()	{		return _shape?TRUE:FALSE;	}
	void DisableShape();
	void EnableShape();
	BOOL SetShapeName(StringID nm);//返回是否发生变化

	BOOL IsBodyEnabled()	{		return _bBodyEnabled;	}
	void EnableBody(BOOL bEnable)	{		_bBodyEnabled=bEnable;	}

	LevelSimpleMem*GetSimpleMem(LevelPlayerID idPlayer);
	CBehaviorMem *GetBehaviorMem(LevelPlayerID idPlayer);
	CLevelBehavior*GetBehavior(LevelPlayerID idPlayer);

	CLevelBehavior* GetBehaviorAI() override;

protected:

	void _VerifyBehavior();
	void _UpdateBehavior();

	void _LoadPersist(LevelPlayerID idPlayer);
	void _SavePersist(LevelPlayerID idPlayer);

	void _LoadPersistS(LevelPlayerID idPlayer);
	void _SavePersistS(LevelPlayerID idPlayer);

	int _GetBehaviorCount();
	int _GetBehaviorIdx(LevelPlayerID idPlayer);

	void _SetLastBehavior(LevelPlayerID idPlayer,BOOL bExist);
	BOOL _CheckLastBehavior(LevelPlayerID idPlayer);

	void _BuildShapeUnits(std::vector<CUnit*> &units,std::vector<i_math::spheref>&shape,i_math::matrix43f &mat);
	void _BuildShape();
	void _ClearShape();

	AnimTick _tUpdate;

	CLevelSkillDriver *_driver;
	CLevelTalks *_talks;
	CLevelTroops *_troops;

	CLevelObjResidable *_residable;

	GeneralAgentShape *_shape;
	StringID _nmShape;
	BOOL _bBodyEnabled;

	LevelAttr_Attackable *_attrAttackable;
	LevelAttr_MagicBoard *_attrMagicBoard;

	CLevelEventSrc *_dmgsrc;

	LevelBuffsWrapper *_buffs;

	LevelSimpleMem _memsSimple[LEVEL_MAX_PLAYER];

	CLevelBehavior *_bhvs[LEVEL_MAX_PLAYER];
	BYTE _bhvsLast[LEVEL_MAX_PLAYER];


};
