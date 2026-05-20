/********************************************************************
	created:	2012/11/21 
	author:		cxi
	
	purpose:	Level Behavior
*********************************************************************/
#include "stdh.h"
#include "LevelBehavior.h"
#include "LevelBasis.h"
#include "Level.h"
#include "LevelTalks.h"
#include "LevelObjMove.h"
#include "LevelAIContext.h"
#include "LevelSensor.h"

#include "LoGeneralAgent.h"


#include "LevelTroops.h"

#include "LevelBGs.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "behaviorgraph/BgnHelper.h"

#include "Random/Random.h"


////////////////////////////////////////////////////////////////////////
//CLevelBgn
LevelBehaviorContext *CLevelBgn::_GetCtx()
{
	if (_bhv)
		return ((CLevelBehavior*)_bhv)->GetContext();
	return NULL;
}

CLevelPlayer *CLevelBgn::_GetOwnerPlayer()
{
	CLevelObj *lo=_GetLo();
	if (lo)
	{
		LevelPlayerID idPlayer=lo->GetPlayerID();
		if (lo->GetLevel())
		{
			return lo->GetLevel()->GetPlayer(idPlayer);
		}
	}
	return NULL;
}


CLevelTalks *CLevelBgn::_GetTalks()
{
	LevelBehaviorContext *ctx=_GetCtx();
	if (ctx)
	{
		if (ctx->lo)
			return ctx->lo->GetTalks();
	}
	return NULL;
}


BOOL CLevelBgn::_GetLocPos(StringID nmLoc,LevelPos &pos)
{
	LevelBehaviorContext *ctx=_GetCtx();
	if (!ctx)
		return FALSE;
	CLevelBasis *basis=ctx->level->GetData()->GetBasis();
	LevelLoc *loc=basis->FindLoc(nmLoc);
	if (!loc)
		return FALSE;
	pos=loc->pos;
	return TRUE;
}

BOOL CLevelBgn::_GetLocXfm(StringID nmLoc,LevelPos &pos,LevelFace &face)
{
	LevelBehaviorContext *ctx=_GetCtx();
	if (!ctx)
		return FALSE;
	CLevelBasis *basis=ctx->level->GetData()->GetBasis();
	LevelLoc *loc=basis->FindLoc(nmLoc);
	if (!loc)
		return FALSE;
	pos=loc->pos;
	face=loc->face;
	return TRUE;
}


BOOL CLevelBgn::_GetRouteStartPos(StringID nmRoute,LevelPos &pos,float &face)
{
	LevelBehaviorContext *ctx=_GetCtx();

	if (!ctx)
		return FALSE;
	CLevelBasis *basis=ctx->level->GetData()->GetBasis();
	LevelRoute *route=basis->FindRoute(nmRoute);
	if (!route)
		return FALSE;
	if (route->nodes.size()<=0)
		return FALSE;
	pos=route->nodes[0];
	if (route->nodes.size()>1)
	{
		LevelPos dir=route->nodes[1]-route->nodes[0];
		face=atan2f(dir.y,dir.x);
	}
	else
	{
		extern float LevelUtil_GenRandomFace();
		face=LevelUtil_GenRandomFace();
	}

	return TRUE;
}

LevelRoute *CLevelBgn::_GetRoute(StringID nmRoute)
{
	LevelBehaviorContext *ctx=_GetCtx();

	if (!ctx)
		return NULL;
	CLevelBasis *basis=ctx->level->GetData()->GetBasis();
	return basis->FindRoute(nmRoute);
}


CLevelObj *CLevelBgn::_GetLo()
{
	LevelBehaviorContext *ctx=_GetCtx();
	if (ctx)
	{
		if (ctx->lo)
		{
			if (ctx->lo->IsAlive())
				return ctx->lo;
		}
	}
	return NULL;
}

CLevelPlayer *CLevelBgn::_GetLockPlayer()
{
	LevelBehaviorContext *ctx=_GetCtx();

	if (ctx)
		return ctx->level->GetPlayer(ctx->idPlayerLock);

	return NULL;
}

CLevelObj *CLevelBgn::_GetLockLo()
{
	CLevelPlayer *player=_GetLockPlayer();
	if (!player)
		return NULL;
	return (CLevelObj *)player->GetLoUnit();
}

