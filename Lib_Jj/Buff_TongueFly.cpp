/********************************************************************
	created:	2020/02/02
	file base:	Buff_TongueFly
	author:		cxi
	
*********************************************************************/
#include "stdh.h"

#include "Level.h"

#include "LevelRecordBuff.h"

#include "Buff_TongueFly.h"

#include "LevelPlayer.h"
#include "LoUnit.h"
#include "LoSnailP1.h"

#include "LevelUtil.h"

#include "PositionBasedDynamics/Simulation/TimeManager.h"

#include "Buff_Dead.h"
#include "Skill_GeneralAdvS.h"

#include "Random/Random.h"


extern float g_stepSize;
extern float g_stiffnessDistance;
extern float g_stiffnessPull;
extern int g_nNodes;
extern float g_dampVel;
extern float g_bendToleranceMin;
extern float g_bendToleranceMax;
extern float g_speed;
extern float g_radiusNode;
extern float g_radiusObstacle;

//////////////////////////////////////////////////////////////////////////
//CLevelGesture_TongueKnot_Move

class CLevelGesture_TongueKnot_Move:public CLevelGesture_BuildIn
{
public:
	IMPLEMENT_REFCOUNT_C;
	DEFINE_CLASS(CLevelGesture_TongueKnot_Move);

	CLevelGesture_TongueKnot_Move()
	{
		Zero();
	}

	void Zero()
	{
		_owner=NULL;
	}

	void Create(Buff_TongueFly *owner)
	{
		_owner=owner;
	}

	virtual void Destroy()	{		Zero();	Release();}
	virtual void Update(CUnit3D *unit,float dt) { return;}
	virtual void Update(CUnit *unit,float dt)
	{
		if (_owner)
		{
			LevelPos pos;
			LevelFace face;
			if (_owner->GetKnotLoc(pos,face))
			{
				unit->_pos=pos;
				unit->_face=face;
			}
		}
	}
	virtual BOOL IsFinished()	{		return _owner==NULL;	}

	void Finish()	{		_owner=NULL;	}

protected:

	Buff_TongueFly *_owner;

};


//////////////////////////////////////////////////////////////////////////
//Buff_TongueFly::Height
const float Buff_TongueFly::Height::_min=0.1f;
const float Buff_TongueFly::Height::_max=7.0f;
const float Buff_TongueFly::Height::_spdMax=10.0f;
const float Buff_TongueFly::Height::_dampSpdUp=0.93f;
const float Buff_TongueFly::Height::_dampSpdDown=0.97f;
const float Buff_TongueFly::Height::_stiffness=6.0f;//根据距离产生加速度的系数

const float Buff_TongueFly::Height::_cycleUpdateTarget=5.0f;
const float Buff_TongueFly::Height::_cycleUpdateTargetVary=2.0f;
const float Buff_TongueFly::Height::_targetMin=1.0f;
const float Buff_TongueFly::Height::_targetMax=1.0f;
const float Buff_TongueFly::Height::_liftMax=4.5f;
const float Buff_TongueFly::Height::_distEnemyMax=7.0f;

const BOOL Buff_TongueFly::Height::_bEasyMode=FALSE;


void Buff_TongueFly::Height::Init(float v)
{
	_v=v;
	_spd=0.0f;
	_acc=0.0f;
	_target=-1.0f;

	_t=0.0f;
	_tUpdateTarget=0.0f;
}

void Buff_TongueFly::Height::Update(float distEnemy,float dt)
{
	_t+=dt;
	if (_t>_tUpdateTarget)
	{
		_tUpdateTarget=_t+CSysRandom::RandVary(_cycleUpdateTarget,_cycleUpdateTargetVary);

		_target=CSysRandom::RandRange(_targetMin,_targetMax);
	}

	if (_target>=0.0f)
	{
		float target=_target;
		float scaleLift=1.0f-i_math::clamp_f(distEnemy/_distEnemyMax,0.0f,1.0f);
		if (_bEasyMode)
			scaleLift=0.0f;
		target+=scaleLift*_liftMax;
		_acc=(target-_v)*_stiffness;
	}
	else
		_acc=0.0f;

	_spd+=_acc*dt;
	if (_spd>=0.0f)
		_spd*=_dampSpdUp;
	else
		_spd*=_dampSpdDown;

	if (_spd>_spdMax)
		_spd=_spdMax;

	_v+=_spd*dt;
	_v=i_math::clamp_f(_v,_min,_max);
}



