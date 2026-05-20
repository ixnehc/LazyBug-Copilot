/********************************************************************
	created:	2011/12/03
	file base:	Skill
	author:		cxi
	
	purpose:	技能
*********************************************************************/
#include "stdh.h"

#include "LevelSkill.h"

#include "LoUnit.h"

#include "Level.h"
#include "LevelOSB.h"


LevelObjID GetSkillTargetObjID(LevelSkillTarget &target)
{
	switch(target.tp)
	{
		case LevelSkillTarget::Target_DefObj:
		case LevelSkillTarget::Target_FixPosAndObj:
		{
			return target.ObjID();
		}
	}

	return LevelObjID_Invalid;

}




////////////////////////////////////////////////////////////////////////
//CLevelSkillCasting
void CLevelSkillCasting::UpdateToCasted(AnimTick dt)
{
	if (!_skill)
		return;
	_bNeedFire=0;
	_bNeedCasted=0;
	_bNeedFinished=0;

	if (_bFired)
		_tFire+=dt;
	if (_skill->_state==SkillState_Casting)
	{
		if (!_skill->_bBroken)
		{//未打断
			//累加Cast的时间
			extern void LevelUtil_AccumCastingTime(CLevelObj *lo,AnimTick dt,AnimTick &tCasting);
			LevelUtil_AccumCastingTime(_skill->_owner,dt,_tCasting);

			if(!_bFired)
			{
				if (_tCasting+LEVEL_FRAME_TICK>_skill->_rec->HitDelay)
				{
					_bFired=1;
					_bNeedFire=1;
				}
			}
			if ((_tCasting>_skill->_rec->CastTime)&&_bFired)
				_bNeedCasted=1;
		}
		else
		{//打断了
			if (!_bFired)
			{//尚未发射
				//结束技能
				_bNeedFinished=1;
			}
			else
			{//已经发射
				_bNeedCasted=1;
			}
		}
	}

}

void CLevelSkillCasting::UpdateToFinished(AnimTick dt)
{
	if (!_skill)
		return;
	_bNeedFire=0;
	_bNeedCasted=0;
	_bNeedFinished=0;
	if (_bFired)
		_tFire+=dt;
	if (_skill->_state==SkillState_Casting)
	{
		if (!_skill->_bBroken)
		{//未打断
			//累加Cast的时间
			extern void LevelUtil_AccumCastingTime(CLevelObj *lo,AnimTick dt,AnimTick &tCasting);
			LevelUtil_AccumCastingTime(_skill->_owner,dt,_tCasting);

			if(!_bFired)
			{
				if (_tCasting+LEVEL_FRAME_TICK>_skill->_rec->HitDelay)
				{
					_bNeedFire=1;
					_bFired=1;
				}
			}
			if ((_tCasting>_skill->_rec->CastTime)&&_bFired)
			{
				_bNeedFinished=1;
			}
		}
		else
			_bNeedFinished=1;
	}

}




//////////////////////////////////////////////////////////////////////////
//CSkill


CLevelOp *CLevelSkill::NewOp(CClass *clssOp,LevelOpLink &link)
{
	extern CLevelOp*NewLevelOp(ClassUID uid);
	CLevelOp *op=NewLevelOp(clssOp->GetUID());
	if (!op)
	{
		assert(FALSE);
		return NULL;
	}

	LevelOpDesc &desc=op->GetDesc();
	FillOpDesc(desc,clssOp,link);

	return op;
}

void CLevelSkill::Init(CLevelSkillDriver *driver,LevelSkillID id,ClientSkillID idClient,LevelSkillGrade grd)
{
	_bPlayer=driver->GetOwner()->IsPlayer();
	_owner=(CLevelObj*)driver->GetOwner();
	_owner->AddRef();
	_target=*driver->GetTarget();
	LevelSkillArg *arg=driver->GetArg();
	if (arg)
		_arg=arg->Clone();
	LevelRecordSkill *rec=driver->GetRec();
	SAFE_REPLACE(_rec,rec);
	_SetParam(_rec->param);
	_id=id;
	_idClient=idClient;
	_grd=grd;
	_tBirth=_owner->GetT();
	_tUpdate=_tBirth;
}

void CLevelSkill::Clear()
{
	SAFE_RELEASE(_owner);
	Safe_Class_Delete(_arg);
	SAFE_RELEASE(_rec);
	Zero();
}

RecordID CLevelSkill::GetRecID()
{		
	return _rec?_rec->GetID():RecordID_Invalid;	
}



void CLevelSkill::Start()
{
	AddRef();//子类有可能在_OnStart()里做一些操作,将这个Skill释放掉,加一个引用计数,提高安全性
	_OnStart();
	Release();
}

void CLevelSkill::Start(LevelOpLink &link)
{
	AddRef();//子类有可能在_OnStart()里做一些操作,将这个Skill释放掉,加一个引用计数,提高安全性
	_OnStart(link);
	Release();
}



void CLevelSkill::Update()
{
	AddRef();//子类有可能在_OnUpdate()里做一些操作,将这个Skill释放掉,加一个引用计数,提高安全性
	LevelTick t=_owner->GetT();
	LevelTick dt=t-_tUpdate;
	_tUpdate=t;
	if (dt>0)
		_OnUpdate(dt);
	Release();
}

void CLevelSkill::_AddStartOp()
{
	_AddStartOp(LevelOpLink());
}


void CLevelSkill::_AddStartOp(LevelOpLink &link)
{
	LevelOp_StartSkill *op=(LevelOp_StartSkill *)NewOp(Class_Ptr2(LevelOp_StartSkill),link);
	op->target=_target;
	op->idRec=_rec->GetSimpleID();
	op->grd=_grd;
	op->arg.CopyFrom(_arg);
	op->idClient=_idClient;
	_MakeTransferTarget(op->target);

	_owner->AddOp(op);
}

