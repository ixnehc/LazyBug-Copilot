#pragma once

#include "LevelBuff.h"

#include "LevelGesture.h"

#include "valueset/valueset.h"


struct BuffParam_UtumBirth;
class CLevelGesture_UtumBirth:public CLevelGesture_BuildIn
{
public:
	IMPLEMENT_REFCOUNT_C;
	DEFINE_CLASS(CLevelGesture_UtumBirth);

	CLevelGesture_UtumBirth()
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

	void Create(i_math::vector3df &posStart,i_math::vector3df &posEnd,GameTileMap *gtm,BuffParam_UtumBirth *param);

	virtual void Destroy()	{		Zero();	Release();}
	virtual void Update(CUnit3D *unit,float dt);
	virtual void Update(CUnit *unit,float dt)	{		return;}//不支持	}
	virtual BOOL IsFinished()	{		return _bFinished;	}

	BOOL IsAlive()	{		return _gtm!=NULL;	}

protected:

	float _t;

	GameTileMap * _gtm;
	BOOL _bFinished;

	BuffParam_UtumBirth *_param;

	i_math::vector3df _posStart;
	i_math::vector3df _posEnd;

};


struct BuffParam_UtumBirth:public LevelBuffParam
{
	DEFINE_BUFFPARAM_CLASS(BuffParam_UtumBirth);

	BuffParam_UtumBirth();
	~BuffParam_UtumBirth();
		

	BEGIN_GOBJ(BuffParam_UtumBirth,1);

		GELEM_VAR_INIT(float,dur,2.0f);
			GELEM_EDITVAR("持续时间",GVT_F,GSem(GSem_Float,"0.1,100,0.1"),"持续时间,单位为秒");

		GELEM_OBJVAR( ValueSet, vsHor);
			GELEM_EDITOBJ_EX( "平移变化", "随时间变化的平移距离的变化", GSem( GSem_Unknown, "0,0,-1,20") );

	END_GOBJ();

	float dur;
	ValueSet vsHor;

};



struct BuffArg_UtumBirth:public LevelBuffArg
{
	DEFINE_CLASS(BuffArg_UtumBirth);
	BuffArg_UtumBirth()
	{
	}

	i_math::vector3df posStart;
	i_math::vector3df posEnd;


};

class Buff_UtumBirth:public CLevelBuff
{
public:
	DEFINE_BUFF_CLASS(Buff_UtumBirth,23)

	Buff_UtumBirth()
	{
	}

	virtual BOOL NeedSync()	{		return TRUE;	}//是否需要同步给客户端
	virtual BOOL NeedSyncTimeUp()	{		return TRUE;	}//结束时要同步给客户端,以避免客户端出现多个UtumBirth的Buff并存的情况
	virtual LevelOp_AddBuff *AccuireSyncOp()	{		return NULL;	}

	virtual LevelBuffMask GetReplaceBuffs();

	virtual void _OnCreate(LevelBuffArg *param);
	virtual void _OnDestroy();

	//Factor Overriding
	BuffFlag GetFlags()	{		return BuffFlag_NotAttackable;	}

	virtual void _WriteData(CBitPacket *dp);

protected:

	float _eulerX;

};

