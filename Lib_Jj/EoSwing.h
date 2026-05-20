#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"


#include "LoEffectObj.h"

#define CLASSUID_Swing 29

struct EoParamSwing:public LevelEoParam
{
	DEFINE_EOPARAM_CLASS(EoParamSwing);

	EoParamSwing()
	{
		GConstructor();

		radius.ResetFloat(1.0f);
		yaw.ResetFloat(0.0f);
	}

	~EoParamSwing()
	{
		GDestructor();
	}


	BEGIN_GOBJ(EoParamSwing,1);

		GELEM_VAR_INIT(BOOL,bLeft,FALSE);
			GELEM_EDITVAR("方向",GVT_S,GSem(GSem_Interger,"右挥,左挥"),"右挥还是左挥");
		GELEM_OBJVAR( ValueSet, radius);
			GELEM_EDITOBJ_EX("半径曲线","半径曲线",GSem( GSem_Unknown, "0,0,1,20" ));

		GELEM_OBJVAR( ValueSet, yaw);
			GELEM_EDITOBJ_EX("偏转曲线","偏转曲线(负值为左,正值为右)",GSem( GSem_Unknown, "0,-90,1,90" ));

		GELEM_VAR_INIT(BOOL,bUseEndEvent,TRUE);
			GELEM_EDITVAR("结束方式",GVT_S,GSem(GSem_Interger,
				"持续时间"		"|结束事件,"
				"结束事件"	"|持续时间"
				),"是否强制重新组建");

		GELEM_VAR_INIT(StringID,nmEndEvent,StringID_Invalid);
			GELEM_EDITVAR("结束事件",GVT_U,GSem(GSem_StringID,"动画事件"),"接受到什么事件后停止挥舞");

		GELEM_VAR_INIT(AnimTick,dur,ANIMTICK_FROM_SECOND(2.0f));
			GELEM_EDITVAR("持续时间",GVT_U,GSem(GSem_AnimTick,"0,2000,0.1"),"持续时间");

	END_GOBJ();

	BOOL bLeft;
	ValueSet radius;
	ValueSet yaw;
	BOOL bUseEndEvent;
	StringID nmEndEvent;
	AnimTick dur;
};



class EoSwing:public CLoEffectObj
{
public:
	EoSwing()
	{
		_tStart=0;
		_tAge=0;
		_faceSwing=0.0f;
		_radiusSwing=0.0f;
		_dur=0;
	}
	DEFINE_LEVELOBJ_CLASS(EoSwing,CLASSUID_Swing);

	virtual const char *GetShowName()	{		return "挥舞";	}
	virtual LevelAttr_AttackMods*GetAttr_AttackMods();

protected:
	virtual void _OnPostCreate();
	virtual void _OnUpdate();
	virtual void _OnDetroy();
	virtual void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);

	void _CalcSwing(EoParamSwing *param,AnimTick tAge,LevelPos &pos,LevelFace &face,float &radius);

	AnimTick _tStart;
	AnimTick _dur;
	AnimTick _tAge;
	LevelFace _faceSwing;//_tAge时的扫描线的朝向
	LevelPos _centerSwing;//_tAge时的扫描线的起始点
	float _radiusSwing;//_tAge时的扫描线的长度


	std::unordered_set<LevelObjID> _handled;
};
