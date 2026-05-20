
#include "stdh.h"

#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelUtil.h"

#include "LoUnit.h"

#include "LoCentipede.h"

#include "Random/Random.h"

#include "LevelRecords.h"
#include "LevelRecordAgent.h"
#include "LevelRecordUnit.h"

#include "LevelOSB.h"

#include "LevelAIContext.h"

#include "Buff_Birth.h"
#include "Buff_CentipedeNode_Crouch.h"
#include "Buff_CentipedeNode_Move.h"
#include "Buff_CentipedeNode_Tendrils.h"

#include "behaviorgraph/BehaviorMem.h"

#include "timer/profiler.h"

static void BuildSpline(CCubicSpline &spline,std::vector<i_math::vector3df> &path)
{
	for (int i=0;i<path.size();i++)
		spline.AddNode(path[i],i_math::quatf());

	spline.BuildSNS();
}


static void BuildSpline(CCubicSpline &spline,LopCentipede *lop)
{
	BuildSpline(spline,lop->path);
}

static void LerpYaws(LevelFaceYaw *yawsFrom,LevelFaceYaw *yawsTo,float r,LevelFace *yawsResult,DWORD nYaws)
{
	if (nYaws<=0)
		return;

	//Root特殊处理
	if (TRUE)
	{
		LevelFace faceFrom=0.0f,faceTo=0.0f;
		LevelFaceApplyYaw(faceFrom,yawsFrom[0]);
		LevelFaceApplyYaw(faceTo,yawsTo[0]);
		LevelFace faceResult=LevelFaceLerp(faceFrom,faceTo,r);
		yawsResult[0]=LevelFaceCalcYaw(0.0f,faceResult);
	}

	for (int i=1;i<nYaws;i++)
		yawsResult[i]=i_math::lerp(yawsFrom[i],yawsTo[i],r);
}



//////////////////////////////////////////////////////////////////////////
//CentipedePose
void CentipedePose::BuildCache(LosCentipede *los,LopCentipede *lop)
{
	if (cache.yaws.size()>0)
		return;//Cache已经Build过了

	CCubicSpline spline;
	BuildSpline(spline,path);

	float distFull=spline.GetDistance();
	float scale=distFull/lop->cache.length;

	cache.yaws.resize(lop->paramsNode.size());

	std::vector<i_math::vector2df> positions;
	positions.resize(lop->paramsNode.size());

	float dist=0.0f;
	for (int i=lop->paramsNode.size()-1;i>=0;i--)
	{
		float t=i_math::clamp_f(dist/distFull,0.0f,1.0f);
		i_math::vector3df pos=spline.GetPosition(t);
		positions[lop->paramsNode.size()-1-i]=pos.getXZ();

		dist+=los->GetNodeRadius(lop->paramsNode[i].tp)*scale;
		if (i-1>=0)
			dist+=los->GetNodeRadius(lop->paramsNode[i-1].tp)*scale;
	}

	cache.yaws.resize(lop->paramsNode.size());

	LevelFace faceBase=0.0f;
	for (int i=0;i<cache.yaws.size()-1;i++)
	{
		LevelFace face=LevelFaceFromDir(positions[i+1]-positions[i]);
		cache.yaws[i]=LevelFaceCalcYaw(faceBase,face);
		faceBase=face;
	}

	cache.yaws[cache.yaws.size()-1]=0.0f;
}

//////////////////////////////////////////////////////////////////////////
//CentipedeAct
BOOL CentipedeAct::ValidatePoses()
{
	if (poses.size()<=0)
		return FALSE;

	if (poses[0].t>0)
		return FALSE;

	for (int i=0;i<poses.size()-1;i++)
	{
		if (poses[i+1].t<=poses[i].t)
			return FALSE;
	}
	return TRUE;
}

AnimTick CentipedeAct::GetDur()
{
	if (poses.size()<=0)
		return 0;

	return poses[poses.size()-1].t;
}




////////////////////////////////////////////////////////////////////////
//LopCentipede
void LopCentipede::BuildCache(LosCentipede *los)
{
	if (cache.length>0.0f)
		return;//Cache已经Build过了

	for (int i=0;i<paramsNode.size()-1;i++)
	{
		cache.length+=los->GetNodeRadius(paramsNode[i].tp);
		cache.length+=los->GetNodeRadius(paramsNode[i+1].tp);
	}

	for (int i=0;i<acts.size();i++)
	{
		CentipedeAct &act=acts[i];
		assert(act.ValidatePoses());
		for (int j=0;j<act.poses.size();j++)
			act.poses[j].BuildCache(los,this);
	}
}


//////////////////////////////////////////////////////////////////////////
//LosCentipede
BOOL LosCentipede::IsValid()
{
	if ((idUnit_Node!=RecordID_Invalid)&&
		(idItem_HeadTail!=RecordID_Invalid)&&
		(idItem_Head!=RecordID_Invalid)&&
		(idItem_Tail!=RecordID_Invalid)&&
		(idItem_NoTailNoTail!=RecordID_Invalid))
		return TRUE;

	return FALSE;
}


float LosCentipede::GetNodeRadius(CentipedeNodeType tp)
{
	switch(tp)
	{
		case CentipedeNode_Head:			return radiusHead;
		case CentipedeNode_Body:			return radiusBody;
		case CentipedeNode_Tail:			return radiusTail;
	}
	return 0.0f;
}

