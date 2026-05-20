
#include "stdh.h"

#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelUtil.h"

#include "LoUnit.h"

#include "LoStarPlate.h"
#include "LoBelly.h"

#include "EoBellyEgg.h"

#include "Buff_SacredOrb.h"

#include "LevelRecords.h"
#include "LevelRecordGlobal.h"

#include "timer/profiler.h"
#include "Random/Random.h"

#define SACREDORB_DISPELL_DISTANCE (12.0f)

extern void UnitCollide_SetAlly(UnitCollide &collide,DWORD ally);
extern void UnitCollide_SetPlayer(UnitCollide &collide,BOOL bPlayer);
extern void UnitCollide_SetLayor(UnitCollide &collide,DWORD layor);



//////////////////////////////////////////////////////////////////////////
//CBellyMinionCombatState
void CBellyMinionCombatState::Init(CLevel *level,std::vector<LevelObjID>&idsMinion,LevelObjID idKing_)
{
	_level=level;

	Entry empty;
	for (int i=0;i<idsMinion.size();i++)
	{
		LevelObjID id=idsMinion[i];
		CLevelObj *lo=LevelUtil_GetAliveLo(level,id);
		if (lo)
		{
			empty.posGuide=lo->GetFramePos();
			_entries[id]=empty;
		}
	}
	_idKing=idKing_;
}

void CBellyMinionCombatState::ResetAttack(CLevelObj *loEnemy)
{
	_stage=Stage_Attack;
	_idEnemy=loEnemy->GetID();

	std::unordered_map<LevelObjID,CBellyMinionCombatState::Entry>::iterator it;
	for (it=_entries.begin();it!=_entries.end();it++)
	{
		CBellyMinionCombatState::Entry&e=(*it).second;
		e.action=CBellyMinionCombatState::ActionType_Hop;
	}
}


void CBellyMinionCombatState::RefreshGuidePos()
{
	CLevelObj *loKing=LevelUtil_GetAliveLo(_level,_idKing);
	if (!loKing)
		return;

	CLevelObj *loEnemy=LevelUtil_GetAliveLo(_level,_idEnemy);
	if (!loEnemy)
		return;

	LevelPos posEnemy=loEnemy->GetFramePos();
	LevelPos posKing=loKing->GetFramePos();
	LevelFace faceMain=LevelFaceFromDir(loEnemy->GetFramePos()-posKing);

	BOOL bEnemySacredOrb=CheckEnemySacredOrb();

	struct CompareNode
	{
		bool operator < (const CompareNode &other)const {return yaw>other.yaw;}
		LevelObjID id;
		LevelFaceYaw yaw;
	};

	std::vector<CompareNode> nodes;
	nodes.resize(_entries.size());
	if (TRUE)
	{
		CLevel *level=loEnemy->GetLevel();
		int c=0;
		std::unordered_map<LevelObjID,Entry>::iterator it;
		for (it=_entries.begin();it!=_entries.end();it++)
		{
			LevelObjID id=(*it).first;
			Entry &e=(*it).second;
			nodes[c].id=id;
			CLevelObj *loMe=LevelUtil_GetAliveLo(level,id);
			if (loMe)
			{
				LevelPos posMe=loMe->GetFramePos();
				if(e.action==ActionType_Hop)
				{
					LevelFace face=LevelFaceFromDir(posMe-posKing);
					nodes[c].yaw=LevelFaceCalcYaw(faceMain,face);
					c++;
				}
				else
					e.posGuide=posMe;
			}
		}

		nodes.resize(c);
	}

	std::sort(nodes.begin(),nodes.end());

	const float radius=2.5f;
	const float fov=i_math::deg2rad(160.0f);
	float step=fov/(float)nodes.size();
	for (int i=0;i<nodes.size();i++)
	{
		float yaw=nodes[i].yaw;
		float face=faceMain-fov*0.5f+step*0.5f+step*(float)i;

		LevelPos posGuide=posKing+LevelFaceToDir(face)*radius;

		if (bEnemySacredOrb)
		{
			if (posGuide.getDistanceFrom(posEnemy)<SACREDORB_DISPELL_DISTANCE)
			{
				LevelPos dir=(posGuide-posEnemy);
				dir.safe_normalize();
				posGuide=posEnemy+dir*SACREDORB_DISPELL_DISTANCE;
			}
		}

		_entries[nodes[i].id].posGuide=posGuide;
	}
}

