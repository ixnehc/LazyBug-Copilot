
#include "stdh.h"

#include "Random/Random.h"

#include "Skill_PhantomAttack.h"

#include "LevelRecordSkill.h"
#include "LevelRecordUnit.h"

#include "LoUnit.h"
#include "Level.h"

#include "LevelOSB.h"

#include "LevelDecider.h"


//////////////////////////////////////////////////////////////////////////
//CSkill_PhantomAttack

BIND_SKILLPARAM(Skill_PhantomAttack,SkillParam_PhantomAttack);

void Skill_PhantomAttack::_CollectCandidates()
{

	LevelPos posTarget;
	extern BOOL LevelUtil_CalcTargetPos(CLevel *level,LevelSkillTarget &target,LevelPos&pos);
	if (FALSE==LevelUtil_CalcTargetPos(_owner->GetLevel(),_target,posTarget))
		return;

	LevelObjID idTarget=LevelObjID_Invalid;
	if (TRUE)
	{
		extern CLevelObj *LevelUtil_GetTargetObj(CLevel *level,LevelSkillTarget &target);
		CLevelObj *loTarget=LevelUtil_GetTargetObj(_owner->GetLevel(),_target);
		if (loTarget)
			idTarget=loTarget->GetID();
	}

	LevelPos org=_owner->GetFramePos();

	float range=posTarget.getDistanceFrom(org)/2.0f+2.0f;

	LevelPos center=(org+posTarget)/2.0f;

	i_math::rectf rc;
	rc.set(center.x,center.y,center.x,center.y);
	rc.inflate(range,range,range,range);

	float radiusOwner=_owner->GetRadius_();

	CUnitMap *mp=GetLevel()->GetUnitMgr()->GetMap();
	mp->Enum(rc);
	DWORD c;
	CUnitBase **units=(CUnitBase **)mp->GetEnums(c);

	for (int i=0;i<c;i++)
	{
		CUnit *unit=(CUnit *)units[i];
		CLevelObj *lo=(CLevelObj *)unit->GetData();

		extern BOOL LevelUtil_CheckSkillTarget(LevelRecordSkill *recSkill,CLevelObj *loSrc,CLevelObj *loTarget);
		if (!LevelUtil_CheckSkillTarget(_rec,_owner,lo))
			continue;

// 		if (lo->GetID()==idTarget)
// 			continue;

		_candidates.push_back(lo->GetID());
	}
}


void Skill_PhantomAttack::_OnStart()
{
	_AddStartOp();

	GetLevel()->AddAffect(_owner);

	extern BOOL LevelUtil_CalcTargetPos(CLevel *level,LevelSkillTarget &target,LevelPos&pos);
	if (!LevelUtil_CalcTargetPos(_owner->GetLevel(),_target,_posTarget))
	{
		_SetState(SkillState_Fail);
		return;
	}

	_SetState(SkillState_Casting);

	_CollectCandidates();
	CSysRandom::GenRandomIndices(_indicesCandidates,_candidates.size());
	_posOrg=_owner->GetFramePos();
	_posLast=_posOrg;


	_tCasting=0;
	_Update(0);
}

BOOL Skill_PhantomAttack::_CreatePhantom()
{
	CLevel *level=_owner->GetLevel();
	CUnitMgrNavMesh*unitmgr=level->GetUnitMgr();

	CLevelObjMove *move=_owner->GetMove();
	assert(move);

	CLevelPlayer *player=NULL;
	if (_owner->IsPlayer())
	{
		extern CLevelPlayer *LevelUtil_PlayerFromLo(CLevelObj *lo);
		player=LevelUtil_PlayerFromLo(_owner);
	}

	LevelPos dirAim=_posTarget-_posOrg;
	float range=dirAim.getLength();
	range+=1.0f;
	dirAim.normalize();

	float radiusOwner=_owner->GetRadius_();

	float d=cosf(150.0f/2.0f*(float)i_math::GRAD_PI2);//张角一半的余弦

	CLevelObj *loFound=NULL;
	float dist2Min=1000000000000.0f;
	LevelPos posTeleport;
	LevelFace faceTeleport;

	for (int i=0;i<_indicesCandidates.size();i++)
	{
		LevelObjID id=_candidates[_indicesCandidates[(i+_iCandidateStart)%_indicesCandidates.size()]];

		extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
		CLevelObj *lo=LevelUtil_GetAliveLo(level,id);

		if (!lo)
			continue;

		extern BOOL LevelUtil_CheckSkillTarget(LevelRecordSkill *recSkill,CLevelObj *loSrc,CLevelObj *loTarget);
		if (!LevelUtil_CheckSkillTarget(_rec,_owner,lo))
			continue;

		LevelPos dir;
		LevelPos pos=lo->GetFramePos();
		dir=pos-_posOrg;
		//过滤掉扇形区域外的
		if (TRUE)
		{
			float dist=dir.getLength();
			if (dist<0.01f)
				continue;
			if (dist>range+lo->GetRadius_()+radiusOwner)
				continue;
			dir/=dist;//normalize
			if (dir.dotProduct(dirAim)<d)
				continue;
		}

		dir=pos-_posLast;
		if (dir.dotProduct(dirAim)<-0.5f)
			continue;//不能后退

		float dist2=_posLast.getDistanceSQFrom(pos);
		if (_posLast.getDistanceSQFrom(pos)<dist2Min)
		{

			if (TRUE)
			{
				extern BOOL LevelUtil_FindPosAround(CLevelObj *lo,float radius0,DWORD nTry,LevelPos &pos,LevelFace &face);
				if (!LevelUtil_FindPosAround(lo,radiusOwner,3,posTeleport,faceTeleport))
					continue;
			}

			loFound=lo;
			dist2Min=dist2;
		}
	}

	_iCandidateStart++;

	//计算要teleport的点和朝向
	if (loFound)
	{

		Phantom phtm;
		phtm.idTarget=loFound->GetID();
		phtm.tStart=_tCasting;
		phtm.posSrc=posTeleport;
		_phantoms.push_back(phtm);

		//进行Teleport
		_DoTeleport(posTeleport,faceTeleport);


		_posLast=loFound->GetFramePos();

		return TRUE;
	}


	return FALSE;
}