CLevelPlayer *CLevelBgn::_GetTalkPlayer()
{
	LevelBehaviorContext *ctx=_GetCtx();

	if (ctx)
		return ctx->level->GetPlayer(_GetTalkPlayerID());

	return NULL;
}

LevelPlayerID CLevelBgn::_GetTalkPlayerID()
{
	CLevelTalks *talks=_GetTalks();
	if (talks)
	{
		if (talks->IsExclusiveMode())
			return talks->GetFirstActive();
	}
	LevelBehaviorContext *ctx=_GetCtx();
	if (ctx)
		return ctx->idPlayerTalk;

	return LevelPlayerID_Invalid;
}


CLevelObj *CLevelBgn::_GetTalkLo()
{
	CLevelPlayer *player=_GetTalkPlayer();
	if (!player)
		return NULL;
	return (CLevelObj *)player->GetLoUnit();
}



float CLevelBgn::_GetLockPlayerSpeed()
{
	float speed=0.0f;
	CLevelObj *loLock=_GetLockLo();
	if (loLock)
	{
		CLevelObjMove *moveLock=loLock->GetMove();
		if (moveLock)
			speed=moveLock->GetUnitSpeed();
	}
	return speed;
}

LevelAIContext *CLevelBgn::_GetAIContext()
{
	CLevelObj *lo=_GetLo();
	if (lo)
	{
		return lo->GetAIContext();
	}
	return NULL;
}





BOOL CLevelBgn::_IsTalkActive()
{
	LevelBehaviorContext *ctx=_GetCtx();
	if (!ctx)
		return FALSE;
	if (!ctx->lo)
		return FALSE;
	CLevelTalks *talks=ctx->lo->GetTalks();
	if (talks)
	{
		if (talks->IsAnyActive())
			return TRUE;
	}
	return FALSE;
}

CLevelSkillDriver *CLevelBgn::_GetSkillDriver()
{
	LevelBehaviorContext *ctx=_GetCtx();

	if (!ctx)
		return NULL;
	if (!ctx->lo)
		return NULL;
	return ctx->lo->GetSkillDriver();
}

CLevelBuffs*CLevelBgn::_GetBuffs()
{
	LevelBehaviorContext *ctx=_GetCtx();
	if (!ctx)
		return NULL;
	if (!ctx->lo)
		return NULL;
	return ctx->lo->GetBuffs();
}

CLevelObjSrc *CLevelBgn::_GetLos()
{
	CLevelObj *lo=_GetLo();
	if (lo)
		return lo->GetLos();
	return NULL;
}

CLevelObjParam *CLevelBgn::_GetLop()
{
	CLevelObj *lo=_GetLo();
	if (lo)
		return lo->GetLop();
	return NULL;
}

LevelSimpleMem *CLevelBgn::_GetSimpleMem()
{
	LevelBehaviorContext *ctx=_GetCtx();
	if (ctx)
		return ctx->memSimple;
	return NULL;
}


CLevelTroops*CLevelBgn::_ObtainTroops()
{
	CLevelObj *lo=_GetLo();
	if (lo)
		return lo->ObtainTroops();
	LevelBehaviorContext *ctx=_GetCtx();
	if (ctx)
	{
		if (ctx->aiLvl)
			return ctx->aiLvl->ObtainTroops();
	}
	return NULL;
}

CLevelTroop *CLevelBgn::_ObtainTroop(StringID nmTroop)
{
	CLevelTroops *troops=_ObtainTroops();
	if (troops)
		return troops->Obtain(nmTroop);
	return NULL;
}


CLevelTroops*CLevelBgn::_GetTroops()
{
	CLevelObj *lo=_GetLo();
	if (lo)
		return lo->GetTroops();

	LevelBehaviorContext *ctx=_GetCtx();
	if (ctx)
	{
		if (ctx->aiLvl)
			return ctx->aiLvl->GetTroops();
	}

	return NULL;
}

CLevelTroop *CLevelBgn::_GetTroop(StringID nmTroop)
{
	CLevelTroops *troops=_GetTroops();
	if (troops)
		return troops->Get(nmTroop);
	return NULL;
}

TroopCombatContext *CLevelBgn::_GetTcc()
{
	CLevelObj *lo=_GetLo();
	if (lo)
	{
		CLevelTroop *troop=lo->GetTroop();
		if (troop)
			return troop->GetCombatContext();
	}

	return NULL;
}

