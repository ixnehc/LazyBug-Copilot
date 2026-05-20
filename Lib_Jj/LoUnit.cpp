
#include "stdh.h"
#include "Level.h"

#include "LoUnit.h"

#include "LevelRecordUnit.h"
#include "LevelRecordItem.h"
#include "LevelRecordItemClass.h"
#include "LevelRecordPosture.h"
#include "LevelRecords.h"

#include "LevelPlayerStates.h"

#include "LevelSkill.h"
#include "LevelBehavior.h"

#include "LevelTalks.h"
#include "LevelCoSkill.h"
#include "LevelCounter.h"
#include "LevelSensor.h"
#include "LevelBlocking.h"
#include "LevelUnitArg.h"

#include "LevelNPCs.h"

#include "Random/Random.h"
#include "timer/profiler.h"

#include "behaviorgraph/BehaviorMem.h"



//////////////////////////////////////////////////////////////////////////
//CLoUnit
void CLoUnit::_InitAttrs(LevelPlayerStates *lps,LevelGrade grd)
{
	_attrBase.Init(_rec,lps,grd);
	_attrResists=_rec->GetResists();
	_attrEvade=_rec->GetEvade();	
	_attrWeaks.CopyFrom(*_rec->GetWeaks());
	_attrDrop.Init(_rec);
	if (lps)
	{
		_attrDefendMods=Class_New2(LevelAttr_DefendMods);
		_attrAttackMods=Class_New2(LevelAttr_AttackMods);
		_attrSpeedMod=Class_New2(LevelAttr_SpeedMod);

		_attrResource=Class_New2(LevelAttr_Resource);
		_attrResource->Init(lps);

		_attrTemple=Class_New2(LevelAttr_Temple);
		_attrTemple->Init(lps);

		_attrMagicBoard=Class_New2(LevelAttr_MagicBoard);
		_attrMagicBoard->Init(lps);

	}
}

void CLoUnit::_PostCreate(LevelPlayerID idPlayer,LevelPlayerStates *lps,RecordID tid,LevelGrade grd,LevelUnitArg *arg,EquipSetPick iPickedEquipSet)
{
	if (lps)
		grd=lps->base.grd;

	CLevelRecords *records=_level->GetRecords();
	_rec=records->GetUnit(tid);
	assert(_rec);

	_tUpdate=_level->GetT_();
	_bPlayer=lps?1:0;
	_lps=lps;
	SetPlayerID(idPlayer);
	_tidUnit=tid;
	_arg=arg;

	_skilldriver.Init(this);
	_cds.Init(this);

	_pauser.Init(this);

	_buffs.Init(this,_level->GetBuffIDPool());

	//Attrs
	_InitAttrs(lps,grd);

	//ExpressedEquips
	if (lps)
	{
		_equipsExpr=Class_New2(ExprEquips);
		extern BOOL LevelUtil_UpdateExprEquips(ExprEquips *equips,LevelPlayerStates *lps,CLevelRecords *records);
		LevelUtil_UpdateExprEquips(_equipsExpr,lps,_level->GetRecords());
	}
	else
	{
		if (_rec)
		{
			for (int i=0;i<_rec->Equips.size();i++)
			{
				LevelRecordItemClass *recItemClass=records->GetItemClassOfItem(_rec->Equips[i]);
				if (recItemClass)
				{
					if (((DWORD)recItemClass->part)<EquipPart_MaxExpress)
					{
						if (!_equipsExpr)
							_equipsExpr=Class_New2(ExprEquips);

						_equipsExpr->SetItem((EquipPart)recItemClass->part,_rec->Equips[i]);
					}
				}
			}

			if (_rec->EquipSets.sets.size()>0)
			{
				int iPick=iPickedEquipSet;

				if (iPick<0)
				{
					float wtTotal=0.0f;
					for (int i=0;i<_rec->EquipSets.sets.size();i++)
						wtTotal+=_rec->EquipSets.sets[i].weight;

					float v=CSysRandom::RandRange(0.0f,wtTotal);

					wtTotal=0.0f;
					for (int i=0;i<_rec->EquipSets.sets.size();i++)
					{
						if ((v>=wtTotal)&&(v<wtTotal+_rec->EquipSets.sets[i].weight))
						{
							iPick=i;
							break;
						}
						wtTotal+=_rec->EquipSets.sets[i].weight;
					}
					if (iPick==-1)
						iPick=_rec->EquipSets.sets.size()-1;
				}

				UnitEquipSet &set=_rec->EquipSets.sets[iPick];

				for (int i=0;i<set.equips.size();i++)
				{
					LevelRecordItemClass *recItemClass=records->GetItemClassOfItem(set.equips[i]);
					if (recItemClass)
					{
						if (((DWORD)recItemClass->part)<EquipPart_MaxExpress)
						{
							if (!_equipsExpr)
								_equipsExpr=Class_New2(ExprEquips);

							_equipsExpr->SetItem((EquipPart)recItemClass->part,set.equips[i]);
						}
					}
				}
				_iPickedEquipSet=iPick;
			}
		}
	}

	if (_rec)
	{
		if (_rec->bTalks)
		{
			_talks=Class_New2(CLevelTalks);
			_talks->Create(this);
		}

		if (_rec->sensor.bEnable)
		{
			_sensor=Class_New2(CLevelSensor);
			_sensor->Create(this,(LevelSensorParam*)&_rec->sensor);
		}
	}

	if (_bPlayer)
	{
		_blocking=Class_New2(CLevelBlocking);
		_blocking->Init(this);
	}


	_UpdateAttrs(TRUE,LevelOpLink());

	UpdateAI(FALSE);//刚创建时不要Run

}

