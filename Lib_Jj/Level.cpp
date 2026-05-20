
#include "stdh.h"

#include "LevelData.h"
#include "LevelObjSrc.h"
#include "Level.h"
#include "LevelRtnus.h"

#include "LevelRecords.h"
#include "LevelDropper.h"
#include "LevelBGs.h"
#include "LevelResources.h"

#include "LevelClasses.h"

#include "LevelBehavior.h"
#include "LevelExploreMap.h"

#include "LevelRecordGlobal.h"

#include "LoUnit.h"
#include "LoMagicBoard.h"
#include "LoSlatesA.h"

#include "LevelRtnus.h"

#include "LevelTalks.h"

#include "LevelRtnuCircum.h"

#include "Protocal.h"

#include "Log/LogFile.h"
#include "log/logdump.h"

#include "timer/profiler.h"
#include "timer/timer.h"

LogFile g_logJj("Jj");

#define RVO_SIMULATOR_ITERATE_COUNT (3)

////////////////////////////////////////////////////////////////////////
//LevelTeleportQuest
void LevelTeleportQuest::Clear()
{
	SAFE_RELEASE(loPlayer);
	Zero();
}


//////////////////////////////////////////////////////////////////////////
//CLevel

void CLevel::Create(CLevelData *data,RecordID idMap,CLevelRecords *records,CLevelResources *resources,CLevelDropper *dropper,CLevelBGs *bgs,CJjWorld *world)
{
	_data=data;
	_idMap=idMap;
	_records=records;
	SAFE_ADDREF(_records);

	_resources=resources;
	SAFE_ADDREF(_resources);

	_dropper=dropper;
	SAFE_ADDREF(_dropper);

	_bgs=bgs;
	SAFE_ADDREF(_bgs);

	_world=world;

	_ids.Init(this);
	_hooks.Init();

	_poolBuffID.Init(this);

	navData *ndata=_data->GetNavData();
	CLevelBasis *basis=_data->GetBasis();

	i_math::recti rcMap;//得到地图的大小

	if (TRUE)
	{
		navMesh *nmesh=(navMesh *)ndata->getNavMesh();
		rcMap.Left()=nmesh->getLeft();
		rcMap.Top()=nmesh->getTop();
		rcMap.Right()=nmesh->getRight();
		rcMap.Bottom()=nmesh->getBottom();
		rcMap*=nmesh->getTileLen();
	}

	_rc=rcMap;

	_mpObj.Create(this,rcMap);
	_mpEvent.Create();

	_unitmgr.Create(ndata);
	_unit3dmgr.Init(data->GetGtm());

	_simRvo.init(&ndata->nmesh,LEVEL_FRAME_INTERVAL/(float)(RVO_SIMULATOR_ITERATE_COUNT));

	_unitmgr.SetMirror(&_simRvo);
	_simRvo.setMirror(&_unitmgr);

	_inactives.Create(rcMap);

	_aovmap.Create(rcMap);

	_tilemap.Create(rcMap);

	_skills.Create(this);

	_decider.Init(this);

	_chancer.Init(&basis->_chancedata);

	_chancer.MakeDice_Spawn(1.0f);//分配Spawn的机会

	_dbgdraw.Init(this);

	//根据basis创建Level里的所有LevelObj
	if (TRUE)
	{
		int sz=basis->_buf.size();
		for (int i=0;i<sz;i++)
		{
			CLevelObjSrc *src=basis->_buf[i].src;
			CLevelObjParam *param=basis->_buf[i].param;

			if (src)
			{
				if (src->IsDisable())
					continue;
			}

			if (param)
			{
				if (param->IsDisable())
					continue;
			}

			if (param)
			{
				if (!param->CheckCreateChance(this,src))
					continue;//不需要创建
			}

			CLevelObj *obj=CreateObj(src,param);
			if (!obj)
				continue;
			obj->PostCreate();//设置初始参数
			_inactives.Add(obj);//将新创建的obj放到未激活的map中去
			SAFE_RELEASE(obj);
		}
	}

	_ais.Init(this);

	_pilesRes.Init(this);

	_secondServer=-1.0f;
	_t=0;
	_iSubFrame=LEVEL_SUBFRAME_COUNT-1;

}

void CLevel::Destroy()
{
	if (TRUE)
	{
		std::unordered_map<LevelServiceType,CLevelService *>::iterator it;
		for (it=_services.begin();it!=_services.end();it++)
		{
			CLevelService *service=((*it).second);
			service->Clear();
			Safe_Class_Delete(service);
		}
		_services.clear();
	}

	_pilesRes.Clear();
	_ais.Clear();

	_skills.Destroy();

	for (int i=0;i<ARRAY_SIZE(_npcs);i++)
		_npcs[i].Destroy(RecordID_Invalid);

	for (int i=0;i<ARRAY_SIZE(_players);i++)
		_players[i].Clear();

	for (int i=0;i<_activesSubframe.size();i++)
		SAFE_RELEASE(_activesSubframe[i]);
	_activesSubframe.clear();

	for (int i=0;i<_actives.size();i++)
		SAFE_DESTROY(_actives[i]);
	_actives.clear();

	_tilemap.Destroy();

	_inactives.Destroy();

	_unitmgr.Destroy();
	_unit3dmgr.Clear();
	_simRvo.clear();

	_mpObj.Destroy();
	_mpEvent.Destroy();

	_hooks.Clear();
	_ids.Clear();

	_decider.Clear();

	_chancer.Clear();

	_dbgdraw.Clear();

	_poolBuffID.Clear();

	SAFE_RELEASE(_records);
	SAFE_RELEASE(_resources);
	SAFE_RELEASE(_dropper);
	SAFE_RELEASE(_bgs);

	for (int i=0;i<ARRAY_SIZE(_uos);i++)
		SAFE_RELEASE(_uos[i]);

	SAFE_RELEASE(_eoEnv);

	Zero();
}

void CLevel::SendNetMsg(LevelPlayerID idPlayer,NetMsgSC *msg)
{
	_world->GetNetProxy()->SendMsg(idPlayer,msg);
}

void CLevel::SendNetMsg(NetMsgSC *msg)
{
	if (GetPlayer((LevelPlayerID)0))
		_world->GetNetProxy()->SendMsg((LevelPlayerID)0,msg);
}



const char*CLevel::GetNavDataPath()	
{		
	return _data->GetNavDataPath();	
}

CLevelObj *CLevel::_CreateObj(CLevelObj *p)
{
	if (!p)
		return NULL;

	p->_level=this;

	if (!p->Create())
	{
		Class_Delete(p);
		return NULL;
	}
	return p;
}


CLevelObj *CLevel::CreateObj(CClass *clss)
{
	return _CreateObj((CLevelObj *)clss->New());
}


CLevelObj *CLevel::CreateObj(ClassUID uid)
{
	CLevelObj *p=NewLevelObj(uid);
	return _CreateObj(p);
}

