#pragma once

#include "LevelBuff.h"

#include "LevelGesture.h"

#include "valueset/valueset.h"


struct BuffParam_FlyBirth;
class CLevelGesture_FlyBirth:public CLevelGesture_BuildIn
{
public:
	IMPLEMENT_REFCOUNT_C;
	DEFINE_CLASS(CLevelGesture_FlyBirth);

	CLevelGesture_FlyBirth()
	{
		Zero();
	}

	void Zero()
	{
		_t=0.0f;
		_gtm=NULL;
		_bFinished=FALSE;
		_param=FALSE;
	}

	void Create(i_math::vector3df &posInit,i_math::vector2df &dir,GameTileMap *gtm,BuffParam_FlyBirth *param);

	virtual void Destroy()	{		Zero();	Release();}
	virtual void Update(CUnit3D *unit,float dt);
	virtual void Update(CUnit *unit,float dt)	{		return;}//不支持	}
	virtual BOOL IsFinished()	{		return _bFinished;	}

	BOOL IsAlive()	{		return _gtm!=NULL;	}

protected:

	float _t;

	GameTileMap * _gtm;
	BOOL _bFinished;

	BuffParam_FlyBirth *_param;

	i_math::vector3df _posInit;
	i_math::vector2df _dir;

};


struct BuffParam_FlyBirth:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_FlyBirth);

	BuffParam_FlyBirth();
	~BuffParam_FlyBirth();
		

	BEGIN_GOBJ(BuffParam_FlyBirth,1);

		GELEM_VAR_INIT(float,dur,2.0f);
			GELEM_EDITVAR("起飞持续时间",GVT_F,GSem(GSem_Float,"0.1,100,0.1"),"持续时间,单位为秒");

		GELEM_OBJVAR( ValueSet, vsVer);
			GELEM_EDITOBJ_EX( "高度变化", "随时间变化的高度变化", GSem( GSem_Unknown, "0,0,1,20") );
		GELEM_OBJVAR( ValueSet, vsHor);
			GELEM_EDITOBJ_EX( "平移变化", "随时间变化的平移距离的变化", GSem( GSem_Unknown, "0,0,1,20") );

	END_GOBJ();

	float dur;

	ValueSet vsVer;
	ValueSet vsHor;

};



struct BuffArg_FlyBirth:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_FlyBirth);
	BuffArg_FlyBirth()
	{
	}
	LevelOpDesc descOp;

	i_math::vector3df posInit;
	i_math::vector2df dir;


};

class Buff_FlyBirth:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_FlyBirth,15)

	Buff_FlyBirth()
	{
		_op=NULL;
	}

	virtual BOOL NeedSync()	{		return TRUE;	}//是否需要同步给客户端
	virtual BOOL NeedSyncTimeUp()	{		return TRUE;	}//结束时要同步给客户端,以避免客户端出现多个RavenBirth的Buff并存的情况
	virtual LevelOp_AddBuff *AccuireSyncOp()	{		return _op;	}

	virtual LevelBuffMask GetReplaceBuffs();

	virtual void _OnCreate(LevelBuffArg *param);
	virtual void _OnDestroy();

	//Factor Overriding
	BuffFlag GetFlags()	{		return BuffFlag_NotAttackable|BuffFlag_Birth;	}

	virtual void _WriteData(CBitPacket *dp);

protected:
	LevelOp_AddBuff *_op;


};