void CLoUnit::_CreateMove()
{
	LevelObjMoveSetting setting;
	setting.radius=_rec->Radius;
	setting.speed=_rec->Speed;
	setting.layorCollide=_rec->layorCollide;
	setting.bAdvWalkable=_rec->bAdvWalkable;
	if (_rec->fly.bEnable)
		setting.fly=&_rec->fly;
	if (_rec->pace.bEnable)
		setting.pace=&_rec->pace;
	_move.Create(this,setting);
}


void CLoUnit::PostCreate(LevelPlayerID idPlayer,LevelPlayerStates *lps,RecordID tid,LevelGrade grd,LevelUnitArg *arg,EquipSetPick iPickedEquipSet,LevelPos&pos)
{
	_PostCreate(idPlayer,lps,tid,grd,arg,iPickedEquipSet);

	//Move
	assert(_rec);
	_CreateMove();
	extern float LevelUtil_GenRandomFace();
	float rad=LevelUtil_GenRandomFace();//随机角度
	_move.SwitchGround(pos,rad,LevelTeleportID_Invalid);
}

void CLoUnit::PostCreate(LevelPlayerID idPlayer,LevelPlayerStates *lps,RecordID tid,LevelGrade grd,LevelUnitArg *arg,EquipSetPick iPickedEquipSet,LevelPos&pos,float face)
{
	_PostCreate(idPlayer,lps,tid,grd,arg,iPickedEquipSet);

	//Move
	assert(_rec);
	_CreateMove();
	_move.SwitchGround(pos,face,LevelTeleportID_Invalid);
}

void CLoUnit::PostCreate(LevelPlayerID idPlayer,LevelPlayerStates *lps,RecordID tid,LevelGrade grd,LevelUnitArg *arg,EquipSetPick iPickedEquipSet,LevelPos3D&pos3D)
{
	_PostCreate(idPlayer,lps,tid,grd,arg,iPickedEquipSet);

	//Move
	assert(_rec);
	_CreateMove();
	extern float LevelUtil_GenRandomFace();
	float rad=LevelUtil_GenRandomFace();//随机角度
	_move.SwitchFlying(pos3D,rad,LevelTeleportID_Invalid);
}


void CLoUnit::PostCreate(LevelPlayerID idPlayer,LevelPlayerStates *lps,RecordID tid,LevelGrade grd,LevelUnitArg *arg,EquipSetPick iPickedEquipSet,LevelPos3D&pos3D,LevelFace face)
{
	_PostCreate(idPlayer,lps,tid,grd,arg,iPickedEquipSet);

	//Move
	assert(_rec);
	_CreateMove();
	_move.SwitchFlying(pos3D,face,LevelTeleportID_Invalid);
}

void CLoUnit::PostCreate_Floating(LevelPlayerID idPlayer,LevelPlayerStates *lps,RecordID tid,LevelGrade grd,LevelUnitArg *arg,EquipSetPick iPickedEquipSet,LevelPos&pos,float htFloating)
{
	_PostCreate(idPlayer,lps,tid,grd,arg,iPickedEquipSet);

	//Move
	assert(_rec);
	_CreateMove();
	extern float LevelUtil_GenRandomFace();
	float rad=LevelUtil_GenRandomFace();//随机角度
	_move.SwitchFloating(pos,rad,htFloating,LevelTeleportID_Invalid);
}

void CLoUnit::OnDestroy()
{
	if (_bhvAI)
	{
		if (_bhvAI)
			_bhvAI->Clear();
		Safe_Class_Delete(_bhvAI);
	}

	if (_talks)
	{
		_talks->Destroy();
		Safe_Class_Delete(_talks);
	}

	if (_coskill)
	{
		Safe_Class_Delete(_coskill);
	}

	if (_sensor)
	{
		_sensor->Destroy();
		Safe_Class_Delete(_sensor);
	}

	_skilldriver.Clear();
	_buffs.Clear();

	_pauser.Clear();

	_move.Destroy();

	Safe_Class_Delete(_equipsExpr);
	Safe_Class_Delete(_attrDefendMods);
	Safe_Class_Delete(_attrAttackMods);
	Safe_Class_Delete(_attrSpeedMod);
	Safe_Class_Delete(_attrResource);
	Safe_Class_Delete(_attrTemple);
	Safe_Class_Delete(_attrMagicBoard);

	_cds.Clear();

	_ops.Clear();

	Safe_Class_Delete(_counter);

	SAFE_RELEASE(_ctxAI);

	if (_blocking)
	{
		_blocking->Clear();
		Safe_Class_Delete(_blocking);
	}

	Zero();
}