CLevelObj *CLevel::CreateObj(CLevelObjSrc *src,CLevelObjParam *param)
{
	CLevelObj *p=NewLevelObj(src->GetClass()->GetUID());
	if (!p)
		return NULL;
	p->_src=src;
	p->_param=param;

	return _CreateObj(p);
}

CLevelItem *CLevel::CreateItem(ClassUID uid)
{
	CLevelItem *p=NewLevelItem(uid);
	if (!p)
		return NULL;

	p->_level=this;

	if (!p->Create())
	{
		Class_Delete(p);
		return NULL;
	}

	return p;

}
CLevelItem *CLevel::CreateItem(CClass *clss)
{
	return CreateItem(clss->GetUID());
}

void CLevel::AddToActives(CLevelObj *obj)
{
	if (obj->IsAlive())
	{
		if (!obj->IsActive())
		{
			obj->Activate();
			obj->AddRef();
			_actives.push_back(obj);
		}
	}
}

void CLevel::_RefreshPlayerIDs()
{
	_nPlayers=0;
	for (int i=0;i<LEVEL_MAX_PLAYER;i++)
	{
		if (GetPlayer(i))
		{
			_idPlayers[_nPlayers]=(LevelPlayerID)i;
			_nPlayers++;
		}
	}
}

void CLevel::_RefreshPlayerIDMask()
{
	_maskPlayerID=0;
	for (int i=0;i<LEVEL_MAX_PLAYER;i++)
	{
		if (GetPlayer(i))
			_maskPlayerID|=(1<<i);
	}
}

void CLevel::CreatePlayer(LevelPlayerID idPlayer,LevelPos&center,LevelPlayerStates *lps)
{
	CLoUnit*unit=(CLoUnit*)CreateObj(Class_Ptr2(CLoUnit)->GetUID());

	RecordID idPlayerUnit=1;//目前主角的tid为1
	if (TRUE)
	{
		CLevelRecords *records=GetRecords();
		if (records)
		{
			LevelRecordGlobal *recGlobal=records->GetGlobal();
			if (recGlobal)
				idPlayerUnit=recGlobal->idPlayerUnit;
		}
	}
	unit->PostCreate(idPlayer,lps,idPlayerUnit,0,NULL,EquipSetPick_None,center);

	CreatePlayer(idPlayer,unit,lps);
	SAFE_RELEASE(unit);
}

void CLevel::CreatePlayer(LevelPlayerID idPlayer,CLevelObj *lo,LevelPlayerStates *lps)
{
	CLevelPlayer *player=&_players[idPlayer];

	AddToActives(lo);

	_players[idPlayer].Init(this,idPlayer,(CLoUnit*)lo,lps);
	_RefreshPlayerIDs();
	_RefreshPlayerIDMask();

	//创建这个Player对应的NPC
	if (TRUE)
	{
		CNPCPendings *pendingsNPC=_world->GetNPCPendings(idPlayer);
		if (pendingsNPC)
		{
			CNPCPendings::Pendings *pendings=pendingsNPC->GetMapPendings(_idMap);
			if (pendings)
			{
				for (int i=0;i<pendings->buf.size();i++)
					_world->DestroyNPC(idPlayer,pendings->buf[i].idNPC);

				_npcs[idPlayer].Create(this,idPlayer,pendings);
 				pendingsNPC->EraseMapPending(_idMap);
			}
		}
	}

	_players[idPlayer].OnEnterLevel();

	_ais.OnPlayerEnter();
	if (TRUE)
	{
		LevelHook_PlayerEnterLevel hk;
		hk.idPlayer=idPlayer;
		GetHooks()->SendHook(hk);
	}
}

void CLevel::DestroyPlayer(LevelPlayerID idPlayer)
{
	if (idPlayer>=ARRAY_SIZE(_players))
		return;

	CLevelPlayer*player=&_players[idPlayer];
	if (player->IsEmpty())
		return;

	if (TRUE)
	{
		LevelHook_PlayerLeaveLevel hk;
		hk.idPlayer=idPlayer;
		GetHooks()->SendHook(hk);
	}
	_ais.OnPlayerLeave();

	player->OnLeaveLevel();

// 	_npcs[idPlayer].Destroy();

	//把大家标记为都不在自己视野中,中断Talk
	LevelPlayerMask mask=player->GetPlayerMask();
	_aovmap.ClearPlayerAov(mask,player->GetAovCenter());
	for (int i=0;i<_actives.size();i++)
	{
		CLevelObj *p=_actives[i];
		if (p)
		{
			p->RemovePlayerMask(mask);
			p->BreakTalk(idPlayer);
		}
	}

	player->_lo->DeferDestroy();

	player->Clear();
	_RefreshPlayerIDs();
	_RefreshPlayerIDMask();

}

void CLevel::DestroyNPCs(LevelPlayerID id,RecordID idNPC)
{
	if (id<LEVEL_MAX_PLAYER)
		_npcs[id].Destroy(idNPC);
}

CLevelNPCs *CLevel::GetNPCs(LevelPlayerID id)
{
	if (id<LEVEL_MAX_PLAYER)
		return &_npcs[id];
	return NULL;
}

CLevelPlayer *CLevel::GetPlayer(LevelPlayerID id)
{
	if (id>=ARRAY_SIZE(_players))
		return NULL;
	if (_players[id].IsEmpty())
		return NULL;
	return &_players[id];
}

LevelPlayerStates *CLevel::GetLPS(LevelPlayerID id)
{
	CLevelPlayer *player=GetPlayer(id);
	if (player)
		return player->GetLPS();
	return NULL;
}

LevelRelationMatrix *CLevel::GetRelationMatrix()
{
	return _world?_world->GetRelationMatrix():NULL;
}

void CLevel::HandlePlayerMove(LevelPlayerID id,ServerSecond t,PlayerMove &move,PlayerMoveReply &reply)
{
	CLevelPlayer *player=GetPlayer(id);
	if (player)
	{
		if (!player->IsEmpty())
			player->GetMove().HandleMove(move,t,reply);
	}
}


BOOL CLevel::HandlePlayerSkill(LevelPlayerID id,PlayerSkill &skill,LevelSkillArg *arg)
{
	BOOL bRet=FALSE;
	CLevelPlayer *player=GetPlayer(id);
	if (player)
	{
		if (!player->IsEmpty())
			bRet=player->HandleSkill(skill,arg);
	}

	return bRet;
}

BOOL CLevel::HandlePlayerSkillCasted(ClientSkillID idClient)
{
	CLevelSkill *skill=_skills.FindSkillByClientID(idClient);
	if (skill)
	{
		skill->NotifyCasted();
		return TRUE;
	}
	return FALSE;
}

BOOL CLevel::HandlePlayerSkillCombine(ClientSkillID idClient,LevelSkillTarget &target)
{
	CLevelSkill *skill=_skills.FindSkillByClientID(idClient);
	if (skill)
	{
		CLevelObj *obj=skill->GetOwner();
		CLevelSkillDriver *driver=obj->GetSkillDriver();
		if (driver)
			return driver->Combine(target,idClient);
	}
	return FALSE;
}