RecordID LosCentipede::GetUnitID_Node(CentipedeNodeType tp)
{
	switch(tp)
	{
		case CentipedeNode_Head:
		case CentipedeNode_Body:
		case CentipedeNode_Tail:
			return idUnit_Node;
	}
	return RecordID_Invalid;
}

RecordID LosCentipede::GetUnitID_Cyst()
{
	return idUnit_Cyst;
}


RecordID LosCentipede::GetEquip(CentipedeNodeType tp)
{
	switch(tp)
	{
		case CentipedeNode_Head:			return idItem_Head;
		case CentipedeNode_Body:			return idItem_NoTailNoTail;
		case CentipedeNode_Tail:			return idItem_Tail;
	}

	return RecordID_Invalid;
}

void CentipedeNodesLoc::WriteSync(CBitPacket *bp,BOOL &bContent)
{
	bp->Bit_Write(bDirty);
	if (bDirty)
	{
		bContent=TRUE;

		DP_WriteVector(*bp,positions);
		DP_WriteVector(*bp,faces);
	}
}

void CentipedeNodesLoc::ReadSync(CBitPacket *bp)
{
	if (bp->Bit_Read())
	{
		DP_ReadVector(*bp,positions);
		DP_ReadVector(*bp,faces);
	}
}

void CentipedeNodesLoc::CalcDists()
{
	dists.resize(positions.size());
	if (dists.size()<=0)
		return;

	dists[dists.size()-1]=0.0f;
	distTotal=0.0f;
	if (positions.size()>1)
	{
		for (int i=positions.size()-2;i>=0;i--)
		{
			distTotal+=positions[i].getDistanceFrom(positions[i+1]);
			dists[i]=distTotal;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//CCentipedeState_Resting
void CCentipedeState_Resting::Clear()
{
	_splinePath.Reset(FALSE);
	Zero();
}

void CCentipedeState_Resting::Reset(CLoCentipede *owner_,AnimTick t)
{
	_owner=owner_;
	_status=Resting;
	_tStatusStart=t;

	_bDirty=TRUE;
}

void CCentipedeState_Resting::StandUp(LevelObjID idFrom_,AnimTick t)
{
	_status=Standing;
	_idFrom=idFrom_;
	_tStatusStart=t;

	_durStandUp=_owner->CalcStandUpDur(idFrom_);

	_bDirty=TRUE;
}

void CCentipedeState_Resting::StartMarch(AnimTick t)
{
	if (_status==Marching)
		return;

	_status=Marching;
	_tStatusStart=t;

	BuildSpline(_splinePath,(LopCentipede*)_owner->GetLop());


	_bDirty=TRUE;
}


void CCentipedeState_Resting::Update(AnimTick t)
{
	LosCentipede *los=(LosCentipede*)_owner->GetLos();
	LopCentipede *lop=(LopCentipede*)_owner->GetLop();

	if (_status==CCentipedeState_Resting::Marching)
	{
		CentipedeNodesLoc &locsNode=_owner->_locsNode;

		AnimTick delta=ANIMTICK_SAFE_MINUS(t,_tStatusStart);
		float off=lop->off-	los->spdMarch*ANIMTICK_TO_SECOND(delta);

		float distFull=_splinePath.GetDistance();
		float dist=off;
		i_math::vector3df pos,vel;
		for (int i=0;i<lop->paramsNode.size();i++)
		{
			float t=i_math::clamp_f(dist/distFull,0.0f,1.0f);
			pos=_splinePath.GetPosition(t);
			vel=_splinePath.GetVelocity(t);

			LevelFace face=LevelFaceFromDir(vel.getXZ());
			face+=i_math::Pi;
			
			locsNode.positions[i]=pos.getXZ();
			locsNode.faces[i]=face;

			dist+=los->GetNodeRadius(lop->paramsNode[i].tp);
			if (i+1<lop->paramsNode.size())
				dist+=los->GetNodeRadius(lop->paramsNode[i+1].tp);
		}

		locsNode.CalcDists();
		locsNode.SetDirty();
	}

	if (_status==Standing)
	{
		if (t>=_tStatusStart+_durStandUp)
			StartMarch(t);
	}
}


void CCentipedeState_Resting::WriteSync(CBitPacket *bp,BOOL &bContent)
{
	bp->Bit_Write(_bDirty);
	if (_bDirty)
	{
		bContent=TRUE;
		bp->Bits_Write(_status,3);
		bp->Data_WriteSimple(_idFrom);
	}
}

//////////////////////////////////////////////////////////////////////////
//CCentipedeState_Combat
void CCentipedeState_Combat::Clear()
{
	_rope.Clear();
	Zero();
}

void CCentipedeState_Combat::Reset(CLoCentipede *owner,AnimTick t)
{
	_owner=owner;
	_bDirty=TRUE;
	_t=t;

	LosCentipede *los=(LosCentipede*)_owner->GetLos();
	LopCentipede *lop=(LopCentipede*)_owner->GetLop();


	CentipedeNodesLoc &locsNode=_owner->_locsNode;

	_rope.Init(locsNode.positions.size(),los->propRope,_owner->GetLevel()->GetUnitMgr());

	_nUnbroken=locsNode.positions.size();

	if (TRUE)
	{
		for (int i=0;i<locsNode.positions.size();i++)
		{
			int iNode=locsNode.positions.size()-1-i;
			Mass *mass=_rope.GetMass(i);
			mass->pos.x=locsNode.positions[iNode].x;
			mass->pos.z=locsNode.positions[iNode].y;
		}

// 		if (locsNode.positions.size()>=2)
// 		{
// 			LevelPos dir=locsNode.positions[locsNode.positions.size()-2]-locsNode.positions[locsNode.positions.size()-1];
// 			_rope.SetRootFace(LevelFaceFromDir(dir));
// 		}
	}

	if (lop->acts.size()>0)
		PlayAct(lop->acts[0].nm,TRUE,t);
}

void CCentipedeState_Combat::WriteSync(CBitPacket *bp,BOOL &bContent)
{
	bp->Bit_Write(_bDirty);
	if (_bDirty)
	{
		bContent=TRUE;
	}
}

void CCentipedeState_Combat::_RefreshRopeUnit()
{
	CLevel *level=_owner->GetLevel();
	for (int i=0;i<_owner->_nodes.size();i++)
	{
		LevelObjID id=_owner->_nodes[i].idLo;

		CUnit *unit=NULL;
		CLevelObj *loUnit=NULL;
		if (TRUE)
		{
			extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
			CLevelObj *lo=LevelUtil_GetAliveLo(level,id);
			if (lo)
			{
				unit=lo->GetUnit();
				loUnit=lo;
			}
		}

		_rope.SetUnit(_owner->_nodes.size()-1-i,unit);

		if (loUnit)
		{
			Mass *mass=_rope.GetMass(_owner->_nodes.size()-1-i);
			if (mass)
			{
				if (mass->bStatic)
				{
					LevelPos pos=loUnit->GetFramePos();
					mass->pos.x=pos.x;
					mass->pos.z=pos.y;
				}
			}
		}
	}
	
}


void CCentipedeState_Combat::Update(AnimTick t)
{
	LosCentipede *los=(LosCentipede*)_owner->GetLos();
	CentipedeNodesLoc &locsNode=_owner->_locsNode;

	if (locsNode.stretch.IsFullyStretchedOut(_owner->GetT()))
		_RefreshRopeUnit();

	AnimTick dt=ANIMTICK_SAFE_MINUS(t,_t);
	_t=t;

	if (!_acts.empty())
	{
		_UpdateAct();

		if (FALSE)
		{
			LevelFace face=_rope.GetRootFace();
			face+=ANIMTICK_TO_SECOND(dt)*90.0f*i_math::GRAD_PI2;
			face=i_math::normalize_radian(face);
			_rope.SetRootFace(face);
		}

		dt*=2;

		int nIterate=los->propRope.nIterate;
		float fDt=ANIMTICK_TO_SECOND(dt)/(float)nIterate;

		ProfilerStart(CentipedeRope_Sim);

		for (int i=0;i<nIterate;i++)
		{
			_rope.Solve(fDt);
			_rope.Simulate(fDt);
		}

		ProfilerEnd();


		//更新
		if (TRUE)
		{

			for (int i=0;i<_rope.GetMassCount();i++)
			{
				int iNode=_rope.GetMassCount()-1-i;
				Mass *mass=_rope.GetMass(i);

				locsNode.positions[iNode]=mass->pos.convert<float>().getXZ();
			}

			for (int i=0;i<locsNode.positions.size()-1;i++)
			{
				LevelPos dir=locsNode.positions[i]-locsNode.positions[i+1];
				LevelFace face=LevelFaceFromDir(dir);
				locsNode.faces[i]=face;
			}
			if (locsNode.faces.size()>=2)
				locsNode.faces[locsNode.faces.size()-1]=locsNode.faces[locsNode.faces.size()-2];

			locsNode.SetDirty();
		}
	}

}

void CCentipedeState_Combat::PlayAct(StringID nm,BOOL bLoop,AnimTick t)
{
	LosCentipede *los=(LosCentipede*)_owner->GetLos();
	LopCentipede *lop=(LopCentipede*)_owner->GetLop();

	int idx;
	VEC_FIND_BY_ELEMENT(lop->acts,nm,nm,idx);
	if (idx!=-1)
	{
		ActState state;
		state.tStart=t;
		state.durBlend=ANIMTICK_FROM_SECOND(0.5f);
		state.idxAct=idx;
		state.bLoop=bLoop;

		_acts.push_back(state);
	}
}

BOOL CCentipedeState_Combat::GetTopActProgress(AnimTick &tCur,AnimTick &dur)
{
	LopCentipede *lop=(LopCentipede*)_owner->GetLop();

	if (_acts.size()<=0)
		return FALSE;
	ActState &state=_acts[_acts.size()-1];

	CentipedeAct &act=lop->acts[state.idxAct];
	AnimTick t=_owner->GetT();
	tCur=ANIMTICK_SAFE_MINUS(t,state.tStart);
	dur=act.GetDur();
	
	return TRUE;
}


void CCentipedeState_Combat::_CalcActYaws(ActState &state,AnimTick t,LevelFaceYaw *yaws)
{
	LopCentipede *lop=(LopCentipede*)_owner->GetLop();

	DWORD nYaws=_owner->GetNodeCount();

	AnimTick tLocal=ANIMTICK_SAFE_MINUS(t,state.tStart);

	CentipedeAct &act=lop->acts[state.idxAct];

	AnimTick dur=act.poses[act.poses.size()-1].t;

	if (dur<=0)
		tLocal=0;
	else
	{
		if (!state.bLoop)
		{
			if (tLocal>dur)
				tLocal=dur;
		}
		else
			tLocal=tLocal%dur;
	}

	if (act.poses.size()==1)
	{
		memcpy(yaws,&act.poses[0].cache.yaws[0],nYaws*sizeof(LevelFaceYaw));
		return;
	}

	for (int i=0;i<act.poses.size()-1;i++)
	{
		CentipedePose &poseCur=act.poses[i];
		CentipedePose &poseNext=act.poses[i+1];
		if ((poseCur.t<=tLocal)&&(poseNext.t>=tLocal))
		{
			float r=((float)(tLocal-poseCur.t))/(float)(poseNext.t-poseCur.t);

			assert(poseCur.cache.yaws.size()==poseNext.cache.yaws.size());
			LerpYaws(&poseCur.cache.yaws[0],&poseNext.cache.yaws[0],r,yaws,nYaws);
			return;
		}
	}

	int iEnd=act.poses.size()-1;
	memcpy(yaws,&act.poses[iEnd].cache.yaws[0],nYaws*sizeof(LevelFaceYaw));
}

void CCentipedeState_Combat::_UpdateAct()
{
	LevelFaceYaw yaws[128];//Big enough

	if (_acts.size()<=0)
		return;

	AnimTick t=_owner->GetT();
	DWORD nYaws=_owner->GetNodeCount();

	//清除已经完全BlendOut的Acts
	for (int i=_acts.size()-1;i>=1;i--)
	{
		ActState &state=_acts[i];
		if (state.CalcBlendRate(t)>=1.0f)
		{
			for (int j=0;j<i;j++)
				_acts.pop_front();
			break;
		}
	}

	_CalcActYaws(_acts[0],t,yaws);

	LevelFaceYaw yaws2[128];
	for(int i=1;i<_acts.size();i++)
	{
		ActState &state=_acts[i];
		_CalcActYaws(state,t,yaws2);
		float r=state.CalcBlendRate(t);

		LerpYaws(yaws,yaws2,r,yaws,nYaws);
	}

	_rope.SetTwists(yaws);
}

static BOOL CalcCystPos(CLevel *level,LosCentipede *los,LevelObjID idNode,BOOL bLeft,LevelPos &posCyst,LevelFace &faceCyst)
{
	LevelPos pos;
	LevelFace face2;
	if (TRUE)
	{
		CLevelObj *loNode=LevelUtil_GetAliveLo(level,idNode);
		if (!loNode)
			return FALSE;

		pos=loNode->GetFramePos();
		LevelFace face=loNode->GetFrameFace();
		LevelFaceYaw yaw;
		if (bLeft)
			yaw=-90.0f*i_math::GRAD_PI2;
		else
			yaw=90.0f*i_math::GRAD_PI2;
		LevelFaceApplyYaw(face,yaw);

		LevelPos dir=LevelFaceToDir(face);
		dir*=los->paramCyst.sideoff;
		pos+=dir;
		face2=LevelFaceFromDir(dir);
	}

	posCyst=pos;
	faceCyst=face2;
	return TRUE;
}


void CCentipedeState_Combat::SpawnCyst(int idxNode,BOOL bLeft)
{
	if (idxNode>=_owner->_nodes.size())
		return;

	CentipedeNode &node=_owner->_nodes[idxNode];
	if (node.cyst.id!=LevelObjID_Invalid)
		return;//已经有cyst了

	LosCentipede *los=(LosCentipede*)_owner->GetLos();
	LopCentipede *lop=(LopCentipede*)_owner->GetLop();
	CLevel *level=_owner->GetLevel();

	LevelPos pos;
	LevelFace face;
	if (!CalcCystPos(level,los,node.idLo,bLeft,pos,face))
		return;

	LevelObjID idCyst=LevelUtil_CreateUnit(level,los->GetUnitID_Cyst(),pos,face,LevelPlayerID_Wild);

	_owner->_SetAgentVar(LevelUtil_GetAliveLo(level,idCyst));

	node.cyst.bLeft=bLeft;
	node.cyst.id=idCyst;

	_owner->_AddCentipedeCystSync(idxNode);
}

void CCentipedeState_Combat::NotifyCystKilled(int idxNode,LevelObjID idCyst)
{
	if (idxNode>=_owner->_nodes.size())
		return;

	CentipedeNode &node=_owner->_nodes[idxNode];
	if (node.cyst.id==idCyst)
		node.cyst.id=LevelObjID_Invalid;
}


void CCentipedeState_Combat::_Break(int idxNode,RecordID idSkill,LevelOpLink &link)
{
	LosCentipede *los=(LosCentipede*)_owner->GetLos();

	CentipedeNode &node=_owner->_nodes[idxNode];

	CLevel *level=_owner->GetLevel();
	extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
	CLevelObj *lo=LevelUtil_GetAliveLo(level,node.idLo);
	if (lo)
	{

		if (TRUE)
		{
			LevelOSB osb(lo);
			CLevelDecider *decide=level->GetDecider();

			if (TRUE)
			{
				CLevelBuff *buff=LevelUtil_FindBuff(lo,Class_Ptr2(Buff_CentipedeNode_Move));
				if (buff)
					decide->RemoveBuff(osb,lo,buff,link);
			}

			if (TRUE)
			{
				CLevelBuff *buff=LevelUtil_FindBuff(lo,Class_Ptr2(Buff_CentipedeNode_Crouch));
				if (buff)
					decide->RemoveBuff(osb,lo,buff,link);
			}

			if (TRUE)
			{
				Buff_CentipedeNode_Tendrils*buff=(Buff_CentipedeNode_Tendrils*)LevelUtil_FindBuff(lo,Class_Ptr2(Buff_CentipedeNode_Tendrils));
				if (buff)
					buff->SetOwnerBroken();
			}
		}

		lo->SetAICmd(LevelAIContext::GetStdCmd_Combat());

		if (idSkill!=RecordID_Invalid)
		{
			CLevelSkillDriver *driver=lo->GetSkillDriver();
			if (driver)
			{
				LevelSkillTarget target;
				driver->StartCast(LevelSkillType(idSkill),target,1,NULL,&link);
			}
		}
	}

	_rope.Break(_owner->_nodes.size()-1-idxNode);

	if (idxNode<_nUnbroken)
		_nUnbroken=idxNode;
}


void CCentipedeState_Combat::BreakFromCyst(int idxNode,LevelOpLink &link)
{
	LosCentipede *los=(LosCentipede*)_owner->GetLos();
	CentipedeNode &node=_owner->_nodes[idxNode];
	RecordID idSkill=node.cyst.bLeft?los->idSkill_NodeBreakL:los->idSkill_NodeBreakR;

	_Break(idxNode,idSkill,link);
}

void CCentipedeState_Combat::Break(int idxNode,LevelOpLink &link)
{
	LosCentipede *los=(LosCentipede*)_owner->GetLos();
	_Break(idxNode,los->idSkill_NodeBreakB,link);
}


////////////////////////////////////////////////////////////////////////
//CLoCentipede

void CLoCentipede::PostCreate()
{
	CLoAgent::PostCreate();

	LopCentipede *lop=(LopCentipede*)_param;
	LosCentipede *los=(LosCentipede*)_src;
	lop->BuildCache(los);

	_BuildNodes();

	if (lop->modeWorking==CentipedeWorkingMode_Resting)
		_stateResting.Reset(this,_level->GetT_());

	if (lop->modeWorking==CentipedeWorkingMode_Combat)
		_stateCombat.Reset(this,_level->GetT_());

	if (TRUE)
	{
		LevelRecordAgent *rec=_GetRec();
		if (rec)
		{
			if (rec->idBG!=StringID_Invalid)
			{
				LevelBehaviorContext ctx;
				ctx.lo=this;
				ctx.memSimple=&_memSimple;
				_bhv=_level->CreateBehavior(rec->idBG,ctx);
				if (_bhv)
					_bhv->Start();
			}
		}
	}

	//地幔喷射者Service,用来控制最多能有几个地幔喷射者同时存在,目前设定为1个
// 	if (TRUE)
// 	{
// 		extern CLevelService *LevelUtil_GetService(CLevelObj *lo,LevelServiceType tp);
// 		CLevelService *service=LevelUtil_GetService(this,LevelService_蜈蚣节点地幔喷射者);
// 		if (service)
// 			service->AddUniqueServer(GetID(),1);
// 	}
}

void CLoCentipede::OnDestroy()
{
	if (_bhv)
	{
		_bhv->Clear();
		Safe_Class_Delete(_bhv);
	}

	_lookupNodeIndex.clear();
	_nodes.clear();

	_locsNode.Clear();
	_stateResting.Clear();
	_stateCombat.Clear();
}


BOOL CLoCentipede::OnActivate()
{
	_level->RegisterCentipede(this);

	return TRUE;
}

void CLoCentipede::OnDeactivate()
{
	_level->UnRegisterCentipede(this);
}

RecordID CLoCentipede::GetNodeUnitID()
{
	LosCentipede *los=(LosCentipede*)_src;
	return los->GetUnitID_Node(CentipedeNode_Body);
}

void CLoCentipede::Update()
{
	LopCentipede *lop=(LopCentipede*)_param;

	if (_bhv)
		_bhv->Update();

	if (lop->modeWorking==CentipedeWorkingMode_Resting)
	{
		_stateResting.Update(_level->GetT_());
	}

	if (lop->modeWorking==CentipedeWorkingMode_Combat)
	{
		_stateCombat.Update(_level->GetT_());
		_UpdateTouch();
	}

}

void CLoCentipede::_SetAgentVar(CLevelObj *lo)
{
	if (!lo)
		return;

	LosCentipede *los=(LosCentipede*)_src;

	if (los->varCentipedeAgent!=StringID_Invalid)
	{
		CLevelBehavior *bhv=lo->GetBehaviorAI();
		if (bhv)
		{
			CBehaviorMem *mem=bhv->GetMem(0);
			if (mem)
				mem->SetID(los->varCentipedeAgent,BehaviorMemType_ObjID,GetID());
		}
	}

}


void CLoCentipede::_BuildNode(CentipedeNode &node,LevelPos3D &pos3D,LevelFace face,CentipedeNodeParam &param)
{
	LosCentipede *los=(LosCentipede*)_src;
	LopCentipede *lop=(LopCentipede*)_param;

	LevelPos pos=pos3D.getXZ();

	LevelObjID idUnit=LevelUtil_CreateUnit(_level,los->GetUnitID_Node(param.tp),pos,face,LevelPlayerID_Wild);
	CLoUnit *lo=(CLoUnit *)LevelUtil_GetAliveLo(_level,idUnit);

	if (TRUE)
	{
		RecordID idItem=los->GetEquip(param.tp);
		if (idItem!=RecordID_Invalid)
			lo->AddExprEquips(idItem);
	}

	_SetAgentVar(lo);

	if (los->senariosWorkingMode[lop->modeWorking]!=StringID_Invalid)
		lo->SetAIScenario(los->senariosWorkingMode[lop->modeWorking]);

	node.owner=this;
	node.param=&param;
	node.idLo=lo->GetID();

	SAFE_RELEASE(lo);
}


void CLoCentipede::_BuildNodes()
{
	LosCentipede *los=(LosCentipede*)_src;
	LopCentipede *lop=(LopCentipede*)_param;
	LevelRecordAgent *rec=los->GetRecord();

	if (lop->path.size()<=1)
		return;
	if (lop->paramsNode.size()<=0)
		return;

	CCubicSpline spline;
	BuildSpline(spline,lop);

	float distFull=spline.GetDistance();

	_nodes.resize(lop->paramsNode.size());
	if (TRUE)
	{
		float dist=lop->off;
		i_math::vector3df pos,vel;

		if (lop->modeWorking==CentipedeWorkingMode_Resting)
		{
			for (int i=0;i<lop->paramsNode.size();i++)
			{
				float t=i_math::clamp_f(dist/distFull,0.0f,1.0f);
				pos=spline.GetPosition(t);
				vel=spline.GetVelocity(t);

				LevelFace face=LevelFaceFromDir(vel.getXZ());
				face+=i_math::Pi;
				
				_BuildNode(_nodes[i],pos,face,lop->paramsNode[i]);

				dist+=los->GetNodeRadius(lop->paramsNode[i].tp);
				if (i+1<lop->paramsNode.size())
					dist+=los->GetNodeRadius(lop->paramsNode[i+1].tp);
			}
		}
		if (lop->modeWorking==CentipedeWorkingMode_Combat)
		{
			for (int i=lop->paramsNode.size()-1;i>=0;i--)
			{
				float t=i_math::clamp_f(dist/distFull,0.0f,1.0f);
				pos=spline.GetPosition(t);
				vel=spline.GetVelocity(t);

				LevelFace face=LevelFaceFromDir(vel.getXZ());
// 				face+=i_math::Pi;

				_BuildNode(_nodes[i],pos,face,lop->paramsNode[i]);

				dist+=los->GetNodeRadius(lop->paramsNode[i].tp);
				if (i-1>=0)
					dist+=los->GetNodeRadius(lop->paramsNode[i-1].tp);
			}
		}

	}

	//初始化_locsNode
	if (TRUE)
	{
		_locsNode.positions.resize(_nodes.size());
		_locsNode.faces.resize(_nodes.size());
		for (int i=0;i<_nodes.size();i++)
		{
			LevelObjID id=_nodes[i].idLo;
			extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
			CLevelObj *lo=LevelUtil_GetAliveLo(_level,id);
			assert(lo);
			if (lo)
			{
				_locsNode.positions[i]=lo->GetFramePos();
				_locsNode.faces[i]=lo->GetFrameFace();
			}
		}
		_locsNode.CalcDists();
		_locsNode.SetDirty();
	}

	//初始化_lookupNodeIndex
	for (int i=0;i<_nodes.size();i++)
	{
		LevelObjID id=_nodes[i].idLo;
		extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
		CLevelObj *lo=LevelUtil_GetAliveLo(_level,id);
		assert(lo);
		if (lo)
			_lookupNodeIndex[lo->GetID()]=i;
	}

}

void CLoCentipede::_OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	LopCentipede *lop=(LopCentipede*)_param;

	DP_WriteVector(*bp,lop->path);

	bp->Data_NextDword()=_nodes.size();
	for (int i=0;i<_nodes.size();i++)
		bp->Data_WriteSimple(_nodes[i].idLo);

	if (lop->modeWorking==CentipedeWorkingMode_Resting)
		_stateResting.WriteSync(bp,bContent);
	if (lop->modeWorking==CentipedeWorkingMode_Combat)
		_stateCombat.WriteSync(bp,bContent);
}

void CLoCentipede::_OnWriteSyncL(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	LopCentipede *lop=(LopCentipede*)_param;

	_locsNode.WriteSync(bp,bContent);

	if (lop->modeWorking==CentipedeWorkingMode_Resting)
		_stateResting.WriteSync(bp,bContent);
	if (lop->modeWorking==CentipedeWorkingMode_Combat)
		_stateCombat.WriteSync(bp,bContent);

	if (_syncsCentipedeCyst.size()<=0)
		bp->Bit_Write_0();
	else
	{
		bp->Bit_Write_1();
		DP_WriteVectorN(*bp,_syncsCentipedeCyst);
	}
}


void CLoCentipede::_OnPostWriteSync()
{
	_stateResting.ClearDirty();
	_stateCombat.ClearDirty();

	_locsNode.ClearDirty();

	_syncsCentipedeCyst.clear();
}

CentipedeWorkingMode CLoCentipede::GetWorkingMode()
{
	LopCentipede *lop=(LopCentipede*)_param;

	return lop->modeWorking;
}


void CLoCentipede::Activate(CLevelObj *loFrom)
{
	LopCentipede *lop=(LopCentipede*)_param;

	if (lop->modeWorking==CentipedeWorkingMode_Resting)
		_stateResting.StandUp(loFrom->GetID(),_level->GetT_());
}

BOOL CLoCentipede::GetNodeIndex(LevelObjID id,DWORD &idx)
{
	std::unordered_map<LevelObjID,int>::iterator it=_lookupNodeIndex.find(id);
	if (it!=_lookupNodeIndex.end())
	{
		idx=(DWORD)(*it).second;
		return TRUE;
	}
	return FALSE;
}


BOOL CLoCentipede::GetNodeLoc(LevelObjID id,LevelPos &pos,LevelFace &face)
{
	std::unordered_map<LevelObjID,int>::iterator it=_lookupNodeIndex.find(id);
	if (it!=_lookupNodeIndex.end())
	{
		int idx=(*it).second;

		float rateStretch=_locsNode.stretch.GetRate(_level->GetT_());
		if ((rateStretch>=1.0f)||(_locsNode.positions.size()<=1))
		{
			pos=_locsNode.positions[idx];
			face=_locsNode.faces[idx];
			return TRUE;
		}

		float dist=_locsNode.dists[idx];
		dist+=(rateStretch-1.0f)*_locsNode.distTotal;

		if (dist<=0.0f)
		{
			LevelPos dir=_locsNode.positions[_locsNode.positions.size()-1]-_locsNode.positions[_locsNode.positions.size()-2];
			dir.normalize();

			if (dist<-2.0f)
				dist=-2.0f;
			dir*=-dist;

			pos=_locsNode.positions[_locsNode.positions.size()-1]+dir;
			face=_locsNode.faces[_locsNode.positions.size()-1];

			return TRUE;
		}

		int idxNextNode=0;
		for (int i=_locsNode.positions.size()-2;i>=0;i--)
		{
			if (_locsNode.dists[i]>dist)
			{
				idxNextNode=i;
				break;
			}
		}

		float r;
		r=(dist-_locsNode.dists[idxNextNode+1])/(_locsNode.dists[idxNextNode]-_locsNode.dists[idxNextNode+1]);

		if (r>1.0f)
			r=1.0f;

		pos=_locsNode.positions[idxNextNode].getInterpolated(_locsNode.positions[idxNextNode+1],r);
		face=LevelFaceLerp(_locsNode.faces[idxNextNode+1],_locsNode.faces[idxNextNode],r);

		return TRUE;
	}
	return FALSE;
}

BOOL CLoCentipede::GetCystLoc(LevelObjID id,LevelPos &pos,LevelFace &face)
{
	LosCentipede *los=(LosCentipede*)_src;

	for (int i=0;i<_nodes.size();i++)
	{
		if (_nodes[i].cyst.id==id)
			return CalcCystPos(_level,los,_nodes[i].idLo,_nodes[i].cyst.bLeft,pos,face);
	}
	return FALSE;
}

BOOL CLoCentipede::GetCystLeftOrRight(LevelObjID idCyst,BOOL &bLeft)
{
	for (int i=0;i<_nodes.size();i++)
	{
		if (_nodes[i].cyst.id==idCyst)
		{
			bLeft=_nodes[i].cyst.bLeft;
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CLoCentipede::GetNodeFromCyst(LevelObjID idCyst,LevelObjID &id)
{
	id=LevelObjID_Invalid;

	for (int i=0;i<_nodes.size();i++)
	{
		if (_nodes[i].cyst.id==idCyst)
		{
			id=_nodes[i].idLo;
			return TRUE;
		}
	}
	return FALSE;
}

LevelObjID CLoCentipede::GetNodeFromIndex(DWORD idx)
{
	if (idx<_nodes.size())
		return _nodes[idx].idLo;
	return LevelObjID_Invalid;
}




AnimTick CLoCentipede::CalcStandUpDur(LevelObjID idFrom)
{
	LosCentipede *los=(LosCentipede*)_src;

	AnimTick dur=0;

	std::unordered_map<LevelObjID,int>::iterator it=_lookupNodeIndex.find(idFrom);
	if (it!=_lookupNodeIndex.end())
	{
		int idx=(*it).second;
		DWORD nBefore=idx+1;
		DWORD nAfter=_nodes.size()-idx;
		DWORD n=nBefore>nAfter?nBefore:nAfter;

		dur=n*los->durStandUpStep+los->durStandUp;
	}

	return dur;
}

void CLoCentipede::_AddCentipedeCystSync(int idxNode)
{
	if (idxNode<_nodes.size())
	{
		CentipedeNode &node=_nodes[idxNode];
		if (node.cyst.id!=LevelObjID_Invalid)
		{
			CentipedeCystSync sync;
			sync.idNode=node.idLo;
			sync.idCyst=node.cyst.id;
			sync.bLeft=node.cyst.bLeft;

			_syncsCentipedeCyst.push_back(sync);
		}
	}
}

BOOL CLoCentipede::BreakFromCyst(LevelObjID id,LevelOpLink &link)
{
	LopCentipede *lop=(LopCentipede*)_param;

	if (lop->modeWorking==CentipedeWorkingMode_Combat)
	{
		DWORD idxNode;
		if (GetNodeIndex(id,idxNode))
		{
			_stateCombat.BreakFromCyst(idxNode,link);
			return TRUE;
		}
	}

	return FALSE;
}

BOOL CLoCentipede::Break(LevelObjID id,LevelOpLink &link)
{
	LopCentipede *lop=(LopCentipede*)_param;

	if (lop->modeWorking==CentipedeWorkingMode_Combat)
	{
		DWORD idxNode;
		if (GetNodeIndex(id,idxNode))
		{
			_stateCombat.Break(idxNode,link);
			return TRUE;
		}
	}

	return FALSE;
}

void CLoCentipede::_UpdateTouch()
{
	LosCentipede *los=(LosCentipede*)_src;

	_bufTouched.clear();

	if (TRUE)
	{
		CLevelPlayer *player=LevelUtil_GetFirstPlayer(_level);
		if (player)
		{
			LevelPlayerID idPlayer=player->GetPlayerID();

			DWORD c;
			CLevelObj **objs=_level->GetActiveObjs(c);

			for (int i=0;i<c;i++)
			{
				CLevelObj *lo=objs[i];
				if (lo->GetType()==LevelObjType_Unit)
				{
					if (lo->GetPlayerID()==idPlayer)
					{
						extern BOOL LevelUtil_CheckDead(CLevelObj *lo);
						if (!LevelUtil_CheckDead(lo))
						{
							_bufTouched.resize(_bufTouched.size()+1);
							Touched &t=_bufTouched[_bufTouched.size()-1];
							t.lo=lo;
							t.pos=lo->GetFramePos();
							t.bTouchedByUnBroken=FALSE;
							t.nTouchedByBroken=0;
						}
					}
				}
			}
		}
	}

	for (int i=0;i<_nodes.size();i++)
	{
		CLevelObj *loNode=LevelUtil_GetAliveLo(_level,_nodes[i].idLo);
		if (loNode)
		{
			Buff_CentipedeNode_Tendrils *buff=(Buff_CentipedeNode_Tendrils*)LevelUtil_FindBuff(loNode,Class_Ptr2(Buff_CentipedeNode_Tendrils));
			if (buff)
			{
				if (!buff->IsDisappear())
				{
					BOOL bBroken=buff->IsOwnerBroken();

					LevelPos posNode=loNode->GetFramePos();
					float radiusNode=loNode->GetRadius_();

					for (int j=0;j<_bufTouched.size();j++)
					{
						Touched &t=_bufTouched[j];

						if ((!bBroken)&&(t.bTouchedByUnBroken))
							continue;

						BOOL bTouching=FALSE;
						if (TRUE)
						{
							float distSQ=posNode.getDistanceSQFrom(t.pos);
							float radiusSQ=radiusNode+t.lo->GetRadius_()+los->paramTouch.radius;
							radiusSQ*=radiusSQ;
							if (distSQ<radiusSQ)
								bTouching=TRUE;
						}

						if (bTouching)
						{
							if (bBroken)
								t.nTouchedByBroken++;
							else
								t.bTouchedByUnBroken=TRUE;
						}
					}
				}
			}
		}
	}

	if (TRUE)
	{
		DealArg arg;
		LevelOSB osb(this);
		for (int i=0;i<_bufTouched.size();i++)
		{
			Touched &t=_bufTouched[i];

			if (t.bTouchedByUnBroken)
				los->paramTouch.deal->Make(osb,t.lo,arg,NULL);

			for (int j=0;j<t.nTouchedByBroken;j++)
				los->paramTouch.deal->Make(osb,t.lo,arg,NULL);
		}
	}

	_bufTouched.clear();
}

float CLoCentipede::GetUnbrokenRate()
{
	LopCentipede *lop=(LopCentipede*)_param;

	if (lop->modeWorking==CentipedeWorkingMode_Combat)
	{
		DWORD nUnbroken=_stateCombat._nUnbroken;
		float r=((float)nUnbroken)/(float)_locsNode.positions.size();
		if (r>1.0f)
			r=1.0f;
		return r;
	}

	return 1.0f;
}

BOOL CLoCentipede::IsLeftArm()
{
	LopCentipede *lop=(LopCentipede*)_param;
	return lop->bLeftArm;
}

void CLoCentipede::StretchOut(AnimTick dur)
{
	if (_locsNode.stretch.IsStretchingOut())
		return;

	_locsNode.stretch.StretchOut(_level->GetT_(),dur);
}

void CLoCentipede::StretchIn(AnimTick dur)
{
	if (!_locsNode.stretch.IsStretchingOut())
		return;

	_locsNode.stretch.StretchIn(_level->GetT_(),dur);
}
