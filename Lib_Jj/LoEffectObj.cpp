
#include "stdh.h"
#include "Level.h"

#include "LevelUtil.h"

#include "LoUnit.h"

#include "LoEffectObj.h"

#include "LevelRecordEO.h"
#include "LevelRecords.h"

#include "LevelOSB.h"

#include "LevelObjHistory.h"

#include "Buff_Dead.h"
#include "Ability_ShieldAmulet.h"


#include "Random/Random.h"
#include "timer/profiler.h"


//////////////////////////////////////////////////////////////////////////
//CLoEffectObj

void CLoEffectObj::PostCreate(LevelPlayerID idPlayer,CRecord *rec,i_math::xformf &xfmInitial,AnimEventZone *eZone,LevelGrade grd,LevelOSB &osb,LevelOpLink &link)
{
	SetPlayerID(idPlayer);

	SAFE_REPLACE(_rec,rec);

	_xfmInitial=xfmInitial;
	_eZone=eZone;

	_grd=grd;

	_tCreate=_level->GetT_();

	if (!osb.IsEmpty())
	{
		_opBirth=osb.NewOp<LevelOp_EoBirth>(link);
		CLevelSkill *skill=osb.GetSkill();
		if (skill)
			_opBirth->tOwnerSkillCastTime=skill->GetCastingTime();
	}

	if (!osb.IsEmpty())
	{
		_owner=osb;
		_owner.AddRef();

		_idSrcOwner=osb.GetRootOwnerID();
	} 

	//创建Behavior
	if (_rec)
	{
		if (_rec->idBG!=StringID_Invalid)
		{
			LevelBehaviorContext ctx;
			ctx.lo=this;
			_bhv=_level->CreateBehavior(_rec->idBG,ctx);
		}
	}


	_OnPostCreate();

}

void CLoEffectObj::PostCreate(LevelPlayerID idPlayer,RecordID idRec,i_math::xformf &xfmInitial,AnimEventZone *eZone,LevelGrade grd,LevelOSB &osb,LevelOpLink &link)
{
	CLevelRecords *records=_level->GetRecords();
	CRecord *rec=records->GetEo(idRec);

	PostCreate(idPlayer,rec,xfmInitial,eZone,grd,osb,link);
}

void CLoEffectObj::PostCreate(LevelPlayerID idPlayer,RecordID idRec,LevelPos3D&pos,LevelPos3D &dir,LevelGrade grd,LevelOSB &osb,LevelOpLink &link)
{
	CLevelRecords *records=_level->GetRecords();
	CRecord *rec=records->GetEo(idRec);

	PostCreate(idPlayer,rec,pos,dir,grd,osb,link);
}

void CLoEffectObj::PostCreate(LevelPlayerID idPlayer,RecordID idRec,LevelPos&pos,LevelPos &dir,LevelGrade grd,LevelOSB &osb,LevelOpLink &link)
{
	LevelPos3D pos3D;
	LevelPos3D dir3D;
	pos3D.setXZ(pos);
	dir3D.setXZ(dir);

	PostCreate(idPlayer,idRec,pos3D,dir3D,grd,osb,link);
}

void CLoEffectObj::PostCreate(LevelPlayerID idPlayer,CRecord *rec,LevelPos3D&pos,LevelPos3D &dir,LevelGrade grd,LevelOSB &osb,LevelOpLink &link)
{
	i_math::xformf xfmInitial;
	xfmInitial.fromZAxis(pos,dir);

	PostCreate(idPlayer,rec,xfmInitial,NULL,grd,osb,link);
}


void CLoEffectObj::PostCreate(LevelPlayerID idPlayer,CRecord *rec,LevelPos&pos,LevelPos &dir,LevelGrade grd,LevelOSB &osb,LevelOpLink &link)
{
	LevelPos3D pos3D;
	LevelPos3D dir3D;
	pos3D.setXZ(pos);
	dir3D.setXZ(dir);

	PostCreate(idPlayer,rec,pos3D,dir3D,grd,osb,link);
}

LevelPos CLoEffectObj::_GetInitialDir()
{
	i_math::vector3df dir(0,0,1);
	dir=_xfmInitial.rot*dir;
	return dir.getXZ();
}