BOOL CLevel::HandlePlayerSkillStopCasting(LevelPlayerID idPlayer,ClientSkillID idClient,AnimTick tCasting)
{
	if (idClient!=ClientSkillID_Invalid)
	{
		CLevelSkill *skill=_skills.FindSkillByClientID(idClient);
		if (skill)
		{
			CLevelObj *obj=skill->GetOwner();
			CLevelSkillDriver *driver=obj->GetSkillDriver();
			if (driver)
				driver->StopCast(tCasting);
		}
	}
	else
	{
		CLevelPlayer *player=GetPlayer(idPlayer);
		if (player)
		{
			CLoUnit *loUnit=player->GetLoUnit();
			if (loUnit)
			{
				CLevelSkillDriver *driver=loUnit->GetSkillDriver();
				if (driver)
					driver->StopCast(ANIMTICK_INFINITE);
			}
		}

	}
	return TRUE;
}


BOOL CLevel::CheckNeedUpdate(ServerSecond second,BOOL &bSubFrame)
{
	if (second>=_secondServer+LEVEL_SUBFRAME_INTERVAL)
	{
		if (_iSubFrame==LEVEL_SUBFRAME_COUNT-1)
			bSubFrame=FALSE;
		else
			bSubFrame=TRUE;
		return TRUE;
	}
	return FALSE;
}

