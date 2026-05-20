#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"


#include "LoEffectObj.h"

#define CLASSUID_Shards 71

struct EoParamShards:public LevelEoParam
{
	DEFINE_EOPARAM_CLASS(EoParamShards);

	EoParamShards::EoParamShards()
	{
		GConstructor();
		vsWithdrawDist.ResetFloat(0.0f);
		vsWithdrawDist.AddFloat(1.0f,1.0f);
	}
	EoParamShards::~EoParamShards()
	{
		GDestructor();
	}

	BEGIN_GOBJ(EoParamShards,1);

		GELEM_VAR_INIT(unsigned __int64,idCoreEffect,0); 
			GELEM_EDITVAR("Core效果Proto",GVT_Bx8,GSem_ProtoPath,"效果的Proto");
		GELEM_VAR_INIT(unsigned __int64,idShardEffect,0); 
			GELEM_EDITVAR("Shard效果Proto",GVT_Bx8,GSem_ProtoPath,"效果的Proto");

		GELEM_VAR_INIT(int,count,12);
			GELEM_EDITVAR("个数",GVT_S,GSem_Interger,"个数");

		GELEM_VAR_INIT(float,spdExplodeVerMin,5.0f);
			GELEM_EDITVAR("爆炸速度(最小竖直方向)",GVT_F,GSem(GSem_Float,"0,100,0.1"),"爆炸速度");
		GELEM_VAR_INIT(float,spdExplodeVerMax,7.0f);
			GELEM_EDITVAR("爆炸速度(最大竖直方向)",GVT_F,GSem(GSem_Float,"0,100,0.1"),"爆炸速度");
		GELEM_VAR_INIT(float,spdExplodeHorMin,5.0f);
			GELEM_EDITVAR("爆炸速度(最小水平方向)",GVT_F,GSem(GSem_Float,"0,100,0.1"),"爆炸速度");
		GELEM_VAR_INIT(float,spdExplodeHorMax,8.0f);
			GELEM_EDITVAR("爆炸速度(最小水平方向)",GVT_F,GSem(GSem_Float,"0,100,0.1"),"爆炸速度");
		GELEM_VAR_INIT(float,spdExplodeRollMin,720.0f);
			GELEM_EDITVAR("最小滚翻速度)",GVT_F,GSem(GSem_Float,"0,6000,0.1"),"爆炸速度");
		GELEM_VAR_INIT(float,spdExplodeRollMax,1080.f);
			GELEM_EDITVAR("最大滚翻速度)",GVT_F,GSem(GSem_Float,"0,6000,0.1"),"爆炸速度");

		GELEM_VAR_INIT(float,scaleShardMin,0.6f);
			GELEM_EDITVAR("Shard缩放最小值",GVT_F,GSem(GSem_Float,"0,100,0.01"),"Shard缩放最小值");
		GELEM_VAR_INIT(float,scaleShardMax,0.9f);
			GELEM_EDITVAR("Shard缩放最大值",GVT_F,GSem(GSem_Float,"0,100,0.01"),"Shard缩放最大值");

		GELEM_VAR_INIT(float,htCore,1.0f);
			GELEM_EDITVAR("Core高度",GVT_F,GSem(GSem_Float,"0,100,0.01"),"Core高度");
		GELEM_VAR_INIT(float,radiusCore,0.25f);
			GELEM_EDITVAR("Core半径",GVT_F,GSem(GSem_Float,"0,100,0.25f"),"Core高度");

		GELEM_OBJVAR( ValueSet, vsWithdrawDist);
			GELEM_EDITOBJ_EX( "回收距离曲线", "随时间变化的距离", GSem( GSem_Unknown, "0,0,1,1") );

		GELEM_VAR_INIT(AnimTick,delayWithdraw,ANIMTICK_FROM_SECOND(10.0f));
			GELEM_EDITVAR("回收前等待时间",GVT_U,GSem(GSem_AnimTick,"1,100,0.1"),"回收前等待时间");

		GELEM_VAR_INIT(AnimTick,durReady,ANIMTICK_FROM_SECOND(10.0f));
			GELEM_EDITVAR("Ready时间",GVT_U,GSem(GSem_AnimTick,"1,100,0.1"),"Ready时间");

		GELEM_VAR_INIT(float,radiusMonitor,5.0f);
			GELEM_EDITVAR("Ready时的监控半径",GVT_F,GSem(GSem_Float,"0,100,0.1"),"Ready时的监控半径");
		GELEM_VAR_INIT(float,radiusFire,10.0f);
			GELEM_EDITVAR("发射半径",GVT_F,GSem(GSem_Float,"0,100,0.1"),"发射半径");
		GELEM_VAR_INIT(AnimTick,durFireCycle,ANIMTICK_FROM_SECOND(0.2f));
			GELEM_EDITVAR("发射周期",GVT_U,GSem(GSem_AnimTick,"0.01,100,0.01"),"发射周期");

		GELEM_VAR_INIT(RecordID,idFriendlyBullet,RecordID_Invalid);
			GELEM_EDITVAR("友好子弹",GVT_U,GSem(GSem_RecordID,"eos"),"EOs");
		GELEM_VAR_INIT(RecordID,idEnemyBullet,RecordID_Invalid);
			GELEM_EDITVAR("敌对子弹",GVT_U,GSem(GSem_RecordID,"eos"),"EOs");

	END_GOBJ();

	unsigned __int64 idCoreEffect;
	unsigned __int64 idShardEffect;
	int count;

	float spdExplodeVerMin;
	float spdExplodeVerMax;
	float spdExplodeHorMin;
	float spdExplodeHorMax;
	float spdExplodeRollMin;
	float spdExplodeRollMax;
	ValueSet vsWithdrawDist;

	AnimTick delayWithdraw;
	float scaleShardMin;
	float scaleShardMax;

	float htCore;
	float radiusCore;

	AnimTick durReady;
	float radiusMonitor;

	float radiusFire;
	AnimTick durFireCycle;

	RecordID idFriendlyBullet;
	RecordID idEnemyBullet;


};

enum ShardsStage
{
	ShardsStage_None,
	ShardsStage_Wait,
	ShardsStage_Withdraw,
	ShardsStage_Ready,
	ShardsStage_PreFire,
	ShardsStage_Fire,
	ShardsStage_PostFire,
};

class EoShards:public CLoEffectObj
{
public:
	EoShards()
	{
		_bSyncDirty=FALSE;
		_stage=ShardsStage_None;
		_faction=0;
		_tStageStart=0;
		_nFired=0;
		_troop=NULL;
	}
	DEFINE_LEVELOBJ_CLASS(EoShards,CLASSUID_Shards);

	virtual const char *GetShowName()	{		return "Shards";	}
	void SetTroop(CLevelTroop *troop) override{		_troop=troop;	}
		

	void NotifyReady();

protected:

	void _OnPostCreate() override;
	void _OnDetroy()override;

	void _OnUpdate() override;

	void _OnWriteSyncL(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer) override;
	void _OnPostWriteSync() override;

	BOOL _bSyncDirty;

	CLevelTroop *_troop;

	//状态
	ShardsStage _stage;
	AnimTick _tStageStart;
	LevelObjID _idFireLock;
	LevelPos3D _posFireLock;
	DWORD _nFired;

	int _faction;//1表示友方,2表示敌方

};