LevelPos3D CLoEffectObj::_GetInitialDir3D()
{
	i_math::vector3df dir(0,0,1);
	dir=_xfmInitial.rot*dir;
	return dir;
}

void CLoEffectObj::OnDestroy()
{
	_OnDetroy();

	if (_bhv)
	{
		_bhv->Clear();
		Safe_Class_Delete(_bhv);
	}

	Safe_Class_Delete(_opBirth);
	_ops.Clear();
	_owner.Release();

	SAFE_RELEASE(_rec);

	Zero();
}



void CLoEffectObj::Update()
{
	if (_opBirth)
	{
		AnimTick tCur=_level->GetT_();
		if (tCur>=_tCreate+ANIMTICK_FROM_SECOND(0.2f))
		{
			Safe_Class_Delete(_opBirth);
		}
	}

	_OnUpdate();

	if (_bhv)
		_bhv->Update();
}

void CLoEffectObj::WriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	if (_rec)
	{
		bp->Bits_Write(_rec->GetSimpleID(),11);
		if (TRUE)
		{
			BYTE *buf;
			DWORD c;
			buf=_rec->GetDelta(c);
			if (buf)
			{
				bp->Bit_Write_1();
				bp->Data_EncodeDword(c);
				bp->Data_WriteData(buf,c);
			}
			else
				bp->Bit_Write_0();
		}
	}
	else
		bp->Bits_Write(0,11);

	bp->Data_WriteSimpleR(_xfmInitial);

	if (_rec->bindingClientInitialXfm==EoClientInitialXfmBinding_OwnerFoot)
		bp->Data_WriteSimple(_owner.GetOwnerID());
	if (_rec->bindingClientInitialXfm==EoClientInitialXfmBinding_OwnerFootXZ)
		bp->Data_WriteSimple(_owner.GetOwnerID());
	if (_rec->bindingClientInitialXfm==EoClientInitialXfmBinding_HostAimPos)
		bp->Data_WriteSimple(_idHost);

	if (_opBirth)
	{
		bp->Bit_Write_1();
		_opBirth->GetDesc().Save(bp);
		_opBirth->Save(bp);
	}
	else
		bp->Bit_Write_0();

	if (_NeedOps())
		_ops.WriteSync(bp,TRUE,bContent);

	bContent=TRUE;

	_OnWriteFirstSync(bp,bContent,idPlayer);
}


void CLoEffectObj::WriteSyncH(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	_OnWriteSyncH(bp,bContent,idPlayer);
}

void CLoEffectObj::WriteSyncL(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	if (_NeedOps())
		_ops.WriteSync(bp,FALSE,bContent);
	_OnWriteSyncL(bp,bContent,idPlayer);
}

void CLoEffectObj::PostWriteSync()
{
	if (_NeedOps())
		_ops.PostWriteSync();
	_OnPostWriteSync();
}

void CLoEffectObj::_MakeDeals(LevelOSB &osbSrc,LevelPos3D &pos,DealArg&arg)
{
	if (_rec)
	{
		_rec->deal->Make(osbSrc,pos,arg,NULL);
		MakeDeals(_rec->deals,osbSrc,pos,arg,NULL);
	}
}

void CLoEffectObj::_MakeDeals(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg)
{
	if (_rec)
	{
		if (_level->GetDecider()->MakeHit(osbSrc,loTarget,_rec->hit.Get(),arg.link))
		{
			_rec->deal->Make(osbSrc,loTarget,arg,NULL);
			MakeDeals(_rec->deals,osbSrc,loTarget,arg,NULL);
		}
	}
}

CLevelObj **CLoEffectObj::_DetectRange(LevelPos &pos,float radius,DWORD &c)
{
	c=0;
	if (_rec)
	{
		LevelUtilDetectParam paramDetect;
		paramDetect.loSrc=this;
		paramDetect.pos=pos;
		paramDetect.rangeMin=0.0f;
		paramDetect.rangeMax=radius;
		paramDetect.flags=&_rec->detects[0];
		paramDetect.nFlags=_rec->detects.size();
		paramDetect.requires=&_rec->requires[0];
		paramDetect.nRequires=_rec->requires.size();
		paramDetect.bTouching=TRUE;

		return LevelUtil_Detect(paramDetect,NULL,c);
	}

	return NULL;
}