BOOL CBellyMinionCombatState::GetGuidePos(LevelObjID id,LevelPos &posGuide)
{
	std::unordered_map<LevelObjID,Entry>::iterator it=_entries.find(id);
	if (it==_entries.end())
		return FALSE;

	posGuide=(*it).second.posGuide;
	return TRUE;
}

CLevelObj *CBellyMinionCombatState::GetEnemyLo()
{
	if (_idEnemy==LevelObjID_Invalid)
		return NULL;
	return LevelUtil_GetAliveLo(_level,_idEnemy);
}

BOOL CBellyMinionCombatState::CheckEnemySacredOrb()
{

	BellySetting &setting=LevelUtil_GetBellySetting(_level);
	if (setting.idBuff_SacredOrb!=RecordID_Invalid)
	{
		CLevelObj *lo=GetEnemyLo();
		Buff_SacredOrb *buff=(Buff_SacredOrb *)LevelUtil_FindBuffByRecordID(lo,setting.idBuff_SacredOrb);
		if (buff)
		{
			if (buff->CanDispel())
				return TRUE;
		}
	}
	return FALSE;
}




////////////////////////////////////////////////////////////////////////
//CLoBelly

EoEnv *CLoBelly::_GetEoEnv()
{
	if (_level)
	{
		CLevelObj *lo=_level->GetEoEnv();
		if (lo)
		{
			if (lo->GetClass()->IsSameWith(Class_Ptr2(EoEnv)))
				return (EoEnv*)lo;
		}
	}
	return NULL;
}


void CLoBelly::PostCreate()
{
	CLoAgent::PostCreate();

	LopBelly *lop=(LopBelly*)_param;
	LosBelly *los=(LosBelly*)_src;
}

void CLoBelly::OnDestroy()
{
	if (_hEnemyLichenDispel)
	{
		EoEnv *eoEnv=_GetEoEnv();
		if (eoEnv)
			eoEnv->StopLichen(_hEnemyLichenDispel);
		_hEnemyLichenDispel=EoEnvLichenHandle_Invalid;
	}
	LevelUtil_DestroyEnvEo(_level);

	LevelUtil_DestroyLo(_level,_idBelly);
	LevelUtil_DestroyLo(_level,_idBellyKing);
	LevelUtil_DestroyLo(_level,_idBellyQueen);

	for(int i=0;i<_idsBellyMinion.size();i++)
		LevelUtil_DestroyLo(_level,_idsBellyMinion[i]);
	_idsBellyMinion.clear();

	if (TRUE)
	{
		std::unordered_map<LevelObjID,CUnit*>::iterator it;
		for (it=_unitsMinion.begin();it!=_unitsMinion.end();it++)
			((*it).second)->Destroy();
		_unitsMinion.clear();
	}
	if (TRUE)
	{
		std::unordered_map<LevelObjID,CUnit*>::iterator it;
		for (it=_unitsEgg.begin();it!=_unitsEgg.end();it++)
			((*it).second)->Destroy();
		_unitsEgg.clear();
	}

	SAFE_DESTROY(_unitEnemy);
	SAFE_DESTROY(_unitPlayerSurround);
	SAFE_DESTROY(_unitKing);

	_unitmgr.Destroy();

	Zero();
}

BellySetting& CLoBelly::_GetBellySetting()
{
	return LevelUtil_GetBellySetting(_level);
}