CLevelObj *CLevelBgn::_GetThreat()
{
	CLevelObj *lo=_GetLo();
	extern CLevelObj *LevelUtil_GetThreat(CLevelObj *lo);
	return LevelUtil_GetThreat(_GetLo());
}

CLevelObj *CLevelBgn::_GetLoFromVar(StringID nm)
{
	LevelObjID id=LevelObjID_Invalid;
	_GetID(nm,BehaviorMemType_ObjID,id);
	extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
	return LevelUtil_GetAliveLo(_GetLevel(),id);
}


CLevelObj *CLevelBgn::_GetLevelSkillTarget_Obj(BgnLevelSkillTarget &target)
{
	if (target.tp==BgnLevelSkillTarget::Threat)
		return _GetThreat();
	if (target.tp==BgnLevelSkillTarget::SkillTarget)
	{
		CLevelObj *loMe=_GetLo();
		if (loMe)
		{
			CLevelSkillDriver *driver=loMe->GetSkillDriver();
			if (driver)
			{
				CLevelSkill *skill=driver->GetSkill();
				if (skill)
				{
					extern CLevelObj *LevelUtil_GetTargetObj(CLevel *level,LevelSkillTarget &target);
					return LevelUtil_GetTargetObj(_GetLevel(),skill->GetTarget());
				}
			}
		}
		return NULL;
	}
	if (target.tp==BgnLevelSkillTarget::Me)
		return _GetLo();
	if (target.tp==BgnLevelSkillTarget::Custom)
		return _GetLoFromVar(target.nmCustom);

	return NULL;
}

BOOL CLevelBgn::_GetLevelSkillTarget_Pos(BgnLevelSkillTarget &target,LevelPos &posTarget)
{
	CLevelObj *lo=_GetLevelSkillTarget_Obj(target);
	if (lo)
	{
		posTarget=lo->GetFramePos();
		return TRUE;
	}
	if (target.tp==BgnLevelSkillTarget::SkillTarget)
	{
		CLevelObj *loMe=_GetLo();
		if (loMe)
		{
			CLevelSkillDriver *driver=loMe->GetSkillDriver();
			if (driver)
			{
				CLevelSkill *skill=driver->GetSkill();
				if (skill)
				{
					extern BOOL LevelUtil_CalcTargetPos(CLevel *level,LevelSkillTarget &target,LevelPos&pos);
					if (LevelUtil_CalcTargetPos(_GetLevel(),skill->GetTarget(),posTarget))
						return TRUE;
				}
			}
		}

	}

	if (target.tp==BgnLevelSkillTarget::CustomPos)
	{
		return _GetPos(target.nmCustomPos,posTarget);
	}
	return FALSE;
}



BOOL CLevelBgn::_SetBit(StringID nmVar,BOOL b)
{
	if (nmVar==StringID_Invalid)
		return FALSE;
	if (nmVar>LevelSimpleVarName_Max)
		return __super::_SetBit(nmVar,b);

	LevelSimpleMem *mem=_GetSimpleMem();
	if (mem)
	{
		LevelSimpleMem memOld;
		memOld.CopyContent(*mem);

		mem->SetValue(nmVar,b?1:0);

		if (!memOld.EqualContent(*mem))
		{
			mem->bSyncDirty=1;
			mem->bPersistDirty=1;
		}

		return TRUE;

	}
	return FALSE;
}

BOOL CLevelBgn::_GetBit(StringID nmVar,BOOL &b)
{
	if (nmVar==StringID_Invalid)
		return FALSE;
	if (nmVar>LevelSimpleVarName_Max)
		return __super::_GetBit(nmVar,b);

	LevelSimpleMem *mem=_GetSimpleMem();
	if (mem)
	{
		DWORD v;
		if (mem->GetValue(nmVar,v))
		{
			if (v!=0)
				b=1;
			else
				b=0;
			return TRUE;
		}
	}

	return FALSE;
}