CLevelObj **CLoEffectObj::_DetectRange_ShieldAmulet(LevelPos &pos,float radius,DWORD &c)
{
	c=0;
	if (_rec)
	{
		LevelUtilDetectParam paramDetect;
		paramDetect.loSrc=this;
		paramDetect.pos=pos;
		paramDetect.rangeMin=0.0f;
		paramDetect.rangeMax=radius+SHIELDAMULET_SHIELDRANGE+0.4f;
		LevelDetectTargetFlag detect=(LevelDetectTargetFlag)(LevelDetectTarget_Enemy|LevelDetectTarget_Ally|LevelDetectTarget_Neutral|LevelDetectTarget_Player|LevelDetectTarget_Ground|LevelDetectTarget_Flying|LevelDetectTarget_Float);
		paramDetect.flags=&detect;
		paramDetect.nFlags=1;
		paramDetect.requires=&_rec->requires[0];
		paramDetect.nRequires=_rec->requires.size();

		return LevelUtil_Detect(paramDetect,NULL,c);
	}

	return NULL;
}

CLevelObj **CLoEffectObj::_DetectInAll(DWORD &c)
{
	c=0;
	if (_rec)
	{
		LevelUtilDetectParam paramDetect;
		paramDetect.loSrc=this;
		paramDetect.pos=LevelPos(0.0f,0.0f);
		paramDetect.rangeMin=0.0f;
		paramDetect.rangeMax=0.0f;
		paramDetect.flags=&_rec->detects[0];
		paramDetect.nFlags=_rec->detects.size();
		paramDetect.requires=&_rec->requires[0];
		paramDetect.nRequires=_rec->requires.size();
		paramDetect.bTouching=TRUE;

		return LevelUtil_DetectInAll(paramDetect,NULL,c);
	}

	return NULL;
}


CLevelObj *CLoEffectObj::_DetectFirstInRange(LevelPos &pos,float radius)
{
	if (_rec)
	{
		LevelUtilDetectParam paramDetect;
		paramDetect.loSrc=this;
		paramDetect.pos=pos;
		paramDetect.rangeMin=0.0f;
		paramDetect.rangeMax=radius;
		paramDetect.flags=&_rec->detects[0];
		paramDetect.nFlags=_rec->detects.size();
		paramDetect.requires=&_rec->requires[0];
		paramDetect.nRequires=_rec->requires.size();

		return LevelUtil_DetectFirst(paramDetect,NULL);
	}

	return NULL;
}

BOOL CLoEffectObj::_CheckCanHit(CLevelObj *lo,LevelEoDetectHitArg &argHit)
{
	if (lo->GetID()==argHit.idIgnore)
		return FALSE;

	if (argHit.idSpecify!=LevelObjID_Invalid)
	{
		if (lo->GetID()!=argHit.idSpecify)
			return FALSE;
	}

	BOOL bCanHit=FALSE;
	if (argHit.bUnit)
	{
		if (lo->GetType()==LevelObjType_Unit)
			bCanHit=TRUE;
	}
	if (argHit.bAgent)
	{
		if (lo->GetType()==LevelObjType_Agent)
			bCanHit=TRUE;
	}
	return bCanHit;

}

CLevelObj *CLoEffectObj::_DetectHit(i_math::line3df &line0,LevelEoDetectHitArg &argHit)
{
	LevelPos center=line0.getMiddle().getXZ();
	float range=line0.getLength()/2.0f+argHit.radius;

	DWORD c;
	CLevelObj **buf=_DetectRange(center,range,c);

	i_math::line3df line=line0;

	CLevelObj *loHit=NULL;
	for (int i=0;i<c;i++)
	{
		//目前只支持Unit
		CLevelObj *lo=buf[i];

		if (!_CheckCanHit(lo,argHit))
			continue;

		LevelPos3D pos3D=lo->GetFramePos3D();

		LevelPos3D posHit;
		extern BOOL LevelUtil_UnitHitTest(i_math::line3df &line,i_math::vector3df &center,float radius,float fall,float height,i_math::vector3df &vHit);
		if (LevelUtil_UnitHitTest(line,pos3D,lo->GetRadius_()+argHit.radius,argHit.fall,lo->GetHeight()+argHit.radius,posHit))
		{
			line.end=posHit;
			loHit=lo;
		}
	}

	return loHit;
}