BOOL CLoBelly::OnActivate()
{
//	return TRUE;
	LopBelly *lop=(LopBelly*)_param;
	BellySetting &setting=_GetBellySetting();

	_level->RegisterUniqueObj(LevelUniqueObj_Belly,this);

	if ((lop->locsBelly.size()>0)&&(setting.idUnit_Belly!=RecordID_Invalid))
	{
		i_math::xformf xfm;
		xfm.fromMatrix(lop->locsBelly[0]);
		LevelFace face=LevelFaceFromQuat(xfm.rot);
		_idBelly=LevelUtil_CreateUnit(_level,setting.idUnit_Belly,xfm.pos.getXZ(),face,LevelPlayerID_Wild);
	}

	if ((lop->locsBellyKing.size()>0)&&(setting.idUnit_BellyKing!=RecordID_Invalid))
	{
		i_math::xformf xfm;
		xfm.fromMatrix(lop->locsBellyKing[0]);
		LevelFace face=LevelFaceFromQuat(xfm.rot);
		_idBellyKing=LevelUtil_CreateUnit(_level,setting.idUnit_BellyKing,xfm.pos.getXZ(),face,LevelPlayerID_Wild);
	}

	if ((lop->locsBellyQueen.size()>0)&&(setting.idUnit_BellyQueen!=RecordID_Invalid))
	{
		i_math::xformf xfm;
		xfm.fromMatrix(lop->locsBellyQueen[0]);
		LevelPos3D pos3D=LevelUtil_GetGroundHeight(_level,xfm.pos.x,xfm.pos.z,TRUE);
		pos3D.y+=setting.htBellyQueen;
		LevelFace face=LevelFaceFromQuat(xfm.rot);

		_idBellyQueen=LevelUtil_CreateUnit(_level,setting.idUnit_BellyQueen,pos3D,face,LevelPlayerID_Wild);
	}

	if (setting.idUnit_BellyMinion!=RecordID_Invalid)
	{
		_idsBellyMinion.resize(lop->locsBellyMinion.size());
		for (int i=0;i<lop->locsBellyMinion.size();i++)
		{
			i_math::xformf xfm;
			xfm.fromMatrix(lop->locsBellyMinion[i]);
			LevelFace face=LevelFaceFromQuat(xfm.rot);

			_idsBellyMinion[i]=LevelUtil_CreateUnit(_level,setting.idUnit_BellyMinion,xfm.pos.getXZ(),face,LevelPlayerID_Wild);
		}

		_stateCombat.Init(_level,_idsBellyMinion,_idBellyKing);

	}

	_unitmgr.Create(_level->GetUnitMgr()->GetNavData());
	if (TRUE)
	{
		UnitCollide collide=UnitCollide_Empty;
		UnitCollide_SetAlly(collide,LevelUtil_PlayerIDToUnitCollideAlly(LevelPlayerID_Wild));
		UnitCollide_SetPlayer(collide,FALSE);
		UnitCollide_SetLayor(collide,3);

		for (int i=0;i<_idsBellyMinion.size();i++)
		{
			LevelObjID id=_idsBellyMinion[i];
			CLevelObj *lo=LevelUtil_GetAliveLo(_level,id);
			CUnit *unit=_unitmgr.CreateUnit(lo->GetFramePos(),lo->GetFrameFace(),lo->GetRadius_(),6.0f,NULL);

			unit->SetCollide(collide);

			_unitsMinion[id]=unit;
		}

		if (TRUE)
		{
			CLevelObj *lo=LevelUtil_GetAliveLo(_level,_idBellyKing);
			if (lo)
			{
				_unitKing=_unitmgr.CreateUnit(lo->GetFramePos(),lo->GetFrameFace(),3.0f,6.0f,NULL);
				_unitKing->SetCollide(collide);
			}
		}

	}



	return TRUE;
}

void CLoBelly::OnDeactivate()
{
	_level->UnregisterUniqueObj(LevelUniqueObj_Belly,this);
}


void CLoBelly::Update()
{
	LopBelly *lop=(LopBelly*)_param;
	BellySetting &setting=_GetBellySetting();

	if (_stateCombat._stage==CBellyMinionCombatState::Stage_None)
	{
		CLevelObj *loPlayer=NULL;
		if(TRUE)
		{
			CLoUnit *loUnit=LevelUtil_GetFirstPlayerLoUnit(_level);
			if (loUnit)
			{
				LevelPos posPlayer=loUnit->GetFramePos();
				for (int i=0;i<lop->zoneCombat.size();i++)
				{
					if (lop->zoneCombat[i].isPointIn(posPlayer))
					{
						loPlayer=loUnit;
						break;
					}
				}
			}
		}
		if (loPlayer)
		{
			LevelUtil_CreateEnvEo(this,setting.idEnvEo);
			_stateCombat.ResetAttack(loPlayer);
		}
	}

	//更新Enemy的LichenDispel
	if (EoEnv *eoEnv=_GetEoEnv())
	{
		CLevelObj *loEnemy=_stateCombat.GetEnemyLo();
		if (_stateCombat.CheckEnemySacredOrb())
		{
			if (_hEnemyLichenDispel==EoEnvLichenHandle_Invalid)
			{
				if (loEnemy)
					_hEnemyLichenDispel=eoEnv->StartLichen(loEnemy->GetID(),6.0f,TRUE,4.0f,1.0f,3.0f);
			}
		}
		else
		{
			if (_hEnemyLichenDispel!=EoEnvLichenHandle_Invalid)
			{
				eoEnv->StopLichen(_hEnemyLichenDispel);
				_hEnemyLichenDispel=EoEnvLichenHandle_Invalid;
			}
		}
		if (_hEnemyLichenDispel!=EoEnvLichenHandle_Invalid)
		{
			if (loEnemy)
				eoEnv->UpdateLichenPos(_hEnemyLichenDispel,loEnemy->GetFramePos());
		}
	}

	if (_stateCombat._stage==CBellyMinionCombatState::Stage_Attack)
	{
		_RefreshGuidePos();

		if (FALSE)
		{
			std::unordered_map<LevelObjID,CBellyMinionCombatState::Entry>::iterator it;
			for (it=_stateCombat._entries.begin();it!=_stateCombat._entries.end();it++)
			{
				CLevelObj *loMinion=LevelUtil_GetAliveLo(_level,(*it).first);
				if (loMinion)
				{
					CLevelSkillDriver *driver=loMinion->GetSkillDriver();
					if (driver)
					{
						LevelSkillTarget target;
						target.SetPos((*it).second.posGuide);
						driver->StartFollow(target,0.02f);
					}
				}
			}
	
		}

	}


}


