
#include "stdh.h"

#include "LevelData.h"
#include "LevelObjSrc.h"
#include "Level.h"

#include "LevelExploreMap.h"

#include "LoUnit.h"
#include "LoItem.h"

#include "Protocal.h"

#include "LevelRecords.h"

#include "LevelRecordItem.h"
#include "LevelRecordItemClass.h"
#include "LevelRecordUnit.h"
#include "LevelRecordGlobal.h"

#include "LevelRtnus.h"
#include "LevelOSB.h"

#include "LevelRtnuCircum.h"
#include "LevelBlocking.h"

#include "Log/LogFile.h"
#include "Log/LogDump.h"

#include "timer/timer.h"



//////////////////////////////////////////////////////////////////////////
//CLevelPlayerMove
unsigned __int64 g_tMoves[100];
int g_iMove=0;
void CLevelPlayerMove::HandleMove(PlayerMove &move,ServerSecond secondServer,PlayerMoveReply &reply)
{
	if (!_lo->IsAlive())
		return;
	if (_lo->IsDeferDestroy())
		return;

	LevelTick tLevel=_ToLocalT(secondServer);
	if (_tClient==ANIMTICK_INFINITE)
	{
		_tLevel=tLevel;
 		_tClient=move.tStart;
	}

	//更新_ks
	if (move.bTeleport)
	{
		_ks.Clean();

		Key_2f *k=(Key_2f *)_ks.NewKey();
		k->v=move.pos1;
		k->t=_ToClientT(tLevel);

		_idTeleport=*(LevelTeleportID*)&move.pos2;
		_posTeleport=move.pos1;
	}
	else
	{
		if (_ks.IsEmpty())
		{
			Key_2f *k=(Key_2f *)_ks.NewKey();
			k->v=_lo->GetFramePos();
			k->t=_ToClientT(tLevel);
		}
		else
		{
			//将新收到的Keyset衔接到旧的上面
			Key_2f kStart;
			LevelTick tCur=_ToClientT(tLevel);
			KeySet_CalcKey(&_ks,&kStart,tCur);
			_ks.DiscardKeysAfter(tCur);

			_ks.DiscardInvalidKeysBefore(ANIMTICK_SAFE_MINUS(tCur,LEVEL_FRAME_TICK));

			Key_2f *k=(Key_2f *)_ks.NewKey();
			k->v=kStart.v;
			k->t=tCur;

			//这边要检查一下是否有超出速度的移动
		}

		DWORD nStep=move.nStep;
		Key_2f *k=(Key_2f *)_ks.NewKey();
		k->v=move.pos1;
		if (nStep<=4)
		{
			k->t=_ToClientT(tLevel+(nStep)*ANIMTICK_FROM_SECOND(LEVEL_FRAME_INTERVAL/2.0));
		}
		else
		{
			k->t=_ToClientT(tLevel+ANIMTICK_FROM_SECOND(LEVEL_FRAME_INTERVAL)*2);

			k=(Key_2f *)_ks.NewKey();
			k->v=move.pos2;
			k->t=_ToClientT(tLevel+(nStep)*ANIMTICK_FROM_SECOND(LEVEL_FRAME_INTERVAL/2.0));
		}

		if (nStep==1)
		{
//			_lo->GetLevel()->GetDbgDraw().DrawCircle(move.pos1,0.2+0.04f*(float)g_iMove,RGB(255,255,0),5.0f);
			if (g_iMove<ARRAY_SIZE(g_tMoves))
			{
				g_tMoves[g_iMove]=GetAbsTick();
				g_iMove++;
			}
		}
		else
			g_iMove=0;
	}

	//更新_ksFace
	if (move.bTeleport)
	{
		_ksFace.Clean();

		if (move.bFace)
		{
			Key_f *k=(Key_f *)_ksFace.NewKey();
			k->v=LevelFaceFromInt(move.face);
			k->t=_ToClientT(tLevel);
		}
	}
	else
	{
		if (!move.bFace)
			_ksFace.Clean();
		else
		{
			if (_ksFace.IsEmpty())
			{
				Key_f *k=(Key_f *)_ksFace.NewKey();
				k->v=_lo->GetFrameFace();
				k->t=_ToClientT(tLevel);
			}
			else
			{
				LevelTick tCur=_ToClientT(tLevel);

				//将新收到的Keyset衔接到旧的上面
				Key_f kStart;
				KeySet_CalcAngleKey(&_ksFace,&kStart,tCur);
				_ksFace.DiscardKeysAfter(tCur);

				_ksFace.DiscardInvalidKeysBefore(ANIMTICK_SAFE_MINUS(tCur,LEVEL_FRAME_TICK));

				Key_f *k=(Key_f *)_ksFace.NewKey();
				k->v=kStart.v;
				k->t=tCur;
			}

			Key_f *k=(Key_f *)_ksFace.NewKey();
			k->v=LevelFaceFromInt(move.face);
			k->t=_ToClientT(tLevel+ANIMTICK_FROM_SECOND(LEVEL_FRAME_INTERVAL/2.0));
		}
	}

	_bReaching=move.bReaching;

	_posExpect=move.posExpect;

	UpdatePosAndFace(_lo->GetLevel()->GetServerSecond()+LEVEL_FRAME_INTERVAL);

	reply.tDiff=(int)(_ToClientT(tLevel)-(move.tStart));

	_lo->GetLevel()->AddAffect(_lo);

}

