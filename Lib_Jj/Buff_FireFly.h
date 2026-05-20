#pragma once

#include "LevelBuff.h"
#include "anim/keyset.h"

#include "BgnFireFly.h"


struct BuffParam_FireFly:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_FireFly);

	BEGIN_GOBJ_PURE(BuffParam_FireFly,1);

		GELEM_VAR_INIT(float,speedFleeStart,10.0f);
			GELEM_EDITVAR("Flee起始速度",GVT_F,GSem(GSem_Float,"0.1,20.0,0.05"),"Flee起始速度");
		GELEM_VAR_INIT(float,speedFlee,1.0f);
			GELEM_EDITVAR("Flee维持速度",GVT_F,GSem(GSem_Float,"0.1,20.0,0.05"),"Flee维持速度");
		GELEM_VAR_INIT(AnimTick,durFleeAge,ANIMTICK_FROM_SECOND(3.0f));
			GELEM_EDITVAR("Flee寿命",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"Flee寿命");
		GELEM_VAR_INIT(AnimTick,durFleeUnrescure,ANIMTICK_FROM_SECOND(1.0f));
			GELEM_EDITVAR("Flee后多久内无法获救",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"Flee后多久内无法获救");
		GELEM_VAR_INIT( StringID,nmLitSignal,StringID_Invalid);	
			GELEM_EDITVAR( "点亮火炬信号", GVT_U, GSem(GSem_StringID,"信号名称"), "点亮火炬信号" );
	END_GOBJ();

	float speedFleeStart;
	float speedFlee;
	AnimTick durFleeAge;
	AnimTick durFleeUnrescure;
	StringID nmLitSignal;


};

struct BuffArg_FireFly:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_FireFly);
};


class EoEnv;
class Buff_FireFly:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_FireFly,50)

	Buff_FireFly()
	{
		_bInTorch=FALSE;
		_guideTorch=NULL;
		_bFleeing=FALSE;
		_bEnteringTorch=FALSE;
		_eoEnvWorking=NULL;
	}

	BOOL IsFleeing()	{		return _bFleeing;	}
	BOOL GetCurFleePos(LevelPos &posFlee);


	virtual BOOL NeedSync()  override	{		return TRUE;	}//是否需要同步给客户端
	virtual BOOL NeedSyncTimeUp()  override	{		return TRUE;	}//结束时要同步给客户端,以避免客户端出现多个FireFly的Buff并存的情况

	virtual LevelBuffMask GetForbiddingBuffs()  override;
	void LoadTeleport(CLevelBuff *buffOrg) override;

	void HandleEvent(LevelEvent &e) override;

	void SetInTorch(std::vector<FireFlyGuide> &guide,i_math::matrix43f &matTorch)
	{
		_bInTorch=TRUE;
		_guideTorch=&guide;
		_matTorch=matTorch;
	}
	BOOL IsInTorch()	{		return _bInTorch;	}
	void LeaveTorch(CLevelObj *loPlayer);

	BOOL EnterTorch(LevelObjID idTorch,std::vector<FireFlyGuide>&guides,i_math::matrix43f &matBase);


	virtual void _OnCreate(LevelBuffArg *param) override;
	virtual void _OnUpdate(AnimTick dt)  override;
	virtual void _OnDestroy() override;

	virtual void _WriteData(CBitPacket *dp) override;


protected:
	BOOL _CheckInArea(LevelPos &pos);
	EoEnv *_eoEnvWorking;

	BOOL _bInTorch;
	std::vector<FireFlyGuide> *_guideTorch;
	i_math::matrix43f _matTorch;

	BOOL _bFleeing;
	KeySet _ksFlee;

	BOOL _bEnteringTorch;
	AnimTick _tEnterTorch;


};