BOOL CLevelBgn::_SetNumber(StringID nmVar,short n)
{
	if (nmVar==StringID_Invalid)
		return FALSE;
	if (nmVar>LevelSimpleVarName_Max)
		return __super::_SetNumber(nmVar,n);
	LevelSimpleMem *mem=_GetSimpleMem();
	if (mem)
	{
		LevelSimpleMem memOld;
		memOld.CopyContent(*mem);

		mem->SetValue(nmVar,(DWORD)(WORD)n);

		if (!memOld.EqualContent(*mem))
		{
			mem->bSyncDirty=1;
			mem->bPersistDirty=1;
		}
	}
	return FALSE;
}

BOOL CLevelBgn::_GetNumber(StringID nmVar,short &n)
{
	if (nmVar==StringID_Invalid)
		return FALSE;
	if (nmVar>LevelSimpleVarName_Max)
		return __super::_GetNumber(nmVar,n);
	LevelSimpleMem *mem=_GetSimpleMem();
	if (mem)
	{
		DWORD v;
		if (mem->GetValue(nmVar,v))
		{
			n=(short)(WORD)v;
			return TRUE;
		}
	}
	return FALSE;
}


BOOL CLevelBgn::_SetID(StringID nmVar,BehaviorMemType tp,DWORD id)
{
	if (nmVar==StringID_Invalid)
		return FALSE;
	if (nmVar>LevelSimpleVarName_Max)
		return __super::_SetID(nmVar,tp,id);

	//目前不支持Simple Var
	//XXXXX:more simple var

	return FALSE;
}

BOOL CLevelBgn::_GetID(StringID nmVar,BehaviorMemType tp,DWORD &id)
{
	if (nmVar==StringID_Invalid)
		return FALSE;
	if (nmVar>LevelSimpleVarName_Max)
		return __super::_GetID(nmVar,tp,id);

	//目前不支持Simple Var
	//XXXXX:more simple var
	return FALSE;
}

void CLevelBgn::_FindParamValue(StringID nm,BhvVal*&value)
{
	value=NULL;
	if (_thrd.call)
	{
		std::unordered_map<StringID,BhvVal*>::iterator it;
		it=_thrd.call->_paramsRT.find(nm);
		if (it!=_thrd.call->_paramsRT.end())
			value=(*it).second;
	}
}


void CLevelBgn::_FindConstValue(StringID nm,BhvVal*&value,BhvValDeclare *&declare)
{
	value=NULL;
	declare=NULL;

	CLevelObjParam *lop=_GetLop();
	if (lop)
	{
		if (IsClass2(lop,LopGeneralAgent))
		{
			value=((LopGeneralAgent*)lop)->valuesBhv.Find(nm);
		}
	}
	if (!value)
	{
		CLevelObjSrc *los=_GetLos();
		if (los)
		{
			if (IsClass2(los,LosGeneralAgent))
			{
				value=((LosGeneralAgent*)los)->valuesBhv.Find(nm);
			}
		}
	}
	if (value)
		return;

	CBehaviorGraph *bg=_GetBg();
	CBehaviorGraphPads *pads=bg->GetPads();
	declare=pads->FindConstDeclare(nm);

}

void CLevelBgn::_FindValue(StringID nm,BhvVal*&value,BhvValDeclare*&declare)
{
	_FindParamValue(nm,value);
	if (value)
		return;
	_FindConstValue(nm,value,declare);
}