void CLevelPlayerMove::UpdatePosAndFace(ServerSecond second)
{
	if (_ks.IsEmpty())
		return;

	if (_bPauseMove)
	{
		if (!_NeedPauseMove())
		{
			_bPauseMove=FALSE;
			//这里我们要检查一下,当前_ks里的位置和_lo的位置是否差别过大,如果
			//过大要把Client的位置拉回来
		}
		else
			return;//PauseMove下,我们不把_ks的位置更新给_lo
	}

	if(_idTeleport!=_idAuthTeleport)
		return;

	if (_lo)//看看当前的技能是否允许我们更新位置
	{
		extern CLevelSkill *LevelUtil_GetCastingSkill(CLevelObj *lo);

		CLevelSkill *skill=LevelUtil_GetCastingSkill(_lo);
		if (skill)
		{
			switch(skill->GetCastMoving())
			{
				case CLevelSkill::CastMoving_None:
					return;
			}
		}
	}

	if (TRUE)
	{
		LevelTick t=_ToLocalT(second);
		LevelTick tClient=_ToClientT(t);
		Key_2f k;
		KeySet_CalcKey(&_ks,&k,tClient);

		float face=0.0f;
		if(_ksFace.IsEmpty())
		{
			AnimTick t2=tClient+ANIMTICK_FROM_SECOND(LEVEL_FRAME_INTERVAL);
			Key_2f k2;
			KeySet_CalcKey(&_ks,&k2,t2);
			LevelPos dir=k2.v-k.v;
			if (dir.getLengthSQ()>0.0001f)
				face=atan2f(dir.y,dir.x);
			else
			{
				CLevelObjMove *move=_lo->GetMove();
				if (move)
					face=move->GetFrameFace();//保持不变
			}
		}
		else
		{
			Key_f kFace;
			KeySet_CalcAngleKey(&_ksFace,&kFace,tClient);
			face=kFace.v;
		}

		BOOL bReaching=FALSE;
		if (tClient>=_ks.GetEndTick())
			bReaching=_bReaching;

		CLevelObjMove *move=_lo->GetMove();
		if (move)
			move->SetMove(k.v,face,bReaching);
	}

}

BOOL CLevelPlayerMove::IsReaching(LevelTick tLocal)
{
	LevelTick tClient=_ToClientT(tLocal);
	if (tClient>=_ks.GetEndTick())
		return _bReaching;

	return FALSE;
}

void CLevelPlayerMove::GetRecentMoveStep(LevelTick tCur,LevelTick durStep,LevelMoveStep &step)
{
	if (_ks.IsEmpty())
	{
		step.speed=0.0f;
		if (_lo)
		{
			step.pos=_lo->GetFramePos();
			step.dir=LevelFaceToDir(_lo->GetFrameFace());
			step.speed=_lo->GetMove()->CalcGroundSpeed();
		}
		else
		{
			step.pos.set(0,0);
			step.dir.set(0,0);
		}
		step.dist=0.0f;
		step.bReaching=TRUE;

		return;
	}

	LevelTick tClient=_ToClientT(tCur);
	Key_2f k;
	KeySet_CalcKey(&_ks,&k,tClient);
	LevelPos posCur=k.v;
	KeySet_CalcKey(&_ks,&k,ANIMTICK_SAFE_MINUS(tClient,durStep));
	step.pos=k.v;
	step.dir=posCur-step.pos;
	step.dist=step.dir.getLength();
	if (step.dist<0.001f)
	{
		step.dir.set(0,0);
		if (_lo)
		{
			CLevelObjMove *move=_lo->GetMove();
			if (move)
				step.dir=LevelFaceToDir(move->GetFrameFace());
		}

		step.dist=0.0f;
	}
	else
		step.dir.normalize();
	step.speed=0.0f;
	if (_lo)
	{
		CLevelObjMove *move=_lo->GetMove();
		if (move)
			step.speed=move->CalcGroundSpeed();
	}

	step.bReaching=FALSE;
	if (tClient>=_ks.GetEndTick())
		step.bReaching=_bReaching;
}



LevelPos CLevelPlayerMove::CalcPos(LevelTick t)
{
	if (_ks.IsEmpty())
		return _lo->GetFramePos();
	LevelTick tClient=_ToClientT(t);
	Key_2f k;
	KeySet_CalcKey(&_ks,&k,tClient);
	return k.v;
}


void CLevelPlayerMove::AuthorizeTeleport(LevelTeleportID idTeleport,LevelPos &posTeleport)
{
	_idAuthTeleport=idTeleport;
	_posAuthTeleport=posTeleport;

}

LevelTick CLevelPlayerMove::_ToLocalT(ServerSecond second)
{
	return _lo->GetT()+(LevelTick)(((second-_lo->GetLevel()->GetServerSecond())/LEVEL_SUBFRAME_INTERVAL)*(float)LEVEL_SUBFRAME_TICK);
}


//对于PauseMove的处理:
//当Player处于PauseMove状态下时,我们将停止将_ks里的位置更新给_lo,
//但仍然接受Client发过来的路点,和非PauseMove状态下没有区别,(也就是实际上
//仍然在根据Client的要求移动,但不更新给_lo)
//(当然,正常情况下,由于我们已经通知了Client _lo已经处于PauseMove状态
//下了,所以Client理论上也不应该发送新的路点了,当然因为网络因素有可能
//仍然有路点发过来),然后,当PauseMove状态结束时,我们继续将_ks里的位置更新
//给_lo,理论上PauseMove后的位置应该和PauseMove前的位置是不应该差很多的
//如果差别很大,需要拉回来 
BOOL CLevelPlayerMove::_NeedPauseMove()
{
	BOOL bNeedPause=FALSE;
	if (_lo->NeedPauseMove())
		bNeedPause=TRUE;

	return bNeedPause;

}