void CLevel::UpdateSubFrame()
{
	_secondServer+=LEVEL_SUBFRAME_INTERVAL;
	_t+=LEVEL_SUBFRAME_TICK;
	_iSubFrame=(_iSubFrame+1)%LEVEL_SUBFRAME_COUNT;
	assert(_iSubFrame!=0);

	if (TRUE)
	{
		int c=0;
		for (int i=0;i<_activesSubframe.size();i++)
		{
			CLevelObj *lo=_activesSubframe[i];
			if (!lo->IsAlive())
			{
				SAFE_RELEASE(lo);
				continue;
			}

			lo->UpdateSubframe();
			_activesSubframe[c++]=lo;
		}

		_activesSubframe.resize(c);
	}

	_skills.Update();

}

 
void CLevel::UpdateFrame(ServerSecond second,CLevelMsgBuf *msgbuf,std::vector<CLevelObj *>&unsyncs)
{
	unsyncs.clear();

	if (TRUE)
	{
		if (_secondServer==-1.0f)
			_secondServer=second;
		else
			_secondServer+=LEVEL_SUBFRAME_INTERVAL;
		_t+=LEVEL_SUBFRAME_TICK;
		_iSubFrame=(_iSubFrame+1)%LEVEL_SUBFRAME_COUNT;
		assert(_iSubFrame==0);
	}

	_msgbuf=msgbuf;
	for (int i=0;i<LEVEL_MAX_PLAYER;i++)
	{
		msgbuf[i].GetFrameBP()->Reset();
		msgbuf[i].GetActionBP()->Reset();
	}

	//更新Player的aov和aoa
	ProfilerStart_Recent(UpdateAovAoa);
	for (int i=0;i<ARRAY_SIZE(_players);i++)
	{
		CLevelPlayer*player=&_players[i];
		if (player->IsEmpty())
			continue;
		LevelPos pos=player->_lo->GetFramePos();
		_aovmap.UpdatePlayerAov(player->GetPlayerMask(),player->GetAovCenter(),pos);
		_inactives.UpdatePlayerAoa(player->GetAoaCenter(),pos,this);
		player->UpdateExlporeMap();
	}
	ProfilerEnd();

	ProfilerStart_Recent(UpdateSight);
	//先更新哪些obj离开了player的视野
	for (int i=0;i<_actives.size();i++)
	{
		CLevelObj *obj=_actives[i];

		if (!obj->IsAlive())
			continue;

		LevelPos pos=obj->GetFramePos();

		//看这个obj不在哪些player的视野中了
		if (TRUE)
		{
			LevelPlayerMask maskActual;
			if (obj->IsGlobalSight())
				maskActual=0xffff;
			else
				maskActual=_aovmap.GetPlayerMask(pos);
			if (TRUE)
			{
				LevelPlayerID idVisibleTo=obj->GetOnlyVisible();
				if (idVisibleTo!=LevelPlayerID_Invalid)
					maskActual&=(1<<idVisibleTo);
			}

			LevelPlayerMask maskCur=obj->GetPlayerMask();

			if (obj->_bDeferDestroy)
			{
				maskActual=0;//马上要删除了,将不出现在任何Player的视野中
			}
			if (obj->IsServerOnly())
				maskActual=0;//不需要同步到client

			//从哪些player的视野中离开
			LevelPlayerMask mask;
			mask=maskCur;
			mask&=~maskActual;
			while(mask!=0)
			{
				LevelPlayerID id=i_math::fastlog2(mask);
				mask&=~(LevelPlayerMask)(1<<id);

				if (id>=LEVEL_MAX_PLAYER)
					continue;
				if (_players[id].IsEmpty())
					continue;
				
				_players[id]._leaves.push_back(obj);//加到即将离开视野的队列中去
			}

// 			for (int i=0;i<ARRAY_SIZE(_players);i++)
// 			{
// 				CLevelPlayer*player=&_players[i];
// 				if (_players[i].IsEmpty())
// 					continue;
// 
// 				CBitPacket *bp=_msgbuf[i].GetFrameBP();
// 
// 				player->WriteFrameLeaveSight(bp);
// 			}
		}
	}
	for (int i=0;i<ARRAY_SIZE(_players);i++)
	{
		CLevelPlayer*player=&_players[i];
		if (_players[i].IsEmpty())
			continue;

		CBitPacket *bp=_msgbuf[i].GetFrameBP();

		player->WriteFrameLeaveSight(bp);
	}

	ProfilerEnd();

	//销毁 bDeferDestroy的obj
	if (TRUE)
	{
		_destroys.clear();
		for (int i=0;i<_actives.size();i++)
		{
			CLevelObj *obj=_actives[i];
			if (!obj->IsAlive())
				continue;

			if (obj->_bDeferDestroy)
			{
				obj->AddRef();
				_destroys.push_back(obj);
			}
		}

		for (int i=0;i<_destroys.size();i++)
		{
			CLevelObj *obj=_destroys[i];
			obj->AddRef();
			obj->Destroy();
			obj->_bDeferDestroy=0;

			SAFE_RELEASE(obj);
		}
		_destroys.clear();
	}


	_mpEvent.Update();

	//更新Player的Unit的next位置,
	//注意在所有的Lo的Update之前更新,这样保证玩家的移动能最快的得到响应(主要是随从对玩家移动的响应)
	for (int i=0;i<ARRAY_SIZE(_players);i++)
	{
		CLevelPlayer*player=&_players[i];
		if (player->IsEmpty())
			continue;
		player->GetMove().UpdatePosAndFace(_secondServer+LEVEL_FRAME_INTERVAL);


// 		if(player->GetRtnuCircum())
// 			player->GetRtnuCircum()->PreSimulate();
	}

	//更新Player的Service
	for (int i=0;i<ARRAY_SIZE(_players);i++)
	{
		CLevelPlayer*player=&_players[i];
		if (player->IsEmpty())
			continue;

// 		for (int j=LevelService_None+1;j<LevelService_Max;j++)
// 		{
// 			CLevelService *service=player->GetService((LevelServiceType)j);
// 			if (service)
// 				service->Update();
// 		}
	}


	ProfilerStart_Recent(UpdateActives);
	//对_actives进行Update(),并且检查哪些进入了player的视野
	if (TRUE)
	{
		BOOL bAnyEnter[LEVEL_MAX_PLAYER];
		memset(bAnyEnter,0,sizeof(bAnyEnter));

		int i=0;
		while(i<_actives.size())
		{
			CLevelObj *obj=_actives[i];
			i++;
			if (!obj->IsAlive())
				continue;

			if (TRUE)
			{
				extern LevelPlayerMask LevelUtil_GetActualPlayerMask(CLevelObj *lo);
				LevelPlayerMask maskActual=LevelUtil_GetActualPlayerMask(obj);

				LevelPlayerMask maskCur=obj->GetPlayerMask();

				//新进入哪些player的视野
				LevelPlayerMask mask=maskActual;
				mask&=~maskCur;
				while(mask!=0)
				{
					LevelPlayerID id=i_math::fastlog2(mask);
					mask&=~(LevelPlayerMask)(1<<id);

					if (id>=LEVEL_MAX_PLAYER)
						continue;
					if (_players[id].IsEmpty())
						continue;

					CBitPacket *bp=_msgbuf[id].GetFrameBP();

					if (!bAnyEnter[id])
					{
						bAnyEnter[id]=TRUE;
						bp->Data_WriteSimple(LEVELFRAME_ENTERSIGHT);
					}

					DWORD idx=_players[id].AddSightEnter(obj);

					bp->Data_WriteSimple(obj->GetID());//ID
					assert((obj->GetClass()->GetUID())<250);
					bp->Data_WriteSimple((BYTE)(obj->GetClass()->GetUID()));//UID
					if (TRUE)
					{
						CLevelObjSrc*los=obj->GetLos();
						if (los)
						{
							if (los->NeedSyncGUID())
							{
								bp->Bit_Write_1();
								bp->Data_WriteSimple(los->GetGUID());
							}
							else
								bp->Bit_Write_0();
						}
						else
							bp->Bit_Write_0();
					}
					bp->Bits_Write(idx,12);

				}

				obj->SetPlayerMask(maskActual);
			}

			obj->Update();//注意:可能会产生新的obj,会加在_actives的末尾,我们会在后续的loop里处理它们 
		}

		for (int i=0;i<LEVEL_MAX_PLAYER;i++)
		{
			if (bAnyEnter[i])
				_msgbuf[i].GetFrameBP()->Data_WriteSimple(LevelObjID_Invalid);//终止id
		}

	}

	ProfilerEnd();

	DWORD nSyncActives=_actives.size();
	//之后的更新可能会产生新的obj,这些obj没有被标记为在任何player的视野中,所以不会被同步,它们会被返回在unsyncs数组中
	
	for (int i=0;i<_nPlayers;i++)
	{
		LevelPlayerID idPlayer=_idPlayers[i];
		_npcs[idPlayer].Update();
	}


	ProfilerStart_Recent(UpdateSkills);
	_skills.Update();
	ProfilerEnd();

	//更新Player的其它状态
	for (int i=0;i<ARRAY_SIZE(_players);i++)
	{
		CLevelPlayer*player=&_players[i];
		if (player->IsEmpty())
			continue;
		player->UpdateAbilities();
		player->UpdateHPSP_New((float)LEVEL_FRAME_INTERVAL);
		player->UpdateRtnuCmds();
	}

	_ais.Update();

	//更新非Player的Unit的next位置
	ProfilerStart_Recent(UnitMgr_Update);
	_unitmgr.Update((float)LEVEL_FRAME_INTERVAL);
	_unit3dmgr.Update((float)LEVEL_FRAME_INTERVAL);
	ProfilerEnd();

	for (int k=0;k<RVO_SIMULATOR_ITERATE_COUNT;k++)
	{
		for (int i=0;i<ARRAY_SIZE(_players);i++)
		{
			CLevelPlayer*player=&_players[i];
			if (player->IsEmpty())
				continue;
			if(player->GetRtnuCircum())
				player->GetRtnuCircum()->PreSimulate();
		}

		ProfilerStart_Recent(RvoSimulation);
		_simRvo.doStep();
		ProfilerEnd();

		for (int i=0;i<ARRAY_SIZE(_players);i++)
		{
			CLevelPlayer*player=&_players[i];
			if (player->IsEmpty())
				continue;
			if(player->GetRtnuCircum())
				player->GetRtnuCircum()->PostSimulate();
		}
	}

	for (int i=0;i<ARRAY_SIZE(_players);i++)
	{
		CLevelPlayer*player=&_players[i];
		if (player->IsEmpty())
			continue;
		player->UpdateRtnus();
	}

	if (TRUE)
	{
		std::unordered_map<LevelServiceType,CLevelService *>::iterator it;
		for (it=_services.begin();it!=_services.end();it++)
			((*it).second)->Update();
	}

	ProfilerStart_Recent(WriteFrameSync);
	//写入要发给每个Player的FrameSync消息数据
	for (int i=0;i<LEVEL_MAX_PLAYER;i++)
	{
		CLevelPlayer*player=&_players[i];
		if (_players[i].IsEmpty())
			continue;

		player->WriteFrameSync(_msgbuf[i].GetFrameBP());
	}
	ProfilerEnd();

	ProfilerStart_Recent(PostWriteFrameSync);
	for (int i=0;i<nSyncActives;i++)
	{
		CLevelObj *obj=_actives[i];

		if (!obj->IsAlive())
			continue;

		obj->PostWriteSync();
	}
	ProfilerEnd();

	//Frame消息的终止符
	for(int i=0;i<LEVEL_MAX_PLAYER;i++)
	{
		CBitPacket *bp=_msgbuf[i].GetFrameBP();
		if (!bp->IsEmpty())
		{//如果有数据要发送给这个player的话,我们要添加一个终止符
			bp->Data_WriteSimple(LEVELFRAME_NULL);
		}
	}

	_msgbuf=NULL;

	for(int i=nSyncActives;i<_actives.size();i++)
	{
		CLevelObj *lo=_actives[i];
		if (lo->IsAlive())
			continue;
		if (lo->IsDeferDestroy())
			continue;
		SAFE_ADDREF(lo);
		unsyncs.push_back(lo);
	}

	_dbgdraw.Update();
}