void CLoBelly::_OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	LopBelly *lop=(LopBelly*)_param;

}

void CLoBelly::_OnWriteSyncL(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	LopBelly *lop=(LopBelly*)_param;

}


void CLoBelly::_OnPostWriteSync()
{
}

void CLoBelly::RegisterEgg(LevelObjID idEgg)
{
	CLevelObj *lo=LevelUtil_GetAliveLo(_level,idEgg);
	if (!lo)
		return;

	_eggs.insert(idEgg);

	UnitCollide collide=UnitCollide_Empty;
	UnitCollide_SetAlly(collide,LevelUtil_PlayerIDToUnitCollideAlly(LevelPlayerID_Wild));
	UnitCollide_SetPlayer(collide,FALSE);
	UnitCollide_SetLayor(collide,3);

	const float radiusEgg=0.2f;
	CUnit *unit=_unitmgr.CreateUnit(lo->GetFramePos(),lo->GetFrameFace(),0.2f,6.0f,NULL);

	unit->SetCollide(collide);
	_unitsEgg[idEgg]=unit;

}

void CLoBelly::UnregisterEgg(LevelObjID idEgg)
{
	if (TRUE)
	{
		std::unordered_set<LevelObjID>::iterator it=_eggs.find(idEgg);
		if (it!=_eggs.end())
			_eggs.erase(it);
	}

	if (TRUE)
	{
		std::unordered_map<LevelObjID,CUnit*>::iterator it=_unitsEgg.find(idEgg);
		if (it!=_unitsEgg.end())
		{
			(*it).second->Destroy();
			_unitsEgg.erase(it);
		}
	}
}

void CLoBelly::_RefreshGuidePos()
{
	_stateCombat.RefreshGuidePos();
}


LevelObjID CLoBelly::_FindEggToStomp(LevelObjID idMinion)
{
	if (_stateCombat._stage==CBellyMinionCombatState::Stage_None)
		return FALSE;

	std::unordered_set<LevelObjID> eggsAvailable=_eggs;
	if (TRUE)
	{
		std::unordered_map<LevelObjID,CBellyMinionCombatState::Entry>::iterator it;
		for (it=_stateCombat._entries.begin();it!=_stateCombat._entries.end();it++)
		{
			CBellyMinionCombatState::Entry &e=(*it).second;
			if (e.action==CBellyMinionCombatState::ActionType_StompEgg)
			{
				std::unordered_set<LevelObjID>::iterator itRemove=eggsAvailable.find(e.idGuideEgg);
				if (itRemove!=eggsAvailable.end())
					eggsAvailable.erase(itRemove);
			}
		}
	}

	const float rangeStomp=3.0f;

	CLevelObj *loMinion=LevelUtil_GetAliveLo(_level,idMinion);
	CLevelObj *loEnemy=LevelUtil_GetAliveLo(_level,_stateCombat._idEnemy);
	if (loMinion&&loEnemy)
	{
		LevelPos posMinion=loMinion->GetFramePos();
		LevelPos posEnemy=loEnemy->GetFramePos();

		std::unordered_set<LevelObjID>::iterator it;
		for(it=eggsAvailable.begin();it!=eggsAvailable.end();it++)
		{
			LevelObjID idEgg=(*it);
			EoBellyEgg*eoEgg=(EoBellyEgg*)LevelUtil_GetAliveLo(_level,idEgg);
			if (eoEgg)
			{
				if (!eoEgg->IsTriggered())
				{
					LevelPos posEgg=eoEgg->GetFramePos();

					if (posMinion.getDistanceFrom(posEgg)<rangeStomp)
					{
						if (posEnemy.getDistanceFrom(posEgg)<eoEgg->GetOffendRange())
							return idEgg;
					}
				}
			}
		}
	}

	return LevelObjID_Invalid;
}