//////////////////////////////////////////////////////////////////////////
//CBuff_TongueFly

Buff_TongueFly *FindTongueFlyBuff(CLevel *level)
{
	if (level)
	{
		CLoSnailP1 *loSnailP1=(CLoSnailP1 *)level->GetUniqueObj(LevelUniqueObj_SnailP1);
		if (loSnailP1)
		{
			LevelObjID idTongueUnit=loSnailP1->GetTongueUnit();
			CLevelObj *loTongue=LevelUtil_GetAliveLo(level,idTongueUnit);
			if (loTongue)
			{
				extern CLevelBuff *LevelUtil_FindBuff(CLevelObj *lo,CClass *clssBuff);
				return (Buff_TongueFly*)LevelUtil_FindBuff(loTongue,Class_Ptr2(Buff_TongueFly));
			}
		}

// 		DWORD c;
// 		CLevelObj **objs=level->GetActiveObjs(c);
// 
// 		for (int i=0;i<c;i++)
// 		{
// 			CLevelObj *lo=objs[i];
// 			if (lo->GetType()==LevelObjType_Unit)
// 			{
// 				CLevelBuffs *buffs=lo->GetBuffs();
// 				if (buffs)
// 				{
// 					Buff_TongueFly *buff=(Buff_TongueFly*)buffs->FindBuff(Class_Ptr2(Buff_TongueFly));
// 					if (buff)
// 						return buff;
// 				}
// 			}
// 		}
	}
	return NULL;
}


BIND_BUFFPARAM(Buff_TongueFly,BuffParam_TongueFly,BuffArg_TongueFly);

BuffFlag Buff_TongueFly::GetFlags()
{
	return 0;
}


LevelBuffMask Buff_TongueFly::GetReplaceBuffs()
{
	LevelBuffMask mask=0;
	mask|=((LevelBuffMask)1)<<(Class_Ptr2(Buff_TongueFly)->GetUID());

	return mask;
}

void Buff_TongueFly::_OnCreate(LevelBuffArg *arg0)
{
	BuffArg_TongueFly *arg=(BuffArg_TongueFly *)arg0;
	BuffParam_TongueFly*param=_rec->GetParam<BuffParam_TongueFly>();

	CLevelObj *owner=_GetOwner();
	assert(owner);

	if (TRUE)
	{
		CLoSnailP1 *loSnailP1=(CLoSnailP1 *)owner->GetLevel()->GetUniqueObj(LevelUniqueObj_SnailP1);
		if (loSnailP1)
		{
			loSnailP1->SetTongueUnit(owner->GetID());
		}
	}


	_sim.init();
	_model.init();
	_sim.setModel(&_model);

	PBD::TimeManager::getCurrent()->setTimeStepSize(static_cast<Real>(g_stepSize));

	_env.Init(&_model);

	_pos=owner->GetFramePos();
	_face=owner->GetFrameFace();

	if (TRUE)
	{
		_posRoot=_pos;
		_faceRoot=_face;

		LevelPos dirRoot;
		dirRoot=LevelFaceToDir(_face);
		_tongue.Init(_env,g_nNodes,g_radiusNode,i_math::vector3df(_posRoot.x,0.0f,_posRoot.y),i_math::vector3df(dirRoot.x,0.0f,dirRoot.y));
	}

	_posTarget=_pos;
	_velTarget.set(0.0f,0.0f);

	_posKnot=_posRoot;

	_tCur=_tUpdate;

	if (TRUE)
	{
		EoEnv *eoEnv=(EoEnv*)_GetLevel()->GetEoEnv();
		if (eoEnv)
			_hLichen=eoEnv->StartLichenTrail(owner->GetID(),5.0f);
	}
}

