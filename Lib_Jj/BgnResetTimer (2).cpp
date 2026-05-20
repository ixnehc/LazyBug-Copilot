/********************************************************************
	created:	2013/5/29 
	author:		cxi
	
	purpose:	GA功能:创建单位
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "BgnSpawnUnit.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoGeneralAgent.h"

#include "LoUnit.h"

#include "Buff_Birth.h"


#include "Log/LogDump.h"
#include "Random/Random.h"

////////////////////////////////////////////////////////////////////////
//CBgn_SpawnUnit
BIND_BGN_CLASS(CBgn_SpawnUnit,CBgp_SpawnUnit);

void CBgn_SpawnUnit::_Spawn(CLevel *level,CLevelTroop *troop,BP_SpawnUnit &info,LevelPos &pos,float eulerX,StringID senarioAI)
{
	CLoUnit* lo=NULL;
	lo=(CLoUnit*)level->CreateObj(Class_Ptr2(CLoUnit));
	lo->PostCreate(LevelPlayerID_Wild,NULL,info.idUnit,info.grd,pos,eulerX);
	level->AddToActives(lo);
	if (info.idBirthBuff!=RecordID_Invalid)
	{
		BuffArg_Birth arg;
		arg.eulerX=eulerX;
		level->GetDecider()->MakeBuff(lo,info.idBirthBuff,0,&arg,FALSE);
	}

	lo->SetAIScenario(senarioAI);

	if (troop)
		troop->AddUnit(NULL,lo->GetID());

	SAFE_RELEASE(lo);
}


void CBgn_SpawnUnit::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_SpawnUnit*pad=_GetPad<CBgp_SpawnUnit>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	BP_SpawnUnit *param=&pad->_param;

	if (param)
	{
		LevelPos pos;
		if(_GetPos(pad->_nmPos,pos))
		{
			float eulerX=CSysRandom::RandRange<float>(0.0f,i_math::Pi*2.0f);//随机角度

			_Spawn(level,_ObtainTroop(pad->_troop),*param,pos,eulerX,param->senarioAI);
		}
	}

	_OutputOk(outputs,1,"结束");
}