CBellyMinionCombatState::ActionType CLoBelly::RequestMinionAction(LevelObjID idMinion,LevelPos &posTarget,LevelObjID &idTarget)
{
	if (_stateCombat._stage==CBellyMinionCombatState::Stage_None)
		return CBellyMinionCombatState::ActionType_None;

	std::unordered_map<LevelObjID,CBellyMinionCombatState::Entry>::iterator it;
	it=_stateCombat._entries.find(idMinion);
	if (it==_stateCombat._entries.end())
		return CBellyMinionCombatState::ActionType_None;

	CLevelObj *loEnemy=_stateCombat.GetEnemyLo();
	if (!loEnemy)
		return CBellyMinionCombatState::ActionType_None;

	BOOL bEnemySacredOrb=_stateCombat.CheckEnemySacredOrb();

	AnimTick tCur=_level->GetT_();

	CBellyMinionCombatState::Entry &e=(*it).second;

	if (e.action==CBellyMinionCombatState::ActionType_StompEgg)
	{
		//Continue stomp egg
		if (FALSE)
		{
			EoBellyEgg *eoEgg=(EoBellyEgg *)LevelUtil_GetAliveLo(_level,e.idGuideEgg);
			if (eoEgg)
			{
				posTarget=eoEgg->GetFramePos();
				idTarget=loEnemy->GetID();
				return CBellyMinionCombatState::ActionType_StompEgg;
			}
		}

		//Go back to hop
		e.action=CBellyMinionCombatState::ActionType_Hop;
		_RefreshGuidePos();
		e.idGuideEgg=LevelObjID_Invalid;

		posTarget=_CalcMinionHopTargetPos(idMinion);
		idTarget=loEnemy->GetID();
		return CBellyMinionCombatState::ActionType_Hop;
	}

	if (e.action==CBellyMinionCombatState::ActionType_Hop)
	{
		//Try to stomp egg
		if (_stateCombat._stage==CBellyMinionCombatState::Stage_Attack)
		{
			if (!bEnemySacredOrb)
			{
				LevelObjID idEgg=_FindEggToStomp(idMinion);
				if (idEgg!=LevelObjID_Invalid)
				{
					CLevelObj *loEgg=LevelUtil_GetAliveLo(_level,idEgg);
					if (loEgg)
					{
						posTarget=loEgg->GetFramePos();
						idTarget=loEnemy->GetID();
						CUnit *unitMinion=_FindMinionUnit(idMinion);
						if (unitMinion)
							unitMinion->Reset(posTarget,0.0f);

						e.action=CBellyMinionCombatState::ActionType_StompEgg;
						e.idGuideEgg=idEgg;
						_RefreshGuidePos();
						return CBellyMinionCombatState::ActionType_StompEgg;
					}
				}
			}
		}

		//Continue hop
		posTarget=_CalcMinionHopTargetPos(idMinion);
		idTarget=loEnemy->GetID();
		return CBellyMinionCombatState::ActionType_Hop;
	}
	return CBellyMinionCombatState::ActionType_None;
}

CUnit *CLoBelly::_FindMinionUnit(LevelObjID idMinion)
{
	std::unordered_map<LevelObjID,CUnit*>::iterator it=_unitsMinion.find(idMinion);
	if (it==_unitsMinion.end())
		return NULL;
	return (*it).second;
}