void CLevel::EnumLo(LevelPos &center,float radius)
{
	CUnitMap *mp=_unitmgr.GetMap();

	mp->Enum(center,radius);

	DWORD c;
	CUnitBase **units=(CUnitBase **)mp->GetEnums(c);

	_enums.reserve(c);
	_enums.clear();

	double radius2=(double)(radius*radius);

	for (int i=0;i<c;i++)
	{
		CUnit *unit=(CUnit *)units[i];
		CLevelObj*lo=(CLevelObj*)unit->GetData();
		if (!lo)
			continue;
		if (!lo->IsAlive())
			continue;

		if ((unit->GetPos()-center).getLengthSQ()<radius2)
			_enums.push_back(lo);
	}
}

void CLevel::_ClearAffects()
{
	for (int i=0;i<_affects.size();i++)
	{
		CLevelObj *lo=_affects[i];
		lo->_bAffect=0;
	}
	_affects.clear();
}

CLevelBehavior *CLevel::CreateBehavior(StringID nmBG,LevelBehaviorContext &ctx)
{
	CBehaviorGraph *bg=_bgs->FindBG(nmBG);
	if (!bg)
	{
		LOG_DUMP_1P("CLevel",Log_Error,"无法找到名为\"%s\"的行为图!",StrLib_GetStr(nmBG));
		return NULL;
	}

	CLevelBehavior *bhv=Class_New2(CLevelBehavior);

	ctx.bg=bg;
	ctx.behavior=bhv;
	ctx.level=this;

	bhv->Init(ctx);

	return bhv;
}

BOOL CLevel::HandleTalkOp(LevelPlayerID idPlayer,CBTalkOp &msg)
{
	CLevelObj *lo=GetIDs()->LoFromID(msg.id);
	if (lo)
	{
		CLevelTalks *talks=lo->GetTalks();
		if (talks)
		{
			if (msg.op==CBTalkOp::Query)
				talks->Query(idPlayer);
			if (msg.op==CBTalkOp::Break)
				talks->ClearActive(idPlayer);
			if (msg.op==CBTalkOp::Accept)
				talks->Accept(idPlayer,msg.choose);
			if (msg.op==CBTalkOp::DialogCmd)
				talks->DoDlgCmd(idPlayer,msg.cmdDlg);

			talks->SetDirty(idPlayer);//有可能失败,我们设置为Dirty给Client一个回复,

			AddAffect(lo);

			return TRUE;
		}

	}
	return FALSE;
}

void CLevel::AddTeleportQuest(LevelTeleportQuest *questTP)
{		
	BOOL bOk=FALSE;

	BOOL bValidQuest=TRUE;
	if (TRUE)
	{
		if (!questTP->loPlayer->IsAlive())
			bValidQuest=FALSE;
		else
		{
			//检查这个player是否已经在等待teleport的队列里了
			LevelPlayerID idPlayer=questTP->loPlayer->GetPlayerID();
			for (int i=0;i<_questsTP.size();i++)
			{
				LevelTeleportQuest *q=_questsTP[i];
				if (q->loPlayer->GetPlayerID()==idPlayer)
				{
					bValidQuest=FALSE;
					break;
				}
			}
		}
	}

	if (bValidQuest)
	{
		if (questTP->loPlayer)
		{
			CWorldData *data=_world->GetData();
			if (data)
			{
				CLevelData *dataLevel=data->FindLevel(questTP->idMap);
				if (dataLevel)
				{
					if (questTP->bSitePos)
					{
						BCPreTeleport msg;
						msg.idMap=questTP->idMap;
						msg.pos=questTP->posSite;

						SendNetMsg(questTP->loPlayer->GetPlayerID(),&msg);
						bOk=TRUE;
					}
					else
					{
						LevelTeleportSite *tpsite=dataLevel->GetBasis()->FindTeleportSite(questTP->idSite);
						if (tpsite)
						{
							BCPreTeleport msg;
							msg.idMap=questTP->idMap;
							msg.pos=tpsite->pos;

							SendNetMsg(questTP->loPlayer->GetPlayerID(),&msg);
							bOk=TRUE;
						}
					}
				}
			}
		}
	}
	if (bOk)
		_questsTP.push_back(questTP);	
	else
	{
		questTP->Clear();
		Safe_Class_Delete(questTP);
	}
}

BOOL CLevel::HandleGatherItem(LevelPlayerID idPlayer,LevelObjID idItem)
{
	CLevelPlayer *player=GetPlayer(idPlayer);
	if (!player)
		return TRUE;

	CLevelObj *lo=player->GetLoUnit();
	if (!lo)
		return TRUE;

	AddAffect(lo);
	_decider.GatherRes(lo,idItem);

	return TRUE;
}

BOOL CLevel::HandleGatherResPile(LevelPlayerID idPlayer,CBGatherResPile &msg)
{
	CLevelPlayer *player=GetPlayer(idPlayer);
	if (!player)
		return TRUE;

	CLevelObj *lo=player->GetLoUnit();
	if (!lo)
		return TRUE;

	AddAffect(lo);
	_decider.GatherResPile(lo,msg.idOwner,msg.tp,msg.amount);

	return TRUE;
}




BOOL CLevel::HandleInvokeMagicBoard(LevelPlayerID idPlayer,MagicBoardInvoke&invoke)
{
	CLoMagicBoard *lo=_ids.LoFromID<CLoMagicBoard>(invoke.id);
	if (!lo)
		return FALSE;

	lo->Invoke(idPlayer,invoke);

	return TRUE;
}

BOOL CLevel::HandleFlipSlate(LevelPlayerID idPlayer,LevelObjID idSlates,LevelSlateIdx idxSlate)
{
	CLoSlatesA *loSlates=GetIDs()->LoFromID<CLoSlatesA>(idSlates);
	if (loSlates)
		loSlates->RequestFlipFromClient(idPlayer,idxSlate);

	return TRUE;
}

BOOL CLevel::HandleIncSlateButtonChip(LevelPlayerID idPlayer,LevelObjID idSlates,LevelSlateIdx idxSlate)
{
	CLoSlatesA *loSlates=GetIDs()->LoFromID<CLoSlatesA>(idSlates);
	if (loSlates)
		loSlates->RequestIncSlateButtonChip(idPlayer,idxSlate);

	return TRUE;
}


void CLevel::RegisterUniqueObj(LevelUniqueObjType tp,CLevelObj *lo)
{
	SAFE_REPLACE(_uos[(int)tp],lo);
}
void CLevel::UnregisterUniqueObj(LevelUniqueObjType tp,CLevelObj *lo)
{
	if (lo)
	{
		if (lo==_uos[(int)tp])
			SAFE_RELEASE(_uos[(int)tp]);
	}
}

CLevelObj *CLevel::GetUniqueObj(LevelUniqueObjType tp)
{
	if(!_uos[(int)tp])
		return NULL;
	return _uos[(int)tp]->IsAlive()?_uos[(int)tp]:NULL;	
}

void CLevel::RegisterEoEnv(CLevelObj *eo)
{
	SAFE_REPLACE(_eoEnv,eo);
}

void CLevel::UnRegisterEoEnv(CLevelObj *eo)
{
	if (eo==_eoEnv)
		SAFE_RELEASE(_eoEnv);
}

void CLevel::RegisterSubframeUpdate(CLevelObj *lo)
{
	SAFE_ADDREF(lo);
	_activesSubframe.push_back(lo);
}

