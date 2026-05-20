#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"


#include "LoEffectObj.h"

#define CLASSUID_Link 44

struct EoParamLink:public LevelEoParam
{
	DEFINE_EOPARAM_CLASS(EoParamLink);

	BEGIN_GOBJ_PURE(EoParamLink,1);

		GELEM_VAR_INIT(int,tpTarget,0);
			GELEM_EDITVAR("目标类型",GVT_U,GSem(GSem_Interger,"Host(缺省),SkillTarget"),"目标类型");
		GELEM_VAR_INIT(float,dur,0.0f);
			GELEM_EDITVAR("持续时间",GVT_F,GSem(GSem_Float,"0.00,100,0.1"),"持续时间,0表示永久");
		GELEM_VAR_INIT(float,cycleDeal,0.5f);
			GELEM_EDITVAR("Deal作用周期",GVT_F,GSem(GSem_Float,"0.1,100,0.1"),"Deal作用周期");
		GELEM_VAR_INIT( StringID,nmFinishEvent,StringID_Invalid);	
			GELEM_EDITVAR( "终止事件", GVT_U, GSem(GSem_StringID,"动画事件"), "收到哪个事件终止" );
		GELEM_VAR_INIT(BOOL,bSkillBound,FALSE);
			GELEM_EDITVAR("技能绑定(技能结束后销毁)",GVT_S,GSem_Boolean,"技能绑定(技能结束后销毁)");
	END_GOBJ();

	int tpTarget;
	float dur;
	float cycleDeal;
	StringID nmFinishEvent;
	BOOL bSkillBound;
};


class EoLink:public CLoEffectObj
{
public:
	EoLink()
	{
		_nCycles=0;
		_idTarget=LevelObjID_Invalid;
		_idSkill=LevelSkillID_Invalid;
	}
	DEFINE_LEVELOBJ_CLASS(EoLink,CLASSUID_Link);

	virtual const char *GetShowName()	{		return "链接";	}

protected:

	void _OnPostCreate() override;

	void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);

	void _OnUpdate();
	void UpdateSubframe() override;

	int _nCycles;

	LevelObjID _idTarget;
	LevelSkillID _idSkill;


};