//////////////////////////////////////////////////////////////////////////
//CLevelPlayerSkills

void CLevelPlayerSkills::Clear()
{
	_skills.clear();
	Zero();
}

void CLevelPlayerSkills::_AddSkill(RecordID idSkill,LevelSkillGrade grd,DWORD stack)
{
	if (idSkill==RecordID_Invalid)
		return;
	std::unordered_map<RecordID,LevelPlayerSkill>::iterator it=_skills.find(idSkill);
	if (it==_skills.end())
	{
		LevelPlayerSkill t;
		t.idSkill=idSkill;
		t.grd=grd;
		t.stack=stack;
		
		_skills[idSkill]=t;
	}
	else
		(*it).second.stack+=stack;
}


void CLevelPlayerSkills::Refresh(LevelPlayerStates *lps,LevelRecordUnit *recUnit,CLevelRecords *records)
{
	if (lps->skills.GetVerDB()==_verDB)
		return;//没有变化
	Clear();

	//学会的技能
	for (int i=0;i<lps->skills.items.size();i++)
	{
		LevelItemState *state=&lps->skills.items[i];
		if (state->nStack<=0)
			continue;
		LevelRecordItem *recItem=records->GetItem(state->tid);
		if (!recItem)
			continue;

		_AddSkill(recItem->skill,(LevelSkillGrade)state->grd,state->nStack);
	}

	//缺省技能
	if (recUnit)
	{
		for (int i=0;i<ARRAY_SIZE(recUnit->attksDef);i++)
		{
			RecordID idSkill=recUnit->attksDef[i];
			_AddSkill(idSkill,1,0xffffffff);
		}
	}

	//其它一些技能
	if (TRUE)
	{
		LevelRecordGlobal *rec=records->GetGlobal();
		_AddSkill(rec->AgentInvoker,1,0xffffffff);
		_AddSkill(rec->ItemInvoker,1,0xffffffff);
	}

	_verDB=lps->skills.GetVerDB();
}

LevelSkillGrade CLevelPlayerSkills::GetSkillGrade(RecordID idSkill)
{
	std::unordered_map<RecordID,LevelPlayerSkill>::iterator it=_skills.find(idSkill);
	if (it==_skills.end())
		return LevelSkillGrade_Invalid;
	return (*it).second.grd;
}

DWORD CLevelPlayerSkills::GetSkillStack(RecordID idSkill)
{
	std::unordered_map<RecordID,LevelPlayerSkill>::iterator it=_skills.find(idSkill);
	if (it==_skills.end())
		return 0;
	return (*it).second.stack;
}



//////////////////////////////////////////////////////////////////////////
//CLevelPlayer

void CLevelPlayer::Init(CLevel *level,LevelPlayerID id,CLoUnit*lo,LevelPlayerStates *lps)
{
	_level=level;

	_circumRtnu=Class_New2(CLevelRtnuCircum);
	_circumRtnu->Init(this);
	_rtnus=Class_New2(CLevelRtnus);
	_rtnus->Init(this);

	_serviceCureHP.Init(level);
	_lps=lps;
	_id=id;
	_mask=1<<_id;

	_lo=lo;
	SAFE_ADDREF(_lo);

	_move.Init(lo);

	//初始化abilities
	if (TRUE)
	{
		_abilities.Init(lo,this);
		LPS_LoadAbilities(_lps,&_abilities);
		_abilities.ClearDirty();

//		UpdateAbilityStates();//更新ability states
	}


	const int reserve=100;
	_sights.resize(reserve);
	_freelistSights.resize(reserve);
	for (int i=0;i<reserve;i++)
		_freelistSights[i]=i;

	//创建/校验ExploreMap
	if (TRUE)
	{
		RecordID idMap=level->GetMapID();
		i_math::recti rcMap=level->GetMapRect();	

		extern LevelExploreMaps LPS_FindExploreMaps(LevelPlayerStates *lps,RecordID idMap);
		extern LevelExploreMaps LPS_NewExploreMaps(LevelPlayerStates *lps,RecordID idMap,i_math::recti &rcMap);
		extern LevelExploreMaps LPS_QueryExploreMaps(LevelPlayerStates *lps,RecordID idMap);
		LevelExploreMaps mps=LPS_FindExploreMaps(lps,idMap);
		if (mps.IsEmpty())
			LPS_NewExploreMaps(lps,idMap,rcMap);
		else
		{//已经存在, 校验大小是否发生了改变
			if (!(mps.sttc->GetMapRect()==rcMap))
			{//地图的大小不一样了
				mps=LPS_QueryExploreMaps(lps,idMap);
				if (!mps.IsEmpty())
				{
					mps.sttc->Clear();
					mps.sttc->Init(rcMap);
					mps.dyn->Clear();
					mps.dyn->Init(rcMap);
				}
			}
		}
	}
}

