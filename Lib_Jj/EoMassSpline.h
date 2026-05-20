#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"


#include "LoEffectObj.h"

#include "EoUtumAttack.h"

#define CLASSUID_MassSpline 68



struct EoParamMassSpline:public LevelEoParam
{
	DEFINE_EOPARAM_CLASS(EoParamMassSpline);

	BEGIN_GOBJ_PURE(EoParamMassSpline,1);

		GELEM_VAR_INIT(float,spdSpawn,10.0f);
			GELEM_EDITVAR("喷射速率",GVT_F,GSem(GSem_Float,"0.0,100.0,0.05"),"每秒喷射几个");
		GELEM_VAR_INIT(RecordID,idEo,RecordID_Invalid);
			GELEM_EDITVAR("喷射Eo",GVT_U,GSem(GSem_RecordID,"eos"),"Eo");
	END_GOBJ();

 	float dur;
 	float spdSpawn;

	RecordID idEo;

};



class EoMassSpline:public CLoEffectObj
{
public:
	EoMassSpline()
	{
		_nSpawned=0;
		_fov=0.0f;
		_tStart=0;
		_dur=0;
	}
	DEFINE_LEVELOBJ_CLASS(EoMassSpline,CLASSUID_MassSpline);

	struct History
	{
		History()
		{
		}
		LevelPos posTarget;
	};

	virtual const char *GetShowName()	{		return "MassSpline";	}
	BOOL _NeedOps() override	{		return TRUE;	}

protected:
	void _OnPostCreate() override;	
	void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer) override;

	void _OnUpdate() override;

	std::deque<History> _history;

	CPathSpline _spline;

	LevelPos3D _posInitial;
	float _faceInitial;
	float _fov;

	DWORD _nSpawned;

	AnimTick _tStart;
	AnimTick _dur;


};