void CLevelSkill::_AddCombineOp(LevelSkillTarget &target)
{
	LevelOp_CombineSkill *op=(LevelOp_CombineSkill*)NewOp(Class_Ptr2(LevelOp_CombineSkill),LevelOpLink());
	op->target=target;

	_owner->AddOp(op);
}

void CLevelSkill::_AddSyncDataOp()
{
	LevelOp_SyncSkillData*op=NewOp<LevelOp_SyncSkillData>(LevelOpLink());

	CBitPacket bp;
	bp.SetBufferPointer(op->data,op->bits);
	if (!_WriteSyncData(&bp))
	{
		Safe_Class_Delete(op);
		return;
	}

	DWORD szData,szBitsInByte;
	bp.GetBufferSize(szData,szBitsInByte);
	assert(szData<=MAX_SKILL_DATA);
	assert(szBitsInByte<=MAX_SKILL_DATA/4);
	op->szData=(BYTE)szData;
	op->szBitsData=(BYTE)szBitsInByte;

	_owner->AddOp(op);

}


LevelPos CLevelSkill::_CalcAimDir(LevelSkillTarget &target)
{
	LevelPos src=_owner->GetFramePos();
	if (target.bOrg)
		src=target.org;

	LevelPos dir;
	extern BOOL LevelUtil_CalcTargetPos(CLevel *level,LevelSkillTarget &target,LevelPos&pos);
	LevelUtil_CalcTargetPos(_owner->GetLevel(),target,dir);

	dir-=src;
	dir.normalize();
	return dir;

}

void CLevelSkill::_FillRndSeed()
{
	if (!_arg)
		_arg=Class_New2(LevelSkillArg);

	if (_arg->seedRnd==LevelRandomSeed_Invalid)
		_arg->seedRnd=(LevelRandomSeed)(_tBirth/10+1);
}



void CLevelSkill::_MakeDeals(LevelPos3D &pos,DealArg&arg)
{
	LevelOSB osbSrc(this);
	if (_rec)
	{
		_rec->deal->Make(osbSrc,pos,arg,NULL);
		MakeDeals(_rec->deals,osbSrc,pos,arg,NULL);
	}
}

void CLevelSkill::_MakeDeals(CLevelObj *loTarget,DealArg&arg)
{
	LevelOSB osbSrc(this);
	if (_rec)
	{
		CLevel *level=GetLevel();
		if (level)
		{
			if (level->GetDecider()->MakeHit(osbSrc,loTarget,_rec->hit.Get(),arg.link))
			{
				_rec->deal->Make(osbSrc,loTarget,arg,NULL);
				MakeDeals(_rec->deals,osbSrc,loTarget,arg,NULL);
			}
		}
	}
}

void CLevelSkill::_MakeFanDeals(float angleFov,float range)
{
	CLevelDecider *decider=GetLevel()->GetDecider();
	LevelPos aimdir=_CalcAimDir(_target);

	float d=cosf(angleFov/2.0f*(float)i_math::GRAD_PI2);//张角一半的余弦

	LevelPos org=_owner->GetFramePos();

	i_math::rectf rc;
	rc.set(org.x,org.y,org.x,org.y);
	rc.inflate(range,range,range,range);

	float radiusOwner=_owner->GetRadius_();

	CUnitMap *mp=GetLevel()->GetUnitMgr()->GetMap();
	mp->Enum(rc);
	DWORD c;
	CUnitBase **units=(CUnitBase **)mp->GetEnums(c);

	for (int i=0;i<c;i++)
	{
		CUnit *unit=(CUnit *)units[i];
		CLevelObj *loTarget=(CLevelObj *)unit->GetData();

		extern BOOL LevelUtil_CheckSkillTarget(LevelRecordSkill *recSkill,CLevelObj *loSrc,CLevelObj *loTarget);
		if (!LevelUtil_CheckSkillTarget(_rec,_owner,loTarget))
			continue;

		LevelPos dir;
		LevelPos pos=unit->GetPos();
		dir=pos-org;
		//过滤掉扇形区域外的
		if (TRUE)
		{
			float dist=dir.getLength();
			if (dist<0.01f)
				continue;
			if (dist>range+loTarget->GetRadius_()+radiusOwner)
				continue;
			dir/=dist;//normalize
			if (dir.dotProduct(aimdir)<d)
				continue;
		}

		DealArg arg;
		arg.dir.setXZ(dir);
		arg.link.id=GetLevel()->GenOpLinkID();
		arg.grd=_grd;

		_MakeDeals(loTarget,arg);
	}
}


void CLevelSkill::_DoTeleport(LevelPos &posTeleport,LevelFace faceTeleport,DWORD flag)
{
	LevelTeleportID idTeleport=_owner->GenTeleportID();
	if (TRUE)
	{
		CLevelObjMove *move=_owner->GetMove();
		if (move)
			move->Teleport(idTeleport,posTeleport,faceTeleport);

		if (_owner->IsPlayer())
		{
			extern CLevelPlayer *LevelUtil_PlayerFromLo(CLevelObj *lo);
			CLevelPlayer *player=LevelUtil_PlayerFromLo(_owner);
			if (player)
			{
				player->GetMove().PauseMove();
				player->GetMove().AuthorizeTeleport(idTeleport,posTeleport);
			}
		}
	}

	LevelOp_SkillTeleport *op=NewOp<LevelOp_SkillTeleport>(LevelOpLink());
	op->id=idTeleport;
	op->target=posTeleport;
	op->face=faceTeleport;
	op->dur=0;
	op->flag=flag;
	_owner->AddOp(op);
}