void CLevelPlayer::Clear()
{
	std::deque<CLevelSight>::iterator it;
	for (it=_sights.begin();it!=_sights.end();it++)
	{
		CLevelObj *obj=(*it).obj;
		SAFE_RELEASE(obj);
	}
	_sights.clear();
	_freelistSights.clear();

	_leaves.clear();

	if (_npcsRtnu)
	{
		_npcsRtnu->Destroy(RecordID_Invalid);
		Safe_Class_Delete(_npcsRtnu);
	}

	SAFE_RELEASE(_lo);

	_abilities.Clear();
	_move.Clear();
// 	_skills.Clear();

	for (int i=0;i<ARRAY_SIZE(_msgAbilities);i++)
		_msgAbilities[i].GClear();

	_serviceCureHP.Clear();

	if (_rtnus)
	{
		_rtnus->Clear();
		Safe_Class_Delete(_rtnus);
	}

	if (_circumRtnu)
	{
		_circumRtnu->Clear();
		Safe_Class_Delete(_circumRtnu);
	}

	Zero();

}

void CLevelPlayer::OnEnterLevel()
{
	_abilities.OnEnterLevel();
}

void CLevelPlayer::OnLeaveLevel()
{
	_abilities.OnLeaveLevel();

	if (_abilities.IsDirty())
	{
		extern void LevelUtil_SaveAbilities(CLevelObj *lo);
		LevelUtil_SaveAbilities(_lo);
		_abilities.ClearDirty();
	}

}


void CLevelPlayer::WriteFrameLeaveSight(CBitPacket *bp)
{
	//离开视野的obj
	if(_leaves.size()>0)
	{
		//先打一个标记
		for (int i=0;i<_leaves.size();i++)
		{
			CLevelObj *p=_leaves[i];
			p->_bEnum=1;
		}

		bp->Data_WriteSimple(LEVELFRAME_LEAVESIGHT);
		std::deque<CLevelSight>::iterator it;
		for (it=_sights.begin();it!=_sights.end();it++)
		{
			CLevelSight *sight=&(*it);
			if (!sight->obj)
				continue;
			if (sight->obj->_bEnum)
			{
				WORD idx=(WORD)(it-_sights.begin());
				bp->Data_WriteSimple(sight->obj->GetID());

				SAFE_RELEASE(sight->obj);
				_freelistSights.push_front(idx);
			}
		}
		bp->Data_WriteSimple(LevelObjID_Invalid);//终止符号

		//清除标记 
		for (int i=0;i<_leaves.size();i++)
		{
			CLevelObj *p=_leaves[i];
			p->_bEnum=0;
		}

		_leaves.clear();
	}

}

DWORD CLevelPlayer::AddSightEnter(CLevelObj *obj,BOOL bFirstSync)
{
	//找一个位置放置这个新的obj
	DWORD idx;
	if (_freelistSights.size()>0)
	{
		idx=_freelistSights[0];
		_freelistSights.pop_front();
	}
	else
	{
		idx=(DWORD)_sights.size();
		_sights.resize(idx+1);
	}
	_sights[idx].obj=obj;
	SAFE_ADDREF(obj);
	_sights[idx].bFirstSync=bFirstSync;
	return idx;
}

inline void WriteLevelObjID(CBitPacket *bp,LevelObjID id)
{
	bp->Data_WriteSimple(id);
}

LevelObjID ReadLevelObjID(CBitPacket *bp)
{
	LevelObjID id=bp->Data_ReadSimple<LevelObjID>();
	return id;
}


extern void WriteLevelObjID(CBitPacket *bp,LevelObjID id);
void CLevelPlayer::WriteFrameSync(CBitPacket *bp)
{
	WORD nActual=_sights.size()-_freelistSights.size();//总的数量-Free的数量
	if (nActual>0)
	{
		if (TRUE)
		{
			CBitPacket::Position posOrg=bp->GetCurPos();
			bp->Data_WriteSimple(LEVELFRAME_SYNC_H);

			BOOL bContent=FALSE;

			int iSerial=0;
			for (int i=0;i<_sights.size();i++)
			{
				CLevelSight *sight=&_sights[i];
				if (!sight->obj)
					continue;

				if (sight->bFirstSync)
					sight->obj->WriteFirstSync(bp,bContent,_id);
				else
					continue;
 
				bp->Data_WriteSimple(iSerial+177);
				bp->Bits_Write(iSerial+122,15);

				iSerial++;
			}

			for (int i=0;i<_sights.size();i++)
			{
				CLevelSight *sight=&_sights[i];
				if (!sight->obj)
					continue;

				if (!sight->bFirstSync)
					sight->obj->WriteSyncH(bp,bContent,_id);
				else
					continue;

				bp->Data_WriteSimple(iSerial+177);
				bp->Bits_Write(iSerial+122,15);

				iSerial++;
			}

			if (!bContent)
				bp->SetCurPos(posOrg);//如果实际并没有需要更新的数据,我们根本就不需要写入任何数据
		}

		if (TRUE)
		{
			CBitPacket::Position posOrg=bp->GetCurPos();
			bp->Data_WriteSimple(LEVELFRAME_SYNC_L);

			BOOL bContent=FALSE;

			CBitPacket::Position posOrg2=bp->GetCurPos();

			for (int i=0;i<_sights.size();i++)
			{
				CLevelSight *sight=&_sights[i];
				if (!sight->obj)
					continue;

				if (!sight->bFirstSync)
				{
					WriteLevelObjID(bp,sight->obj->GetID());
					BOOL bContent2=FALSE;
					sight->obj->WriteSyncL(bp,bContent2,_id);
					if (!bContent2)
						bp->SetCurPos(posOrg2);
					else
					{
						posOrg2=bp->GetCurPos();
						bContent=TRUE;
					}
				}

				sight->bFirstSync=FALSE;
			}

			WriteLevelObjID(bp,LevelObjID_Invalid);//结束符

			if (!bContent)
				bp->SetCurPos(posOrg);//如果实际并没有需要更新的数据,我们根本就不需要写入任何数据
		}
	}
}

