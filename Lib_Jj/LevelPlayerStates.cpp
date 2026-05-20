
#include "stdh.h"

#include "commondefines/general_stl.h"

#include "LevelExploreMap.h"
#include "LevelPersistData.h"

#include "LevelAbility.h"
#include "LevelArtifactUpgrade.h"

#include "LevelPlayerStates.h"

#include "LevelRecords.h"
#include "LevelRecordGlobal.h"
#include "LevelRecordItem.h"
#include "LevelRecordItemClass.h"
#include "LevelRecordUpgrade.h"
#include "LevelRecordUnit.h"
#include "LevelRecordPosture.h"

#include "LoUnit.h"

#include "LevelUtil.h"


////////////////////////////////////////////////////////////////////////
//LPSBase

DWORD *LPSBase::GetRes(LevelResourceType tpRes)
{
	switch(tpRes)
	{
		case LevelResource_Gold:
			return &gold_;
		case LevelResource_Gem:
			return &gem_;
		case LevelResource_Soul:
			return &soul_;
		case LevelResource_Labor:
			return &labor_;
		case LevelResource_Crystal:
			return &crystal;
		case LevelResource_DemonBlood:
			return &demonblood;
		//XXXXX:More LevelResourceType
	}
	return NULL;
}

LevelTempleInfo *LPSBase::GetTemple(LevelTempleType tpTemple)
{
	switch(tpTemple)
	{
		case LevelTemple_Sun:
			return &templeSun;
		case LevelTemple_Moon:
			return &templeMoon;
		case LevelTemple_Fire:
			return &templeFire;
		case LevelTemple_Star:
			return &templeStar;
		case LevelTemple_Sand:
			return &templeSand;
		case LevelTemple_Craft:
			return &templeCraft;
	//XXXXX:More LevelTempleType
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//LPSBag
DWORD LPSBag::CalcItemStackCount(RecordID idItem)
{
	DWORD sum=0;
	for (int i=0;i<items.size();i++)
	{
		if (items[i].tid==idItem)
			sum+=items[i].nStack;
	}
	return sum;
}

//////////////////////////////////////////////////////////////////////////
//LPSArtifacts
BOOL LPSArtifacts::Find(RecordID idItem)
{
	for (int i=0;i<items.size();i++)
	{
		if (items[i].tid==idItem)
			return TRUE;
	}
	return FALSE;
}



////////////////////////////////////////////////////////////////////////
//LPSNpcStates
void LPSNpcSet::Save(CDataPacket &dp)
{
	dp.Data_NextByte()=LPSNpcSet_CurVer;
	dp.Data_NextWord()=datas.size();
	std::unordered_map<RecordID,LPSNpcData *>::iterator it;
	for (it=datas.begin();it!=datas.end();it++)
	{
		dp.Data_WriteSimple((*it).first);
		LPSNpcData *p=(*it).second;
		assert(p);
		dp.Data_NextByte()=(BYTE)p->state;
		DP_WriteVector(dp,p->dataBhv);
		if (p->idMap!=RecordID_Invalid)
		{
			dp.Data_NextByte()=1;
			dp.Data_WriteSimple(p->idMap);
			dp.Data_WriteSimple(p->pos);
		}
		else
			dp.Data_NextByte()=0;
	}
}

void LPSNpcSet::Load(CDataPacket &dp)
{
	BYTE ver=dp.Data_NextByte();
	WORD sz=dp.Data_NextWord();
	for (int i=0;i<sz;i++)
	{
		RecordID idRec;
		dp.Data_ReadSimple(idRec);
		LPSNpcData *p=Class_New2(LPSNpcData);
		p->state=(NPCState)dp.Data_NextByte();
		DP_ReadVector(dp,p->dataBhv);
		if (dp.Data_NextByte()!=0)
		{
			dp.Data_ReadSimple(p->idMap);
			dp.Data_ReadSimple(p->pos);
		}
		else
			p->idMap=RecordID_Invalid;
		datas[idRec]=p;
	}
}

void LPSNpcSet::CopyFrom(LPSField *src0)
{
	LPSNpcSet *src=(LPSNpcSet *)src0;
	std::unordered_map<RecordID,LPSNpcData *>::iterator it;
	for (it=src->datas.begin();it!=src->datas.end();it++)
	{
		LPSNpcData *p=Class_New2(LPSNpcData);
		p->dataBhv=((*it).second)->dataBhv;
		p->state=((*it).second)->state;
		p->idMap=((*it).second)->idMap;
		p->pos=((*it).second)->pos;
		datas[(*it).first]=p;
	}
}

void LPSNpcSet::Clear()
{
	std::unordered_map<RecordID,LPSNpcData *>::iterator it;
	for (it=datas.begin();it!=datas.end();it++)
	{
		LPSNpcData *p=(*it).second;
		Safe_Class_Delete(p);
	}
	datas.clear();
}

////////////////////////////////////////////////////////////////////////
//LPSRetinueSet
void LPSRetinueSet::Save(CDataPacket &dp)
{
	dp.Data_NextByte()=LPSRetinueSet_CurVer;
	dp.Data_NextWord()=datas.size();
	std::unordered_map<RetinueUID,LPSRetinueData*>::iterator it;
	for (it=datas.begin();it!=datas.end();it++)
	{
		LPSRetinueData*p=(*it).second;
		assert(p);

		dp.Data_WriteSimple(p->uid);
		dp.Data_NextByte()=p->tp;
		dp.Data_WriteSimple(p->idRec);
		dp.Data_WriteSimple(p->grd_);
		dp.Data_WriteSimple(p->iPickedEquipSet);
		dp.Data_NextByte()=p->method;
	}
}

void LPSRetinueSet::Load(CDataPacket &dp)
{
	BYTE ver=dp.Data_NextByte();
	WORD sz=dp.Data_NextWord();
	for (int i=0;i<sz;i++)
	{
		LPSRetinueData*p=Class_New2(LPSRetinueData);

		dp.Data_ReadSimple(p->uid);
		p->tp=(RetinueType)dp.Data_NextByte();
		dp.Data_ReadSimple(p->idRec);
		dp.Data_ReadSimple(p->grd_);
		dp.Data_ReadSimple(p->iPickedEquipSet);
		p->method=(LevelMoveMethod)dp.Data_NextByte();

		datas[p->uid]=p;
	}
}

void LPSRetinueSet::CopyFrom(LPSField *src0)
{
	LPSRetinueSet*src=(LPSRetinueSet*)src0;
	std::unordered_map<RetinueUID,LPSRetinueData*>::iterator it;
	for (it=src->datas.begin();it!=src->datas.end();it++)
	{
		LPSRetinueData*q=(*it).second;
		LPSRetinueData*p=Class_New2(LPSRetinueData);
		p->uid=q->uid;
		p->tp=q->tp;
		p->idRec=q->idRec;
		p->grd_=q->grd_;
		p->iPickedEquipSet=q->iPickedEquipSet;
		p->method=q->method;
		datas[(*it).first]=p;
	}
}

void LPSRetinueSet::Clear()
{
	std::unordered_map<RetinueUID,LPSRetinueData*>::iterator it;
	for (it=datas.begin();it!=datas.end();it++)
	{
		LPSRetinueData*p=(*it).second;
		Safe_Class_Delete(p);
	}
	datas.clear();
}


////////////////////////////////////////////////////////////////////////
//LPSExploreMapSet
void LPSExploreMapSet::Save(CDataPacket &dp)
{
	dp.Data_NextByte()=LPSExploreMapSet_CurVer;
	dp.Data_NextWord()=mps.size();

	std::unordered_map<StringID,LevelExploreMaps>::iterator it;
	for (it=mps.begin();it!=mps.end();it++)
	{
		dp.Data_WriteSimple((*it).first);
		LevelExploreMaps mps=(*it).second;
		mps.sttc->Save(dp);
		mps.dyn->Save(dp);
	}
}

void LPSExploreMapSet::Load(CDataPacket &dp)
{
	BYTE ver=dp.Data_NextByte();
	WORD sz=dp.Data_NextWord();
	for (int i=0;i<sz;i++)
	{
		StringID nm;
		dp.Data_ReadSimple(nm);
		LevelExploreMaps mps2;
		mps2.sttc=Class_New2(CLevelExploreMap);
		mps2.sttc->Load(dp);
		mps2.dyn=Class_New2(CLevelExploreMap);
		mps2.dyn->Load(dp);
		mps[nm]=mps2;
	}
}

void LPSExploreMapSet::CopyFrom(LPSField *src0)
{
	Clear();

	LPSExploreMapSet*src=(LPSExploreMapSet*)src0;
	std::unordered_map<StringID,LevelExploreMaps>::iterator it;
	for (it=src->mps.begin();it!=src->mps.end();it++)
	{
		LevelExploreMaps mps2;
		mps2.sttc=Class_New2(CLevelExploreMap);
		mps2.sttc->CopyFrom((*it).second.sttc);
		mps2.dyn=Class_New2(CLevelExploreMap);
		mps2.dyn->CopyFrom((*it).second.dyn);
		mps[(*it).first]=mps2;
	}
}

void LPSExploreMapSet::Clear()
{
	std::unordered_map<StringID,LevelExploreMaps>::iterator it;
	for (it=mps.begin();it!=mps.end();it++)
	{
		CLevelExploreMap *mp=(*it).second.sttc;
		mp->Clear();
		Safe_Class_Delete(mp);
		mp=(*it).second.dyn;
		mp->Clear();
		Safe_Class_Delete(mp);
	}
	mps.clear();
}

void LPSExploreMapSet::ClearDyn()
{
	std::unordered_map<RecordID,LevelExploreMaps>::iterator it;
	for (it=mps.begin();it!=mps.end();it++)
	{
		LevelExploreMaps &t=(*it).second;
		if (t.dyn)
			t.dyn->ClearContent();
	}
}

LevelExploreMaps LPSExploreMapSet::Find(StringID nmLevel)
{
	std::unordered_map<StringID,LevelExploreMaps>::iterator it=mps.find(nmLevel);
	if (it==mps.end())
		return LevelExploreMaps();
	return (*it).second;
}

LevelExploreMaps LPSExploreMapSet::New(StringID nmLevel,i_math::recti &rcMap)
{
	LevelExploreMaps mps2;
	mps2.sttc=Class_New2(CLevelExploreMap);
	mps2.sttc->Init(rcMap);
	mps2.dyn=Class_New2(CLevelExploreMap);
	mps2.dyn->Init(rcMap);
	mps[nmLevel]=mps2;
	return mps2;
}


////////////////////////////////////////////////////////////////////////
//LPSPersistSet_Agent

void LPSPersistSet_Agent::Save(CDataPacket &dp)
{
	dp.Data_NextByte()=LPSPersistSet_Agent_CurVer;
	dp.Data_NextWord()=persists.size();

	std::unordered_map<RecordID,LevelPersistData_Agent*>::iterator it;
	for (it=persists.begin();it!=persists.end();it++)
	{
		dp.Data_WriteSimple((*it).first);
		LevelPersistData_Agent *persist=(*it).second;
		persist->Save(dp);
	}
}

void LPSPersistSet_Agent::Load(CDataPacket &dp)
{
	BYTE ver=dp.Data_NextByte();
	WORD sz=dp.Data_NextWord();
	for (int i=0;i<sz;i++)
	{
		RecordID id;
		dp.Data_ReadSimple(id);
		LevelPersistData_Agent *persist=Class_New2(LevelPersistData_Agent);
		persist->Load(dp);
		persists[id]=persist;
	}
}

void LPSPersistSet_Agent::CopyFrom(LPSField *src0)
{
	Clear();

	LPSPersistSet_Agent*src=(LPSPersistSet_Agent*)src0;
	std::unordered_map<RecordID,LevelPersistData_Agent*>::iterator it;
	for (it=src->persists.begin();it!=src->persists.end();it++)
	{
		LevelPersistData_Agent *persist=Class_New2(LevelPersistData_Agent);
		persist->CopyFrom((*it).second);
		persists[(*it).first]=persist;
	}
}

void LPSPersistSet_Agent::Clear()
{
	std::unordered_map<RecordID,LevelPersistData_Agent*>::iterator it;
	for (it=persists.begin();it!=persists.end();it++)
	{
		LevelPersistData_Agent *persist=(*it).second;
		persist->Clear();
		Safe_Class_Delete(persist);
	}
	persists.clear();
}

LevelPersistData_Agent*LPSPersistSet_Agent::Find(RecordID idMap)
{
	std::unordered_map<RecordID,LevelPersistData_Agent*>::iterator it=persists.find(idMap);
	if (it==persists.end())
		return NULL;
	return (*it).second;
}

LevelPersistData_Agent*LPSPersistSet_Agent::Obtain(RecordID idMap)
{
	LevelPersistData_Agent*p=Find(idMap);
	if (!p)
	{
		p=Class_New2(LevelPersistData_Agent);
		persists[idMap]=p;
	}
	return p;
}

////////////////////////////////////////////////////////////////////////
//LPSPersistSet_AgentS

void LPSPersistSet_AgentS::Save(CDataPacket &dp)
{
	dp.Data_NextByte()=LPSPersistSet_AgentS_CurVer;
	dp.Data_NextWord()=persists.size();

	std::unordered_map<RecordID,LevelPersistData_AgentS*>::iterator it;
	for (it=persists.begin();it!=persists.end();it++)
	{
		dp.Data_WriteSimple((*it).first);
		LevelPersistData_AgentS *persist=(*it).second;
		persist->Save(dp);
	}
}

void LPSPersistSet_AgentS::Load(CDataPacket &dp)
{
	BYTE ver=dp.Data_NextByte();
	WORD sz=dp.Data_NextWord();
	for (int i=0;i<sz;i++)
	{
		RecordID id;
		dp.Data_ReadSimple(id);
		LevelPersistData_AgentS *persist=Class_New2(LevelPersistData_AgentS);
		persist->Load(dp);
		persists[id]=persist;
	}
}

void LPSPersistSet_AgentS::CopyFrom(LPSField *src0)
{
	Clear();

	LPSPersistSet_AgentS*src=(LPSPersistSet_AgentS*)src0;
	std::unordered_map<RecordID,LevelPersistData_AgentS*>::iterator it;
	for (it=src->persists.begin();it!=src->persists.end();it++)
	{
		LevelPersistData_AgentS *persist=Class_New2(LevelPersistData_AgentS);
		persist->CopyFrom((*it).second);
		persists[(*it).first]=persist;
	}
}

void LPSPersistSet_AgentS::Clear()
{
	std::unordered_map<RecordID,LevelPersistData_AgentS*>::iterator it;
	for (it=persists.begin();it!=persists.end();it++)
	{
		LevelPersistData_AgentS *persist=(*it).second;
		persist->Clear();
		Safe_Class_Delete(persist);
	}
	persists.clear();
}

LevelPersistData_AgentS*LPSPersistSet_AgentS::Find(RecordID idMap)
{
	std::unordered_map<RecordID,LevelPersistData_AgentS*>::iterator it=persists.find(idMap);
	if (it==persists.end())
		return NULL;
	return (*it).second;
}

LevelPersistData_AgentS*LPSPersistSet_AgentS::Obtain(RecordID idMap)
{
	LevelPersistData_AgentS*p=Find(idMap);
	if (!p)
	{
		p=Class_New2(LevelPersistData_AgentS);
		persists[idMap]=p;
	}
	return p;
}

////////////////////////////////////////////////////////////////////////
//LPSPersistSet_AgentBrief

void LPSPersistSet_AgentBrief::Save(CDataPacket &dp)
{
	dp.Data_NextByte()=LPSPersistSet_AgentBrief_CurVer;
	dp.Data_NextWord()=persists.size();

	std::unordered_map<RecordID,LevelPersistData_AgentBrief*>::iterator it;
	for (it=persists.begin();it!=persists.end();it++)
	{
		dp.Data_WriteSimple((*it).first);
		LevelPersistData_AgentBrief *persist=(*it).second;
		persist->Save(dp);
	}
}

void LPSPersistSet_AgentBrief::Load(CDataPacket &dp)
{
	BYTE ver=dp.Data_NextByte();
	WORD sz=dp.Data_NextWord();
	for (int i=0;i<sz;i++)
	{
		RecordID id;
		dp.Data_ReadSimple(id);
		LevelPersistData_AgentBrief *persist=Class_New2(LevelPersistData_AgentBrief);
		persist->Load(dp);
		persists[id]=persist;
	}
}

void LPSPersistSet_AgentBrief::CopyFrom(LPSField *src0)
{
	Clear();

	LPSPersistSet_AgentBrief*src=(LPSPersistSet_AgentBrief*)src0;
	std::unordered_map<RecordID,LevelPersistData_AgentBrief*>::iterator it;
	for (it=src->persists.begin();it!=src->persists.end();it++)
	{
		LevelPersistData_AgentBrief *persist=Class_New2(LevelPersistData_AgentBrief);
		persist->CopyFrom((*it).second);
		persists[(*it).first]=persist;
	}
}

void LPSPersistSet_AgentBrief::Clear()
{
	std::unordered_map<RecordID,LevelPersistData_AgentBrief*>::iterator it;
	for (it=persists.begin();it!=persists.end();it++)
	{
		LevelPersistData_AgentBrief *persist=(*it).second;
		persist->Clear();
		Safe_Class_Delete(persist);
	}
	persists.clear();
}

LevelPersistData_AgentBrief*LPSPersistSet_AgentBrief::Find(RecordID idMap)
{
	std::unordered_map<RecordID,LevelPersistData_AgentBrief*>::iterator it=persists.find(idMap);
	if (it==persists.end())
		return NULL;
	return (*it).second;
}

LevelPersistData_AgentBrief*LPSPersistSet_AgentBrief::Obtain(RecordID idMap)
{
	LevelPersistData_AgentBrief*p=Find(idMap);
	if (!p)
	{
		p=Class_New2(LevelPersistData_AgentBrief);
		persists[idMap]=p;
	}
	return p;
}

void LPSPersistSet_AgentBrief::ClearCur()
{
	std::unordered_map<RecordID,LevelPersistData_AgentBrief*>::iterator it;
	for (it=persists.begin();it!=persists.end();it++)
	{
		((*it).second)->ClearCur();
	}
}


////////////////////////////////////////////////////////////////////////
//LPSPersistSet_LevelAI

void LPSPersistSet_LevelAI::Save(CDataPacket &dp)
{
	dp.Data_NextByte()=LPSPersistSet_LevelAI_CurVer;
	dp.Data_NextWord()=persists.size();

	std::unordered_map<RecordID,LevelPersistData_LevelAI*>::iterator it;
	for (it=persists.begin();it!=persists.end();it++)
	{
		dp.Data_WriteSimple((*it).first);
		LevelPersistData_LevelAI *persist=(*it).second;
		persist->Save(dp);
	}
}

void LPSPersistSet_LevelAI::Load(CDataPacket &dp)
{
	BYTE ver=dp.Data_NextByte();
	WORD sz=dp.Data_NextWord();
	for (int i=0;i<sz;i++)
	{
		RecordID id;
		dp.Data_ReadSimple(id);
		LevelPersistData_LevelAI *persist=Class_New2(LevelPersistData_LevelAI);
		persist->Load(dp);
		persists[id]=persist;
	}
}

void LPSPersistSet_LevelAI::CopyFrom(LPSField *src0)
{
	Clear();

	LPSPersistSet_LevelAI*src=(LPSPersistSet_LevelAI*)src0;
	std::unordered_map<RecordID,LevelPersistData_LevelAI*>::iterator it;
	for (it=src->persists.begin();it!=src->persists.end();it++)
	{
		LevelPersistData_LevelAI *persist=Class_New2(LevelPersistData_LevelAI);
		persist->CopyFrom((*it).second);
		persists[(*it).first]=persist;
	}
}

void LPSPersistSet_LevelAI::Clear()
{
	std::unordered_map<RecordID,LevelPersistData_LevelAI*>::iterator it;
	for (it=persists.begin();it!=persists.end();it++)
	{
		LevelPersistData_LevelAI *persist=(*it).second;
		persist->Clear();
		Safe_Class_Delete(persist);
	}
	persists.clear();
}

LevelPersistData_LevelAI*LPSPersistSet_LevelAI::Find(RecordID idMap)
{
	std::unordered_map<RecordID,LevelPersistData_LevelAI*>::iterator it=persists.find(idMap);
	if (it==persists.end())
		return NULL;
	return (*it).second;
}

LevelPersistData_LevelAI*LPSPersistSet_LevelAI::Obtain(RecordID idMap)
{
	LevelPersistData_LevelAI*p=Find(idMap);
	if (!p)
	{
		p=Class_New2(LevelPersistData_LevelAI);
		persists[idMap]=p;
	}
	return p;
}



///////////////////////////////////////////////////////////////////
//LPS_FIELD
void LPSField::SetDirtyDB_Low()
{
	bDirtyDB=TRUE;
	verDB++;
	if (owner)
		owner->SetDirtyDB(ANIMTICK_FROM_SECOND(3600.0f));
}
void LPSField::SetDirtyDB_High()
{
	bDirtyDB=TRUE;
	verDB++;
	if (owner)
		owner->SetDirtyDB(ANIMTICK_FROM_SECOND(60.0f));
}
void LPSField::SetDirtyDB_Urgent()
{
	bDirtyDB=TRUE;
	verDB++;
	if (owner)
		owner->SetDirtyDB(ANIMTICK_FROM_SECOND(0.05f));
}


LPSFieldInfo *LPS_GetFieldInfos(DWORD &c)
{
	static std::vector<LPSFieldInfo>infoes;
	if (infoes.size()<=0)
	{
		LevelPlayerStates t;

		LPS_FIELD(base);
		LPS_FIELD(equip);

		if (TRUE)
		{
			static char nmBags[MAX_PLAYER_BAG][16];
			for (int i=0;i<ARRAY_SIZE(t.bags);i++)
			{
				LPSFieldInfo info;
				info.off=(DWORD)((BYTE*)&t.bags[i]-(BYTE*)&t);
				sprintf(nmBags[i], "Bag%02d",i);
				info.nm=nmBags[i];
				infoes.push_back(info);
			}
		}

		LPS_FIELD(pickup);

		LPS_FIELD(skills);
		LPS_FIELD(skillfasts);

		if (TRUE)
		{
			static char nms[MAX_PLAYER_NPCSET][16];
			for (int i=0;i<ARRAY_SIZE(t.npcsets);i++)
			{
				LPSFieldInfo info;
				info.off=(DWORD)((BYTE*)&t.npcsets[i]-(BYTE*)&t);
				sprintf(nms[i], "NpcSet%03d",i);
				info.nm=nms[i];
				infoes.push_back(info);
			}
		}

		if (TRUE)
		{
			static char nms[MAX_PLAYER_RETINUESET][16];
			for (int i=0;i<ARRAY_SIZE(t.rtnusets);i++)
			{
				LPSFieldInfo info;
				info.off=(DWORD)((BYTE*)&t.rtnusets[i]-(BYTE*)&t);
				sprintf(nms[i], "RtnuSet%03d",i);
				info.nm=nms[i];
				infoes.push_back(info);
			}
		}

		if (TRUE)
		{
			static char nms[MAX_PLAYER_EXPLOREMAPSET][16];
			for (int i=0;i<ARRAY_SIZE(t.emsets);i++)
			{
				LPSFieldInfo info;
				info.off=(DWORD)((BYTE*)&t.emsets[i]-(BYTE*)&t);
				sprintf(nms[i], "EmSet%03d",i);
				info.nm=nms[i];
				infoes.push_back(info);
			}
		}

		if (TRUE)
		{
			static char nms[MAX_PLAYER_PERSISTSET_AGENT][16];
			for (int i=0;i<ARRAY_SIZE(t.persistsetsAgent);i++)
			{
				LPSFieldInfo info;
				info.off=(DWORD)((BYTE*)&t.persistsetsAgent[i]-(BYTE*)&t);
				sprintf(nms[i], "PsstSetAg%03d",i);
				info.nm=nms[i];
				infoes.push_back(info);
			}
		}

		if (TRUE)
		{
			static char nms[MAX_PLAYER_PERSISTSET_AGENT_S][16];
			for (int i=0;i<ARRAY_SIZE(t.persistsetsAgentS);i++)
			{
				LPSFieldInfo info;
				info.off=(DWORD)((BYTE*)&t.persistsetsAgentS[i]-(BYTE*)&t);
				sprintf(nms[i], "PsstSetAgS%03d",i);
				info.nm=nms[i];
				infoes.push_back(info);
			}
		}

		if (TRUE)
		{
			static char nms[MAX_PLAYER_PERSISTSET_AGENT_BRIEF][16];
			for (int i=0;i<ARRAY_SIZE(t.persistsetsAgentBrief);i++)
			{
				LPSFieldInfo info;
				info.off=(DWORD)((BYTE*)&t.persistsetsAgentBrief[i]-(BYTE*)&t);
				sprintf(nms[i], "PsstSetAgB%03d",i);
				info.nm=nms[i];
				infoes.push_back(info);
			}
		}


		if (TRUE)
		{
			static char nms[MAX_PLAYER_PERSISTSET_LEVELAI][16];
			for (int i=0;i<ARRAY_SIZE(t.persistsetsLevelAI);i++)
			{
				LPSFieldInfo info;
				info.off=(DWORD)((BYTE*)&t.persistsetsLevelAI[i]-(BYTE*)&t);
				sprintf(nms[i], "PsstSetLvlAI%03d",i);
				info.nm=nms[i];
				infoes.push_back(info);
			}
		}


		LPS_FIELD(entrances);
		LPS_FIELD(abilities);
		LPS_FIELD(artifacts);
		LPS_FIELD(itemmemory);
		LPS_FIELD(fasts);
		LPS_FIELD(killings);


		LPS_FIELD(misc);

		//XXXXX:more LPS Field

	}

	c=infoes.size();
	return infoes.data();
}


void LPS_Build(LevelPlayerStates *lps,PlayerClass clss,const char *nm,CLevelRecords *records)
{
	switch(clss)
	{
		case PlayerClass_Hunter:
		{
			//Base
			lps->base.clss=clss;
			lps->base.grd=1;
			lps->base.MaxHP=300;
			lps->base.FullSP=400;

			lps->base.str=1;
			lps->base.dex=10;
			lps->base.magic=5;
			lps->base.ldr=5;

			extern void StrSafeCopy(char *dest,const char *src,DWORD sz);
			StrSafeCopy(lps->base.nm,nm,ARRAY_SIZE(lps->base.nm));
			lps->base.idMap=records->GetGlobal()->idStartMap;
			lps->base.pos=LevelPos_Invalid;

			lps->base.iDay=1;

			//Bag
			lps->bags[0].bActivated=1;
			lps->bags[0].sz.set(10,6);

			//资源
			if (TRUE)
			{
				lps->base.labor_=5;
			}

			//Items
			if (TRUE)
			{
				LevelRecordGlobal*global=records->GetGlobal();
				for (int i=0;i<global->itemsInitial.size();i++)
				{
					RecordID idItem=global->itemsInitial[i];
					LevelRecordItem *recItem=records->GetItem(idItem);
					if (recItem)
					{
						if (recItem->tpArtifact!=LevelArtifact_None)
						{
							extern BOOL LPS_AddArtifact(LevelPlayerStates *lps,CLevelRecords *records,RecordID idItem,int nStack);
							LPS_AddArtifact(lps,records,idItem,1);
						}
					}
				}
			}

			//Abilities
			if (TRUE)
			{
				CLevelAbilities abilities;
				abilities.Init(NULL,NULL);

				LevelUtil_ActivateAbility(&abilities,LevelAbilityType_Unarmed,records);
				LevelUtil_ActivateAbility(&abilities,LevelAbilityType_PushSlideway,records);
				if (TRUE)
				{
					LevelRecordGlobal*global=records->GetGlobal();
					for (int i=0;i<global->itemsInitial.size();i++)
					{
						RecordID idItem=global->itemsInitial[i];
						LevelRecordItem *recItem=records->GetItem(idItem);
						if (recItem)
						{
							if (recItem->tpAbility!=LevelAbilityType_None)
								LevelUtil_ActivateAbility(&abilities,recItem->tpAbility,records);
						}
					}
				}

				DP_BeginSave(dp,lps->abilities.data);
				abilities.Save(dp);
				DP_EndSave();

				abilities.Clear();

				//初始化Fast
				lps->fasts.skillSel=LevelFastTarget();
			}
			
			break;
		}
	}
	LPS_SetValid(lps);
}

void LPS_Restart(LevelPlayerStates *lps,CLevelRecords *records)
{
	PlayerClass clss;
	clss=(PlayerClass)lps->base.clss;
	std::string nm=lps->base.nm;

	//清除
	DWORD c;
	LPSFieldInfo *infos=LPS_GetFieldInfos(c);
	for (int i=0;i<c;i++)
	{
		LPSFieldInfo *info=&infos[i];
		if (memcmp(info->nm,"EmSet",5)==0)
		{
			LPSExploreMapSet*field=(LPSExploreMapSet*)(((BYTE*)lps)+info->off);
			field->ClearDyn();
			continue;
		}
		if (memcmp(info->nm,"PsstSetAgB",strlen("PsstSetAgB"))==0)
		{
			LPSPersistSet_AgentBrief*field=(LPSPersistSet_AgentBrief*)(((BYTE*)lps)+info->off);
			field->ClearCur();
			continue;
		}


		LPSField *field=(LPSField *)(((BYTE*)lps)+info->off);
		field->Clear();
	}

	LPS_Build(lps,clss,nm.c_str(),records);

	//Dirty
	for (int i=0;i<c;i++)
	{
		LPSFieldInfo *info=&infos[i];
		LPSField *field=(LPSField *)(((BYTE*)lps)+info->off);
		field->SetDirtyDB_Urgent();
	}
}

void LPS_Clear(LevelPlayerStates *lps)
{
	DWORD c;
	LPSFieldInfo *infos=LPS_GetFieldInfos(c);
	for (int i=0;i<c;i++)
	{
		LPSFieldInfo *info=&infos[i];
		LPSField *field=(LPSField *)(((BYTE*)lps)+info->off);
		field->Clear();
	}
}

void LPS_SetValid(LevelPlayerStates *lps)
{
	DWORD c;
	LPSFieldInfo *infos=LPS_GetFieldInfos(c);
	for (int i=0;i<c;i++)
	{
		LPSFieldInfo *info=&infos[i];
		LPSField *field=(LPSField *)(((BYTE*)lps)+info->off);
		field->bValid=1;
	}
}


void LPS_Copy(LevelPlayerStates *lpsTo,LevelPlayerStates *lpsFrom)
{
	DWORD c;
	LPSFieldInfo *infos=LPS_GetFieldInfos(c);
	for (int i=0;i<c;i++)
	{
		LPSFieldInfo *info=&infos[i];
		LPSField *fieldTo=(LPSField *)(((BYTE*)lpsTo)+info->off);
		LPSField *fieldFrom=(LPSField *)(((BYTE*)lpsFrom)+info->off);
		fieldTo->CopyFrom(fieldTo);
	}
}

int LPS_FindFieldIdx(const char *nmField)
{
	DWORD c;
	LPSFieldInfo *infos=LPS_GetFieldInfos(c);
	for (int i=0;i<c;i++)
	{
		LPSFieldInfo *info=&infos[i];
		if (strcmp(info->nm,nmField)==0)
			return i;
	}
	return -1;
}

int LPS_FindBaseIdx()
{
	static int idx=-1;
	if (idx==-1)
		idx=LPS_FindFieldIdx("base");
	return idx;
}


int LPS_FindEquipIdx()
{
	static int idx=-1;
	if (idx==-1)
		idx=LPS_FindFieldIdx("equip");
	return idx;
}


int LPS_FindPickUpIdx()
{
	static int idx=-1;
	if (idx==-1)
		idx=LPS_FindFieldIdx("pickup");
	return idx;
}

int LPS_FindSkillsIdx()
{
	static int idx=-1;
	if (idx==-1)
		idx=LPS_FindFieldIdx("skills");
	return idx;
}

int LPS_FindSkillFastsIdx()
{
	static int idx=-1;
	if (idx==-1)
		idx=LPS_FindFieldIdx("skillfasts");
	return idx;
}

int LPS_FindEntrancesIdx()
{
	static int idx=-1;
	if (idx==-1)
		idx=LPS_FindFieldIdx("entrances");
	return idx;
}

int LPS_FindAbilitiesIdx()
{
	static int idx=-1;
	if (idx==-1)
		idx=LPS_FindFieldIdx("abilities");
	return idx;
}

int LPS_FindArtifactsIdx()
{
	static int idx=-1;
	if (idx==-1)
		idx=LPS_FindFieldIdx("artifacts");
	return idx;
}

int LPS_FindItemMemoryIdx()
{
	static int idx=-1;
	if (idx==-1)
		idx=LPS_FindFieldIdx("itemmemory");
	return idx;
}

int LPS_FindFastsIdx()
{
	static int idx=-1;
	if (idx==-1)
		idx=LPS_FindFieldIdx("fasts");
	return idx;
}
int LPS_FindKillingsIdx()
{
	static int idx=-1;
	if (idx==-1)
		idx=LPS_FindFieldIdx("killings");
	return idx;
}

int LPS_FindMiscIdx()
{
	static int idx=-1;
	if (idx==-1)
		idx=LPS_FindFieldIdx("misc");
	return idx;
}





int LPS_FindBagIdx(DWORD iBag)
{
	static BOOL bInit=FALSE;
	static int idx[MAX_PLAYER_BAG];
	if (!bInit)
	{
		char nmBags[16];
		for (int i=0;i<MAX_PLAYER_BAG;i++)
		{
			sprintf(nmBags, "Bag%02d",i);
			idx[i]=LPS_FindFieldIdx(nmBags);
		}
		bInit=TRUE;
	}
	if (iBag>=MAX_PLAYER_BAG)
		return -1;
	return idx[iBag];
}




BOOL LPS_FindBagSlotForItem(i_math::rect_c &rcSlot,LevelPlayerStates *lps,DWORD iBag,DWORD wSlot,DWORD hSlot)
{
	if (iBag>=MAX_PLAYER_BAG)
		return FALSE;
	LPSBag *bag=&lps->bags[iBag];

	bag->UpdateSlots();

	int w,h;
	w=((int)bag->sz.w)-wSlot;
	h=((int)bag->sz.h)-hSlot;

	BYTE *p0=&bag->slots[0];
	for (int j=0;j<h;j++)
	{
		for (int i=0;i<w;i++)
		{
			BYTE *p=p0+i;

			for (int jj=0;jj<hSlot;jj++)
			{
				for (int ii=0;ii<wSlot;ii++)
				{
					if (*(p+ii))
						goto _out;
				}
				p+=bag->sz.w;
			}

			rcSlot.set((char)i,(char)j,(char)(i+wSlot),(char)(j+hSlot));
			return TRUE;

_out:;

		}
		p0+=bag->sz.w;
	}
	return FALSE;
}

BOOL LPS_IsPickUp(LevelPlayerStates *lps)
{
	return lps->pickup.item.IsValid();
}


LPSNpcSet *LPS_FindNpcSet(LevelPlayerStates *lps,RecordID idRec)
{
	DWORD idx=idRec%MAX_PLAYER_NPCSET;
	return &lps->npcsets[idx];
}

LPSNpcData *LPS_FindNpcData(LevelPlayerStates *lps,RecordID idRec)
{
	LPSNpcSet *npcset=LPS_FindNpcSet(lps,idRec);
	if (npcset)
		return npcset->FindData(idRec);
	return NULL;
}

LPSNpcData *LPS_QueryNpcData(LevelPlayerStates *lps,RecordID idRec)
{
	LPSNpcSet *npcset=LPS_FindNpcSet(lps,idRec);
	LPSNpcData *data=npcset->ObtainData(idRec);
	npcset->SetDirtyDB_Urgent();
	return data;
}

void LPS_EraseRetinue(LevelPlayerStates *lps,RetinueUID uid)
{
	DWORD idx=uid%MAX_PLAYER_RETINUESET;
	LPSRetinueSet *rtnuset=&lps->rtnusets[idx];
	std::unordered_map<RetinueUID,LPSRetinueData*>::iterator it=rtnuset->datas.find(uid);
	if (it==rtnuset->datas.end())
		return;
	Safe_Class_Delete((*it).second);
	rtnuset->datas.erase(it);
	rtnuset->SetDirtyDB_Urgent();
}

LPSRetinueData *LPS_FindRetinue(LevelPlayerStates *lps,RetinueUID uid)
{
	DWORD idx=uid%MAX_PLAYER_RETINUESET;
	LPSRetinueSet *rtnuset=&lps->rtnusets[idx];
	std::unordered_map<RetinueUID,LPSRetinueData*>::iterator it=rtnuset->datas.find(uid);
	if (it==rtnuset->datas.end())
		return NULL;
	return (*it).second;
}

LPSRetinueData *LPS_QueryRetinue(LevelPlayerStates *lps,RetinueUID uid)
{
	LPSRetinueData *ret=LPS_FindRetinue(lps,uid);
	if (!ret)
		return NULL;
	DWORD idx=uid%MAX_PLAYER_RETINUESET;
	LPSRetinueSet *rtnuset=&lps->rtnusets[idx];
	rtnuset->SetDirtyDB_Urgent();
	return ret;
}

LPSRetinueData *LPS_NewRetinue(LevelPlayerStates *lps,RetinueType tp,RecordID idRec,LevelGrade grd,EquipSetPick iPickedEquipSet,LevelMoveMethod method)
{
	lps->base.sdRtnu++;
	RetinueUID uid=lps->base.sdRtnu;
	lps->base.SetDirtyDB_Urgent();

	DWORD idx=uid%MAX_PLAYER_RETINUESET;
	LPSRetinueSet *rtnuset=&lps->rtnusets[idx];
	LPSRetinueData *data=rtnuset->New(uid,tp,idRec,grd,method,iPickedEquipSet);

	rtnuset->SetDirtyDB_Urgent();

	return data;
}

LevelExploreMaps LPS_FindExploreMaps(LevelPlayerStates *lps,RecordID idMap)
{
	DWORD idx=idMap%MAX_PLAYER_EXPLOREMAPSET;
	LPSExploreMapSet*emset=&lps->emsets[idx];
	std::unordered_map<RecordID,LevelExploreMaps>::iterator it=emset->mps.find(idMap);
	if (it==emset->mps.end())
		return LevelExploreMaps();
	return (*it).second;
}

LevelExploreMaps LPS_QueryExploreMaps(LevelPlayerStates *lps,RecordID idMap)
{
	LevelExploreMaps ret=LPS_FindExploreMaps(lps,idMap);
	if (ret.IsEmpty())
		return ret;

	DWORD idx=idMap%MAX_PLAYER_EXPLOREMAPSET;
	LPSExploreMapSet*emset=&lps->emsets[idx];
	emset->SetDirtyDB_Urgent();
	return ret;
}

LevelExploreMaps LPS_NewExploreMaps(LevelPlayerStates *lps,RecordID idMap,i_math::recti &rcMap)
{
	DWORD idx=idMap%MAX_PLAYER_EXPLOREMAPSET;
	LPSExploreMapSet*emset=&lps->emsets[idx];

	std::unordered_map<StringID,LevelExploreMaps>::iterator it=emset->mps.find(idMap);
	if (it!=emset->mps.end())
		return LevelExploreMaps();

	LevelExploreMaps mps;
	mps.sttc=Class_New2(CLevelExploreMap);
	mps.sttc->Init(rcMap);
	mps.dyn=Class_New2(CLevelExploreMap);
	mps.dyn->Init(rcMap);
	emset->mps[idMap]=mps;

	emset->SetDirtyDB_Urgent();
	return mps;
}

LevelPersistData_Agent*LPS_FindPersistData_Agent(LevelPlayerStates *lps,RecordID idMap)
{
	DWORD idx=idMap%MAX_PLAYER_PERSISTSET_AGENT;
	LPSPersistSet_Agent*persists=&lps->persistsetsAgent[idx];

	return persists->Find(idMap);
}


LevelPersistEntry_Agent *LPS_FindPersistEntry_Agent(LevelPlayerStates *lps,RecordID idMap,LevelGUID guid)
{
	DWORD idx=idMap%MAX_PLAYER_PERSISTSET_AGENT;
	LPSPersistSet_Agent*persists=&lps->persistsetsAgent[idx];

	LevelPersistData_Agent*persist=persists->Find(idMap);
	if (!persist)
		return NULL;
	return persist->Find(guid);
}

LevelPersistEntry_Agent *LPS_QueryPersistEntry_Agent(LevelPlayerStates *lps,RecordID idMap,LevelGUID guid)
{
	DWORD idx=idMap%MAX_PLAYER_PERSISTSET_AGENT;
	LPSPersistSet_Agent*persists=&lps->persistsetsAgent[idx];

	LevelPersistData_Agent*persist=persists->Obtain(idMap);
	if (!persist)
		return NULL;
	persists->SetDirtyDB_Urgent();
	return persist->Obtain(guid);
}


void LPS_ErasePersistEntry_Agent(LevelPlayerStates *lps,RecordID idMap,LevelGUID guid)
{
	DWORD idx=idMap%MAX_PLAYER_PERSISTSET_AGENT;
	LPSPersistSet_Agent*persists=&lps->persistsetsAgent[idx];
	LevelPersistData_Agent*persist=persists->Find(idMap);
	if (!persist)
		return;
	persist->Erase(guid);
	persists->SetDirtyDB_Urgent();
}

void LPS_SetPersistEntry_Agent(LevelPlayerStates *lps,RecordID idMap,LevelGUID guid,LevelPersistEntry_Agent &entry)
{
	DWORD idx=idMap%MAX_PLAYER_PERSISTSET_AGENT;
	LPSPersistSet_Agent*persists=&lps->persistsetsAgent[idx];
	LevelPersistData_Agent*persist=persists->Obtain(idMap);
	if (!persist)
		return;
	persist->Set(guid,entry);
	persists->SetDirtyDB_Urgent();
}

LevelPersistData_AgentS*LPS_FindPersistData_AgentS(LevelPlayerStates *lps,RecordID idMap)
{
	DWORD idx=idMap%MAX_PLAYER_PERSISTSET_AGENT_S;
	LPSPersistSet_AgentS*persists=&lps->persistsetsAgentS[idx];

	return persists->Find(idMap);
}


LevelPersistEntry_AgentS *LPS_FindPersistEntry_AgentS(LevelPlayerStates *lps,RecordID idMap,LevelGUID guid)
{
	DWORD idx=idMap%MAX_PLAYER_PERSISTSET_AGENT_S;
	LPSPersistSet_AgentS*persists=&lps->persistsetsAgentS[idx];

	LevelPersistData_AgentS*persist=persists->Find(idMap);
	if (!persist)
		return NULL;
	return persist->Find(guid);
}

LevelPersistEntry_AgentS *LPS_QueryPersistEntry_AgentS(LevelPlayerStates *lps,RecordID idMap,LevelGUID guid)
{
	DWORD idx=idMap%MAX_PLAYER_PERSISTSET_AGENT_S;
	LPSPersistSet_AgentS*persists=&lps->persistsetsAgentS[idx];

	LevelPersistData_AgentS*persist=persists->Obtain(idMap);
	if (!persist)
		return NULL;
	persists->SetDirtyDB_Urgent();
	return persist->Obtain(guid);
}


void LPS_ErasePersistEntry_AgentS(LevelPlayerStates *lps,RecordID idMap,LevelGUID guid)
{
	DWORD idx=idMap%MAX_PLAYER_PERSISTSET_AGENT_S;
	LPSPersistSet_AgentS*persists=&lps->persistsetsAgentS[idx];
	LevelPersistData_AgentS*persist=persists->Find(idMap);
	if (!persist)
		return;
	persist->Erase(guid);
	persists->SetDirtyDB_Urgent();
}

void LPS_SetPersistEntry_AgentS(LevelPlayerStates *lps,RecordID idMap,LevelGUID guid,LevelPersistEntry_AgentS &entry)
{
	DWORD idx=idMap%MAX_PLAYER_PERSISTSET_AGENT_S;
	LPSPersistSet_AgentS*persists=&lps->persistsetsAgentS[idx];
	LevelPersistData_AgentS*persist=persists->Obtain(idMap);
	if (!persist)
		return;
	persist->Set(guid,entry);
	persists->SetDirtyDB_Urgent();
}

LevelPersistData_AgentBrief*LPS_FindPersistData_AgentBrief(LevelPlayerStates *lps,RecordID idMap)
{
	DWORD idx=idMap%MAX_PLAYER_PERSISTSET_AGENT_BRIEF;
	LPSPersistSet_AgentBrief*persists=&lps->persistsetsAgentBrief[idx];

	return persists->Find(idMap);
}


LevelAgentBrief *LPS_FindPersistEntry_AgentBrief(LevelPlayerStates *lps,RecordID idMap,LevelGUID guid)
{
	DWORD idx=idMap%MAX_PLAYER_PERSISTSET_AGENT_BRIEF;
	LPSPersistSet_AgentBrief*persists=&lps->persistsetsAgentBrief[idx];

	LevelPersistData_AgentBrief*persist=persists->Find(idMap);
	if (!persist)
		return NULL;
	return persist->Find(guid);
}

LevelAgentBrief*LPS_QueryPersistEntry_AgentBrief(LevelPlayerStates *lps,RecordID idMap,LevelGUID guid)
{
	DWORD idx=idMap%MAX_PLAYER_PERSISTSET_AGENT_BRIEF;
	LPSPersistSet_AgentBrief*persists=&lps->persistsetsAgentBrief[idx];

	LevelPersistData_AgentBrief*persist=persists->Obtain(idMap);
	if (!persist)
		return NULL;
	persists->SetDirtyDB_Urgent();
	return persist->Obtain(guid);
}


void LPS_ErasePersistEntry_AgentBrief(LevelPlayerStates *lps,RecordID idMap,LevelGUID guid)
{
	DWORD idx=idMap%MAX_PLAYER_PERSISTSET_AGENT_BRIEF;
	LPSPersistSet_AgentBrief*persists=&lps->persistsetsAgentBrief[idx];
	LevelPersistData_AgentBrief*persist=persists->Find(idMap);
	if (!persist)
		return;
	persist->Erase(guid);
	persists->SetDirtyDB_Urgent();
}

void LPS_SetPersistEntry_AgentBrief(LevelPlayerStates *lps,RecordID idMap,LevelGUID guid,LevelAgentBrief &entry)
{
	DWORD idx=idMap%MAX_PLAYER_PERSISTSET_AGENT_BRIEF;
	LPSPersistSet_AgentBrief*persists=&lps->persistsetsAgentBrief[idx];
	LevelPersistData_AgentBrief*persist=persists->Obtain(idMap);
	if (!persist)
		return;
	persist->Set(guid,entry);
	persists->SetDirtyDB_Urgent();
}

LevelPersistData_LevelAI*LPS_FindPersistData_LevelAI(LevelPlayerStates *lps,RecordID idMap)
{
	DWORD idx=idMap%MAX_PLAYER_PERSISTSET_LEVELAI;
	LPSPersistSet_LevelAI*persists=&lps->persistsetsLevelAI[idx];

	return persists->Find(idMap);
}


LevelPersistEntry_LevelAI*LPS_FindPersistEntry_LevelAI(LevelPlayerStates *lps,RecordID idMap,StringID nmAI)
{
	DWORD idx=idMap%MAX_PLAYER_PERSISTSET_LEVELAI;
	LPSPersistSet_LevelAI*persists=&lps->persistsetsLevelAI[idx];

	LevelPersistData_LevelAI*persist=persists->Find(idMap);
	if (!persist)
		return NULL;
	return persist->Find(nmAI);
}

LevelPersistEntry_LevelAI *LPS_QueryPersistEntry_LevelAI(LevelPlayerStates *lps,RecordID idMap,StringID nmAI)
{
	DWORD idx=idMap%MAX_PLAYER_PERSISTSET_LEVELAI;
	LPSPersistSet_LevelAI*persists=&lps->persistsetsLevelAI[idx];

	LevelPersistData_LevelAI*persist=persists->Obtain(idMap);
	if (!persist)
		return NULL;
	persists->SetDirtyDB_Urgent();
	return persist->Obtain(nmAI);
}


void LPS_ErasePersistEntry_LevelAI(LevelPlayerStates *lps,RecordID idMap,StringID nmAI)
{
	DWORD idx=idMap%MAX_PLAYER_PERSISTSET_LEVELAI;
	LPSPersistSet_LevelAI*persists=&lps->persistsetsLevelAI[idx];
	LevelPersistData_LevelAI*persist=persists->Find(idMap);
	if (!persist)
		return;
	persist->Erase(nmAI);
	persists->SetDirtyDB_Urgent();
}

void LPS_SetPersistEntry_LevelAI(LevelPlayerStates *lps,RecordID idMap,StringID nmAI,LevelPersistEntry_LevelAI &entry)
{
	DWORD idx=idMap%MAX_PLAYER_PERSISTSET_LEVELAI;
	LPSPersistSet_LevelAI*persists=&lps->persistsetsLevelAI[idx];
	LevelPersistData_LevelAI*persist=persists->Obtain(idMap);
	if (!persist)
		return;
	persist->Set(nmAI,entry);
	persists->SetDirtyDB_Urgent();
}


LevelSkillGrade LPS_GetSkillGrade(LevelPlayerStates *lps,CLevelRecords *records,RecordID idSkill)
{
	for (int i=0;i<lps->skills.items.size();i++)
	{
		LevelItemState *item=&lps->skills.items[i];
		LevelRecordItem *recItem=records->GetItem(item->tid);
		if (!recItem)
			continue;

		if (recItem->skill==idSkill)
			return (LevelSkillGrade)item->grd;
	}
	return LevelSkillGrade_Invalid;
}

DWORD LPS_GetSkillStack(LevelPlayerStates *lps,CLevelRecords *records,RecordID idSkill)
{
	for (int i=0;i<lps->skills.items.size();i++)
	{
		LevelItemState *item=&lps->skills.items[i];
		LevelRecordItem *recItem=records->GetItem(item->tid);
		if (!recItem)
			continue;

		if (recItem->skill==idSkill)
			return (DWORD)item->nStack;
	}
	return 0;
}

BOOL LPS_DecSkillStack(LevelPlayerStates *lps,CLevelRecords *records,RecordID idSkill,DWORD nStack)
{
	for (int i=0;i<lps->skills.items.size();i++)
	{
		LevelItemState *item=&lps->skills.items[i];
		LevelRecordItem *recItem=records->GetItem(item->tid);
		if (!recItem)
			continue;

		if (recItem->skill==idSkill)
		{
			if (item->nStack>0)
			{
				if (item->nStack>nStack)
					item->nStack-=(WORD)nStack;
				else
					item->nStack=0;

				lps->skills.SetDirtyDB_Urgent();
				lps->SetDirtyClient();//标记为需要更新给client
				return TRUE;
			}
		}
	}
	return FALSE;
}


BOOL LPS_ExistSkill(LevelPlayerStates *lps,CLevelRecords *records,RecordID idSkill)
{
	for (int i=0;i<lps->skills.items.size();i++)
	{
		LevelItemState *item=&lps->skills.items[i];
		LevelRecordItem *recItem=records->GetItem(item->tid);
		if (!recItem)
			continue;

		if (recItem->skill==idSkill)
			return TRUE;
	}
	return FALSE;
}


BOOL LPS_AddSkillItem(LevelPlayerStates *lps,CLevelRecords *records,RecordID idSkillItem,DWORD nStack)
{
	for (int i=0;i<lps->skills.items.size();i++)
	{
		LevelItemState *item=&lps->skills.items[i];
		if (item->tid==idSkillItem)
		{
			item->nStack+=(WORD)nStack;

			lps->skills.SetDirtyDB_Urgent();
			return TRUE;
		}
	}

	//不存在,新建一个
	LevelItemState itemNew;
	itemNew.tid=idSkillItem;
	itemNew.nStack=(WORD)nStack;
	itemNew.grd=1;

	lps->skills.items.push_back(itemNew);

	lps->skills.SetDirtyDB_Urgent();
	lps->SetDirtyClient();
	return TRUE;
}


DWORD LPS_CalcItemStackCount(LevelPlayerStates *lps,RecordID idItem)
{
	DWORD sum=0;
	if (lps)
	{
		for (int i=0;i<ARRAY_SIZE(lps->bags);i++)
		{
			if (lps->bags[i].bActivated)
				sum+=lps->bags[i].CalcItemStackCount(idItem);
		}
	}
	return sum;
}

BOOL LPS_CheckOwnArtifact(LevelPlayerStates *lps,RecordID idItem)
{
	if (!lps)
		return FALSE;
	return lps->artifacts.Find(idItem);
}


BOOL LPS_CheckOwnArtifact(LevelPlayerStates *lps,CLevelRecords *records,LevelArtifactType tp)
{
	if (!lps)
		return FALSE;
	LPSArtifacts *artifacts=&lps->artifacts;
	for (int j=0;j<artifacts->items.size();j++)
	{
		LevelItemState *item=&artifacts->items[j];
		LevelRecordItem *rec=records->GetItem(item->tid);
		if (rec)
		{
			if (rec->tpArtifact==tp)
				return TRUE;
		}
	}
	return FALSE;
}


void LPS_SaveAbilities(LevelPlayerStates *lps,CLevelAbilities *abilities)
{
	DP_BeginSave(dp,lps->abilities.data);
	abilities->Save(dp);
	DP_EndSave();
	lps->abilities.SetDirtyDB_Urgent();
	lps->SetDirtyClient();
}

void LPS_LoadAbilities(LevelPlayerStates *lps,CLevelAbilities *abilities)
{
	CDataPacket dp;
	dp.SetDataBufferPointer(&lps->abilities.data[0]);
	abilities->Load(dp);
}

LevelItemState *LPS_FindArtifact(LevelPlayerStates *lps,RecordID idArtifactItem)
{
	for (int i=0;i<lps->artifacts.items.size();i++)
	{
		LevelItemState *item=&lps->artifacts.items[i];
		if (item->tid==idArtifactItem)
			return item;
	}

	for (int i=0;i<ARRAY_SIZE(lps->equip.parts);i++)
	{
		if (lps->equip.parts[i].tid==idArtifactItem)
			return &lps->equip.parts[i];
	}

	return NULL;
}


BOOL LPS_AddArtifact(LevelPlayerStates *lps,CLevelRecords *records,RecordID idArtifactItem,int nStack)
{
	LevelRecordItem *recArtifact=records->GetItem(idArtifactItem);
	if (!recArtifact)
		return FALSE;
	if (recArtifact->tpArtifact==LevelArtifact_None)
		return FALSE;

	EquipPart part=EquipPart_Invalid;
	if (TRUE)
	{
		LevelRecordItemClass *recClss=records->GetItemClassOfItem(idArtifactItem);
		if (recClss)
			part=(EquipPart)recClss->part;
	}

	BOOL bEquip=TRUE;
	if ((part>=ARRAY_SIZE(lps->equip.parts))||(part==EquipPart_MagicItem))
		bEquip=FALSE;

	LevelItemState *itemOld=LPS_FindArtifact(lps,idArtifactItem);
	if (itemOld)
	{
		if (recArtifact->bAllowStack)
		{
			itemOld->nStack+=nStack;

			if (!bEquip)
				lps->artifacts.SetDirtyDB_Urgent();
			else
				lps->equip.SetDirtyDB_Urgent();
			lps->SetDirtyClient();
		}
		return TRUE;//已经有了
	}

	//不存在,新建一个
	LevelItemState itemNew;
	itemNew.tid=idArtifactItem;
	if (recArtifact->bAllowStack)
		itemNew.nStack=nStack;
	else
		itemNew.nStack=1;
	itemNew.grd=1;

	LevelItemState *stateArtifact=NULL;
	if (!bEquip)
		stateArtifact=lps->artifacts.AddItem(itemNew);
	else
	{
		stateArtifact=&lps->equip.parts[part];
		if (part==EquipPart_Weapon)
		{
			lps->equip.wpnActive=EquipPart_Weapon;
			if (lps->equip.parts[part].IsValid())
			{
				if (!lps->equip.parts[EquipPart_Weapon2nd].IsValid())
				{
					lps->equip.parts[EquipPart_Weapon2nd]=itemNew;
					stateArtifact=&lps->equip.parts[EquipPart_Weapon2nd];
					lps->equip.wpnActive=EquipPart_Weapon2nd;
				}
				else
				{
					lps->equip.parts[EquipPart_Weapon2nd]=lps->equip.parts[EquipPart_Weapon];
					lps->equip.parts[EquipPart_Weapon]=itemNew;
				}
			}
			else
				lps->equip.parts[EquipPart_Weapon]=itemNew;
		}
		else
			lps->equip.parts[part]=itemNew;
		bEquip=TRUE;
	}

	LPS_AddItemMemory(lps,idArtifactItem);

	if (!bEquip)
		lps->artifacts.SetDirtyDB_Urgent();
	else
		lps->equip.SetDirtyDB_Urgent();
	lps->SetDirtyClient();
	return TRUE;
}

BOOL LPS_RemoveArtifact(LevelPlayerStates *lps,CLevelRecords *records,LevelArtifactType tpArtifact)
{
	if(lps)
	{
		if (records)
		{
			for (int i=0;i<lps->artifacts.items.size();i++)
			{
				LevelRecordItem *recItem_=records->GetItem(lps->artifacts.items[i].tid);
				if(recItem_)
				{
					if (recItem_->tpArtifact==tpArtifact)
					{
						BOOL bNeedRemove=TRUE;
						if (recItem_->bAllowStack)
						{
							if (lps->artifacts.items[i].nStack>1)
							{
								lps->artifacts.items[i].nStack--;
								bNeedRemove=FALSE;
							}
						}
						if (bNeedRemove)
							lps->artifacts.items.erase(lps->artifacts.items.begin()+i);
						lps->artifacts.SetDirtyDB_Urgent();
						lps->SetDirtyClient();
						return TRUE;
					}
				}
			}

			for (int i=0;i<ARRAY_SIZE(lps->equip.parts);i++)
			{
				LevelRecordItem *recItem_=records->GetItem(lps->equip.parts[i].tid);
				if(recItem_)
				{
					if (recItem_->tpArtifact==tpArtifact)
					{
						lps->equip.parts[i].Clear();
						lps->equip.SetDirtyDB_Urgent();
						lps->SetDirtyClient();
						return TRUE;
					}
				}
			}

		}
	}

	return FALSE;

}


static LevelItemState *LPS_FindArtifact(LevelPlayerStates *lps,CLevelRecords *records,LevelArtifactType tp,LevelRecordItem *&recItem,BOOL &bEquip)
{
	bEquip=FALSE;
	recItem=NULL;
	if(lps)
	{
		if (records)
		{
			for (int i=0;i<lps->artifacts.items.size();i++)
			{
				LevelRecordItem *recItem_=records->GetItem(lps->artifacts.items[i].tid);
				if(recItem_)
				{
					if (recItem_->tpArtifact==tp)
					{
						recItem=recItem_;
						bEquip=FALSE;
						return &lps->artifacts.items[i];
					}
				}
			}

			for (int i=0;i<ARRAY_SIZE(lps->equip.parts);i++)
			{
				LevelRecordItem *recItem_=records->GetItem(lps->equip.parts[i].tid);
				if(recItem_)
				{
					if (recItem_->tpArtifact==tp)
					{
						recItem=recItem_;
						bEquip=TRUE;
						return &lps->equip.parts[i];
					}
				}
			}

		}
	}
	return NULL;
}

LevelItemState *LPS_FindArtifact(LevelPlayerStates *lps,CLevelRecords *records,LevelArtifactType tp)
{
	BOOL bEquip;
	LevelRecordItem *recItem;
	return LPS_FindArtifact(lps,records,tp,recItem,bEquip);
}

int LPS_DecArtifactStack(LevelPlayerStates *lps,CLevelRecords *records,LevelArtifactType tpArtifact,int nStack)
{
	BOOL bEquip;
	LevelRecordItem *recItem;
	LevelItemState *is=LPS_FindArtifact(lps,records,tpArtifact,recItem,bEquip);
	if (recItem->bAllowStack)
	{
		if (is->nStack<nStack)
			nStack=is->nStack;
		if (nStack>0)
		{
			is->nStack-=nStack;
			if (bEquip)
				lps->equip.SetDirtyDB_Urgent();
			else
				lps->artifacts.SetDirtyDB_Urgent();
			lps->SetDirtyClient();

			return nStack;
		}
	}

	return 0;
}

RecordID LPS_GetActiveShield(LevelPlayerStates *lps,CLevelRecords *records)
{
	if (!lps->equip.parts[EquipPart_Shield].IsValid())
		return RecordID_Invalid;

	if (lps->equip.parts[EquipPart_MagicItem].IsValid())
		return lps->equip.parts[EquipPart_Shield].tid;

	if ((lps->equip.wpnActive!=EquipPart_Weapon)&&(lps->equip.wpnActive!=EquipPart_Weapon2nd))
		return RecordID_Invalid;

	LevelRecordPosture *recPosture=records->GetPostureOfItem(lps->equip.parts[lps->equip.wpnActive].tid);
	if (recPosture)
	{
		switch(recPosture->tp)
		{
			case LevelPosture_ShortWpn:
			case LevelPosture_LongWpn:
			//XXXXX:more LevelPostureType
			{
				return lps->equip.parts[EquipPart_Shield].tid;
			}
		}
	}

	return RecordID_Invalid;
}

EquipPart LPS_EquipPartFromItem(LevelPlayerStates *lps,RecordID idItem)
{
	for (int i=0;i<ARRAY_SIZE(lps->equip.parts);i++)
	{
		if (lps->equip.parts[i].tid==idItem)
			return (EquipPart)i;
	}
	return EquipPart_Invalid;
}


//
EquipPart LPS_FindEquipedBow(LevelPlayerStates *lps,CLevelRecords *records)
{
	if ((lps->equip.wpnActive!=EquipPart_Weapon)&&(lps->equip.wpnActive!=EquipPart_Weapon2nd))
		return EquipPart_Invalid;
	if (lps->equip.parts[lps->equip.wpnActive].IsValid())
	{
		if (LevelUtil_IsBow(lps->equip.parts[lps->equip.wpnActive].tid,records))
			return lps->equip.wpnActive;
	}
	EquipPart part=(lps->equip.wpnActive==EquipPart_Weapon)?EquipPart_Weapon2nd:EquipPart_Weapon;
	if (lps->equip.parts[part].IsValid())
	{
		if (LevelUtil_IsBow(lps->equip.parts[part].tid,records))
			return part;
	}

	return EquipPart_Invalid;
}


void LPS_RemoveEquipment(LevelPlayerStates *lps,EquipPart part)
{
	if (part<EquipPart_Max)
	{
		lps->equip.parts[part].Clear();
		lps->equip.SetDirtyDB_Urgent();
		lps->SetDirtyClient();
	}
}

void LPS_AddItemMemory(LevelPlayerStates *lps,RecordID idItem)
{
	if (lps->itemmemory.items.find(idItem)==lps->itemmemory.items.end())
	{
		lps->itemmemory.items.insert(idItem);
		lps->itemmemory.SetDirtyDB_Urgent();
	}
}

BOOL LPS_CheckItemMemory(LevelPlayerStates *lps,RecordID idItem)
{
	if (!lps)
		return FALSE;
	if (lps->itemmemory.items.find(idItem)==lps->itemmemory.items.end())
		return FALSE;
	return TRUE;
}

BOOL LPS_GetSelectedSkill(LevelPlayerStates *lps,LevelFastTarget &target)
{
	if (!lps)
		return FALSE;
	target=lps->fasts.skillSel;
	return TRUE;
}


BOOL LPS_SetSelectedSkill(LevelPlayerStates *lps,LevelFastTarget &target)
{
	if (!lps)
		return FALSE;

	if (target.Equals(lps->fasts.skillSel))
		return FALSE;

	if (!target.IsEmpty())
	{
		if (!lps->fasts.skillSel.IsEmpty())
			return FALSE;
	}

	lps->fasts.skillSel=target;

	lps->fasts.SetDirtyDB_Low();
	lps->equip.SetDirtyDB_Low();

	return TRUE;
}

void LPS_AddKilling(LevelPlayerStates *lps,CLevelObj *lo)
{
	if (!lps)
		return;
	if (!lo)
		return;

	if (lo->GetType()==LevelObjType_Unit)
	{
		LevelRecordUnit *rec=((CLoUnit*)lo)->GetRec();
		if (rec)
		{
			RecordID id=rec->GetID();
			int idx;
			VEC_FIND_BY_ELEMENT(lps->killings.entries,idUnit,id,idx);
			if (idx!=-1)
				lps->killings.entries[idx].c++;
			else
			{
				LPSKillings::Entry e;
				e.idUnit=id;
				e.c=1;
				lps->killings.entries.push_back(e);
			}
			lps->killings.SetDirtyDB_Low();
		}
	}
}

void LPS_IncKillingHonor(LevelPlayerStates *lps,CLevelObj *lo)
{
	if (!lps)
		return;
	if (!lo)
		return;

	if (lo->GetType()==LevelObjType_Unit)
	{
		LevelRecordUnit *rec=((CLoUnit*)lo)->GetRec();
		if (rec)
		{
			if (rec->rateDrop.amntHnr>0)
			{
				lps->base.hnr+=rec->rateDrop.amntHnr;
				lps->base.SetDirtyDB_Urgent();
			}
		}
	}
}