extern float LevelUtil_GenRandomFace();
void CLoUnit::PostCreate_Teleport(CLoUnit *loUnitOrg,LevelPos &pos)
{
	//将loUnitOrg里面的内容有选择的转移到自己这个对象中来

	//CLevelObj的内容
	_bAlive=loUnitOrg->_bAlive;
	_bActive=0;
	_bDeferDestroy=0;
	_bEnum=0;
	_bAffect=0;
	_bPlayer=loUnitOrg->_bPlayer;
	_lps=loUnitOrg->_lps;
	_verLPS=loUnitOrg->_verLPS;
	_idPlayer=loUnitOrg->_idPlayer;
	_idOnlyVisible=loUnitOrg->_idOnlyVisible;

	//_id;
	//_level;

	_src=loUnitOrg->_src;
	_param=loUnitOrg->_param;

	_tile=NULL;

	_maskPlayer=0;

	//以下为CLoUnit的内容

	_tUpdate=_level->GetT_();


	_rec=loUnitOrg->_rec;;
	_tidUnit=loUnitOrg->_tidUnit;
	_arg=loUnitOrg->_arg;

	_iPickedEquipSet=loUnitOrg->_iPickedEquipSet;
	_equipsExpr=loUnitOrg->_equipsExpr;
	loUnitOrg->_equipsExpr=NULL;
	loUnitOrg->_iPickedEquipSet=-1;

	_attrBase=loUnitOrg->_attrBase;
	_attrResists=loUnitOrg->_attrResists;
	_attrEvade=loUnitOrg->_attrEvade;
	_attrWeaks.CopyFrom(loUnitOrg->_attrWeaks);
	if (loUnitOrg->_attrDefendMods)
	{
		_attrDefendMods=Class_New2(LevelAttr_DefendMods);
		_attrDefendMods->CopyFrom(*loUnitOrg->_attrDefendMods);
	}
	if (loUnitOrg->_attrAttackMods)
	{
		_attrAttackMods=Class_New2(LevelAttr_AttackMods);
		_attrAttackMods->CopyFrom(*loUnitOrg->_attrAttackMods);
	}
	if (loUnitOrg->_attrSpeedMod)
	{
		_attrSpeedMod=Class_New2(LevelAttr_SpeedMod);
		_attrSpeedMod->CopyFrom(*loUnitOrg->_attrSpeedMod);
	}
	_attrDrop=loUnitOrg->_attrDrop;
	if (loUnitOrg->_attrResource)
	{
		_attrResource=Class_New2(LevelAttr_Resource);
		_attrResource->CopyFrom(*loUnitOrg->_attrResource);
	}
	if (loUnitOrg->_attrTemple)
	{
		_attrTemple=Class_New2(LevelAttr_Temple);
		_attrTemple->CopyFrom(*loUnitOrg->_attrTemple);
	}

	if (loUnitOrg->_attrMagicBoard)
	{
		_attrMagicBoard=Class_New2(LevelAttr_MagicBoard);
		_attrMagicBoard->CopyFrom(*loUnitOrg->_attrMagicBoard);
	}

	_skilldriver.Init(this);
	_cds.Init(this);
	_pauser.Init(this);

	_buffs.Init_Teleport(this,_level->GetBuffIDPool(),loUnitOrg->GetBuffs());

	assert(_rec);
	_CreateMove();
	switch(loUnitOrg->_move.GetMethod())
	{
		case LevelMoveMethod_Ground:
		case LevelMoveMethod_Resided_:
		case LevelMoveMethod_Mount:
		{
			float rad=LevelUtil_GenRandomFace();//随机角度
			_move.SwitchGround(pos,rad,LevelTeleportID_Invalid);
			break;
		}
		case LevelMoveMethod_Flying:
		{
			extern LevelPos3D LevelUtil_GetGroundHeight(CLevel *lvl,float x,float y,BOOL bHiReso);
			LevelPos3D pos3D=LevelUtil_GetGroundHeight(_level,pos.x,pos.y,FALSE);
			pos3D.y+=_rec->fly.hang;
			float rad=LevelUtil_GenRandomFace();//随机角度
			_move.SwitchFlying(pos3D,rad,LevelTeleportID_Invalid);
			break;
		}
	}

	_talks=loUnitOrg->_talks;
	loUnitOrg->_talks=NULL;

	if (loUnitOrg->_sensor)
	{
		_sensor=Class_New2(CLevelSensor);
		_sensor->Create(this,(LevelSensorParam*)&_rec->sensor);
	}

 	_bDisableAI=loUnitOrg->_bDisableAI;
// 	_bhvAI=loUnitOrg->_bhvAI;
// 	if (_bhvAI)
// 	{
// 		LevelBehaviorContext *ctx=_bhvAI->GetContext();
// 		if (ctx)
// 		{
// 			ctx->level=_level;
// 			ctx->lo=this;
// 		}
// 	}
// 	loUnitOrg->_bhvAI=NULL;

	_UpdateAttrs(TRUE,LevelOpLink());
}

void CLoUnit::ResetAI()
{
	//把AI清掉,在Update时候会重新创建一个AI
	if (_bhvAI)
	{
		_bhvAI->Clear();
		Safe_Class_Delete(_bhvAI);
	}
}