LevelPos CLoBelly::_CalcMinionTargetPos(LevelObjID idMinion,CLevelObj *loEnemy,BOOL bEnemySacredOrb,LevelPos &posTarget)
{
	CUnit *unitMinion=_FindMinionUnit(idMinion);
	if (unitMinion)//更新Minion的Unit的位置
	{
		CLevelObj *lo=LevelUtil_GetAliveLo(_level,idMinion);
		if (lo)
			unitMinion->Reset(lo->GetFramePos(),lo->GetFrameFace());
	}

	CLevelObj *loKing=NULL;
	if (_unitKing)//更新King的Unit的位置
	{
		CLevelObj *lo=LevelUtil_GetAliveLo(_level,_idBellyKing);
		if (lo)
			_unitKing->Reset(lo->GetFramePos(),lo->GetFrameFace());
		loKing=lo;
	}

	//更新enemy的位置
	if (TRUE)
	{
		LevelPos pos=loEnemy->GetFramePos();
		LevelFace face=loEnemy->GetFrameFace();

		UnitCollide collide=UnitCollide_Empty;
		UnitCollide_SetAlly(collide,loEnemy->GetPlayerID());
		UnitCollide_SetPlayer(collide,TRUE);
		UnitCollide_SetLayor(collide,3);

		if (!_unitEnemy)
		{
			_unitEnemy=_unitmgr.CreateUnit(pos,face,0.5f,0.0f,NULL);
			_unitEnemy->SetCollide(collide);
		}
		else
			_unitEnemy->Reset(pos,face);
	}

	//更新enemy的半径
	if (TRUE)
	{
		if (bEnemySacredOrb)
			_unitEnemy->SetRadius(SACREDORB_DISPELL_DISTANCE);
		else
			_unitEnemy->SetRadius(0.5f);
	}

	unitMinion->RequestTargetPos(posTarget);

	if (FALSE)
	{
		const float spdMax=8.0f;
		const float spdMin=1.0f;
		const float distMax=8.0f;
		const float distMin=3.0f;
		float dist=unitMinion->GetPos().getDistanceFrom(posTarget);
		dist=i_math::clamp_f(dist,distMin,distMax);
		float ratio=(dist-distMin)/(distMax-distMin);
		float spd=i_math::lerp(spdMin,spdMax,ratio);
		unitMinion->SetSpeed(spd);
	}
	else
		unitMinion->SetSpeed(6.0f);

	const float step=0.05f;
	const float dur=0.5f;

	if (TRUE)
	{
		int nIterate=FloatToNearestInt(dur/step);
		for (int i=0;i<nIterate;i++)
			_unitmgr.Update(step);
		unitMinion->Reset();

	}

	return unitMinion->GetPos();
}

LevelPos CLoBelly::_CalcMinionHopTargetPos(LevelObjID idMinion)
{
	LevelPos posTarget;
	if (_stateCombat.GetGuidePos(idMinion,posTarget))
	{
		BOOL bEnemySacredOrb=_stateCombat.CheckEnemySacredOrb();
		CLevelObj *loEnemy=_stateCombat.GetEnemyLo();
		return _CalcMinionTargetPos(idMinion,loEnemy,bEnemySacredOrb,posTarget);
	}

	CLevelObj *lo=LevelUtil_GetAliveLo(_level,idMinion);
	if (lo)
		return lo->GetFramePos();

	return LevelPos(0.0f,0.0f);
}


BOOL CLoBelly::RequestKingSpawnEgg(float rangeMin,float rangeMax,LevelPos &posEgg)
{
	CLevelObj *loKing=LevelUtil_GetAliveLo(_level,_idBellyKing);
	if (!loKing)
		return FALSE;

	if (_stateCombat._stage!=CBellyMinionCombatState::Stage_Attack)
		return FALSE;

	CLevelObj *loEnemy=LevelUtil_GetAliveLo(_level,_stateCombat._idEnemy);
	if (!loEnemy)
		return FALSE;

	LevelPos posKing=loKing->GetFramePos();
	LevelPos posEnemy=loEnemy->GetFramePos();

	const float gapMin=2.0f;

	const float rangeEnemy=3.0f;

	const int nTry=10;
	for(int i=0;i<nTry;i++)
	{
		float rad=CSysRandom::RandRange(0.0f,i_math::Pi*2.0f);
		float radius=CSysRandom::RandRange(0.0f,rangeEnemy);

		LevelPos pos;
		pos.x=posEnemy.x+cosf(rad)*radius;
		pos.y=posEnemy.y+sinf(rad)*radius;

		float dist=pos.getDistanceFrom(posKing);
		if ((dist>=rangeMin)&&(dist<=rangeMax))
		{
			BOOL bOk=TRUE;
			std::unordered_set<LevelObjID>::iterator it;
			for (it=_eggs.begin();it!=_eggs.end();it++)
			{
				CLevelObj *loEgg=LevelUtil_GetAliveLo(_level,*it);
				if (loEgg)
				{
					float distToEgg=loEgg->GetFramePos().getDistanceFrom(pos);
					if (distToEgg<gapMin)
					{
						bOk=FALSE;
						break;
					}
				}
			}
			if (bOk)
			{
				posEgg=pos;
				return TRUE;
			}
		}
	}

	return FALSE;
}