void Buff_TongueFly::_OnDestroy()
{
	_StopLichen();

	if (_gesKnot)
		_gesKnot->Finish();
	SAFE_RELEASE(_gesKnot);

	if (_idKnot!=LevelObjID_Invalid)
	{
		CLevelObj *lo=LevelUtil_GetAliveLo(_GetLevel(),_idKnot);
		if (lo)
			lo->DeferDestroy();
		_idKnot=LevelObjID_Invalid;
	}

	_tongue.Clear();
	_env.Clear();

	_sim.reset();
	_sim.setModel(NULL);
	_model.cleanup();

	if (TRUE)
	{
		CLoSnailP1 *loSnailP1=(CLoSnailP1 *)_GetLevel()->GetUniqueObj(LevelUniqueObj_SnailP1);
		if (loSnailP1)
		{
			loSnailP1->SetTongueUnit(LevelObjID_Invalid);
		}
	}

}

void Buff_TongueFly::_OnUpdate(AnimTick dt)
{
	if (!_bLiberated)
	{
		_OnUpdate_Finish();
		if (!_bFinished)
		{
			_OnUpdate_Core(_tUpdate);
			_OnUpdate_CreateKnot();
			_OnUpdate_Liberated();
		}
	}
}

void Buff_TongueFly::_OnUpdate_Liberated()
{
	if (_bLiberated)
		return;

	if (IsTongueBrokenForAWhile(ANIMTICK_FROM_SECOND(0.5f)))
	{
		_bLiberated=TRUE;
		_StopLichen();

	}
}


void Buff_TongueFly::_OnUpdate_CreateKnot()
{
	if (_idKnot!=LevelObjID_Invalid)
		return;

	if (_tAge<ANIMTICK_FROM_SECOND(2.0f))
		return;

	BuffParam_TongueFly*param=_rec->GetParam<BuffParam_TongueFly>();
	_idKnot=LevelUtil_CreateUnit(_GetLevel(),param->idKnot,_posKnot,_faceRoot,LevelPlayerID_Wild);

	if (_idKnot!=LevelObjID_Invalid)
	{
		CLevelObj *loUnit=LevelUtil_GetAliveLo(_GetLevel(),_idKnot);
		if (loUnit)
		{
			//启动一个Gesture
			CUnit *unit=loUnit->GetUnit();
			if (unit)
			{
				CLevelGesture_TongueKnot_Move *ges=Class_New2(CLevelGesture_TongueKnot_Move);
				ges->Create(this);
				unit->SetGesture(ges);

				SAFE_REPLACE(_gesKnot,ges);
			}

			//设置初始HP
			if (TRUE)
			{
				if (_GetLevel()->GetUniqueObj(LevelUniqueObj_SnailP1))
				{
					CLoSnailP1 *loSnailP1=(CLoSnailP1 *)_GetLevel()->GetUniqueObj(LevelUniqueObj_SnailP1);
					int hp=loSnailP1->GetTongueFlyKnotHP();
					if (hp>=0)
					{
						LevelAttr_Base *attrBase=loUnit->GetAttr_Base();
						if (attrBase)
						{
							attrBase->hp.SetCur_Int(hp);
						}
					}
				}
			}

		}

		_AddSyncDataOp();
	}

}

BOOL Buff_TongueFly::CheckKnotKilled()
{
	if (_idKnot!=LevelObjID_Invalid)
	{
		if (LevelUtil_CheckDead(_GetLevel(),_idKnot))
			return TRUE;
	}
	return FALSE;

}