void CLevelBgn::_ResolvePad(CBehaviorGraphPad *pad)
{
	CDataPacket dp;

	for (int i=0;i<_lpad->refs.size();i++)
	{
		BGPad::RefInfo *info=&_lpad->refs[i];
		if (info->idRef==StringID_Invalid)
			continue;
		switch(info->tp)
		{
			case BGPad::RefInfo::Param:
			{
				BhvVal *val=NULL;
				_FindParamValue(info->idRef,val);
				if (val)
				{
					dp.SetDataBufferPointer(&val->data[0]);
					info->elem->Load(pad,dp,TRUE);
				}
				break;
			}
			case BGPad::RefInfo::Mem:
			{
				CBehaviorMem *mem=_GetMem();
				if (mem)
				{
					if (mem->FillBehaviorValue(info->idRef,info->value))
					{
						dp.SetDataBufferPointer(&info->value.data[0]);
						info->elem->Load(pad,dp,TRUE);
					}
				}
				break;
			}
			case BGPad::RefInfo::SimpleMem:
			{
				LevelSimpleMem *mem=_GetSimpleMem();
				if (mem)
				{
					if (mem->FillBehaviorValue(info->idRef,info->value))
					{
						dp.SetDataBufferPointer(&info->value.data[0]);
						info->elem->Load(pad,dp,TRUE);
					}
				}
				break;
			}
			case BGPad::RefInfo::Const:
			{
				BhvVal *val=NULL;
				BhvValDeclare *declare=NULL;
				_FindConstValue(info->idRef,val,declare);
				if (val||declare)
				{
					if (val)
					{
						dp.SetDataBufferPointer(&val->data[0]);
						info->elem->Load(pad,dp,TRUE);
					}
					else
					{
						dp.SetDataBufferPointer(&declare->dataDef[0]);
						info->elem->Load(pad,dp,TRUE);
					}
				}
				break;
			}
			case BGPad::RefInfo::None:
			{
				//尝试找Param
				if (TRUE)
				{
					BhvVal *val=NULL;
					_FindParamValue(info->idRef,val);
					if (val)
					{
						info->tp=BGPad::RefInfo::Param;
						if (val->tp.IsCompatible(info->elem))
						{
							dp.SetDataBufferPointer(&val->data[0]);
							info->elem->Load(pad,dp,TRUE);
						}
						else
						{
							//类型不一致,放弃这个ref
							info->Zero();
						}
						break;
					}
				}

				//尝试在Mem中找
				info->value.tp.From(info->elem);
				if (TRUE)
				{
					CBehaviorMem *mem=_GetMem();
					if (mem)
					{
						if (mem->FillBehaviorValue(info->idRef,info->value))
						{
							info->tp=BGPad::RefInfo::Mem;
							dp.SetDataBufferPointer(&info->value.data[0]);
							info->elem->Load(pad,dp,TRUE);
							break;
						}
					}
				}

				//尝试在SimpleMem中找
				if (TRUE)
				{
					LevelSimpleMem*mem=_GetSimpleMem();
					if (mem)
					{
						if (mem->FillBehaviorValue(info->idRef,info->value))
						{
							info->tp=BGPad::RefInfo::SimpleMem;
							dp.SetDataBufferPointer(&info->value.data[0]);
							info->elem->Load(pad,dp,TRUE);
							break;
						}
					}
				}

				//尝试找Const
				if (TRUE)
				{
					BhvVal *val=NULL;
					BhvValDeclare *declare=NULL;
					_FindConstValue(info->idRef,val,declare);
					if (val||declare)
					{
						info->tp=BGPad::RefInfo::Const;
						if (val)
						{
							if (val->tp.IsCompatible(info->elem))
							{
								dp.SetDataBufferPointer(&val->data[0]);
								info->elem->Load(pad,dp,TRUE);
							}
							else
							{
								//类型不一致,放弃这个ref
								info->Zero();
							}
						}
						else
						{
							if (declare->tp.IsCompatible(info->elem))
							{
								dp.SetDataBufferPointer(&declare->dataDef[0]);
								info->elem->Load(pad,dp,TRUE);
							}
							else
							{
								//类型不一致,放弃这个ref
								info->Zero();
							}
						}
						break;
					}
				}

				info->Zero();

				break;
			}

		}
	}
}



CBehaviorGraphPad *CLevelBgn::_GetPad()
{
	if (_padCache)
		return _padCache;
	if (!_lpad)
		return NULL;

	return _lpad->pad;
}


BOOL CLevelBgn::_GetBPR(BPR_Bool &bpr)
{
// 	if (!bpr.bRef)
// 		return bpr.v;
// 
// 	BOOL b;
// 	if (_GetBit(bpr.nmRef,b))
// 		return b;
// 
// 	BhvVal *value;
// 	BhvValDeclare *declare;
// 
// 	_FindValue(bpr.nmRef,value,declare);
// 
// 	if (value)
// 	{
// 		switch(value->tp.tpMem)
// 		{
// 			case BehaviorMemType_Bit:
// 			case BehaviorMemType_Integer:
// 			case BehaviorMemType_Float:
// 				return (value->value!=0);
// 		}
// 		assert(FALSE);
// 		return FALSE;//缺省值
// 	}
// 
// 	if (declare)
// 	{
// 		switch(declare->tp.tpMem)
// 		{
// 			case BehaviorMemType_Bit:
// 				return declare->b;
// 			case BehaviorMemType_Integer:
// 				return declare->n!=0;
// 			case BehaviorMemType_Float:
// 				return declare->f!=0.0f;
// 		}
// 		assert(FALSE);
// 		return FALSE;//缺省值
// 	}

	return FALSE;
}