BOOL CLoBelly::RequestKingEvadeJump(float rangeMin,float rangeMax,LevelPos &posEvadeJump)
{
	CLevelObj *loKing=LevelUtil_GetAliveLo(_level,_idBellyKing);
	if (!loKing)
		return FALSE;

	if (_stateCombat._stage!=CBellyMinionCombatState::Stage_Attack)
		return FALSE;

	CLevelObj *loEnemy=LevelUtil_GetAliveLo(_level,_stateCombat._idEnemy);
	if (!loEnemy)
		return FALSE;

	LevelPos posKing=loKing->GetFramePos();
	LevelPos posEnemy=loEnemy->GetFramePos();

	const float gapMin=2.0f;

	const float rangeEnemy=3.0f;

	CUnitMgrNavMesh *unitmgr=_level->GetUnitMgr();

	LevelPos posBest;
	float distBest=-1.0f;
	const int nTry=10;
	for(int i=0;i<nTry;i++)
	{
		LevelFace faceEvade=LevelFaceFromDir(posKing-posEnemy);
		faceEvade+=CSysRandom::RandRange(i_math::deg2rad(-60.0f),i_math::deg2rad(60.0f));

		LevelPos dirEvade=LevelFaceToDir(faceEvade);

		float distEvade=rangeMax;
		LevelPos posEvade;
		if (TRUE)
		{
			LevelPos posHit;
			if (unitmgr->StaticRayCast(UnitFindPath_Walkable,posKing,posKing+dirEvade*rangeMax,posHit))
			{
				distEvade=posHit.getDistanceFrom(posKing);
				posEvade=posHit;
			}
			else
			{
				posEvade=posKing+dirEvade*rangeMax;
				distEvade=rangeMax;
			}
		}

		if (distEvade>=rangeMin)
		{
			posEvadeJump=posEvade;
			return TRUE;
		}

		if (distEvade>distBest)
		{
			distBest=distEvade;
			posBest=posEvade;
		}
	}

	posEvadeJump=posBest;

	return TRUE;
}


void CLoBelly::StompEgg(CLevelObj *loStomper,LevelOpLink &link)
{
	std::unordered_map<LevelObjID,CBellyMinionCombatState::Entry>::iterator it=_stateCombat._entries.find(loStomper->GetID());
	if (it!=_stateCombat._entries.end())
	{
		CBellyMinionCombatState::Entry&e=(*it).second;
		if (e.action==CBellyMinionCombatState::ActionType_StompEgg)
		{
			EoBellyEgg *eoEgg=(EoBellyEgg *)LevelUtil_GetAliveLo(_level,e.idGuideEgg);
			if (eoEgg)
			{
				if (CLevelSkill *skill=LevelUtil_GetCastingSkill(loStomper))
				{
					i_math::xformf xfm;
					if (LevelUtil_CalcSkillCastingXfm(skill,xfm))
					{
						if (eoEgg->GetFramePos().equals(xfm.pos.getXZ(),0.2f))
						{
							eoEgg->Trigger(link);
						}
					}
				}
			}
		}
	}
	
}

LevelPos CLoBelly::_FindStepPosAroundCircle(LevelPos &posSrc0,LevelPos &posTarget,float distStep0,i_math::circlef &circle)
{
	LevelPos posSrc=posSrc0;
	if (circle.isPointIn(posSrc))
		circle.snapToBoundary(posSrc);

	if (TRUE)
	{
		float dist=posSrc.getDistanceFrom(posTarget);
		if (distStep0>dist)
			distStep0=dist;
	}

	LevelPos posCur=posSrc;
	float distStep=distStep0;
	for (int i=0;i<10;i++)
	{
		LevelPos dir=posTarget-posCur;
		dir.normalize();

		LevelPos posNext=posCur+dir*distStep;
		if (circle.isPointIn(posNext))
		{
			circle.snapToBoundary(posNext);
		}

		if (posNext.getDistanceFrom(posCur)<0.1f*distStep)
		{
			dir.rotateBy(30.0f,i_math::vector2df(0.0f,0.0f));

			LevelPos posNext=posCur+dir*distStep;
			if (circle.isPointIn(posNext))
				circle.snapToBoundary(posNext);
		}

		posCur=posNext;

		distStep=distStep0-posCur.getDistanceFrom(posSrc);
		if (distStep<distStep0*0.2f)
			return posCur;
	}
	return posCur;
}