void Buff_TongueFly::_OnUpdate_Core(AnimTick t)
{
	if (t<=_tCur)
		return;

	PBD::ParticleData &pd=_model.getParticles();

	AnimTick dt=t-_tCur;
	_tCur=t;

	float fDt=ANIMTICK_TO_SECOND(dt);

	CLevelObj *owner=_GetOwner();
	assert(owner);

	if (_bPassive)
	{
		LevelPos posTarget;
		posTarget=owner->GetFramePos();
		_velTarget=(posTarget-_posTarget)/fDt;
		_posTarget=posTarget;
	}
	else
	{
		LevelPos posTarget=_pos;
		if (_mode==WorkMode_Tracking)
		{
			CLevelPlayer *player=LevelUtil_GetFirstPlayer(_GetLevel());
			if (player)
			{
				CLoUnit *lo=player->GetLoUnit();
				if (lo)
					posTarget=lo->GetFramePos();
			}
		}
		if ((_mode==WorkMode_Withdraw)||(_mode==WorkMode_FastWithdraw))
		{
			posTarget=_posRoot;
		}

		const float speedTargetTracking=10.0f;
		const float blendTargetTracking=0.02f;
		LevelPos velTarget=posTarget-_posTarget;
		if (TRUE)
		{
			float dist=velTarget.getLength();
			if (dist>0.001f)
			{
				float speed=dist/fDt;
				if (speed>speedTargetTracking)
					speed=speedTargetTracking;

				velTarget.safe_normalize();
				velTarget*=speed;
			}
		}

		velTarget=velTarget.getInterpolated(_velTarget,blendTargetTracking);
		_velTarget=velTarget;

		posTarget=_posTarget+_velTarget*fDt;
		_posTarget=posTarget;
	}

	_tongue.SetTargetPos(i_math::vector3df(_posTarget.x,0.0f,_posTarget.y));

	if (TRUE)
	{
		float fDt=ANIMTICK_TO_SECOND(dt);
		DWORD nSteps=(DWORD)(fDt/PBD::TimeManager::getCurrent()->getTimeStepSize());

		if(_bPassive)
			nSteps*=2;//足够抵达目标的steps

		LevelPos posPrev;

		for (int i=0;i<nSteps;i++)
		{
			_tongue.Update(_model);
			_env.Update();
			_sim.getTimeStep()->step(_model);

			//得到端点的位置
			if (TRUE)
			{
				CTongueBranchPhys::Node &node=_tongue._nodes[_tongue._nodes.size()-1];
				_pos=pd.getPosition(node.particle).getXZ();
				CTongueBranchPhys::Node &nodePrev=_tongue._nodes[_tongue._nodes.size()-2];
				posPrev=pd.getPosition(nodePrev.particle).getXZ();
			}

			if (_pos.getDistanceSQFrom(_posTarget)<0.05f*0.05f)
				break;
		}

		if (posPrev.getDistanceSQFrom(_pos)>0.01f*0.01f)
			_face=LevelFaceFromDir(_pos-posPrev);
	}

	//更新高度
	if (TRUE)
	{
		float htCur;
		if (TRUE)
		{
			float distEnemy=1000.0f;
			if (TRUE)
			{
				CLevelPlayer *player=LevelUtil_GetFirstPlayer(_GetLevel());
				if (player)
				{
					CLoUnit *lo=player->GetLoUnit();
					if (lo)
					{
						LevelPos posEnemy;
						posEnemy=lo->GetFramePos();
						distEnemy=_pos.getDistanceFrom(posEnemy);
					}
				}
			}

			_htSkillPath.Update(distEnemy,fDt);
			htCur=_htSkillPath.GetCur();
		}

		float htFinal=htCur;

		if (TRUE)
		{
			float ratioBlend=1.0f;

			//随着舌头的长度的缩短,高度要渐渐趋于_htRoot
			if (TRUE)
			{
				ratioBlend=0.0f;
				float len=_tongue.GetFullLength();
				if (len>4.0f)
				{
					ratioBlend=(len-4.0f)/5.0f;
					ratioBlend=i_math::clamp_f(ratioBlend,0.0f,1.0f);
				}
				htFinal=i_math::lerp(_htRoot,htFinal,ratioBlend);
			}

			//刚开始的Blend
			if (TRUE)
			{
				float durBlend=1.0f;
				if (t<_tSkillPathStart+ANIMTICK_FROM_SECOND(durBlend))
				{
					ratioBlend=(((float)t)-((float)_tSkillPathStart))/(float)(ANIMTICK_FROM_SECOND(durBlend));
					ratioBlend=i_math::clamp_f(ratioBlend,0.0f,1.0f);
				}
				htFinal=i_math::lerp(_htSkillPathStart,htFinal,ratioBlend);
			}
		}

		_ht=htFinal;
	}


	//更新Knot
	if (TRUE)
	{
		const int idxKnot=_GetKnotNodeIndex();
		const float distKnotOut=2.0f;

		//更新Knot位置
		if (TRUE)
		{
			CTongueBranchPhys::Node &node=_tongue._nodes[idxKnot];
			_posKnot=pd.getPosition(node.particle).getXZ();
		}

		//记录Knot的HP
		if (_idKnot!=LevelObjID_Invalid)
		{
			CLevelObj *lo=LevelUtil_GetAliveLo(_GetLevel(),_idKnot);
			if (lo)
			{
				LevelAttr_Base *attrBase=lo->GetAttr_Base();
				if (attrBase)
				{
					int hp=attrBase->hp.GetCur_Int();
					if (_GetLevel()->GetUniqueObj(LevelUniqueObj_SnailP1))
					{
						CLoSnailP1 *loSnailP1=(CLoSnailP1 *)_GetLevel()->GetUniqueObj(LevelUniqueObj_SnailP1);
						loSnailP1->SetTongueFlyKnotHP(hp);
					}
				}
			}
		}

		//检测打断与否
		if (CheckKnotKilled())
		{
			if (_GetLevel()->GetUniqueObj(LevelUniqueObj_SnailP1))
			{
				CLoSnailP1 *loSnailP1=(CLoSnailP1 *)_GetLevel()->GetUniqueObj(LevelUniqueObj_SnailP1);
				loSnailP1->BreakTongue();
			}
		}

	}
}