CLevelObj *CLoEffectObj::DetectHit_ShieldAmulet(i_math::line3df &line0,LevelEoDetectHitArg &argHit)
{
	LevelPos center=line0.getMiddle().getXZ();
	float range=line0.getLength()/2.0f+argHit.radius;

	DWORD c;
	CLevelObj **buf=_DetectRange_ShieldAmulet(center,range,c);

	i_math::line3df line=line0;

	CLevelObj *loHit=NULL;
	for (int i=0;i<c;i++)
	{
		//目前只支持Unit
		CLevelObj *lo=buf[i];

		if (!_CheckCanHit(lo,argHit))
			continue;

		LevelPos3D pos3D=lo->GetFramePos3D();

		LevelPos3D posHit;
		extern BOOL LevelUtil_ShieldAmuletHitTest(i_math::line3df &line,float radius,CLevelObj * loUnit,i_math::vector3df &vHit);
		if (LevelUtil_ShieldAmuletHitTest(line,argHit.radius,lo,posHit))
		{
			line.end=posHit;
			loHit=lo;
		}
	}

	return loHit;
}


void CLoEffectObj::DetectHits(i_math::line3df &line0,LevelEoDetectHitArg &argHit,LevelObjHits &hits,CLevelObjHistory &history)
{
	hits.Zero();

	LevelPos center=line0.getMiddle().getXZ();
	float range=line0.getLength()/2.0f+argHit.radius;

	DWORD c;
	CLevelObj **buf=_DetectRange(center,range,c);

	i_math::line3df line=line0;

	CLevelObj *loHit=NULL;
	for (int i=0;i<c;i++)
	{
		//目前只支持Unit
		CLevelObj *lo=buf[i];

		if (!_CheckCanHit(lo,argHit))
			continue;

		if (history.Exist(lo->GetID()))
			continue;

		LevelPos3D pos3D=lo->GetFramePos3D();

		LevelPos3D posHit;
		extern BOOL LevelUtil_UnitHitTest(i_math::line3df &line,i_math::vector3df &center,float radius,float fall,float height,i_math::vector3df &vHit);
		if (LevelUtil_UnitHitTest(line,pos3D,lo->GetRadius_()+argHit.radius,argHit.fall,lo->GetHeight()+argHit.radius,posHit))
			hits.Add(lo->GetID());
	}
}



void CLoEffectObj::_MakeRangeDeal(LevelOSB &osbSrc,float radius,DealArg &arg,BOOL bIgnoreHost)
{
	DWORD c;
	CLevelObj **los=_DetectRange(GetFramePos(),radius,c);

	for (int i=0;i<c;i++)
	{
		CLevelObj *loTarget=los[i];
		if (bIgnoreHost)
		{
			if (loTarget->GetID()==_idHost)
				continue;
		}
		LevelPos dir=loTarget->GetFramePos()-_GetInitialPos();

		//override some values
		arg.dir.setXZ(dir.safe_normalize());
		arg.link.id=GetLevel()->GenOpLinkID();
		arg.grd=0;

		_MakeDeals(osbSrc,loTarget,arg);
	}
}

void CLoEffectObj::_MakeRangeDeal3D(LevelOSB &osbSrc,float radius)
{
	DealArg arg;

	LevelPos3D pos3D=GetFramePos3D();
	DWORD c;
	CLevelObj **los=_DetectRange(pos3D.getXZ(),radius,c);

	for (int i=0;i<c;i++)
	{
		CLevelObj *loTarget=los[i];
		if (loTarget->GetType()==LevelObjType_Unit)
		{
			if (TRUE)
			{
				CLoUnit *loUnit=(CLoUnit *)loTarget;
				i_math::spheref sphs[4];
				DWORD nSphs=loUnit->GetSimulateSpheres(sphs,4);

				i_math::spheref sphCenter;
				sphCenter.set(_GetInitialPos3D(),radius);

				BOOL bIntersected=FALSE;;
				for (int j=0;j<nSphs;j++)
				{
					if (sphCenter.intersectWithSphere(sphs[j]))
					{
						bIntersected=TRUE;
						break;
					}
				}

				if (!bIntersected)
					continue;
			}

			LevelPos3D dir=loTarget->GetFramePos3D()-_GetInitialPos3D();
			dir.normalize();

			//override some values
			arg.dir=dir;
			arg.link.id=GetLevel()->GenOpLinkID();
			arg.grd=0;

			_MakeDeals(osbSrc,loTarget,arg);
		}
	}

}