CLevelService *CLevel::ObtainService(LevelServiceType tp)
{
	std::unordered_map<LevelServiceType,CLevelService *>::iterator it=_services.find(tp);
	if (it!=_services.end())
		return (*it).second;

	CLevelService *service=Class_New2(CLevelService);
	service->Init(this);
	_services[tp]=service;

	return service;
}


CLevelService *CLevel::GetService_(LevelServiceType tp)
{
	std::unordered_map<LevelServiceType,CLevelService *>::iterator it=_services.find(tp);
	if (it==_services.end())
		return NULL;
	return (*it).second;
}


void CLevel::RegisterCentipede(CLevelObj *lo)
{
	SAFE_ADDREF(lo);
	_loCentipedes.insert(lo);
}

void CLevel::UnRegisterCentipede(CLevelObj *lo)
{
	std::set<CLevelObj *>::iterator it=_loCentipedes.find(lo);
	if (it!=_loCentipedes.end())
	{
		SAFE_RELEASE(lo);
		_loCentipedes.erase(it);
	}
}

CLevelObj *CLevel::Get1stCentipede()
{
	if (_loCentipedes.size()<=0)
		return NULL;
	return *_loCentipedes.begin();
}



//////////////////////////////////////////////////////////////////////////
//CWorld
void CJjWorld::Create(NetProxy *net,CWorldData *data,CLevelRecords *records,CLevelResources *resources,CLevelDropper *dropper,CLevelBGs *bgs)
{
	_net=net;
	_data=data;
	_records=records;
	_resources=resources;
	_dropper=dropper;
	_bgs=bgs;

	_idUnique=(int)GetTSC();
}

void CJjWorld::_ClearAllLevel()
{
	std::unordered_map<RecordID,CLevel *>::iterator it;
	for (it=_levels.begin();it!=_levels.end();it++)
	{
		CLevel *lvl=(*it).second;
		lvl->Destroy();
		Safe_Class_Delete(lvl);
	}
	_levels.clear();
	_levels2.clear();
}


void CJjWorld::Destroy()
{
	EndDay(TRUE);
	for (int i=0;i<ARRAY_SIZE(_players);i++)
	{
		if (_players[i].states)
			LeavePlayer((LevelPlayerID)i);
	}

	_ClearAllLevel();
	Zero();
}


CLevel *CJjWorld::LevelFromPlayer(LevelPlayerID idPlayer)
{
	if (idPlayer>=ARRAY_SIZE(_players))
		return NULL;

	return _players[idPlayer].level;
}

CLevelPlayer *CJjWorld::LevelPlayerFromID(LevelPlayerID idPlayer)
{
	CLevel *level=LevelFromPlayer(idPlayer);
	if (!level)
		return NULL;
	return level->GetPlayer(idPlayer);
}

LevelPlayerStates *CJjWorld::GetLPS(LevelPlayerID idPlayer)
{
	if (idPlayer>=ARRAY_SIZE(_players))
		return NULL;

	return _players[idPlayer].states;
}

CNPCPendings *CJjWorld::GetNPCPendings(LevelPlayerID idPlayer)
{
	if (idPlayer>=ARRAY_SIZE(_players))
		return NULL;

	return &_players[idPlayer].pendingsNPC;
}

LevelPlayerID CJjWorld::GetPlayerIDFromLPS(LevelPlayerStates *lps)
{
	LevelPlayerID idPlayer=LevelPlayerID_Invalid;
	for (int i=0;i<ARRAY_SIZE(_players);i++)
	{
		if (_players[i].states==lps)
			return (LevelPlayerID)i;
	}
	return LevelPlayerID_Invalid;
}

LevelPlayerID CJjWorld::EnterPlayer(LevelPlayerStates *lps)
{
	//分配一个PlayerID
	LevelPlayerID idPlayer=LevelPlayerID_Invalid;
	for (int i=0;i<ARRAY_SIZE(_players);i++)
	{
		if (!_players[i].states)
		{
			idPlayer=(LevelPlayerID)i;
			break;
		}
	}
	if (idPlayer==LevelPlayerID_Invalid)
		return LevelPlayerID_Invalid;//已满

	_players[idPlayer].states=lps;
	_players[idPlayer].level=NULL;
	_players[idPlayer].pendingsNPC.Init(this,idPlayer);

	LevelHook_PlayerEnterWorld hk;
	hk.idPlayer=idPlayer;
	if (TRUE)
	{
		std::unordered_map<RecordID,CLevel *>::iterator it;
		for (it=_levels.begin();it!=_levels.end();it++)
		{
			CLevel *level=(*it).second;
			level->GetHooks()->SendHook(hk);
		}
	}

	return idPlayer;
}

void CJjWorld::DestroyNPC(LevelPlayerID idPlayer,RecordID idNPC)
{
	std::unordered_map<RecordID,CLevel *>::iterator it;
	for (it=_levels.begin();it!=_levels.end();it++)
	{
		CLevel *level=(*it).second;
		level->DestroyNPCs(idPlayer,idNPC);
	}

}


void CJjWorld::StartDay()
{
	for (int i=0;i<ARRAY_SIZE(_players);i++)
	{
		if (!_players[i].states)
			continue;
		LevelPlayerStates *lps=_players[i].states;

		RecordID idMap=lps->base.idMap;

		CLevelData *dataLevel=_data->FindLevel(idMap);
		if (!dataLevel)
			continue;

		LevelPos center;
		center=lps->base.pos;
		if (center==LevelPos_Invalid)
		{
			LevelLoc *loc=dataLevel->GetBasis()->FindLoc(StringID_Invalid);
			if (loc)
				center=loc->pos;
		}


		CLevel *level=_ObtainLevel(idMap);
		if (!level)
			continue;

		LevelPlayerID idPlayer=(LevelPlayerID)i;
		_players[idPlayer].states=lps;
		_players[idPlayer].level=level;
		_players[idPlayer].emsSent.insert(idMap);//这个level的ExploreMap应该已经在之前发送过了
		_players[idPlayer].pendingsNPC.Init(this,idPlayer);

		level->CreatePlayer(idPlayer,center,lps);

		CLevelPlayer *player=level->GetPlayer(idPlayer);

		//创建这个Player的Retinues
		if (TRUE)
		{
			CLevelRecords *records=level->GetRecords();
			CCircumSites &csites=dataLevel->GetBasis()->GetDefCircumSites();
			csites.BeginGen();
			for (int i=0;i<ARRAY_SIZE(lps->rtnusets);i++)
			{
				LPSRetinueSet *rtnuset=&lps->rtnusets[i];
				std::unordered_map<RetinueUID,LPSRetinueData*>::iterator it;
				for (it=rtnuset->datas.begin();it!=rtnuset->datas.end();it++)
				{
					LPSRetinueData *p=(*it).second;
					if (!p)
						continue;

					LevelPos *pos=csites.Gen();
					assert(pos);
					if (!pos)
						continue;

					if (player->GetRtnus())
						player->GetRtnus()->Add_FromData(p,center+*pos);
				}
			}
		}

		player->CreateRtnuNPCs();
	}

	for (int i=0;i<ARRAY_SIZE(_players);i++)
	{
		LevelPlayerID idPlayer=(LevelPlayerID)i;

		CLevelPlayer *player=LevelPlayerFromID(idPlayer);
		if (player)
			player->HandleStartDay();
	}
}