void Buff_TongueFly::_OnUpdate_Finish()
{
	if (_bFinished)
		return;
	if (IsTongueBrokenForAWhile(0))
		return;
	if (_IsWithdrawing())
	{
		if (_tongue.GetFullLength()<=0.1f)
		{
			CLevelObj *loKnot=LevelUtil_GetAliveLo(_GetLevel(),_idKnot);
			if (loKnot)
				loKnot->DeferDestroy();

			if (_GetOwner())
				_GetOwner()->DeferDestroy();

			_StopLichen();
			_bFinished=TRUE;
		}
	}
}

DWORD Buff_TongueFly::GetTongueNodesPos(LevelPos *buf,DWORD szBuf)
{
	DWORD c=0;
	if (!_bLiberated)
	{
		int iStart=_tongue.GetLastPulledIn();

		c=_tongue._nodes.size()-iStart;
		if (c>szBuf)
			c=szBuf;

		PBD::ParticleData &pd=_model.getParticles();

		for (int i=iStart;i<iStart+c;i++)
		{
			CTongueBranchPhys::Node &node=_tongue._nodes[i];
			buf[i-iStart]=pd.getPosition(node.particle).getXZ();
		}
	}
	return c;
}

float Buff_TongueFly::GetTongueNodeRadius()
{
	return _tongue.GetNodeRadius();
}


void Buff_TongueFly::_WriteBigData(CDataPacket *dp)
{
	LevelPos buf[128];
	DWORD c=GetTongueNodesPos(buf,ARRAY_SIZE(buf));
	dp->Data_EncodeDword(c);
	if (c>0)
		dp->Data_WriteData(buf,c*sizeof(buf[0]));
}