void CLoEffectObj::_MakeRangeDeal(LevelOSB &osbSrc,float radius,BOOL bIgnoreHost)
{
	DealArg arg;
	_MakeRangeDeal(osbSrc,radius,arg,bIgnoreHost);
}

void CLoEffectObj::_MakeZoneDeal(LevelOSB &osbSrc,DealArg &arg,BOOL bIgnoreHost)
{

}


CLevelObj *CLoEffectObj::_GetOwner()
{
	return _owner.GetOwner();
}

CLevelSkill *CLoEffectObj::_GetOwnerSkill()
{
	CLevelSkill *skill=_owner.GetSkill();
	if (skill)
	{
		if (skill->CheckOwnerAlive())
			return skill;
	}
	return NULL;
}

LevelSkillID CLoEffectObj::_GetRootSkillID()
{
	CLevelSkill *skill=GetRootSkill();
	if (skill)
		return skill->GetID();
	return LevelSkillID_Invalid;
}


CLevelSkill *CLoEffectObj::GetRootSkill()
{
	CLevelSkill *skill=_GetOwnerSkill();
	if (skill)
		return skill;

	CLevelObj *lo=_GetOwner();
	if (lo)
	{
		if (lo->GetType()==LevelObjType_Eo)
			return ((CLoEffectObj*)lo)->GetRootSkill();
	}
	return NULL;
}


AnimTick CLoEffectObj::_GetSkillCastingTime()
{
	CLevelSkill *skill;
	if (skill=_owner.GetSkill())
	{
		if (skill->GetState()==SkillState_Casting)
			return skill->GetCastingTime();
	}

	return ANIMTICK_INFINITE;
}

LevelSkillTarget* CLoEffectObj::_GetSkillTarget()
{
	CLevelSkill *skill;
	if (skill=_owner.GetSkill())
		return &skill->GetTarget();
	return NULL;
}


BOOL CLoEffectObj::_GetSkillCastingXfm(i_math::xformf &xfm)
{
	CLevelSkill *skill;
	if (skill=_owner.GetSkill())
	{
		extern BOOL LevelUtil_CalcSkillCastingXfm(CLevelSkill *skill,i_math::xformf &xfm);
		if (LevelUtil_CalcSkillCastingXfm(skill,xfm))
			return TRUE;
	}

	return FALSE;
}


BOOL CLoEffectObj::_CheckSkillCastingEvent(StringID nmEvent)
{
	CLevelSkill *skill;
	if (skill=_owner.GetSkill())
	{
		if (skill->GetState()==SkillState_Casting)
			return skill->CheckCastingEvent(nmEvent);
	}

	return FALSE;
}

AnimTick CLoEffectObj::_GetSkillCastingEventTime(StringID nmEvent)
{
	CLevelSkill *skill;
	if (skill=_owner.GetSkill())
	{
		if (skill->GetState()==SkillState_Casting)
			return skill->GetCastingEventTime(nmEvent);
	}

	return ANIMTICK_INFINITE;
}

AnimTick CLoEffectObj::_GetAge()
{
	AnimTick t=_GetT();
	return ANIMTICK_SAFE_MINUS(t,_tCreate);
}

AnimTick CLoEffectObj::_GetT()
{		
	return _level->GetT_();	
}

CBehaviorMem *CLoEffectObj::GetMem()
{
	if (!_bhv)
		return NULL;
	CBehaviorMem *mem=_bhv->GetMem(0);
	if (!mem)
		return NULL;
	return mem;
}

BOOL CLoEffectObj::_CalcEZoneInfo(AnimEventZone::KeyFan &kFan,i_math::vector3df &pos,i_math::vector3df &dir,float &fov)
{
	if (_eZone)
	{
		i_math::xformf xfm;
		if (_GetSkillCastingXfm(xfm))
		{
			if (_eZone->CalcKeyFan(_eZone->t,kFan))
			{
				kFan.xfmCenter.applyBase(xfm);

				return kFan.CalcInfo(pos,dir,fov);
			}
		}
	}
	return FALSE;
}