int CLevelBgn::_GetBPR(BPR_Int &bpr)
{
// 	if (!bpr.bRef)
// 		return bpr.v;
// 
// 	short n;
// 	if (_GetNumber(bpr.nmRef,n))
// 		return n;
// 
// 	BehaviorValue *value;
// 	BehaviorValueDeclare *declare;
// 
// 	_FindValue(bpr.nmRef,value,declare);
// 
// 	if (value)
// 	{
// 		switch(value->tp)
// 		{
// 			case BhvValueType_Bit:
// 			case BhvValueType_Integer:
// 				return (int)value->value;
// 			case BhvValueType_Float:
// 				return (int)(*(float*)&value->value);
// 		}
// 		assert(FALSE);
// 		return FALSE;//缺省值
// 	}
// 
// 	if (declare)
// 	{
// 		switch(declare->tp)
// 		{
// 			case BhvValueType_Bit:
// 				return (int)declare->b;
// 			case BhvValueType_Integer:
// 				return (int)declare->n;
// 			case BhvValueType_Float:
// 				return (int)declare->f;
// 		}
// 		assert(FALSE);
// 		return FALSE;//缺省值
// 	}

	return FALSE;
}

float CLevelBgn::_GetBPR(BPR_Float &bpr)
{
// 	if (!bpr.bRef)
// 		return bpr.v;
// 
// 	float f;
// 	if (_GetFloat(bpr.nmRef,f))
// 		return f;
// 
// 
// 	BehaviorValue *value;
// 	BehaviorValueDeclare *declare;
// 
// 	_FindValue(bpr.nmRef,value,declare);
// 
// 	if (value)
// 	{
// 		switch(value->tp)
// 		{
// 			case BhvValueType_Bit:
// 			case BhvValueType_Integer:
// 				return (float)value->value;
// 			case BhvValueType_Float:
// 				return *(float*)&value->value;
// 		}
// 		assert(FALSE);
// 		return 0.0f;//缺省值
// 	}
// 
// 	if (declare)
// 	{
// 		switch(declare->tp)
// 		{
// 			case BhvValueType_Bit:
// 				return (float)declare->b;
// 			case BhvValueType_Integer:
// 				return (float)declare->n;
// 			case BhvValueType_Float:
// 				return declare->f;
// 		}
// 		assert(FALSE);
// 		return 0.0f;//缺省值
// 	}

	return 0.0f;//缺省值
}


RecordID CLevelBgn::_GetBPR(BPR_ItemID &bpr)
{
// 	if (!bpr.bRef)
// 		return bpr.v;
// 
// 	RecordID id;
// 	if (_GetID(bpr.nmRef,BhvValueType_ItemRecord,id))
// 		return id;
// 
// 	BehaviorValue *value;
// 	BehaviorValueDeclare *declare;
// 
// 	_FindValue(bpr.nmRef,value,declare);
// 
// 	if (value)
// 	{
// 		if(value->tp==BhvValueType_ItemRecord)
// 			return value->value;
// 		assert(FALSE);
// 		return RecordID_Invalid;//缺省值
// 	}
// 
// 	if (declare)
// 	{
// 		if(declare->tp==BhvValueType_ItemRecord)
// 			return declare->idItem;
// 		assert(FALSE);
// 		return RecordID_Invalid;//缺省值
// 	}
// 	return RecordID_Invalid;
// }
// 
// RecordID CLevelBgn::_GetBPR(BPR_UnitID &bpr)
// {
// 	if (!bpr.bRef)
// 		return bpr.v;
// 
// 	RecordID id;
// 	if (_GetID(bpr.nmRef,BhvValueType_UnitRecord,id))
// 		return id;
// 
// 	BehaviorValue *value;
// 	BehaviorValueDeclare *declare;
// 
// 	_FindValue(bpr.nmRef,value,declare);
// 
// 	if (value)
// 	{
// 		if(value->tp==BhvValueType_UnitRecord)
// 			return value->value;
// 		assert(FALSE);
// 		return RecordID_Invalid;//缺省值
// 	}
// 
// 	if (declare)
// 	{
// 		if(declare->tp==BhvValueType_UnitRecord)
// 			return declare->idItem;
// 		assert(FALSE);
// 		return RecordID_Invalid;//缺省值
// 	}
	return RecordID_Invalid;
}