unsigned __int64 g_tStartSkill=0;

BOOL CLevelPlayer::HandleSkill(PlayerSkill &skill,LevelSkillArg *arg)
{
	if (!_lo->IsAlive())
		return FALSE;
	if (_lo->IsDeferDestroy())
		return FALSE;

	//由ability验证一下
	if(skill.tpSkill.tpAbility_!=LevelAbilityType_None)
	{
		CLevelAbility *ability=_abilities.GetActiveAbility(skill.tpSkill.tpAbility_);
		if (!ability)
			return FALSE;
		if (!ability->TestStartSkill(skill.tpSkill))
			return FALSE;
		ability->NotifyStartSkill(skill.tpSkill);
	}

	CLevelSkillDriver *driver=_lo->GetSkillDriver();

	extern LevelRecordSkill *LevelUtil_GetSkillRecord(CLevelObj *lo,LevelSkillType tpSkill);
	LevelRecordSkill *rec=LevelUtil_GetSkillRecord(_lo,skill.tpSkill);
	if (!rec)
		return FALSE;

// 	_skills.Refresh(_lps,_lo->GetRec(),_level->GetRecords());
// 	DWORD nStack=_skills.GetSkillStack(skill.tidSkill);
// 	if (nStack==0)
// 		return FALSE;

//	LevelSkillGrade grd=_skills.GetSkillGrade(skill.tidSkill);

	g_tStartSkill=GetAbsTick();

	extern LevelSkillGrade LevelUtil_GetAbilitySkillGrade(CLevelObj *lo,LevelAbilityType tpAbility);
	LevelSkillGrade grd=LevelUtil_GetAbilitySkillGrade(_lo,skill.tpSkill.tpAbility_);

	BOOL bRet=driver->Start(skill.tpSkill,skill.target,TRUE,skill.idClient,grd,arg);
	if (bRet)
	{
		if (_rtnus)
			_rtnus->AddCoSkillCharge(rec,grd,skill.target);

		if (_npcsRtnu)
			_npcsRtnu->AddCoSkillCharge(rec,grd,skill.target);
	}
	else
	{
//		assert(FALSE);
	}
	return bRet;
}

BOOL CLevelPlayer::HandleUnEquipToPickUp(EquipPart part,BOOL &bAffectBag0)
{
	bAffectBag0=FALSE;
	if (!_lps->pickup.item.IsValid())
	{
		LevelItemState *item=&_lps->equip.parts[part];

		if (item->IsValid())
		{
			_lps->pickup.item.CopyFrom(item);
			item->Clear();

			_lps->equip.SetDirtyDB_Urgent();
			_lps->pickup.SetDirtyDB_Urgent();

			_lo->UpdateExprEquips(_lps);//更新角色的外观

			return TRUE;
		}
	}

	return FALSE;
}


BOOL CLevelPlayer::HandleEquipFromPickUp(EquipPart part,BOOL &bAffectBag0)
{
	bAffectBag0=FALSE;
	if (_lps->pickup.item.IsValid())
	{
		LevelItemState *item=&_lps->equip.parts[part];

		if (!item->IsValid())
		{
			item->CopyFrom(&_lps->pickup.item);
			_lps->pickup.item.Clear();
		}
		else
			item->Swap(&_lps->pickup.item);

		_lps->equip.SetDirtyDB_Urgent();
		_lps->pickup.SetDirtyDB_Urgent();

		_lo->UpdateExprEquips(_lps);//更新角色的外观

		return TRUE;
	}
	return FALSE;

}

BOOL CLevelPlayer::HandleEquipFromBag(DWORD iBag,i_math::rect_c &rc,BOOL &bAffectBag0)
{
	bAffectBag0=FALSE;

	if (!_lps->pickup.item.IsValid())
	{
		if (iBag<ARRAY_SIZE(_lps->bags))
		{
			LPSBag *bag=&_lps->bags[iBag];
			for (int i=0;i<bag->items.size();i++)
			{
				if (bag->items[i].rc==rc)
				{
					BagItemState *itemBag=&bag->items[i];

					CLevelRecords *records=_level->GetRecords();
					if (bag->items[i].tid!=RecordID_Invalid)
					{
						LevelRecordItemClass *rec=records->GetItemClassOfItem(itemBag->tid);
						if (rec->part<EquipPart_Max)
						{
							if (_lps->equip.parts[rec->part].IsValid())
							{//如果已经有装备,

								LevelRecordItemClass *recEquip=records->GetItemClassOfItem(_lps->equip.parts[rec->part].tid);

								if ((recEquip->wSlot>rec->wSlot)||(recEquip->hSlot>rec->hSlot))
								{//装备的道具的大小大于等于背包道具的大小

									i_math::rect_c rcNew;//交换后在背包里的位置
									rcNew=bag->items[i].rc;
									rcNew.Right()=rcNew.Left()+recEquip->wSlot;
									rcNew.Bottom()=rcNew.Top()+recEquip->hSlot;

									for (int j=0;j<bag->items.size();j++)
									{
										if (j==i)
											continue;

										if (rcNew.isRectCollided(bag->items[j].rc))
											return FALSE;//无法交换,周围没有空余的位置
									}
								}

								//交换
								_lps->equip.parts[rec->part].Swap(itemBag);
								itemBag->rc.Right()=itemBag->rc.Left()+recEquip->wSlot;
								itemBag->rc.Bottom()=itemBag->rc.Top()+recEquip->hSlot;
							}
							else
							{//装备栏里没有装备
								//移动过去
								_lps->equip.parts[rec->part].CopyFrom(itemBag);
								bag->items.erase(bag->items.begin()+i);
							}

							bag->SetDirtyDB_Urgent();
							_lps->equip.SetDirtyDB_Urgent();

							_lo->UpdateExprEquips(_lps);//更新角色的外观

							return TRUE;
						}
					}
					
					break;
				}
			}
		}
	}

	return FALSE;

}

