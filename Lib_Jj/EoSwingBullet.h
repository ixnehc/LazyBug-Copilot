#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"


#include "LoEffectObj.h"

#define CLASSUID_SwingBullet 31

struct EoParamSwingBase:public LevelEoParam
{
	BOOL bLeft;
	ValueSet radius;
	ValueSet yaw;
	BOOL bUseEndEvent;
	StringID nmEndEvent;
	AnimTick dur;
};


class EoSwingBulletBase:public CLoEffectObj
{
public:
	EoSwingBulletBase()
	{
		_tStart=0;
		_tAge=0;
		_faceSwing=0.0f;
		_radiusSwing=0.0f;
		_dur=0;
	}
protected:
	virtual void _OnPostCreate();
	virtual void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);

	void _CalcSwing(EoParamSwingBase *param,AnimTick tAge,LevelPos &pos,LevelFace &face,float &radius);
	void _ClampSwing(EoParamSwingBase *param,LevelFace &faceSwing);

	AnimTick _tStart;
	AnimTick _dur;
	AnimTick _tAge;
	LevelFace _faceSwing;//_tAge时的扫描线的朝向
	LevelPos _centerSwing;//_tAge时的扫描线的起始点
	float _radiusSwing;//_tAge时的扫描线的长度
};




struct EoParamSwingBullet:public EoParamSwingBase
{
	DEFINE_EOPARAM_CLASS(EoParamSwingBullet);

	EoParamSwingBullet()
	{
		GConstructor();

		radius.ResetFloat(1.0f);
		yaw.ResetFloat(0.0f);
	}

	~EoParamSwingBullet()
	{
		GDestructor();
	}


	BEGIN_GOBJ(EoParamSwingBullet,1);

		GELEM_VAR_INIT(BOOL,bLeft,FALSE);
			GELEM_EDITVAR("方向",GVT_S,GSem(GSem_Interger,"右挥,左挥"),"右挥还是左挥");
		GELEM_OBJVAR( ValueSet, radius);
			GELEM_EDITOBJ_EX("半径曲线","半径曲线",GSem( GSem_Unknown, "0,0,1,20" ));

		GELEM_OBJVAR( ValueSet, yaw);
			GELEM_EDITOBJ_EX("偏转曲线","偏转曲线(负值为左,正值为右)",GSem( GSem_Unknown, "0,-180,1,180" ));

		GELEM_VAR_INIT(BOOL,bUseEndEvent,TRUE);
			GELEM_EDITVAR("结束方式",GVT_S,GSem(GSem_Interger,
				"持续时间"		"|结束事件,"
				"结束事件"	"|持续时间"
				),"是否强制重新组建");

		GELEM_VAR_INIT(StringID,nmEndEvent,StringID_Invalid);
			GELEM_EDITVAR("结束事件",GVT_U,GSem(GSem_StringID,"动画事件"),"接受到什么事件后停止挥舞");

		GELEM_VAR_INIT(AnimTick,dur,ANIMTICK_FROM_SECOND(2.0f));
			GELEM_EDITVAR("持续时间",GVT_U,GSem(GSem_AnimTick,"0,2000,0.1"),"持续时间");

		GELEM_VAR_INIT(RecordID,idBulletEo,RecordID_Invalid);
			GELEM_EDITVAR("子弹Eo",GVT_U,GSem(GSem_RecordID,"eos"),"子弹Eo");
		GELEM_VAR_INIT(DWORD,nBullet,3);
			GELEM_EDITVAR("子弹个数",GVT_U,GSem(GSem_Interger,"0,1,2,3,4,5,6,7,8,9"),"有几个子弹");
		GELEM_VAR_INIT(float,random,0.0f);
			GELEM_EDITVAR("子弹分布随机程度",GVT_F,GSem(GSem_Float,"0.0,1.0,0.05"),"0表示完全均匀分布,1表示完全随机");
	END_GOBJ();


	RecordID idBulletEo;
	int nBullet;
	float random;

};



class EoSwingBullet:public EoSwingBulletBase
{
public:
	EoSwingBullet()
	{
	}
	DEFINE_LEVELOBJ_CLASS(EoSwingBullet,CLASSUID_SwingBullet);

	virtual const char *GetShowName()	{		return "挥舞子弹";	}

protected:
	virtual void _OnPostCreate();
	virtual void _OnUpdate();
//	virtual void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);

	std::deque<AnimTick> _timesBullet;


};