RecordID CLevelBgn::_GetBPR(BPR_SkillID &bpr)
{
// 	if (!bpr.bRef)
// 		return bpr.v;
// 
// 	RecordID id;
// 	if (_GetID(bpr.nmRef,BhvValueType_SkillRecord,id))
// 		return id;
// 
// 	BehaviorValue *value;
// 	BehaviorValueDeclare *declare;
// 
// 	_FindValue(bpr.nmRef,value,declare);
// 
// 	if (value)
// 	{
// 		if(value->tp==BhvValueType_SkillRecord)
// 			return value->value;
// 		assert(FALSE);
// 		return RecordID_Invalid;//缺省值
// 	}
// 
// 	if (declare)
// 	{
// 		if(declare->tp==BhvValueType_SkillRecord)
// 			return declare->idSkill;
// 		assert(FALSE);
// 		return RecordID_Invalid;//缺省值
// 	}
	return RecordID_Invalid;
}

RecordID CLevelBgn::_GetBPR(BPR_BuffID &bpr)
{
// 	if (!bpr.bRef)
// 		return bpr.v;
// 
// 	RecordID id;
// 	if (_GetID(bpr.nmRef,BhvValueType_BuffRecord,id))
// 		return id;
// 
// 	BehaviorValue *value;
// 	BehaviorValueDeclare *declare;
// 
// 	_FindValue(bpr.nmRef,value,declare);
// 
// 	if (value)
// 	{
// 		if(value->tp==BhvValueType_BuffRecord)
// 			return value->value;
// 		assert(FALSE);
// 		return RecordID_Invalid;//缺省值
// 	}
// 
// 	if (declare)
// 	{
// 		if(declare->tp==BhvValueType_BuffRecord)
// 			return declare->idBuff;
// 		assert(FALSE);
// 		return RecordID_Invalid;//缺省值
// 	}
	return RecordID_Invalid;
}

StringID CLevelBgn::_GetBPR(BPR_StringID &bpr)
{
// 	if (!bpr.bRef)
// 		return bpr.v;
// 
// 	RecordID id;
// 	if (_GetID(bpr.nmRef,BhvValueType_StringID,id))
// 		return id;
// 
// 	BehaviorValue *value;
// 	BehaviorValueDeclare *declare;
// 
// 	_FindValue(bpr.nmRef,value,declare);
// 
// 	if (value)
// 	{
// 		if(value->tp==BhvValueType_StringID)
// 			return value->value;
// 		assert(FALSE);
// 		return StringID_Invalid;//缺省值
// 	}
// 
// 	if (declare)
// 	{
// 		if(declare->tp==BhvValueType_StringID)
// 			return declare->idBuff;
// 		assert(FALSE);
// 		return StringID_Invalid;//缺省值
// 	}
	return StringID_Invalid;
}



////////////////////////////////////////////////////////////////////////
//CLevelBehavior

void CLevelBehavior::Init(LevelBehaviorContext &ctx)
{
	_ctx=ctx;
	_ctx.behavior=this;

	__super::Init(_ctx.bg,ctx.lo?ctx.lo->GetID():0);

}

void CLevelBehavior::Clear()
{
	__super::Clear();

	Zero();
}


void CLevelBehavior::Start()
{
	_nEvents=0;
	__super::Start();
	_bStarted=1;
}


void CLevelBehavior::Update()
{
	if (!_bStarted)
	{
		Start();
		return;
	}
	_nEvents=0;
	__super::Update();

}

void CLevelBehavior::AddEvent(BYTE e)
{
	if (_nEvents>=ARRAY_SIZE(_events))
		return;
	BehaviorEvent be;
	be.tp=e;
	be.idPlayerLock=_ctx.idPlayerLock;
	_events[_nEvents]=be;
	_nEvents++;
}

BehaviorEvent*CLevelBehavior::FetchEvents(DWORD &c)
{
	c=_nEvents;
	_nEvents=0;
	return _events;
}


AnimTick CLevelBehavior::_GetT()
{
	return _ctx.level->GetT_();
}