BOOL CJjWorld::EndDay(BOOL bBreak)
{
	//正常结束要检测一下是否能够结束
	if (!bBreak)
	{
		BOOL bCanEnd=TRUE;

		if (!bCanEnd)
			return FALSE;
	}

	for (int i=0;i<ARRAY_SIZE(_players);i++)
	{
		LevelPlayerID idPlayer=(LevelPlayerID)i;

		CLevelPlayer *player=LevelPlayerFromID(idPlayer);
		if (player)
			player->HandleEndDay();
	}


	for (int i=0;i<ARRAY_SIZE(_players);i++)
	{
		if (!_players[i].states)
			continue;
		LevelPlayerStates *lps=_players[i].states;

		LevelPlayerID idPlayer=(LevelPlayerID)i;


		CLevel *lvl=LevelFromPlayer(idPlayer);

		//记录下当前的位置
		if (lvl)
		{
			CLevelPlayer *player=lvl->GetPlayer(idPlayer);
			if (player->GetLoUnit())
			{
				lps->base.pos=player->GetLoUnit()->GetFramePos();
				lps->base.idMap=lvl->GetMapID();
				lps->base.SetDirtyDB_Urgent();
			}
		}

		if (lvl)
			lvl->DestroyPlayer(idPlayer);

		_players[i].level=NULL;

		//在所有Level中清除属于这个Player的NPC
		for (int i=0;i<_levels2.size();i++)
			_levels2[i]->DestroyNPCs(idPlayer,RecordID_Invalid);

		if (!bBreak)
		{
			lps->base.iDay++;
			lps->base.SetDirtyDB_Urgent();
		}
	}

	_ClearAllLevel();

	return TRUE;
}


void CJjWorld::LeavePlayer(LevelPlayerID idPlayer)
{
	LevelHook_PlayerLeaveWorld hk;
	hk.idPlayer=idPlayer;
	if (TRUE)
	{
		std::unordered_map<RecordID,CLevel *>::iterator it;
		for (it=_levels.begin();it!=_levels.end();it++)
		{
			CLevel *level=(*it).second;
			level->GetHooks()->SendHook(hk);
		}
	}

	DestroyNPC(idPlayer,RecordID_Invalid);


	_players[idPlayer].Clear();

}


BOOL CJjWorld::ExistPlayer(LevelPlayerID idPlayer)
{
	return LevelFromPlayer(idPlayer)!=NULL;
}

CLevel *CJjWorld::FindLevel(RecordID idMap)
{
	std::unordered_map<RecordID,CLevel *>::iterator it=_levels.find(idMap);
	if (it!=_levels.end())
		return (*it).second;
	return NULL;
}


CLevel *CJjWorld::_ObtainLevel(RecordID idMap)
{
	std::unordered_map<RecordID,CLevel *>::iterator it=_levels.find(idMap);
	if (it!=_levels.end())
		return (*it).second;

	CLevelData *data=_data->FindLevel(idMap);
	if (!data)
		return NULL;
	CLevel *lvl=Class_New2(CLevel);
	lvl->Create(data,idMap,_records,_resources,_dropper,_bgs,this);

	_levels[idMap]=lvl;
	_levels2.push_back(lvl);

	return lvl;
}

void CJjWorld::AcceptTeleport(LevelPlayerID idPlayer)
{
	CLevel *lvl=LevelFromPlayer(idPlayer);
	if (!lvl)
		return;

	for (int j=0;j<lvl->_questsTP.size();j++)
	{
		LevelTeleportQuest *q=lvl->_questsTP[j];
		if (q->loPlayer)
		{
			if (q->loPlayer->GetPlayerID()==idPlayer)
			{
				q->bAccept=TRUE;
				return;
			}
		}
	}
}