void CLoUnit::UpdateAI(BOOL bRun)
{
	ProfilerStart_Recent(UpdateAI);

	BOOL bNeedAI=!_bDisableAI;
	if (TRUE)
	{
		extern BOOL LevelUtil_CheckNeedAI(CLevelObj *lo);//判断一个LevelObj是否需要AI(比如当这个LevelObj已经死亡了,就不需要AI了)
		if (!LevelUtil_CheckNeedAI(this))
			bNeedAI=FALSE;
	}
	if (_bhvAI)
	{
		if (bNeedAI)
		{
			if (bRun)
				_bhvAI->Update();
		}
		else
		{
			_bhvAI->Clear();
			Safe_Class_Delete(_bhvAI);
		}
	}
	else
	{//尚未初始化
		if (bNeedAI)
		{
			StringID bhvAI=StringID_Invalid;
			bhvAI=_rec?_rec->ais.nmBg:NULL;
			if (bhvAI!=StringID_Invalid)
			{
				LevelBehaviorContext ctx;
				ctx.lo=this;
				ctx.idPlayerLock=_idPlayer;
				_bhvAI=_level->CreateBehavior(bhvAI,ctx);
				if (_bhvAI)
				{
					if (bRun)
						_bhvAI->Start();
				}
			}
		}
	}
	ProfilerEnd();
}


void CLoUnit::Update()
{
	AnimTick dt,t;
	t=_level->GetT_();
	dt=t-_tUpdate;
	_tUpdate=t;

	_UpdateAttrs(FALSE,LevelOpLink());

	if (_sensor)
		_sensor->Update();

	UpdateAI(TRUE);

	_cycleUpdate++;

	_buffs.Update(t);
	_pauser.Update();


	_skilldriver.Update(dt);

	_move.Update();


	_cds.Update(dt);

	_level->GetObjMap()->UpdateLo(this);

}

LoMiscFlags*CLoUnit::GetMiscFlags()
{
	if (_rec)
		return &_rec->flagsMisc;
	return NULL;
}


LevelPos CLoUnit::GetFramePos()
{
	return _move.GetFramePos();
}

float CLoUnit::GetFrameFace()
{
	return _move.GetFrameFace();
}


LevelPos3D CLoUnit::GetFramePos3D()
{
	LevelPos3D ret;
	if (_move.GetFramePos3D(ret))
		return ret;

	return __super::GetFramePos3D();
}


CBehaviorMem *CLoUnit::_GetBhvMem()
{
	if (_bhvAI)
		return _bhvAI->GetMem(0);
	return NULL;
}

struct UnitArgProxy
{
	BEGIN_GOBJ_PURE(UnitArgProxy,1);

		GELEM_DYNOBJPTR_UNITPARAM(LevelUnitArg,arg,UnitArg_Null, "特定单位参数", "特定单位参数" );
			GELEM_DYNOBJPTR_CLASS_UNITPARAM_LIST();

	END_GOBJ();

	LevelUnitArg *arg;
};

void CLoUnit::WriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	//Player标志
	if (_bPlayer)
		bp->Bit_Write_1();
	else
		bp->Bit_Write_0();

	//OwnerID
	bp->Bits_Write(_idPlayer,5);

	//Ops
	if (_rec->Name=="变形怪Slime")
	{
		BOOL bStartSkill=FALSE;
		for (int i=0;i<_ops._ops.size();i++)
		{
			CLevelOp*op=_ops._ops[i];
			if (op->ToPtr<LevelOp_StartSkill>())
			{
				bStartSkill=TRUE;
				break;
			}
		}
		if (!bStartSkill)
		{
			int v=0;
			v++;
		}
	}
	_ops.WriteSync(bp,TRUE,bContent);

	//TypeID
	bp->Bits_Write((WORD)_tidUnit,11);

	//Arg
	if (_arg)
	{
		bp->Bit_Write_1();

		UnitArgProxy t;
		LevelUnitArg *argT=t.arg;
		t.arg=_arg;
		t.GSave(*bp->GetDP());
		t.arg=argT;
	}
	else
		bp->Bit_Write_0();

	//Attrs
	_attrBase.WriteFirstSync(bp,_bPlayer);
	if (_bPlayer)
	{
		assert(_attrResource);
		_attrResource->WriteFirstSync(bp);
		assert(_attrTemple);
		_attrTemple->WriteSync(bp);
		assert(_attrMagicBoard);
		_attrMagicBoard->WriteFirstSync(bp);
		assert(_attrSpeedMod);
		_attrSpeedMod->WriteFirstSync(bp);
	}

	//Move
	_move.WriteFirstSync(bp);

	//外观装备
	if (_equipsExpr)
	{
		bp->Bit_Write_1();
		_equipsExpr->Write(bp);
	}
	else
		bp->Bit_Write_0();


	//Buffs
	BOOL bBirthing=_buffs.TestFlag(BuffFlag_Birth);
	bp->Bit_Write(bBirthing);	//Birth 标志,这个标志告诉客户端这个单位正在出生过程中
	_buffs.WriteFirstSync(bp);

	if (_talks)
		_talks->WriteFirst(bp,idPlayer,bContent);

	if (_rec->ais.bSync)
	{//需要同步AI
		CBehaviorMem *mem=_GetBhvMem();

		if (mem)
		{
			bp->Bit_Write_1();//有内容
			mem->SaveSync(bp->GetDP());
		}
		else
			bp->Bit_Write_0();//没有内容
	}

	bContent=TRUE;
}


void CLoUnit::WriteSyncH(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	_move.WriteSyncH(bp,bContent);
}