BOOL CLoBelly::RequestKingApproachPos(LevelPos &posApproach)
{
	LevelPos posTarget;
	LevelPos posPlayer;

	if (TRUE)
	{
		CLoUnit *loPlayer=LevelUtil_GetFirstPlayerLoUnit(_level);
		if (!loPlayer)
			return FALSE;
		posPlayer=loPlayer->GetFramePos();
		posTarget=posPlayer;
	}

	CLoStarPlate *loStarPlate=(CLoStarPlate *)_level->GetUniqueObj(LevelUniqueObj_StarPlate);
	if (loStarPlate)
	{
		BOOL bStarPlateActivated=loStarPlate->CheckAnySiteActivated();
		if (TRUE)
		{
			LevelPos pos;
			if (bStarPlateActivated)
			{
				if (loStarPlate->GetNextSitePosToActivate(pos))
					posTarget=pos;
			}
			else
			{
				LevelObjID idClosestSite=loStarPlate->FindClosestSite(posPlayer);
				if (idClosestSite!=LevelObjID_Invalid)
				{
					if (loStarPlate->GetNextSitePos(idClosestSite,pos))
						posTarget=pos;
				}
			}
		}

		if (bStarPlateActivated)
		{
			i_math::circlef circle;
			if (loStarPlate->GetCenterCircle(circle))
			{
				CLevelObj *loKing=LevelUtil_GetAliveLo(_level,_idBellyKing);
				if (loKing)
				{
					LevelPos posSrc=loKing->GetFramePos();
					posApproach=_FindStepPosAroundCircle(posSrc,posTarget,5.0f,circle);
					return TRUE;
				}
			}
		}
	}

	posApproach=posTarget;
	return TRUE;
}

BOOL CLoBelly::ValidateEelTargetPos(LevelPos &posCur,LevelPos &posTarget)
{
	LopBelly *lop=(LopBelly*)_param;

	BOOL bInZone=FALSE;
	for (int i=0;i<lop->zonesBellyEel.size();i++)
	{
		if (lop->zonesBellyEel[i].isPointIn(posTarget))
		{
			bInZone=TRUE;
			break;
		}
	}

	if (!bInZone)
		return FALSE;

	i_math::line2df line;
	line.start=posCur;
	line.end=posTarget;
	const float distEscape=3.0f;
	const float distOvershoot=4.0f;

	i_math::vector2df dir=line.getVector();
	dir.normalize();

	line.start+=dir*distEscape;
	line.end+=dir*distOvershoot;

	for (int i=0;i<lop->zonesBellyEelObstacle.size();i++)
	{
		if (lop->zonesBellyEelObstacle[i].isPointIn(posTarget))
			continue;
		if (lop->zonesBellyEelObstacle[i].isPointIn(posCur))
			continue;
		if (lop->zonesBellyEelObstacle[i].testIntersectionWithLine2D(line))
			return FALSE;
	}

	return TRUE;
}

BOOL CLoBelly::FindValidEelTargetPos(LevelPos &posCur,float distMin,LevelPos &posTarget)
{
	const int nIterate=10;

	LopBelly *lop=(LopBelly*)_param;

	if (lop->zonesBellyEel.size()<=0)
		return FALSE;

	for (int i=0;i<nIterate;i++)
	{
		int iZone=CSysRandom::RandRangeInt<int>(0,lop->zonesBellyEel.size());

		posTarget=CSysRandom::GenRandomPos2DIn(lop->zonesBellyEel[iZone]);

		if (posTarget.getDistanceSQFrom(posCur)<distMin*distMin)
			continue;

		if (!ValidateEelTargetPos(posCur,posTarget))
			continue;

		return TRUE;
	}

	for (int i=0;i<nIterate;i++)
	{
		int iZone=CSysRandom::RandRangeInt<int>(0,lop->zonesBellyEel.size());

		posTarget=CSysRandom::GenRandomPos2DIn(lop->zonesBellyEel[iZone]);

		if (posTarget.getDistanceSQFrom(posCur)<distMin*distMin)
			continue;

		return TRUE;
	}

	if(TRUE)
	{
		int iZone=CSysRandom::RandRangeInt<int>(0,lop->zonesBellyEel.size());

		posTarget=CSysRandom::GenRandomPos2DIn(lop->zonesBellyEel[iZone]);

		return TRUE;
	}


	return FALSE;
}