BOOL CLevelPlayer::HandleUnEquipToBag(EquipPart part,DWORD iBag,i_math::rect_c &rc,BOOL &bAffectBag0)
{
	bAffectBag0=FALSE;
	if (!_lps->pickup.item.IsValid())
	{
		if (((DWORD)part)<EquipPart_Max)
		{
			LevelItemState *item=&_lps->equip.parts[part];

			if (item->IsValid())
			{
				if (iBag<ARRAY_SIZE(_lps->bags))
				{

					CLevelRecords *records=_level->GetRecords();
					LevelRecordItemClass *recItemClass=records->GetItemClassOfItem(item->tid);

					if (recItemClass)
					{
						if ((recItemClass->wSlot==rc.getWidth())&&(recItemClass->hSlot==rc.getHeight()))
						{

							LPSBag *bag=&_lps->bags[iBag];
							BOOL bCollided=FALSE;
							for (int i=0;i<bag->items.size();i++)
							{
								if (bag->items[i].rc.isRectCollided(rc))
								{
									bCollided=TRUE;
									break;
								}
							}

							if (!bCollided)
							{
								bag->AddItem(*item,rc);

								item->Clear();

								_lps->equip.SetDirtyDB_Urgent();
								bag->SetDirtyDB_Urgent();

								_lo->UpdateExprEquips(_lps);//更新角色的外观
								return TRUE;
							}
						}
					}
				}
			}
		}
	}
	return FALSE;

}

BOOL CLevelPlayer::HandleDiscardPickUp(LevelPos &pos)
{
	if (!_lps->pickup.item.IsValid())
		return FALSE;

	if (!_level->GetUnitMgr()->IsWalkable(UnitFindPath_Walkable,pos))
		return FALSE;

	CLoItem* lo=(CLoItem*)_level->CreateObj(Class_Ptr2(CLoItem));

	lo->PostCreate(&_lps->pickup.item,pos,LevelOSB(),LevelOpLink());

	_level->AddToActives(lo);

	SAFE_RELEASE(lo);

	_lps->pickup.item.Clear();

	_lps->pickup.SetDirtyDB_Urgent();

	return TRUE;
}

BOOL CLevelPlayer::HandleInvokeItem(CLoItem *loItem,BOOL bToBag)
{
	if (!loItem)
		return FALSE;
	if (!loItem->IsAlive())
		return FALSE;
	if (loItem->IsDeferDestroy())
		return FALSE;
	if (!loItem->GetState().IsValid())
		return FALSE;

	BOOL bOk=FALSE;
	if (bToBag)
	{
		LevelRecordItem *recItem=loItem->GetRec();
		if (recItem)
		{
			LevelRecordItemClass *recClss=_level->GetRecords()->GetItemClass(recItem->clss);
			if (recClss)
			{
				i_math::rect_c rc;
				if (LPS_FindBagSlotForItem(rc,_lps,0,recClss->wSlot,recClss->hSlot))
				{
					LPSBag *bag=&_lps->bags[0];
					bag->AddItem(loItem->GetState(),rc);
					loItem->DeferDestroy();
					bag->SetDirtyDB_Urgent();
					bOk=TRUE;
				}
			}
			else
			{
				LOG_DUMP_1P("HandleInvokeItem",Log_Error,"道具(%s)未指定ItemClass,所以无法拾取!",recItem->Name.c_str());
			}
		}
	}
	else
	{
		if (!_lps->pickup.item.IsValid())
		{
			_lps->pickup.item.CopyFrom(&loItem->GetState());
			loItem->DeferDestroy();
			_lps->pickup.SetDirtyDB_Urgent();
			bOk=TRUE;
		}
	}

	return bOk;

}

BOOL CLevelPlayer::HandleRtnuCmd(LevelRtnuCmd &cmd)
{
	//加到队列
	if (TRUE)
	{
		LevelRtnuCmdEx cmdEx;
		(LevelRtnuCmd &)cmdEx=cmd;
		cmdEx.t=_level->GetT_();

		_cmdsRtnu.push_back(cmdEx);
	}

	return TRUE;
}

BOOL CLevelPlayer::HandleRtnuHint(LevelRtnuHint &hint)
{
	return TRUE;
}


void BuildDecideInfo_Equip(DecideInfo_Equip *di,LevelPlayerStates *lps)
{

}

DecideInfo_Equip *CLevelPlayer::GetDecideInfo_Equip()
{
	if (_verEquip!=_lps->equip.GetVerDB())
	{
		BuildDecideInfo_Equip(&_diEquip,_lps);
		_verEquip=_lps->equip.GetVerDB();
	}
	return &_diEquip;
}