void CJjWorld::FlushTeleportQuest()
{
	int i=0;
	while(i<_levels2.size())
	{
		CLevel *lvl=_levels2[i];
		i++;
		DWORD nQuests=0;
		for (int j=0;j<lvl->_questsTP.size();j++)
		{
			LevelTeleportQuest *q=lvl->_questsTP[j];
			if (q->bAccept)//客户端确认了
			{
				if (q->tDoTeleport<lvl->GetT_())
				{//时间到了

					CLevel *lvlNew=_ObtainLevel(q->idMap);
					CCircumSites *csites=NULL;
					BOOL bPosSite=FALSE;
					LevelPos posSite;
					if (TRUE)
					{
						LevelTeleportSite *tpsite=lvlNew->GetData()->GetBasis()->FindTeleportSite(q->idSite);
						if (q->bSitePos)
						{
							posSite=q->posSite;
							bPosSite=TRUE;
						}
						else
						{
							if (tpsite)
							{
								posSite=tpsite->pos;
								bPosSite=TRUE;
							}
						}
						if (tpsite)
							csites=&tpsite->csites;
					}
					LevelPlayerID idPlayer=q->loPlayer->GetPlayerID();

					//看看能不能进行Teleport
					BOOL bCanTeleport=TRUE;
					if (bPosSite)
					{
						if (q->loPlayer)
						{
							if (q->loPlayer->IsAlive())
							{
								if (q->loPlayer->GetType()==LevelObjType_Unit)
								{
									if ((lvlNew)&&(idPlayer!=LevelPlayerID_Invalid)
										&&q->loPlayer->IsPlayer())
										bCanTeleport=TRUE;
								}
							}
						}
					}

					if (bCanTeleport)
					{
						//先把旧的player里所有的Retinue拿出来
						std::vector<CLevelRtnu*>rtnusOld;
						CLevelNPCs *npcsRtnuOld=NULL;
						if (TRUE)
						{
							CLevelPlayer *playerOld=lvl->GetPlayer(idPlayer);
							if (playerOld)
							{
								if (playerOld->GetRtnus())
									playerOld->GetRtnus()->FetchValidRetinues(rtnusOld);
								npcsRtnuOld=playerOld->FetchRtnuNPCs();
							}
						}

						//Move切换为非Rtnu模式
						if (TRUE)
						{
							for (int i=0;i<rtnusOld.size();i++)
							{
								if (!rtnusOld[i])
									continue;
								CLoUnit *lo=rtnusOld[i]->GetLo();
								if (lo)
								{
									CLevelObjMove *move=lo->GetMove();
									if (move)
										move->SwitchRtnuMode(FALSE,LevelRtnuRank_None);
								}
							}
						}


						//把旧的lo保存一个引用计数
						CLoUnit *loPlayerOld=(CLoUnit *)q->loPlayer;
						SAFE_ADDREF(loPlayerOld);

						//在旧的level中删除Player,注意旧的lo被defer destroy了
						lvl->DestroyPlayer(idPlayer);

						//在New Level创建新的主角的LevelObj
						CLoUnit *loPlayerNew=(CLoUnit *)lvlNew->CreateObj(q->loPlayer->GetClass());
						loPlayerNew->PostCreate_Teleport(loPlayerOld,posSite);//loPlayerOld被DeferDestroy了,但内部信息还在,可以用来创建新的LevelObj

						LevelPlayerStates *lps=GetLPS(idPlayer);
						if (lps)
						{
							lps->base.idMap=q->idMap;
							lps->base.pos=posSite;
							lps->base.SetDirtyDB_Urgent();
						}

						//loPlayerOld已经没用了
						SAFE_RELEASE(loPlayerOld);

						//在New Level中创建新的Player
						lvlNew->CreatePlayer(idPlayer,loPlayerNew,GetLPS(idPlayer));

						//在新的Player里创建Retinue(根据从旧Level中拿出来的Retinues)
						if (TRUE)
						{
							CLevelPlayer *playerNew=lvlNew->GetPlayer(idPlayer);
							if (playerNew)
							{
								BOOL bLocalSite=FALSE;
								if (!csites)
								{
									csites=&lvlNew->GetData()->GetBasis()->GetDefCircumSites();//没有足够的位点,使用缺省位点
									bLocalSite=TRUE;
								}
								else
								{
									if (csites->GetCapacity()<rtnusOld.size())
									{
										csites=&lvlNew->GetData()->GetBasis()->GetDefCircumSites();//没有足够的位点,使用缺省位点
										bLocalSite=TRUE;
									}
								}

								float rateGen=((float)rtnusOld.size())/(float)csites->GetCapacity();
								if (rateGen<0.75f)
									rateGen=0.75f;

								csites->BeginGen(rateGen);

								for (int i=0;i<rtnusOld.size();i++)
								{
									LevelPos *pos=csites->Gen();
									assert(pos);
									if (!pos)
										continue;

									LevelPos posRtnu=*pos;
									if (bLocalSite)
										posRtnu+=posSite;

									if (playerNew->GetRtnus())
										playerNew->GetRtnus()->Add_Teleport(rtnusOld[i],posRtnu);
								}
							}
						}

						//删除从旧的Level中拿出来的Retinues
						if (TRUE)
						{
							for (int i=0;i<rtnusOld.size();i++)
							{
								if (!rtnusOld[i])
									continue;
								rtnusOld[i]->Destroy();
								Safe_Class_Delete(rtnusOld[i]);
							}
							rtnusOld.clear();
						}

						//在新的Player里创建RetinueNPC 
						if (npcsRtnuOld)
						{
							CLevelPlayer *playerNew=lvlNew->GetPlayer(idPlayer);
							if (playerNew)
								playerNew->CreateRtnuNPCs_Teleport(npcsRtnuOld);
						}

						//删除从旧的Level中拿出来的Retinue NPC
						if (npcsRtnuOld)
						{
							npcsRtnuOld->Destroy(RecordID_Invalid);
							Safe_Class_Delete(npcsRtnuOld);
						}

						if (TRUE)//转移到新的level中
						{
							PlayerEntry *entry=&_players[idPlayer];
							entry->level=lvlNew;
						}

						//发送新的level的ExploreMap
						if (TRUE)
						{
							PlayerEntry *entry=&_players[idPlayer];
							if (entry->emsSent.find(q->idMap)==entry->emsSent.end())
							{//新地图的ExploreMap还没有发送给Client过
								LevelExploreMaps mps=LPS_FindExploreMaps(entry->states,q->idMap);
								if (!mps.IsEmpty())
								{
									SCExploreMapData msg;
									msg.idMap=q->idMap;
									DP_BeginSave(dp,msg.dataEMs);
									mps.sttc->Save(dp);
									mps.dyn->Save(dp);
									DP_EndSave();

									_net->SendMsg(idPlayer,&msg);
								}
								else
									LPS_NewExploreMaps(entry->states,q->idMap,lvlNew->GetMapRect());

								entry->emsSent.insert(q->idMap);//标记为已经发送过了
							}
						}

						//给Client发一个通知的消息
						if (TRUE)
						{
							BCTeleport msg;
							msg.idMap=q->idMap;
							msg.pos=posSite;
							msg.idPlayer=idPlayer;
							msg.idPlayerUnit=loPlayerNew->GetID();
							_net->SendMsg(idPlayer,&msg);
						}

						SAFE_RELEASE(loPlayerNew);//创建出来的Lo有一个引用计数,我们要释放它
					}

					q->Clear();
					Safe_Class_Delete(q);

					continue;//这个Quest已经处理完毕了,可以去掉了
				}
			}
			//这个Quest还没有处理完毕,
			lvl->_questsTP[nQuests]=q;
			nQuests++;
		}

		lvl->_questsTP.resize(nQuests);
	}
}

RecordID CJjWorld::FetchExploreMapToSend(LevelPlayerID idPlayer)
{
	if (idPlayer>=LEVEL_MAX_PLAYER)
		return RecordID_Invalid;
	PlayerEntry *entry=&_players[idPlayer];
	if (entry->emsToSend.size()>0)
	{
		RecordID idMap=entry->emsToSend[0];
		entry->emsToSend.pop_front();
		return idMap;
	}

	return RecordID_Invalid;
}


void CJjWorld::AddExploreMapToSend(LevelPlayerID idPlayer,RecordID idMap)
{
	if (idPlayer>=LEVEL_MAX_PLAYER)
		return;
	PlayerEntry *entry=&_players[idPlayer];
	entry->emsToSend.push_back(idMap);
}


BOOL CJjWorld::IsExploreMapSent(LevelPlayerID idPlayer,RecordID idMap)
{
	if (idPlayer>=LEVEL_MAX_PLAYER)
		return FALSE;
	PlayerEntry *entry=&_players[idPlayer];
	return entry->emsSent.find(idMap)!=entry->emsSent.end();
}

void CJjWorld::SetExploreMapSent(LevelPlayerID idPlayer,RecordID idMap)
{
	if (idPlayer>=LEVEL_MAX_PLAYER)
		return;
	PlayerEntry *entry=&_players[idPlayer];
	entry->emsSent.insert(idMap);
}

void CJjWorld::AddAgentBriefEntryToSend(LevelPlayerID idPlayer,LevelAgentGuid &pending)
{
	if (idPlayer>=LEVEL_MAX_PLAYER)
		return;
	PlayerEntry *entry=&_players[idPlayer];
	entry->pendingAgentBrief.push_back(pending);
}

void CJjWorld::FetchPendingAgentBriefEntry(LevelPlayerID idPlayer,std::vector<LevelAgentGuid> &pendings)
{
	if (idPlayer>=LEVEL_MAX_PLAYER)
	{
		pendings.clear();
		return;
	}
	PlayerEntry *entry=&_players[idPlayer];
	entry->pendingAgentBrief.swap(pendings);
	entry->pendingAgentBrief.clear();
}
