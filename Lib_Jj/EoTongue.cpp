
#include "stdh.h"

#include "Level.h"

#include "LevelUtil.h"

#include "LoUnit.h"

#include "EoTongue.h"

#include "LevelRecords.h" 

#include "LevelOSB.h"

#include "Random/Random.h"

#include "PositionBasedDynamics/Simulation/Simulation.h"


//////////////////////////////////////////////////////////////////////////
//CTongueBranch
void CTongueBranch::Init(EoTongue *tongue,LevelPos &pos,float face)
{
	_owner=tongue;
	if (_owner)
		_param=_owner->GetParam<EoParamTongue>();

	_posRoot=pos;
	_faceRoot=face;

	_nodes.resize(_param->nNodes);
	_length=0.0f;

	_stage=CTongueBranch::Stage_Ready;

 	PBD::Simulation *simulation=new PBD::Simulation;



}


//////////////////////////////////////////////////////////////////////////
//EoTongue
BIND_EOPARAM(EoTongue,EoParamTongue);

void EoTongue::_OnPostCreate()
{
	EoParamTongue *paramEo=GetParam<EoParamTongue>();




	//寻找锁定目标
// 	_idTarget=LevelObjID_Invalid;
// 	if (TRUE)
// 	{
// 		CLevelSkill *skill=GetRootSkill();
// 		LevelSkillTarget &target=skill->GetTarget();
// 		CLevelObj *loTarget=LevelUtil_GetTargetObj(_level,target);
// 		if (loTarget)
// 			_idTarget=loTarget->GetID();
// 	}
// 
// 	_core.Init(_level,paramEo,_GetInitialPos(), LevelFaceFromDir(_GetInitialDir()),_level->GetT_());
}

void EoTongue::_WriteState(CBitPacket *bp)
{
}


void EoTongue::_OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	_WriteState(bp);
	bContent=TRUE;
}

void EoTongue::_OnWriteSyncH(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	_WriteState(bp);
	bContent=TRUE;
}



void EoTongue::_OnUpdate()
{


}