void CLoUnit::WriteSyncL(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	_WriteSync_PlayerID(bp,bContent);
	_ops.WriteSync(bp,FALSE,bContent);
	if (_talks)
		_talks->Write(bp,idPlayer,bContent);

	if (_rec->ais.bSync)
	{
		CBehaviorMem *mem=_GetBhvMem();
		if (mem)
		{//现在有内容
			if ((mem->IsSyncDirty())||(!_bLastAI))
			{//内容有变化或者之前没有内容
				bContent=1;
				bp->Bit_Write_1();//有变化
				bp->Bit_Write_1();//有内容
				mem->SaveSync(bp->GetDP());
			}
			else
			{
				bp->Bit_Write_0();//没有变化
			}
		}
		else
		{//当前没有内容
			if (_bLastAI)
			{//之前有内容
				bContent=1;
				bp->Bit_Write_1();//有变化
				bp->Bit_Write_0();//没有内容
			}
			else
				bp->Bit_Write_0();//没有变化
		}
	}

	_buffs.WriteSyncL(bp,bContent);
}

void CLoUnit::PostWriteSync()
{
	_PostWriteSync_PlayerID();
	_move.PostWriteSync();
	_ops.PostWriteSync();
	if (_talks)
		_talks->PostWrite();

	if (_rec->ais.bSync)
	{
		CBehaviorMem *mem=_GetBhvMem();
		if (mem)
		{
			_bLastAI=1;
			mem->ClearSyncDirty();
		}
		else
			_bLastAI=0;
	}
	_buffs.PostWriteSync();
}



float CLoUnit::GetRadius_()
{
	if (_rec)
		return _rec->Radius;
	return 0.0f;
}

float CLoUnit::GetHeight()
{
	if (_rec)
		return _rec->Height;
	return 0.0f;
}

float CLoUnit::GetAimHeight()
{
	if (_rec)
		return _rec->AimHeight;
	return 0.0f;
}

float CLoUnit::GetCastHeight()
{
	if (_rec)
		return _rec->CastHeight;
	return 0.0f;
}

float CLoUnit::GetModelScale()
{
	if (_rec)
		return _rec->scaleModel;
	return 1.0f;
}

LevelRtnuRank CLoUnit::GetRtnuRank()
{
	if (_rec)
		return _rec->rankRtnu;
	return LevelRtnuRank_None;
}


BOOL CLoUnit::NeedPauseMove()
{
	return _buffs.TestFlag(BuffFlag_Pausing);
}

void CLoUnit::UpdateExprEquips(LevelPlayerStates *lps)
{
	if (!_equipsExpr)
		return;

	extern BOOL LevelUtil_UpdateExprEquips(ExprEquips *equips,LevelPlayerStates *lps,CLevelRecords *records);
	BOOL bMod=LevelUtil_UpdateExprEquips(_equipsExpr,lps,_level->GetRecords());
 
	if (bMod)
	{
		LevelOp_ExprEquip *op=NewOp<LevelOp_ExprEquip>(LevelOpLink());
		op->equips=*_equipsExpr;
		_ops.AddOp(op);
	}
}

BOOL CLoUnit::AddExprEquips(RecordID idItem)
{
	CLevelRecords *records=_level->GetRecords();
	LevelRecordItemClass *recClss=records->GetItemClassOfItem(idItem);
	if (!recClss)
		return FALSE;
	if (recClss->part>=EquipPart_MaxExpress)
		return FALSE;
	if (!_equipsExpr)
		_equipsExpr=Class_New2(ExprEquips);

	if (_equipsExpr->items[recClss->part]==idItem)
		return FALSE;

	_equipsExpr->SetItem((EquipPart)recClss->part,idItem);

	LevelOp_ExprEquip *op=NewOp<LevelOp_ExprEquip>(LevelOpLink());
	op->equips=*_equipsExpr;
	_ops.AddOp(op);

	return TRUE;
}

BOOL CLoUnit::RemoveExprEquips(RecordID idItem)
{
	CLevelRecords *records=_level->GetRecords();
	LevelRecordItemClass *recClss=records->GetItemClassOfItem(idItem);
	if (!recClss)
		return FALSE;
	if (recClss->part>=EquipPart_MaxExpress)
		return FALSE;
	if (!_equipsExpr)
		return FALSE;

	if (_equipsExpr->items[recClss->part]!=idItem)
		return FALSE;

	_equipsExpr->SetItem((EquipPart)recClss->part,RecordID_Invalid);

	LevelOp_ExprEquip *op=NewOp<LevelOp_ExprEquip>(LevelOpLink());
	op->equips=*_equipsExpr;
	_ops.AddOp(op);

	return TRUE;
}



DWORD CLoUnit::GetBuffID_Dead()
{
	if (_rec)
	{
		if (_rec->buffs.dead!=RecordID_Invalid)
			return _rec->buffs.dead;
		return _rec->buffDead;
	}
	return RecordID_Invalid;
}

DWORD CLoUnit::GetBuffID_Stun()
{
	if (_rec)
		return _rec->buffs.stun;
	return RecordID_Invalid;
}

DWORD CLoUnit::GetBuffID_KB()
{
	if (_rec)
		return _rec->buffs.kb;
	return RecordID_Invalid;
}


DWORD CLoUnit::GetBuffID_Bleed()
{
	if (_rec)
		return _rec->buffs.bleed;
	return RecordID_Invalid;
}

DWORD CLoUnit::GetBuffID_Ash()
{
	if (_rec)
		return _rec->buffs.ash;
	return RecordID_Invalid;
}

DWORD CLoUnit::GetBuffHandler_Jink()
{
	if (_rec)
		return _rec->buffs.jink;
	return StringID_Invalid;
}

