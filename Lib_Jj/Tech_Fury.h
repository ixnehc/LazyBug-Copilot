#pragma once

#include "LevelTech.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"


#include "Random/Random.h"

#include "LevelTech.h"

struct TechParam_Fury:public LevelTechParam
{
	DEFINE_TECHPARAM();
	BEGIN_GOBJ_PURE(TechParam_Fury,1)
		GELEM_TECHPARAM_BASE();
		GELEM_VAR_INIT(float,furyPerHit,0.2f);	GELEM_UID(1);
		GELEM_VAR_INIT(float,furyCD,0.05f);			GELEM_UID(2);
		GELEM_VAR_INIT(AnimTick,durActiveLasting,ANIMTICK_FROM_SECOND(12.0f));					GELEM_UID(3);
		GELEM_VAR_INIT(float,rateCrush,0.4f);			GELEM_UID(4);
		GELEM_VAR_INIT(float,rateHit,1.0f);				GELEM_UID(5);
		GELEM_VAR_INIT(short,dmgFire,100);			GELEM_UID(6);
		GELEM_VAR_INIT(AnimTick,durActiveLastingAdd,ANIMTICK_FROM_SECOND(0.5f));					GELEM_UID(7);
		GELEM_VAR_INIT(float,dmgFireRate,0.1f);			GELEM_UID(8);
	END_GOBJ();

	AnimTick durActiveLasting;
	float furyPerHit;//每次命中加多少Fury
	float furyCD;//每秒降多少Fury
	float rateHit;
	float rateCrush;
	short dmgFire;

	//升级
	AnimTick durActiveLastingAdd;
	float dmgFireRate;

};

struct TechSync_Fury:public LevelTechSync
{
	DEFINE_TECHSYNC(TechSync_Fury);

	TechSync_Fury()
	{
		fury=0;
		bActive=0;
	}

	BYTE fury:7;
	BYTE bActive:1;
};

class CTech_Fury:public CLevelTech
{
public:
	DEFINE_CLASS(CTech_Fury);

	CTech_Fury()
	{
		_fury=0.0f;
		_bActive=FALSE;
		_durActive=0;
		_durActiveLastingRT=0;
		_dmgFireRT=0;
		_tLastDamage=0;
	}

	virtual void OnCreate(){}
	virtual void OnDestroy(){}
	virtual void OnUpdate(AnimTick dt);
	virtual void OnEvent(LevelEvent &e);
	virtual void OnBuildRT();
	virtual void OnClearRT();

	virtual void SaveSync(LevelTechSync &sync);

protected:
	float _fury;
	BOOL _bActive;
	AnimTick _durActive;

	//RT
	AnimTick _durActiveLastingRT;
	short _dmgFireRT;
	AnimTick _tLastDamage;


};


