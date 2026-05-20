#pragma once

#include "LevelBuff.h"

#include "Buff_Stun.h"

#include "LevelObjPauser.h"

#include "LevelStrike.h"

#include "behaviorgraph/BehaviorParam.h"

struct BuffParam_CentipedeNode_Tendrils:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_CentipedeNode_Tendrils);

	BEGIN_GOBJ_PURE(BuffParam_CentipedeNode_Tendrils,1);

		GELEM_BEHAVIORMEM_OBJID(varCentipedeAgent,"记录蜈蚣对象的变量","记录蜈蚣对象的变量");
		GELEM_VAR_INIT(unsigned __int64,tendril,0);
			GELEM_EDITVAR("触须Proto",GVT_Bx8,GSem_ProtoPath,"触须Proto");
		GELEM_VAR_INIT(unsigned __int64,hang,0);
			GELEM_EDITVAR("耷拉Proto",GVT_Bx8,GSem_ProtoPath,"耷拉Proto");
		GELEM_DYNOBJPTR_DEAL(CLevelDeal,dealDisappear,Deal_MakeBuff, "消失的Deal", "消失的Deal" );
			GELEMS_LEVELDEAL_CANDIDATES();
		GELEM_VAR_INIT(float,hpratioToDisappear,0.5f);
			GELEM_EDITVAR("HP比率降到多少消失",GVT_F,GSem(GSem_Float,"0.01,1.0,0.05"),"HP比率降到多少消失");

	END_GOBJ();

	StringID varCentipedeAgent;

	unsigned __int64 tendril;
	unsigned __int64 hang;

	CLevelDeal *dealDisappear;
	float hpratioToDisappear;


};


struct BuffArg_CentipedeNode_Tendrils:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_CentipedeNode_Tendrils);
	BuffArg_CentipedeNode_Tendrils()
	{
	}
};

class Buff_CentipedeNode_Tendrils:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_CentipedeNode_Tendrils,44)

	Buff_CentipedeNode_Tendrils()
	{
		_idAgent=LevelObjID_Invalid;
		_iNode=0;
		_bOwnerBroken=FALSE;
		_tOwnerBroken=0;
		_idDisappearBuff=RecordID_Invalid;
	}

	virtual BOOL NeedSync()	{		return TRUE;	}//是否需要同步给客户端

	virtual void _OnCreate(LevelBuffArg *param) override;
	virtual void _OnDestroy() override;
	virtual void _OnUpdate(AnimTick dt);

	virtual void HandleEvent(LevelEvent &e) override;

	//Factor Overriding
	BuffFlag GetFlags()	{		return 0;	}

	void SetOwnerBroken();
	BOOL IsOwnerBroken()	{		return _bOwnerBroken;	}

	BOOL IsDisappear();

protected:

	virtual void _WriteData(CBitPacket *dp);

	DWORD _idAgent;

	DWORD _iNode;//第几个体节

	BOOL _bOwnerBroken;
	AnimTick _tOwnerBroken;

	RecordID _idDisappearBuff;

};