DWORD CLoUnit::GetBuffHandler_SkillStun()
{
	if (_rec)
		return _rec->buffs.skillstun;
	return StringID_Invalid;
}


CLevelCoSkill *CLoUnit::ObtainCoSkill()
{
	if (_coskill)
		return _coskill;
	_coskill=Class_New2(CLevelCoSkill);
	_coskill->Init(_rec);
	return _coskill;
}

void CLoUnit::_UpdateAttrs(BOOL bInit,LevelOpLink &link)
{
	extern CLevelPlayer *LevelUtil_PlayerFromLo(CLevelObj *lo);
	CLevelPlayer *player=LevelUtil_PlayerFromLo(this);
	if (!player)
		return;

	if (!_lps)
		return;

	CLevelAbilities &abilities=player->GetAbilities();

	CLevelRecords *records=_level->GetRecords();

	_verLPS=_lps->GetVer();

	LevelGrade grd=_lps->base.grd;

	int hpMax=_lps->base.MaxHP;
	int spFull=_lps->base.FullSP;

	DWORD hnr=_lps->base.hnr;
	BYTE vita=_lps->base.vita_;
	WORD worm=_lps->base.worm;
	WORD str=_lps->base.str;
	WORD magic=_lps->base.magic;
	BYTE hpRecover=0;

	short ims=0;
	short ias=0;

	BOOL bIsBow=FALSE;
	if (TRUE)
	{
		extern RecordID LevelUtil_GetEquippingWeapon(CLevelObj *lo,RecordID *id2ndWpn);
		RecordID idItem=LevelUtil_GetEquippingWeapon(this,NULL);
		if (idItem!=RecordID_Invalid)
		{
			LevelRecordPosture*rec=records->GetPostureOfItem(idItem);
			if (rec)
			{
				if (rec->tp==LevelPosture_Bow)
					bIsBow=TRUE;
			}
		}
	}

	if (_lps)
	{
		assert(_attrDefendMods);
		_attrDefendMods->Zero();
		_attrSpeedMod->Zero();
		_attrBase.spRecover=0;

		LevelItemState isAbilityOverriden;

		//装备的加成
		for (int i=0;i<ARRAY_SIZE(_lps->equip.parts)+_lps->artifacts.items.size();i++)
		{
			LevelItemState *is=NULL;
			if (i<ARRAY_SIZE(_lps->equip.parts))
				is=&_lps->equip.parts[i];
			else
				is=&_lps->artifacts.items[i-ARRAY_SIZE(_lps->equip.parts)];
			
			if (!is->IsValid())
				continue;

			LevelRecordItem *recItem=records->GetItem(is->tid);
			if (!recItem)
				continue;

			LevelRecordItemClass *recItemClass=records->GetItemClass(recItem->clss);
			if (!recItemClass)
				continue;

			if (recItem->tpAbility!=LevelAbilityType_None)
			{
				CLevelAbility *ability=abilities.GetActiveAbility(recItem->tpAbility);
				if (!ability)
					continue;

				isAbilityOverriden.CopyFrom(is);
				is=&isAbilityOverriden;

				ability->BuildArtifactState(isAbilityOverriden);
			}

			if (recItem->tpArtifact==LevelArtifact_StrenghPotion)
				str+=is->nStack;
			if (recItem->tpArtifact==LevelArtifact_MagicPowerPotion)
				_attrBase.magic+=is->nStack;

			int dmgItemLo=0;
			int dmgItemHi=0;
			int defItem=0;
			dmgItemLo+=recItem->DmgLo;
			dmgItemHi+=recItem->DmgHi;
			defItem+=recItem->Def;

			int dmgrateItem=0;
			int defrateItem=0;

			for (int j=0;j<is->nBuffs;j++)
			{
				ItemBuff *buff=&is->buffs[j];
				EItemBuffType tp=(EItemBuffType)buff->tp;

				switch(tp)
				{
					case ItemBuff_AddMaxHP://加最大HP
					{
						hpMax+=(short)buff->sh;
						break;
					}
					case ItemBuff_RecoverHP://HP恢复
					{
						_attrBase.hpRecover+=(short)buff->sh;
						break;
					}
					case ItemBuff_AddFullSP://加最大SP
					{
						spFull+=(short)buff->sh;
						break;
					}
					case ItemBuff_RecoverSP://SP恢复
					{
						_attrBase.spRecover+=(short)buff->sh;
						break;
					}
					case ItemBuff_FireResist://火抗
					{
						_attrDefendMods->modsResist[DamageAttrType_Fire].defAdd+=(short)buff->sh;
						break;
					}
					case ItemBuff_ElecResist://电抗
					{
						_attrDefendMods->modsResist[DamageAttrType_Lightning].defAdd+=(short)buff->sh;
						break;
					}
					case ItemBuff_ColdResist://冰抗
					{
						_attrDefendMods->modsResist[DamageAttrType_Cold].defAdd+=(short)buff->sh;
						break;
					}
					case ItemBuff_PoisonResist://毒抗
					{
						_attrDefendMods->modsResist[DamageAttrType_Poison].defAdd+=(short)buff->sh;
						break;
					}
					case ItemBuff_MoveSpeed://移动速度
					{
						ims+=(short)buff->sh;
						break;
					}
					case ItemBuff_AttackSpeed://攻击速度
					{
						ias+=(short)buff->sh;
						break;
					}
					case ItemBuff_PhysDmg://物理伤害
					{
						_attrAttackMods->modsDamage[DamageAttrType_Pierce].atkAdd+=buff->sh;
						_attrAttackMods->modsDamage[DamageAttrType_Crush].atkAdd+=buff->sh;
						break;
					}
					case ItemBuff_PhysDmg_Bow://物理伤害
					{
						if (bIsBow)
						{
							_attrAttackMods->modsDamage[DamageAttrType_Pierce].atkAdd+=buff->sh;
							_attrAttackMods->modsDamage[DamageAttrType_Crush].atkAdd+=buff->sh;
						}
						break;
					}
					case ItemBuff_PhysDmgRate://物理伤害百分比
					{
						_attrAttackMods->modsDamage[DamageAttrType_Pierce].atkRate+=((float)buff->sh)/100.0f;
						_attrAttackMods->modsDamage[DamageAttrType_Crush].atkRate+=((float)buff->sh)/100.0f;
						break;
					}
					case ItemBuff_PhysDmgRate_Bow://弓物理伤害百分比
					{
						if (bIsBow)
						{
							_attrAttackMods->modsDamage[DamageAttrType_Pierce].atkRate+=((float)buff->sh)/100.0f;
							_attrAttackMods->modsDamage[DamageAttrType_Crush].atkRate+=((float)buff->sh)/100.0f;
						}
						break;
					}
					case ItemBuff_FireDmg://火伤
					{
						_attrAttackMods->modsDamage[DamageAttrType_Fire].atkAdd+=buff->sh;
						break;
					}
					case ItemBuff_ElecDmg://电伤
					{
						_attrAttackMods->modsDamage[DamageAttrType_Lightning].atkAdd+=buff->sh;
						break;
					}
					case ItemBuff_ColdDmg://冰伤
					{
						_attrAttackMods->modsDamage[DamageAttrType_Cold].atkAdd+=buff->sh;
						break;
					}
					case ItemBuff_PoisonDmg://毒伤
					{
						_attrAttackMods->modsDamage[DamageAttrType_Poison].atkAdd+=buff->sh;
						break;
					}
					case ItemBuff_ElementDmgRate://元素伤害百分比
					{
						_attrAttackMods->modsDamage[DamageAttrType_Fire].atkRate+=((float)buff->sh)/100.0f;
						_attrAttackMods->modsDamage[DamageAttrType_Lightning].atkRate+=((float)buff->sh)/100.0f;
						_attrAttackMods->modsDamage[DamageAttrType_Cold].atkRate+=((float)buff->sh)/100.0f;
						break;
					}
					case ItemBuff_PhysDef_Base://物理防御基础值
					{
						_attrDefendMods->modsResist[DamageAttrType_Pierce].defBase+=(short)buff->sh;
						_attrDefendMods->modsResist[DamageAttrType_Crush].defBase+=(short)buff->sh;
						break;
					}
					case ItemBuff_PhysDef://物理防御
					{
						_attrDefendMods->modsResist[DamageAttrType_Pierce].defAdd+=(short)buff->sh;
						_attrDefendMods->modsResist[DamageAttrType_Crush].defAdd+=(short)buff->sh;
						break;
					}
					case ItemBuff_PhysDefRate://物理防御百分比
					{
						_attrDefendMods->modsResist[DamageAttrType_Pierce].defRate+=((float)buff->sh)/100.0f;
						_attrDefendMods->modsResist[DamageAttrType_Crush].defRate+=((float)buff->sh)/100.0f;
						break;
					}
					case ItemBuff_AttackSpeed_Bow://攻击速度
					{
						if (bIsBow)
							ias+=(short)buff->sh;
						break;
					}
				//XXXXX: more ItemBuffType
				//XXXXX: More DamageAttrType
				}
			}

		}

// 		//属性的加成
// 		if (TRUE)
// 		{
// 			_attrBattle.atkLo=_attrBattle.atkLo+grd*str;
// 			_attrBattle.atkHi=_attrBattle.atkHi+grd*str;
// 			_attrBattle.accu=_attrBattle.accu+grd*dex;
// 			_attrBattle.evade=_attrBattle.evade+grd*dex;
// 			_attrBattle.rateMagic=_attrBattle.rateMagic+magic;
// 			hpMax=hpMax+str;
// 		}

	}

	if (TRUE)
	{
		LevelAttr_BaseMod mod;
		if (LeModBaseAttrs::Send(this,&mod))
		{
			hpMax+=FloatToNearestInt(mod.hpAdd);
			spFull+=FloatToNearestInt(mod.spAdd);
			hnr+=mod.hnrAdd;
			hpRecover+=mod.hpRecoverAdd;
		}

	}

	_attrBase.hpRecover=hpRecover;
	if (bInit)
	{
		_attrBase.grade=grd;
		_attrBase.hp.Reset(hpMax,hpMax);
		_attrBase.sp.Reset(spFull,spFull);
		_attrBase.spFull.Reset(spFull,spFull);
		_attrBase.hnr=hnr;
		_attrBase.str=str;
		_attrBase.magic=magic;
		_attrBase.vita_=vita;
		_attrBase.worm=worm;
	}
	else
	{
		//Commit变化,并将变化发给client
		if (((float)hpMax)!=_attrBase.hp.GetMax_Float())
		{
			float delta=((float)hpMax)-_attrBase.hp.GetMax_Float();
			LevelOp_HPMod *op=NewOp<LevelOp_HPMod>(link);
			_attrBase.hp.MakeMaxMod(delta,op->mod);
			AddOp(op);
		}

		if (((float)spFull)!=_attrBase.spFull.GetMax_Float())
		{
			float delta=((float)spFull)-_attrBase.spFull.GetMax_Float();
			if (TRUE)
			{
				LevelOp_FullSPMod *op=NewOp<LevelOp_FullSPMod>(link);
				_attrBase.spFull.MakeMaxMod(delta,op->mod);
				AddOp(op);
			}
			if (TRUE)
			{
				LevelOp_SPMod *op=NewOp<LevelOp_SPMod>(link);
				_attrBase.sp.MakeMaxMod(delta,op->mod);
				AddOp(op);
			}
		}

		if (grd!=_attrBase.grade)
		{
			_attrBase.grade=grd;
			LevelOp_GradeMod *op=NewOp<LevelOp_GradeMod>(link);
			op->grd=grd;
			AddOp(op);
		}

		if (hnr!=_attrBase.hnr)
		{
			_attrBase.hnr=hnr;
			LevelOp_HonorMod *op=NewOp<LevelOp_HonorMod>(link);
			op->hnr=hnr;
			AddOp(op);
		}

		if (str!=_attrBase.str)
		{
			_attrBase.str=str;
			LevelOp_StrengthMod *op=NewOp<LevelOp_StrengthMod>(link);
			op->str=str;
			AddOp(op);
		}

		if (magic!=_attrBase.magic)
		{
			_attrBase.magic=magic;
			LevelOp_MagicMod *op=NewOp<LevelOp_MagicMod>(link);
			op->magic=magic;
			AddOp(op);
		}

		if (vita!=_attrBase.vita_)
		{
			_attrBase.vita_=vita;
		}

		if (worm!=_attrBase.worm)
		{
			_attrBase.worm=worm;
		}

		if (_attrSpeedMod)
		{
			if ((ims!=_attrSpeedMod->ims)||(ias!=_attrSpeedMod->ias))
			{
				_attrSpeedMod->ims=ims;
				_attrSpeedMod->ias=ias;

				LevelOp_SpeedMod *op=NewOp<LevelOp_SpeedMod>(link);
				op->ims=ims;
				op->ias=ias;
				AddOp(op);
			}
		}
	}

}