void Skill_PhantomAttack::_Update(AnimTick dt)
{
	if (_state==SkillState_Casting)
	{
		if (_bBroken)
		{
			_SetState(SkillState_Finished);
			return;
		}

		//累加Cast的时间
		extern void LevelUtil_AccumCastingTime(CLevelObj *lo,AnimTick dt,AnimTick &tCasting);
		LevelUtil_AccumCastingTime(_owner,dt,_tCasting);

		SkillParam_PhantomAttack *param=_rec->GetParam<SkillParam_PhantomAttack>();

		int nPhantoms=0;
		if (_tCasting>=param->delayPhantom)
			nPhantoms=1+(_tCasting-param->delayPhantom)/param->durStep;

		if (nPhantoms<=param->nPhantoms)
		{
			if (nPhantoms>_phantoms.size())
			{
				if (FALSE==_CreatePhantom())
				{
					Phantom phtmEmpty;
					while(param->nPhantoms>_phantoms.size())
						_phantoms.push_back(phtmEmpty);
					nPhantoms=param->nPhantoms+1;
				}
			}
		}

		if (nPhantoms==param->nPhantoms+1)
		{
			if (nPhantoms>_phantoms.size())
			{
				extern CLevelObj *LevelUtil_GetTargetObj(CLevel *level,LevelSkillTarget &target);
				CLevelObj *loTarget=LevelUtil_GetTargetObj(GetLevel(),_target);

				//最终的那个Phantom
				Phantom phtm;
				phtm.tStart=_tCasting;

				if (loTarget)
					phtm.idTarget=loTarget->GetID();

				extern BOOL LevelUtil_FindPosAround(CLevelObj *lo,float radius0,DWORD nTry,LevelPos &pos,LevelFace &face);
				LevelPos posTeleport=_posTarget;
				LevelFace faceTeleport=LevelFaceFromDir(_posTarget-_posOrg);

				if (loTarget)
					LevelUtil_FindPosAround(loTarget,_owner->GetRadius_(),6,posTeleport,faceTeleport);

				phtm.posSrc=posTeleport;
				_phantoms.push_back(phtm);


				//进行Teleport
				_DoTeleport(posTeleport,faceTeleport,1);//flag设为1,表示为终结的Teleport
			}
		}

		for (int i=0;i<_phantoms.size();i++)
		{
			Phantom &phtm=_phantoms[i];
			if (phtm.idTarget==LevelObjID_Invalid)
				continue;
			if (phtm.bHit)
				continue;
			if (phtm.tStart+param->delayHit<_tCasting)
				continue;

			DealArg arg;
			extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
			CLevelObj *loTarget=LevelUtil_GetAliveLo(GetLevel(),phtm.idTarget);
			if (loTarget)
			{
				LevelPos posTarget=loTarget->GetFramePos();
				LevelPos dir=posTarget-phtm.posSrc;
				dir.normalize();
				arg.dir.setXZ(dir);
				arg.link.id=GetLevel()->GenOpLinkID();
				_MakeDeals(loTarget,arg);
			}

			phtm.bHit=TRUE;
		}

		if (_phantoms.size()>=param->nPhantoms+1)
		{
			AnimTick tEnd=_phantoms[_phantoms.size()-1].tStart;
			if (_phantoms[_phantoms.size()-1].idTarget)
				tEnd+=param->durPhantom;

			if (_tCasting>tEnd)
			{
				_SetState(SkillState_Finished);
			}
		}

	}
}


void Skill_PhantomAttack::_OnUpdate(AnimTick dt)
{
	_Update(dt);

}