void CLevelPlayer::SetLPSDirty()
{
	if (_lps)
		_lps->SetDirtyClient();
}


void CLevelPlayer::UpdateExlporeMap()
{
	if (!_lo)
		return;

	RecordID idMap=_level->GetMapID();
	LevelPos3D center=_lo->GetFramePos3D();
	i_math::pos2di posTile;
	if (TRUE)
	{
		extern LevelExploreMaps LPS_FindExploreMaps(LevelPlayerStates *lps,RecordID idMap);
		LevelExploreMaps mps=LPS_FindExploreMaps(_lps,idMap);
		if (mps.IsEmpty())
			return;
		posTile=mps.sttc->TilePosFromWorldPos(center);
	}

	if (posTile==_posExplore)
		return;

	_posExplore=posTile;

	extern LevelExploreMaps LPS_QueryExploreMaps(LevelPlayerStates *lps,RecordID idMap);
	LevelExploreMaps mps=LPS_QueryExploreMaps(_lps,idMap);
	if (!mps.IsEmpty())
	{
		mps.sttc->AddExplore(center,2);
		mps.dyn->AddExplore(center,2);
	}

}


void CLevelPlayer::CreateRtnuNPCs()
{
	if (_npcsRtnu)
	{
		assert(FALSE);
		return;
	}

	_npcsRtnu=Class_New2(CLevelNPCs);

	CNPCPendings *pendingsNPC=_level->GetWorld()->GetNPCPendings(_id);
	CNPCPendings::Pendings *pendings=pendingsNPC->GetRtnuPendings();
	_npcsRtnu->Create(_level,_id,pendings);
	pendingsNPC->ClearRtnuPendings();
}

void CLevelPlayer::CreateRtnuNPCs_Teleport(CLevelNPCs *npcsOrg)
{
	if (_npcsRtnu)
	{
		assert(FALSE);
		return;
	}

	_npcsRtnu=Class_New2(CLevelNPCs);
	_npcsRtnu->CreateTeleport(_level,_id,npcsOrg);
}

CLevelService*CLevelPlayer::GetService(LevelServiceType tp)
{
	return &_serviceCureHP;
}

void CLevelPlayer::AddPendingAgentBriefEntry(LevelGUID guid)
{
	if (!_level)
		return;
	CJjWorld *world=_level->GetWorld();
	if (world)
	{
		LevelAgentGuid pending;
		pending.idMap=_level->GetMapID();
		pending.guid=guid;
		world->AddAgentBriefEntryToSend(_id,pending);
	}
}

void CLevelPlayer::FetchPendingAgentBriefEntry(std::vector<LevelAgentGuid> &pendings)
{
	if (_level)
	{
		CJjWorld *world=_level->GetWorld();
		if (world)
		{
			world->FetchPendingAgentBriefEntry(_id,pendings);
			return;
		}
	}
	pendings.clear();
}

BOOL CLevelPlayer::UpgradeAbility(CLevelAbilityUpgrade &upgrade,LevelAwardSeed &seed)
{
	if (_abilities.ApplyUpgrade(&upgrade,seed))
	{
		LPS_SaveAbilities(_lps,&_abilities);
		return TRUE;
	}
	return FALSE;
}

void CLevelPlayer::UpdateAbilities()
{
	_abilities.Update();

	std::vector<BYTE> data;

	for (int i=1;i<LevelAbilityType_Max;i++)
	{
		CLevelAbility *ability=_abilities.GetAbility((LevelAbilityType)i);
		if (ability)
		{
			if (ability->IsActive())
			{
				DP_BeginSave(dp,data);
				ability->SaveSync(dp);
				DP_EndSave();
			}
			else
				data.clear();
			if (!(data==_msgAbilities[i].data))
			{
				//有变化
				_msgAbilities[i].tp=(LevelAbilityType)i;
				_msgAbilities[i].data=data;
				_level->SendNetMsg(_id,&_msgAbilities[i]);
			}
		}
	}

	if (_abilities.IsDirty())
	{
		extern void LevelUtil_SaveAbilities(CLevelObj *lo);
		LevelUtil_SaveAbilities(_lo);
		_abilities.ClearDirty();
	}
}

void CLevelPlayer::UpdateHPSP(float dt)
{
	if (!_lo)
		return;

	LevelRecordUnit *rec=_lo->GetRec();
	if (!rec)
		return;

	if (dt<=0.001f)
		return;

	LevelAttr_Base *attr=_lo->GetAttr_Base();

	BOOL bExhausted=FALSE;
	if (attr->sp.GetMax_Int()<=(int)rec->ExhaustedSP)
		bExhausted=TRUE;

	LevelTick t=_level->GetT_();

	if (bExhausted)
	{
		if (_tExhaustedStart==ANIMTICK_INFINITE)
			_tExhaustedStart=t;
	}
	else
		_tExhaustedStart=ANIMTICK_INFINITE;

	LevelPos posFrame=_lo->GetFramePos();
	float dist=posFrame.getDistanceFrom(_posLastFrame);
	_posLastFrame=posFrame;
	if (dist/dt>20.0f)
		dist=0.0f;//忽略瞬间的高速移动

	if (!bExhausted)
	{
		CLevelDecider::CommitHPAutoRecovery(_lo,dt);
		float spCost=dist*0.25f;
		float spMaxCost=0.0f;
		extern void LevelUtil_ModSPCost(CLevelObj *lo,float &sp,float &spCost);
		LevelUtil_ModSPCost(_lo,spCost,spMaxCost);
		if (spCost>0.0f)
			CLevelDecider::CommitSPDrain(_lo,spCost);
	}
	else
	{
		if (t>_tExhaustedStart+rec->ExhaustedCountDown)
			_level->GetDecider()->CommitSuicide(_lo);
		else
		{
			if (dist<0.001f)
			{
				//没有移动
				float spRecover=5.0f*dt;
				if (attr->sp.GetCur_Float()+spRecover>rec->ExhaustedSP)
				{
					spRecover=rec->ExhaustedSP-attr->sp.GetCur_Float();
				}

				CLevelDecider::CommitSPMod(spRecover,LevelOSB(_lo),_lo,LevelOpLink(),FALSE);
			}
		}

	}
}


