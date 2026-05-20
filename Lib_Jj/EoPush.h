#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"


#include "LoEffectObj.h"

#define CLASSUID_Push 33

struct EoParamPush:public LevelEoParam
{
	DEFINE_EOPARAM_CLASS(EoParamPush);

	EoParamPush()
	{
		GConstructor();

		radius.ResetFloat(1.0f);
		yaw.ResetFloat(0.0f);
		fov.ResetFloat(60.0f);
	}

	~EoParamPush()
	{
		GDestructor();
	}


	BEGIN_GOBJ(EoParamPush,1);

		GELEM_OBJVAR( ValueSet, radius);
			GELEM_EDITOBJ_EX("半径曲线","半径曲线",GSem( GSem_Unknown, "0,0,1,20" ));
		GELEM_OBJVAR( ValueSet, fov);
			GELEM_EDITOBJ_EX("FOV","张角[0..180]",GSem( GSem_Unknown, "0,0,1,180" ));
		GELEM_OBJVAR( ValueSet, yaw);
			GELEM_EDITOBJ_EX("偏转曲线","偏转曲线(负值为左,正值为右)",GSem( GSem_Unknown, "0,-90,1,90" ));
		GELEM_VAR_INIT(AnimTick,dur,ANIMTICK_FROM_SECOND(0.2f));
			GELEM_EDITVAR("持续时间",GVT_U,GSem(GSem_AnimTick,"0.01,10.0,0.02"),"持续时间");
		GELEM_VAR_INIT(BOOL,bSkillTargetOnly,FALSE);
			GELEM_EDITVAR("只作用于技能对象",GVT_S,GSem_Boolean,"只作用于技能对象");
	END_GOBJ();

	ValueSet radius;
	ValueSet yaw;
	ValueSet fov;
	AnimTick dur;
	BOOL bSkillTargetOnly;
};



class EoPush:public CLoEffectObj
{
public:
	EoPush()
	{
		_tStart=0;
		_tAge=0;
		_bFirstUpdate=FALSE;
		_bEndEvent=FALSE;
		_temp=NULL;
		_iUpdate=0;
	}
	DEFINE_LEVELOBJ_CLASS(EoPush,CLASSUID_Push);

	virtual const char *GetShowName()	{		return "推压";	}
	virtual LevelAttr_AttackMods*GetAttr_AttackMods();

protected:
	virtual void _OnPostCreate();
	virtual void _OnUpdate();
	virtual void _OnDetroy();
	virtual void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);
	virtual void UpdateSubframe() override;

	void _CalcPush(EoParamPush *param,AnimTick tAge,i_math::line2df &left,i_math::line2df &right,float &radius);

	AnimTick _tStart;
	AnimTick _tAge;
	BOOL _bFirstUpdate;
	BOOL _bEndEvent;

	CLevelObj *_temp;

	int _iUpdate;

	std::unordered_set<LevelObjID> _handled;
};