void Buff_TongueFly::_WriteData(CBitPacket *bp)
{
	bp->Data_WriteSimpleR(_posRoot);
	bp->Data_WriteSimpleR(_faceRoot);

	LevelObjID idSnail=LevelObjID_Invalid;
	if (_GetLevel()->GetUniqueObj(LevelUniqueObj_SnailP1))
	{
		CLoSnailP1 *loSnailP1=(CLoSnailP1 *)_GetLevel()->GetUniqueObj(LevelUniqueObj_SnailP1);
		if (loSnailP1)
		{
			if (loSnailP1->GetSnailUnit())
				idSnail=loSnailP1->GetSnailUnit()->GetID();
		}
	}
	bp->Data_WriteSimple(idSnail);
	bp->Data_WriteSimple(_idKnot);
	bp->Bits_Write(_GetKnotNodeIndex(),7);
}

void Buff_TongueFly::StartSkillPath(LevelPos &pos,float ht)
{
	_bPassive=FALSE;

	if (_htRoot<0.0f)
		_htRoot=ht;

	_htSkillPathStart=ht;
	_tSkillPathStart=_GetOwner()->GetT();
	_modeSkillPathStart=_mode;
	_htSkillPath.Init(ht);
}

void Buff_TongueFly::StopSkillPath()
{
	_bPassive=TRUE;
	_htSkillPath.Zero();
}


BOOL Buff_TongueFly::CalcSkillPathXfm(AnimTick t,LevelPos &pos,float &ht,LevelFace &face)
{
//	t+=ANIMTICK_FROM_SECOND(0.1f);
	_OnUpdate_Core(t);
	pos=_pos;
	face=_face;
	ht=_ht;

	return TRUE;
}

void Buff_TongueFly::_SwitchMoveMethod(BOOL bFlying)
{
	extern CLevelSkill *LevelUtil_GetCastingSkill(CLevelObj *lo);
	if (CLevelSkill *skill=LevelUtil_GetCastingSkill(_GetOwner()))
	{
		if (skill->GetClass()->IsSameWith(Class_Ptr2(Skill_GeneralAdvS)))
		{
			Skill_GeneralAdvS *skillGeneral=(Skill_GeneralAdvS *)skill;
			SkillParam_GeneralAdvS::OpEntry entryOp;
			if (bFlying)
				entryOp.op=SkillParam_GeneralAdvS::OpEntry::Op_TakeOff;
			else
				entryOp.op=SkillParam_GeneralAdvS::OpEntry::Op_Landing;

			skillGeneral->OnOp(entryOp,NULL);
		}
	}

	_bFlying=bFlying;
}


BOOL Buff_TongueFly::IsNearlyWithdrawn()
{
	if (_IsWithdrawing())
	{
		if(_tongue.GetFullLength()<0.5f)
			return TRUE;
	}
	return FALSE;
}

BOOL Buff_TongueFly::IsTongueBrokenForAWhile(AnimTick dur)
{
	if (_GetLevel()->GetUniqueObj(LevelUniqueObj_SnailP1))
	{
		CLoSnailP1 *loSnailP1=(CLoSnailP1 *)_GetLevel()->GetUniqueObj(LevelUniqueObj_SnailP1);
		return loSnailP1->IsTongueBrokenForAWhile(dur);
	}
	return FALSE;
}

void Buff_TongueFly::_StopLichen()
{
	if (_hLichen==EoEnvLichenHandle_Invalid)
		return;

	if (TRUE)
	{
		EoEnv *eoEnv=(EoEnv*)_GetLevel()->GetEoEnv();
		if (eoEnv)
			eoEnv->StopLichen(_hLichen);
		_hLichen=EoEnvLichenHandle_Invalid;
	}
}

void Buff_TongueFly::HandleEvent(LevelEvent &e0)
{
	if (e0.GetType()==LET_PreDamage)
	{
		LePreDamage *e=(LePreDamage*)&e0;

		if (!IsTongueBrokenForAWhile(ANIMTICK_FROM_SECOND(0.5f)))
		{
			if (e->loTarget==_GetOwner())
			{
				if (e->result)
				{
					if (e->result->GetHPTotal()>0)
					{
						e->overrideDmg=1.0f;
						if (!e->bStun)
							e->bAbandon=TRUE;
					}
				}
				e->bHandled=TRUE;
				return;
			}
		}
	}

	__super::HandleEvent(e0);

}