void CLevelPlayer::UpdateHPSP_New(float dt)
{
	if (!_lo)
		return;

	LevelRecordUnit *rec=_lo->GetRec();
	if (!rec)
		return;

	if (dt<=0.001f)
		return;

	LevelAttr_Base *attr=_lo->GetAttr_Base();

	LevelTick t=_level->GetT_();

	float spRecover=0.0f;
	if (attr->sp.GetCur_Float()<attr->sp.GetMax_Float())
	{
		float durFullRecover=8.0f;
		spRecover=attr->sp.GetMax_Float()/durFullRecover*dt;
	}

	if (spRecover>0.0f)
	{
		extern CLevelSkill *LevelUtil_GetCastingSkill(CLevelObj *lo);
		if (LevelUtil_GetCastingSkill(_lo))
			spRecover=0.0f;
	}

	if (spRecover>0)
	{
		CLevelBlocking *blocking=_lo->GetBlocking();
		if (blocking)
		{
			if (blocking->IsActive())
				spRecover=0.0f;
		}
	}
 	CLevelDecider::CommitHPAutoRecovery_New(_lo,dt);

	CLevelDecider::CommitSPMod(spRecover,LevelOSB(_lo),_lo,LevelOpLink(),TRUE);

// 	float spDrain=1.0f*dt;
// 	spDrain*=(1.0f-attr->sp_.GetRatio());
// 
// 	if (spDrain>attr->sp_.GetCur_Float()-rec->ExhaustedSP)
// 		spDrain=attr->sp_.GetCur_Float()-rec->ExhaustedSP;
// 	if (spDrain>0.0f)
// 	{
// 		LevelOSB osb(_lo);
// 		if (TRUE)
// 		{
// 			LevelOp_SPMod*op=osb.NewOp<LevelOp_SPMod>(LevelOpLink());
// 			attr->sp_.MakeMaxMod(-spDrain,op->mod);
// 			_lo->AddOp(op);
// 		}
// 
// 		if (TRUE)
// 		{
// 			LevelOp_FullSPMod*op=osb.NewOp<LevelOp_FullSPMod>(LevelOpLink());
// 			attr->spFull.MakeMod(-spDrain,TRUE,op->mod);
// 			_lo->AddOp(op);
// 		}
// 	}

}

void CLevelPlayer::UpdateRtnus()
{
	if (_rtnus)
		_rtnus->Update();

	if (_npcsRtnu)
		_npcsRtnu->Update();
}

void CLevelPlayer::UpdateRtnuCmds()
{
	AnimTick t=_level->GetT_();
	AnimTick dur=ANIMTICK_FROM_SECOND(0.5f);

	while(_cmdsRtnu.size()>0)
	{
		if (_cmdsRtnu[0].t+dur<t)
			_cmdsRtnu.pop_front();
		else
			break;
	}

}

DWORD CLevelPlayer::GetRecentRtnuCmds(float durF,LevelRtnuCmd **buf,DWORD szBuf)
{
	if (szBuf<=0)
		return 0;

	AnimTick t=_level->GetT_();
	AnimTick dur=ANIMTICK_FROM_SECOND(durF);

	int n=_cmdsRtnu.size();

	DWORD c=0;
	for (int i=n-1;i>=0;i--)
	{
		if (_cmdsRtnu[i].t+dur<t)
			break;

		buf[c]=&_cmdsRtnu[i];
		c++;
		if (c>=szBuf)
			break;
	}

	return c;
}



BOOL CLevelPlayer::HandleSwitchWpn(EquipPart wpnActive)
{
	if (!_level)
		return FALSE;
	if ((wpnActive!=EquipPart_Weapon)&&(wpnActive!=EquipPart_Weapon2nd))
		return FALSE;
	if (!_lps)
		return FALSE;

	_lps->equip.wpnActive=wpnActive;

	if (GetLoUnit())
	{
		GetLoUnit()->UpdateExprEquips(_lps);
		_level->AddAffect(GetLoUnit());
	}

	_lps->equip.SetDirtyDB_Low();
	_lps->SetDirtyClient();

	return TRUE;
}

void CLevelPlayer::HandleStartDay()
{
	_abilities.HandleStartDay();
}


void CLevelPlayer::HandleEndDay()
{
	for (int i=1;i<LevelAbilityType_Max;i++)
	{
		CLevelAbility *ability=_abilities.GetAbility((LevelAbilityType)i);
		if (ability)
		{
			if (ability->IsActive())
				ability->HandleEndDay();
		}
	}
}
