/********************************************************************
	created:	2013/5/14 
	author:		cxi
	
	purpose:	滚翻释放eo
*********************************************************************/
#include "stdh.h"


#include "Skill_Roll.h"

#include "LevelRecordSkill.h"

#include "LoUnit.h"
#include "Level.h"

#include "LevelOSB.h"

#include "LevelDecider.h"

#include "LevelObjMove.h"



////////////////////////////////////////////////////////////////////////
//CSkillGesture_RollEO
void CSkillGesture_Roll::Create(KeySet *ks,float dur,float dur2)
{
	_tCur=0.0f;
	_dur=dur;
	_dur2=dur2;

	assert(ks);
	assert(ks->GetKeyCount()>0);
	_ks=ks;

	_bAlive=TRUE;
}


void CSkillGesture_Roll::Update(CUnit *unit,float dt)
{
	if (_bFinished)
		return;
	if (!_ks)
		return;
	assert(_ks->GetKeyCount()>0);
	if (_ks->GetKeyCount()<=0)
		return;
	_tCur+=dt;

	if (_tCur>_dur)
		_tCur=_dur;

	Key_2f k;
	_ks->CalcKey(ANIMTICK_FROM_SECOND(_tCur),&k);
	unit->_pos=k.v;

//	if (_tCur>=_dur+_dur2)
	if (_tCur>=_dur)
		_bFinished=TRUE;

}



//////////////////////////////////////////////////////////////////////////
//CSkill_Roll
BIND_SKILLPARAM(Skill_Roll,SkillParam_Roll);


void Skill_Roll::_OnStart()
{
	_AddStartOp();

	GetLevel()->AddAffect(_owner);
	_SetState(SkillState_Casting);


	BOOL bOk=FALSE;
	LevelPos posSrc=_owner->GetFramePos();
	if (_arg)
	{
		if (_arg->sites.size()>0)
		{
			SkillParam_Roll*param=_rec->GetParam<SkillParam_Roll>();
			if (param)
			{
				CUnit *unit=_owner->GetUnit();
				if (unit)
				{
					_ges=Class_New2(CSkillGesture_Roll);

					extern void LevelUtil_BuildPathKeyset(KeySet &ks,std::vector<LevelPos>&sites,float speed);
					LevelUtil_BuildPathKeyset(_ksPath,_arg->sites,param->speed);
					
					_ges->Create(&_ksPath,ANIMTICK_TO_SECOND(param->dur),
												ANIMTICK_TO_SECOND(param->dur2));
					_ges->AddRef();
					unit->SetGesture(_ges);
					bOk=TRUE;

				}
			}
		}
	}

	if (!bOk)
		_SetState(SkillState_Fail);

}


void Skill_Roll::_OnUpdate(AnimTick dt)
{
	if (_state==SkillState_Casting)
	{
		SkillParam_Roll*param=_rec->GetParam<SkillParam_Roll>();


		//释放Deal
		if (_nSpawned<param->nDeal)
		{
			if (param->nDeal>0)
			{
				_tCur+=dt;
				AnimTick durPerEO=param->dur/param->nDeal;
				DWORD nToSpawn;
				if (durPerEO<=0)
					nToSpawn=param->nDeal;
				else
				{
					nToSpawn=_tCur/durPerEO+1;
					if (nToSpawn>param->nDeal)
						nToSpawn=param->nDeal;
				}

				if (_ges)
				{
					if (!_ges->IsAlive())
						nToSpawn=param->nDeal;
				}


				while(nToSpawn>_nSpawned)
				{
					AnimTick t=_nSpawned*durPerEO;

					Key_2f k;
					_ksPath.CalcKey(t,&k);
					LevelPos pos=k.v;

					//在pos的位置释放一个Deal
					if (_rec->deal)
					{
						DealArg arg;
						arg.grd=_grd;
						arg.dir.set(0,0,0);
						arg.link=LevelOpLink();
						extern LevelPos3D LevelUtil_GetGroundHeight(CLevel *lvl,float x,float y,BOOL bHiReso);
						LevelPos3D pos3D=LevelUtil_GetGroundHeight(GetLevel(),pos.x,pos.y,TRUE);
						_MakeDeals(pos3D,arg);
					}
					_nSpawned++;
				}
			}
		}


		if (_ges)
		{
			if (!_ges->IsAlive())
				_Finish();
		}
	}
}

void Skill_Roll::_OnBreak()
{
	_Finish();
}


void Skill_Roll::_Finish()
{
	if (_ges)
		_ges->Stop();
	SAFE_RELEASE(_ges);
	_ksPath.Clean();
	_SetState(SkillState_Finished);
}
