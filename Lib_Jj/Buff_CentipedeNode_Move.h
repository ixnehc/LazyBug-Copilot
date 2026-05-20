#pragma once

#include "LevelBuff.h"

#include "LevelGesture.h"

#include "valueset/valueset.h"

#include "behaviorgraph/BehaviorParam.h"

struct BuffParam_CentipedeNode_Move;
class Buff_CentipedeNode_Move;
class CLevelGesture_CentipedeNode_Move:public CLevelGesture_BuildIn
{
public:
	IMPLEMENT_REFCOUNT_C;
	DEFINE_CLASS(CLevelGesture_CentipedeNode_Move);

	CLevelGesture_CentipedeNode_Move()
	{
		Zero();
	}

	void Zero()
	{
		_owner=NULL;
	}

	void Create(Buff_CentipedeNode_Move *owner);

	virtual void Destroy()	{		Zero();	Release();}
	virtual void Update(CUnit3D *unit,float dt) { return;}
	virtual void Update(CUnit *unit,float dt);
	virtual BOOL IsFinished()	{		return _owner==NULL;	}

	void Finish()	{		_owner=NULL;	}

protected:

	Buff_CentipedeNode_Move *_owner;

};


struct BuffParam_CentipedeNode_Move:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_CentipedeNode_Move);

	BEGIN_GOBJ_PURE(BuffParam_CentipedeNode_Move,1);

		GELEM_BEHAVIORMEM_OBJID(varCentipedeAgent,"记录蜈蚣对象的变量","CentipedeNode_Crouch单位里记录蜈蚣对象的变量");

		GELEM_VAR_INIT(BOOL,bCyst,FALSE);
			GELEM_EDITVAR("是否为囊",GVT_S,GSem_Boolean,"是否为囊");
		
	END_GOBJ();

	StringID varCentipedeAgent;
	BOOL bCyst;

};



struct BuffArg_CentipedeNode_Move:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_CentipedeNode_Move);
	BuffArg_CentipedeNode_Move()
	{
	}

};

class CLoCentipede;
class Buff_CentipedeNode_Move:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_CentipedeNode_Move,46)

	Buff_CentipedeNode_Move()
	{
		_ges=NULL;
		_idAgent=LevelObjID_Invalid;
	}

	virtual BOOL NeedSync()	{		return TRUE;	}//是否需要同步给客户端
	virtual BOOL NeedSyncTimeUp()	{		return TRUE;	}//结束时要同步给客户端,以避免客户端出现多个RavenBirth的Buff并存的情况

	virtual LevelBuffMask GetReplaceBuffs();

	BOOL GetLoc(LevelPos &pos,LevelFace &face);

	CLoCentipede *GetLoCentipede();


	virtual void _OnCreate(LevelBuffArg *param);
	virtual void _OnDestroy();

	//Factor Overriding
	BuffFlag GetFlags();

	virtual void _WriteData(CBitPacket *dp);


protected:

	CLevelGesture_CentipedeNode_Move *_ges;
	LevelObjID _idAgent;

};