CLevelCounter *CLoUnit::ObtainCounter()
{
	if (!_counter)
		_counter=Class_New2(CLevelCounter);
	return _counter;
}

void CLoUnit::_EnsureAIContext()
{
	if (_ctxAI)
		return;

	_ctxAI=Class_New2(LevelAIContext);
	_ctxAI->AddRef();
}

LevelAIContext *CLoUnit::ObtainAIContext()
{
	_EnsureAIContext();
	return _ctxAI;
}


void CLoUnit::SetAICmd(StringID idCmd)
{
	_EnsureAIContext();
	_ctxAI->idCmd=idCmd;
}


StringID CLoUnit::GetAICmd()
{
	if (_ctxAI)	
		return _ctxAI->idCmd;
	return StringID_Invalid;
}

DWORD CLoUnit::GetSimulateSpheres(i_math::spheref *sphs,DWORD nMaxSphs)
{
	//粗略版本,有待细化
	if (nMaxSphs<3)
		return 0;

	float ht=GetHeight();
	float r=GetRadius_();
	i_math::vector3df posBase=GetFramePos3D();
	if (ht>r)
	{
		i_math::vector3df pos=posBase;
		pos.y+=r;
		sphs[0].set(pos,r);

		pos=posBase;
		pos.y+=ht-r;
		sphs[1].set(pos,r);

		pos=posBase;
		pos.y+=ht/2.0f;
		sphs[2].set(pos,r);

		return 3;
	}
	else
	{
// 		LevelFace face=GetFrameFace();
// 		i_math::vector2df dir2D=LevelFaceToDir(face);
// 		i_math::vector3df dir;
// 		dir.setXZ(dir2D);


		i_math::vector3df pos=posBase;
		pos.y+=ht/2.0f;
		sphs[0].set(pos,ht/2.0f);

		return 1;
	}
}

void CLoUnit::EquipNpc(CLevelNPC *npc)
{
	if (!_bhvAI)
		UpdateAI(FALSE);
	if (!_bhvAI)
		return;

	LevelBehaviorContext *ctx=_bhvAI->GetContext();
	if (ctx)
	{
		ctx->npc=npc;
		ctx->idPlayerLock=npc->GetNPCs()->GetPlayerID();
	}

	CBehaviorMem *mem=_bhvAI->GetMem(0);
	if (mem)
	{
		LPSNpcData *dataNPC=npc->GetNpcData();
		if (dataNPC)
		{
			if (dataNPC->dataBhv.size()>0)
			{
				CDataPacket dp;
				dp.SetDataBufferPointer(&dataNPC->dataBhv[0]);

				mem->LoadPersist(&dp);
			}
		}
	}

}

void CLoUnit::OnPlayerIDChanged()
{
	if (_bhvAI)
	{
		if (_bhvAI->GetContext())
			_bhvAI->GetContext()->idPlayerLock=_idPlayer;
	}
}
