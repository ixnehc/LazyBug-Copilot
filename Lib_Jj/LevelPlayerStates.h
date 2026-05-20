#pragma once

#include "gds/GObj.h"

#include "datapacket/BitPacket.h"

#include "anim/animdefines.h"


#include "strlib/strlibdefines.h"

#include "LevelDefines.h"
#include "LevelRtnuDefines.h"

#include "LevelItemState.h"
#include "LevelPersistData.h"

class CLevelRecords;
class CLevelObj;



#define LPS_FIELD(nm_)																\
{																									\
	LPSFieldInfo info;																		\
	info.off=(DWORD)((BYTE*)&t.nm_-(BYTE*)&t);											\
	info.nm=#nm_;																			\
	infoes.push_back(info);															\
}

typedef DWORD LPSVer;
#define LPSVer_Invalid 0

struct LevelPlayerStates;
struct LPSField
{

	LPSField()
	{
		owner=NULL;
		bDirtyDB=0;
		bValid=0;
		bSubscribe=0;
		verDB=1;
		verClient=0;
	}

	virtual void Save(CDataPacket &dp)=0;
	virtual void Load(CDataPacket &dp)=0;
	virtual void CopyFrom(LPSField *src)=0;
	virtual void Clear()=0;

	virtual BOOL IsAutoSync()	{		return TRUE;	}

	void ClearDirtyDB()
	{
		bDirtyDB=FALSE;
	}
	void SetDirtyDB_Low();
	void SetDirtyDB_High();
	void SetDirtyDB_Urgent();
	DWORD GetVerDB()	{		return verDB;	}


	void SetOwner(LevelPlayerStates *owner_)	{		owner=owner_;	}

	LevelPlayerStates *owner;
	DWORD bValid:1;
	DWORD bDirtyDB:1;
	DWORD bSubscribe:1;//Client是否订阅了这个field的状态改变
	LPSVer verDB;
	LPSVer verClient;

};

struct LPSBase:public LPSField
{
	virtual void CopyFrom(LPSField *src)
	{
		memcpy(&__start,&((LPSBase*)src)->__start,((BYTE*)&__end)-((BYTE*)&__start));
	}
	virtual void Save(CDataPacket &dp)
	{
		SaveGObj(dp,GetGObj());
	}
	virtual void Load(CDataPacket &dp)
	{
		LoadGObj(dp,GetGObj(),NULL);
	}
	virtual void Clear()
	{
		GetGObj()->Clear(FALSE);
		GetGObj()->Zero(FALSE);
	}

	DWORD *GetRes(LevelResourceType tpRes);
	LevelTempleInfo *GetTemple(LevelTempleType tpTemple);

DWORD __start;

	char nm[MAX_PLAYER_NAME];//角色名字
	RecordID idMap;//在哪个地图里面
	LevelPos pos;//在哪个位置

	DWORD iDay;//第几天,初始值为0,注意第一天这个值为1(而不是0)

	union
	{
		struct
		{
			DWORD bRest:1;//表示Player是否处于休息状态,初始值为1,如果为0,表示玩家正在iDay里战斗(还没有结束iDay)

		};
		DWORD flags;
	};


	BYTE clss;//PlayerClass_XXXX
	LevelGrade grd;//等级
	DWORD MaxHP;
	DWORD FullSP;

	DWORD gold_;
	DWORD gem_;
	DWORD soul_;
	DWORD labor_;
	DWORD crystal;
	DWORD demonblood;
	//XXXXX:More LevelResourceType

	WORD str;//力量
	WORD dex;//敏捷
	WORD magic;//魔法
	WORD ldr;//统御
	BYTE vita_;//生命虫数量
	WORD worm;//蛔虫数量
	DWORD hnr;//荣耀

	LevelTempleInfo templeSun;
	LevelTempleInfo templeMoon;
	LevelTempleInfo templeFire;
	LevelTempleInfo templeStar;
	LevelTempleInfo templeSand;
	LevelTempleInfo templeCraft;
	//XXXXX:More LevelTempleType

	RetinueUID sdRtnu;//seed of retinue uid

DWORD __end;


	BEGIN_GOBJ_PURE(LPSBase,1)

		GELEM_VARARRAY_INIT(char,nm,0);
		GELEM_VAR_INIT(RecordID,idMap,RecordID_Invalid);
		GELEM_VAR_INIT(LevelPos,pos,LevelPos_Invalid);
		GELEM_VAR_INIT(DWORD,iDay,0);
		GELEM_VAR_INIT(DWORD,flags,0);
		GELEM_VAR_INIT(BYTE,clss,0);
		GELEM_VAR_INIT(LevelGrade,grd,0);
		GELEM_VAR_INIT(DWORD,MaxHP,0);
		GELEM_VAR_INIT(DWORD,FullSP,0);

		GELEM_VAR_INIT(DWORD,gold_,0);
		GELEM_VAR_INIT(DWORD,gem_,0);
		GELEM_VAR_INIT(DWORD,soul_,0);
		GELEM_VAR_INIT(DWORD,labor_,0);
		GELEM_VAR_INIT(DWORD,crystal,0);
		GELEM_VAR_INIT(DWORD,demonblood,0);
		//XXXXX:More LevelResourceType

		GELEM_VAR_INIT(WORD,magic,0);
		GELEM_VAR_INIT(WORD,str,0);
		GELEM_VAR_INIT(WORD,dex,0);
		GELEM_VAR_INIT(WORD,magic,0);
		GELEM_VAR_INIT(WORD,ldr,0);
		GELEM_VAR_INIT(BYTE,vita_,0);
		GELEM_VAR_INIT(WORD,worm,0);
		GELEM_VAR_INIT(DWORD,hnr,0);
		GELEM_VAR_INIT(RetinueUID,sdRtnu,0);
		GELEM_VAR_INIT(LevelTempleInfo,templeSun,LevelTempleInfo());
		GELEM_VAR_INIT(LevelTempleInfo,templeMoon,LevelTempleInfo());
		GELEM_VAR_INIT(LevelTempleInfo,templeFire,LevelTempleInfo());
		GELEM_VAR_INIT(LevelTempleInfo,templeStar,LevelTempleInfo());
		GELEM_VAR_INIT(LevelTempleInfo,templeSand,LevelTempleInfo());
		GELEM_VAR_INIT(LevelTempleInfo,templeCraft,LevelTempleInfo());
		//XXXXX:More LevelTempleType
	END_GOBJ();

};

#define LPSEquip_CurVer 1


struct LPSEquip:public LPSField
{
	LPSEquip()
	{
		wpnActive=EquipPart_Invalid;
	}
	virtual void CopyFrom(LPSField *src0)
	{
		LPSEquip * src=(LPSEquip *)src0;
		for (int i=0;i<ARRAY_SIZE(parts);i++)
			parts[i].CopyFrom(&src->parts[i]);
		wpnActive=src->wpnActive;
	}
	virtual void Save(CDataPacket &dp)
	{
		dp.Data_NextByte()=LPSEquip_CurVer;
		for (int i=0;i<ARRAY_SIZE(parts);i++)
			parts[i].Save(dp);
		dp.Data_NextWord()=(WORD)wpnActive;
	}
	virtual void Load(CDataPacket &dp)
	{
		BYTE ver=dp.Data_NextByte();
		for (int i=0;i<ARRAY_SIZE(parts);i++)
			parts[i].Load(dp);
		wpnActive=(EquipPart)dp.Data_NextWord();
	}
	virtual void Clear()
	{
		for (int i=0;i<ARRAY_SIZE(parts);i++)
			parts[i].Clear();
		wpnActive=EquipPart_Invalid;
	}

	EquipPart GetActiveWpn()
	{
		if (wpnActive==EquipPart_Invalid)
			return EquipPart_Invalid;
		if (parts[wpnActive].IsValid())
			return wpnActive;
		if (parts[EquipPart_Weapon].IsValid())
			return EquipPart_Weapon;
		if (parts[EquipPart_Weapon2nd].IsValid())
			return EquipPart_Weapon2nd;
		return EquipPart_Invalid;
	}

	BOOL IsWeapon(RecordID idItem)
	{
		if ((parts[EquipPart_Weapon].tid==idItem)||(parts[EquipPart_Weapon2nd].tid==idItem))
			return TRUE;
		return FALSE;
	}
	BOOL IsShield(RecordID idItem)
	{
		if (parts[EquipPart_Shield].tid==idItem)
			return TRUE;
		return FALSE;
	}

	LevelItemState parts[EquipPart_Max];
	EquipPart wpnActive;
};

struct BagItemState:public LevelItemState
{
	void CopyFrom(BagItemState*src)
	{
		rc=src->rc;
		LevelItemState::CopyFrom((LevelItemState*)src);
	}
	virtual void Save(CDataPacket &dp)
	{
		LevelItemState::Save(dp);
		DP_WriteVar(dp,rc);
	}
	virtual void Load(CDataPacket &dp)
	{
		LevelItemState::Load(dp);
		DP_ReadVar(dp,rc);
	}
	virtual void Clear()
	{
		LevelItemState::Clear();
		rc.set(0,0,0,0);
	}

	i_math::rect_c rc;//在bag里的占用位置
};


#define LPSBag_CurVer 1
struct LPSBag:public LPSField
{
	LPSBag()
	{
		bActivated=0;
		verSlots=0;
	}
	virtual void CopyFrom(LPSField *src0)
	{
		Clear();
		LPSBag* src=(LPSBag*)src0;
		bActivated=src->bActivated;
		if (bActivated)
		{
			sz=src->sz;
			items.resize(src->items.size());
			for (int i=0;i<items.size();i++)
				items[i].CopyFrom(&src->items[i]);
		}
	}
	virtual void Save(CDataPacket &dp)
	{
		dp.Data_NextByte()=LPSBag_CurVer;
		dp.Data_NextByte()=bActivated;

		if(bActivated)
		{
			DP_WriteVar(dp,sz);
			dp.Data_NextWord()=items.size();
			for (int i=0;i<items.size();i++)
				items[i].Save(dp);
		}
	}
	virtual void Load(CDataPacket &dp)
	{
		Clear();

		BYTE ver=dp.Data_NextByte();
		bActivated=dp.Data_NextByte();
		if (bActivated)
		{
			DP_ReadVar(dp,sz);
			items.resize(dp.Data_NextWord());
			for (int i=0;i<items.size();i++)
				items[i].Load(dp);
		}
	}
	virtual void Clear()
	{
		for (int i=0;i<items.size();i++)
			items[i].Clear();
		items.clear();
		sz.set(0,0);
		bActivated=0;

		verSlots=0;
		slots.clear();
	}

	//根据Item,将所有被占据的slot填充1
	void UpdateSlots()
	{
		if (verSlots==verDB)
			return;//没变化
		DWORD w=sz.w;
		DWORD h=sz.h;
		slots.resize(((DWORD)sz.w)*((DWORD)sz.h));
		memset(slots.data(),0,slots.size()*sizeof(BYTE));

		i_math::rect_c rcTotal;
		rcTotal.set(0,0,sz.w,sz.h);

		for (int i=0;i<items.size();i++)
		{
			i_math::rect_c rc=items[i].rc;
			rc.clipAgainst(rcTotal);

			BYTE *p=&slots[rc.Top()*sz.w+rc.Left()];

			for (int j=0;j<rc.getHeight();j++)
			{
				memset(p,1,rc.getWidth());
				p+=sz.w;
			}
		}
	}

	void AddItem(LevelItemState &item,i_math::rect_c &rc)
	{
		items.resize(items.size()+1);
		items[items.size()-1].LevelItemState::CopyFrom(&item);
		items[items.size()-1].rc=rc;
	}

	DWORD CalcItemStackCount(RecordID idItem);

	BYTE bActivated:1;
	i_math::size2db sz;//宽高
	std::vector<BagItemState> items;

	DWORD verSlots;
	std::vector<BYTE> slots;//记录每个slots里面是否放了东西,1表示有东西,0表示没有东西
};


#define LPSArtifacts_CurVer 1
struct LPSArtifacts:public LPSField
{
	LPSArtifacts()
	{
	}
	virtual void CopyFrom(LPSField *src0)
	{
		Clear();
		LPSArtifacts* src=(LPSArtifacts*)src0;
		items.resize(src->items.size());
		for (int i=0;i<items.size();i++)
			items[i].CopyFrom(&src->items[i]);
	}
	virtual void Save(CDataPacket &dp)
	{
		dp.Data_NextByte()=LPSArtifacts_CurVer;

		dp.Data_NextWord()=items.size();
		for (int i=0;i<items.size();i++)
			items[i].Save(dp);
	}
	virtual void Load(CDataPacket &dp)
	{
		Clear();

		BYTE ver=dp.Data_NextByte();
		items.resize(dp.Data_NextWord());
		for (int i=0;i<items.size();i++)
			items[i].Load(dp);
	}
	virtual void Clear()
	{
		for (int i=0;i<items.size();i++)
			items[i].Clear();
		items.clear();
	}


	LevelItemState *AddItem(LevelItemState &item)
	{
		items.resize(items.size()+1);
		items[items.size()-1].LevelItemState::CopyFrom(&item);
		return &items[items.size()-1];
	}

	BOOL Find(RecordID idItem);

	std::deque<LevelItemState> items;
};

#define LPSItemMemory_CurVer 1
struct LPSItemMemory:public LPSField
{
	virtual void CopyFrom(LPSField *src0)
	{
		Clear();
		LPSItemMemory* src=(LPSItemMemory*)src0;
		items=src->items;
	}
	virtual void Save(CDataPacket &dp)
	{
		dp.Data_NextByte()=LPSItemMemory_CurVer;

		dp.Data_NextWord()=items.size();
		std::unordered_set<RecordID>::iterator it;
		for (it=items.begin();it!=items.end();it++)
			dp.Data_NextDword()=(*it);
	}
	virtual void Load(CDataPacket &dp)
	{
		Clear();

		BYTE ver=dp.Data_NextByte();
		DWORD c=dp.Data_NextWord();
		for (int i=0;i<c;i++)
		{
			RecordID id=dp.Data_NextDword();
			items.insert(id);
		}
	}
	virtual void Clear()
	{
		items.clear();
	}

	std::unordered_set<RecordID> items;
};



//拿在鼠标上的东西
#define LPSPickUp_CurVer 1
struct LPSPickUp:public LPSField
{
	virtual void CopyFrom(LPSField *src0)
	{
		LPSPickUp* src=(LPSPickUp*)src0;
		item.CopyFrom(&src->item);
	}
	virtual void Save(CDataPacket &dp)
	{
		dp.Data_NextByte()=LPSPickUp_CurVer;
		item.Save(dp);
	}
	virtual void Load(CDataPacket &dp)
	{
		BYTE ver=dp.Data_NextByte();
		item.Load(dp);
	}
	virtual void Clear()
	{
		item.Clear();
	}

	LevelItemState item;

};

#define LPSSkills_CurVer 1
struct LPSSkills:public LPSField
{
	virtual void CopyFrom(LPSField *src0)
	{
		Clear();
		LPSSkills* src=(LPSSkills*)src0;
		items.resize(src->items.size());
		for (int i=0;i<items.size();i++)
			items[i].CopyFrom(&src->items[i]);
	}
	virtual void Save(CDataPacket &dp)
	{
		dp.Data_NextByte()=LPSSkills_CurVer;
		dp.Data_NextWord()=items.size();
		for (int i=0;i<items.size();i++)
			items[i].Save(dp);
	}
	virtual void Load(CDataPacket &dp)
	{
		Clear();

		BYTE ver=dp.Data_NextByte();
		items.resize(dp.Data_NextWord());
		for (int i=0;i<items.size();i++)
			items[i].Load(dp);
	}
	virtual void Clear()
	{
		for (int i=0;i<items.size();i++)
			items[i].Clear();
		items.clear();
	}

	std::vector<LevelItemState> items;//技能书
};

//快捷键
#define LPSSkillFasts_CurVer 1
struct LPSSkillFasts:public LPSField
{
	LPSSkillFasts()
	{
		Zero();
	}
	void Zero()
	{
		memset(fasts,0,sizeof(fasts));
		memcpy(hotkeys,"QWERASDF",sizeof(hotkeys));
		idLSkill=idRSkill=RecordID_Invalid;
		nRtnuSkills=0;
	}
	struct Fast
	{
		BYTE bValid;
		BYTE bLorR;//鼠标左键还是右键
		RecordID idSkill;
	};

	struct RtnuSkill
	{
		RecordID idUnit;
		RecordID idSkill;
	};

	virtual void CopyFrom(LPSField *src0)
	{
		Clear();
		LPSSkillFasts* src=(LPSSkillFasts*)src0;
		idLSkill=src->idLSkill;
		idRSkill=src->idRSkill;
		memcpy(fasts,src->fasts,sizeof(fasts));
		memcpy(hotkeys,src->hotkeys,sizeof(hotkeys));
		nRtnuSkills=src->nRtnuSkills;
		memcpy(rtnuskills,src->rtnuskills,nRtnuSkills*sizeof(rtnuskills[0]));
	}
	virtual void Save(CDataPacket &dp)
	{
		dp.Data_NextByte()=LPSSkillFasts_CurVer;
		dp.Data_WriteSimple(idLSkill);
		dp.Data_WriteSimple(idRSkill);
		dp.Data_WriteData(fasts,sizeof(fasts));
		dp.Data_WriteData(hotkeys,sizeof(hotkeys));
		dp.Data_NextByte()=(BYTE)nRtnuSkills;
		dp.Data_WriteData(rtnuskills,nRtnuSkills*sizeof(rtnuskills[0]));
	}
	virtual void Load(CDataPacket &dp)
	{
		Clear();

		BYTE ver=dp.Data_NextByte();
		dp.Data_ReadSimple(idLSkill);
		dp.Data_ReadSimple(idRSkill);
		dp.Data_ReadData(fasts,sizeof(fasts));
		dp.Data_ReadData(hotkeys,sizeof(hotkeys));
		nRtnuSkills=dp.Data_NextByte();
		dp.Data_ReadData(rtnuskills,nRtnuSkills*sizeof(rtnuskills[0]));
	}
	virtual void Clear()
	{
		Zero();
	}

	void Bind(DWORD idx,RecordID idSkill,BOOL bLorR)
	{
		//先清除已经绑定idSkill的热键
		for (int i=0;i<ARRAY_SIZE(fasts);i++)
		{
			if (fasts[i].bValid)
			{
				if (fasts[i].idSkill==idSkill)
				{
					fasts[i].bValid=0;
					fasts[i].idSkill=RecordID_Invalid;
				}
			}
		}
		if (idx<ARRAY_SIZE(fasts))
		{
			fasts[idx].bValid=1;
			fasts[idx].idSkill=idSkill;
			fasts[idx].bLorR=bLorR;
		}
	}

	BOOL AddRtnuSkill(RecordID idRtnu,RecordID idSkill)
	{
		for(int i=0;i<nRtnuSkills;i++)
		{
			if (rtnuskills[i].idUnit==idRtnu)
			{
				rtnuskills[i].idSkill=idSkill;
				return TRUE;
			}
		}
		if (nRtnuSkills>=ARRAY_SIZE(rtnuskills))
			return FALSE;
		rtnuskills[nRtnuSkills].idUnit=idRtnu;
		rtnuskills[nRtnuSkills].idSkill=idSkill;
		nRtnuSkills++;
		return TRUE;
	}

	RecordID FindRtnuSkill(RecordID idUnit)
	{
		for (int i=0;i<nRtnuSkills;i++)
		{
			if (rtnuskills[i].idUnit==idUnit)
				return rtnuskills[i].idSkill;
		}
		return RecordID_Invalid;
	}

	RecordID idLSkill;//当前左键技能
	RecordID idRSkill;//当前右键技能
	Fast fasts[LevelSkillMaxFast];
	char hotkeys[LevelSkillMaxFast];
	RtnuSkill rtnuskills[LEVEL_MAX_RTNUTYPE];
	DWORD nRtnuSkills;

};

struct LPSNpcData
{
	DEFINE_CLASS(LPSNpcData);

	LPSNpcData()
	{
		state=NPCState_Default;
		idMap=RecordID_Invalid;
	}

	NPCState state;
	std::vector<BYTE>dataBhv;//行为图的保存数据

	//出现位置(当这个NPC在一个新的世界里出现时,在哪个位置出现)
	RecordID idMap;
	LevelPos pos;

};

#define LPSNpcSet_CurVer 1
struct LPSNpcSet:public LPSField
{
	virtual void Save(CDataPacket &dp);
	virtual void Load(CDataPacket &dp);
	virtual void CopyFrom(LPSField *src);
	virtual void Clear();

	virtual BOOL IsAutoSync()	{		return FALSE;	}

	LPSNpcData *FindData(RecordID id)
	{
		std::unordered_map<RecordID,LPSNpcData *>::iterator it=datas.find(id);
		if (it==datas.end())
			return NULL;
		return (*it).second;
	}
	LPSNpcData *ObtainData(RecordID id)
	{
		LPSNpcData *p=FindData(id);
		if (!p)
		{
			p=Class_New2(LPSNpcData);
			datas[id]=p;
		}
		return p;
	}

	std::unordered_map<RecordID,LPSNpcData *>datas;
};


struct LPSRetinueData
{
	LPSRetinueData()
	{
		tp=Retinue_None;
	}
	DEFINE_CLASS(LPSRetinueData);
	RetinueUID uid;
	RetinueType tp;
	RecordID idRec;//LevelRecordUnit的RecordID
	LevelGrade grd_;
	EquipSetPick iPickedEquipSet;
	LevelMoveMethod method;
};

#define LPSRetinueSet_CurVer 1
struct LPSRetinueSet:public LPSField
{
	LPSRetinueSet()
	{
	}

	virtual void Save(CDataPacket &dp);
	virtual void Load(CDataPacket &dp);
	virtual void CopyFrom(LPSField *src);
	virtual void Clear();

	virtual BOOL IsAutoSync()	{		return FALSE;	}

	LPSRetinueData*Find(RetinueUID uid)
	{
		std::unordered_map<RetinueUID,LPSRetinueData*>::iterator it=datas.find(uid);
		if (it==datas.end())
			return NULL;
		return (*it).second;
	}

	LPSRetinueData*New(RetinueUID uid,RetinueType tp,RecordID idRec,LevelGrade grd,LevelMoveMethod method,EquipSetPick iPickedEquipSet)
	{
		LPSRetinueData *p=Class_New2(LPSRetinueData);
		p->tp=tp;
		p->uid=uid;
		p->idRec=idRec;
		p->grd_=grd;
		p->method=method;
		p->iPickedEquipSet=iPickedEquipSet;

		datas[p->uid]=p;
		return p;
	}

	std::unordered_map<RetinueUID,LPSRetinueData*>datas;
};

class CLevelExploreMap;
#define LPSExploreMapSet_CurVer 1
struct LPSExploreMapSet:LPSField
{
	virtual void Save(CDataPacket &dp);
	virtual void Load(CDataPacket &dp);
	virtual void CopyFrom(LPSField *src);
	virtual void Clear();

	virtual BOOL IsAutoSync()	{		return FALSE;	}

	void ClearDyn();

	LevelExploreMaps Find(RecordID idMap);
	LevelExploreMaps New(RecordID idMap,i_math::recti &rcMap);

	std::unordered_map<RecordID,LevelExploreMaps>mps;
};


struct LevelPersistData_Agent;
#define LPSPersistSet_Agent_CurVer 1
struct LPSPersistSet_Agent:LPSField
{
	virtual void Save(CDataPacket &dp);
	virtual void Load(CDataPacket &dp);
	virtual void CopyFrom(LPSField *src);
	virtual void Clear();

	virtual BOOL IsAutoSync()	{		return FALSE;	}

	LevelPersistData_Agent*Find(RecordID idMap);
	LevelPersistData_Agent*Obtain(RecordID idMap);

	std::unordered_map<RecordID,LevelPersistData_Agent*>persists;
};

struct LevelPersistData_AgentS;
#define LPSPersistSet_AgentS_CurVer 1
struct LPSPersistSet_AgentS:LPSField
{
	virtual void Save(CDataPacket &dp);
	virtual void Load(CDataPacket &dp);
	virtual void CopyFrom(LPSField *src);
	virtual void Clear();

	virtual BOOL IsAutoSync()	{		return FALSE;	}

	LevelPersistData_AgentS*Find(RecordID idMap);
	LevelPersistData_AgentS*Obtain(RecordID idMap);

	std::unordered_map<RecordID,LevelPersistData_AgentS*>persists;
};

struct LevelPersistData_AgentBrief;
#define LPSPersistSet_AgentBrief_CurVer 1
struct LPSPersistSet_AgentBrief:LPSField
{
	virtual void Save(CDataPacket &dp);
	virtual void Load(CDataPacket &dp);
	virtual void CopyFrom(LPSField *src);
	virtual void Clear();
	void ClearCur();

	virtual BOOL IsAutoSync()	{		return FALSE;	}

	LevelPersistData_AgentBrief*Find(RecordID idMap);
	LevelPersistData_AgentBrief*Obtain(RecordID idMap);

	std::unordered_map<RecordID,LevelPersistData_AgentBrief*>persists;
};


struct LevelPersistData_LevelAI;
#define LPSPersistSet_LevelAI_CurVer 1
struct LPSPersistSet_LevelAI:LPSField
{
	virtual void Save(CDataPacket &dp);
	virtual void Load(CDataPacket &dp);
	virtual void CopyFrom(LPSField *src);
	virtual void Clear();

	virtual BOOL IsAutoSync()	{		return FALSE;	}

	LevelPersistData_LevelAI*Find(RecordID idMap);
	LevelPersistData_LevelAI*Obtain(RecordID idMap);

	std::unordered_map<RecordID,LevelPersistData_LevelAI*>persists;
};

struct LPSMisc:LPSField
{
	virtual void Save(CDataPacket &dp)
	{
		SaveGObj(dp,GetGObj());
	}
	virtual void Load(CDataPacket &dp)
	{
		LoadGObj(dp,GetGObj(),NULL);
	}
	virtual void CopyFrom(LPSField *src0)
	{
		LPSMisc* src=(LPSMisc*)src0;
		GetGObj()->Copy(src->GetGObj());
	}
	virtual void Clear()
	{
		GetGObj()->Clear(FALSE);
		GetGObj()->Zero(FALSE);
	}


	virtual BOOL IsAutoSync()	{		return FALSE;	}

	BEGIN_GOBJ_PURE(LPSMisc,1)

		GELEM_VAR_INIT(BYTE,iDayVendor,0);
		GELEM_VAR_INIT(short,moodVendor,0);
		GELEM_VAR_INIT(BYTE,nGoldMines,0);

	END_GOBJ();

	BYTE iDayVendor;//最近碰到Vender在哪一天
	short moodVendor;//vender的心情
	BYTE nGoldMines;
};

struct LPSEntrances:LPSField
{
	struct Entry
	{
		Entry()
		{
			idMap=RecordID_Invalid;
			idCP=StringID_Invalid;
		}
		RecordID idMap;
		StringID idCP;
	};

	virtual void CopyFrom(LPSField *src0)
	{
		LPSEntrances* src=(LPSEntrances*)src0;
		GetGObj()->Copy(src->GetGObj());
	}
	virtual void Save(CDataPacket &dp)
	{
		SaveGObj(dp,GetGObj());
	}
	virtual void Load(CDataPacket &dp)
	{
		LoadGObj(dp,GetGObj(),NULL);
	}
	virtual void Clear()	
	{	
		GetGObj()->Clear(FALSE);
	}

	BOOL Exist(RecordID idMap,StringID idCP)
	{
		for (int i=0;i<entries.size();i++)
		{
			if ((entries[i].idMap==idMap)&&(entries[i].idCP==idCP))
				return TRUE;
		}
		return FALSE;
	}


	BEGIN_GOBJ_PURE(LPSEntrances,1)

		GELEM_VARVECTOR(Entry,entries);

	END_GOBJ();


	std::vector<Entry> entries;
};


struct LPSAbilities:LPSField
{
	virtual void CopyFrom(LPSField *src0)
	{
		LPSAbilities* src=(LPSAbilities*)src0;
		GetGObj()->Copy(src->GetGObj());
	}
	virtual void Save(CDataPacket &dp)
	{
		SaveGObj(dp,GetGObj());
	}
	virtual void Load(CDataPacket &dp)
	{
		LoadGObj(dp,GetGObj(),NULL);
	}
	virtual void Clear()	
	{	
		GetGObj()->Clear(FALSE);
	}


	BEGIN_GOBJ_PURE(LPSAbilities,1)

		GELEM_VARVECTOR(BYTE,data);

	END_GOBJ();


	std::vector<BYTE> data;
};


#define LPSFasts_CurVer 1
struct LPSFasts:public LPSField
{
	LPSFasts()
	{
		Zero();
	}
	void Zero()
	{
		skillSel.Zero();
	}

	struct Fast
	{
		Fast()
		{
			Zero();
		}

		void Zero()
		{
			key=0;
			target.Zero();
		}

		char key;
		LevelFastTarget target;
	};

	virtual void CopyFrom(LPSField *src0)
	{
		Clear();
		LPSFasts* src=(LPSFasts*)src0;
		skillSel=src->skillSel;
		fasts=src->fasts;
	}
	virtual void Save(CDataPacket &dp)
	{
		dp.Data_NextByte()=LPSFasts_CurVer;
		dp.Data_WriteSimple(skillSel);
		
		DP_WriteVector(dp,fasts);
	}
	virtual void Load(CDataPacket &dp)
	{
		Clear();

		BYTE ver=dp.Data_NextByte();
		dp.Data_ReadSimple(skillSel);
		DP_ReadVector(dp,fasts);
	}
	virtual void Clear()
	{
		fasts.clear();
		Zero();
	}

	void Bind(char key,LevelFastTarget &target)
	{
		//先清除已有的绑定
		for (int i=0;i<fasts.size();i++)
		{
			if (fasts[i].target.Equals(target))
			{
				fasts[i]=fasts[fasts.size()-1];
				fasts.pop_back();
				break;
			}
		}
		for (int i=0;i<fasts.size();i++)
		{
			if (fasts[i].key==key)
			{
				fasts[i]=fasts[fasts.size()-1];
				fasts.pop_back();
				break;
			}
		}

		Fast fast;
		fast.key=key;
		fast.target=target;
		fasts.push_back(fast);
	}

	void SetSelectedSkill(LevelFastTarget &target)
	{
		skillSel=target;
	}

	Fast* FindFastByKey(char key)
	{
		for (int i=0;i<fasts.size();i++)
		{
			if (fasts[i].key==key)
				return &fasts[i];
		}
		return NULL;
	}

// 	//返回TRUE,表示target被设为当前术能
// 	//返回FALSE,表示weapon被设为当前术能
// 	BOOL ToggleWithWeapon(LevelFastTarget &target)
// 	{
// 		if (btn.Equals(target))
// 		{
// 			btn=LevelFastTarget::GetWeapon();
// 			return FALSE;
// 		}
// 		else
// 		{
// 			btn=target;
// 			return TRUE;
// 		}
// 	}

	char FindHotKey(LevelAbilityType tp)
	{
		for (int i=0;i<fasts.size();i++)
		{
			if (fasts[i].target.tp==tp)
				return fasts[i].key;
		}
		return 0;
	}

	char FindHotKey(LevelFastTarget &target)
	{
		for (int i=0;i<fasts.size();i++)
		{
			if (fasts[i].target.Equals(target))
				return fasts[i].key;
		}
		return 0;
	}

	LevelFastTarget skillSel;
	std::vector<Fast> fasts;

};

#define LPSKillings_CurVer 1
struct LPSKillings:public LPSField
{
	LPSKillings()
	{
		Zero();
	}
	void Zero()
	{
	}


	virtual void CopyFrom(LPSField *src0)
	{
		Clear();
		LPSKillings* src=(LPSKillings*)src0;
		entries=src->entries;
	}
	virtual void Save(CDataPacket &dp)
	{
		dp.Data_NextByte()=LPSFasts_CurVer;
		DP_WriteVector(dp,entries);
	}
	virtual void Load(CDataPacket &dp)
	{
		Clear();

		BYTE ver=dp.Data_NextByte();
		DP_ReadVector(dp,entries);
	}
	virtual void Clear()
	{
		Zero();
	}

	struct Entry
	{
		Entry()
		{
			idUnit=RecordID_Invalid;
			c=0;
		}
		RecordID idUnit;
		DWORD c;
	};

	std::vector<Entry> entries;

};



#define MAX_PLAYER_BAG (8)
#define MAX_PLAYER_NPCSET (8)
#define MAX_PLAYER_RETINUESET (8)
#define MAX_PLAYER_EXPLOREMAPSET (16)
#define MAX_PLAYER_PERSISTSET_AGENT (16)
#define MAX_PLAYER_PERSISTSET_AGENT_S (16)
#define MAX_PLAYER_PERSISTSET_AGENT_BRIEF (16)
#define MAX_PLAYER_PERSISTSET_LEVELAI (16)
struct LevelPlayerStates
{
	LevelPlayerStates()
	{
		ClearDirtyDB();
		verDB=1;

		base.SetOwner(this);
		equip.SetOwner(this);
		for (int i=0;i<MAX_PLAYER_BAG;i++)
			bags[i].SetOwner(this);
		pickup.SetOwner(this);
		skills.SetOwner(this);
		skillfasts.SetOwner(this);
		for (int i=0;i<MAX_PLAYER_NPCSET;i++)
			npcsets[i].SetOwner(this);
		for (int i=0;i<MAX_PLAYER_RETINUESET;i++)
			rtnusets[i].SetOwner(this);
		for (int i=0;i<MAX_PLAYER_EXPLOREMAPSET;i++)
			emsets[i].SetOwner(this);
		for (int i=0;i<MAX_PLAYER_PERSISTSET_AGENT;i++)
			persistsetsAgent[i].SetOwner(this);
		for (int i=0;i<MAX_PLAYER_PERSISTSET_AGENT_S;i++)
			persistsetsAgentS[i].SetOwner(this);
		for (int i=0;i<MAX_PLAYER_PERSISTSET_AGENT_BRIEF;i++)
			persistsetsAgentBrief[i].SetOwner(this);
		for (int i=0;i<MAX_PLAYER_PERSISTSET_LEVELAI;i++)
			persistsetsLevelAI[i].SetOwner(this);
		entrances.SetOwner(this);
		abilities.SetOwner(this);
		artifacts.SetOwner(this);
		itemmemory.SetOwner(this);
		fasts.SetOwner(this);
		misc.SetOwner(this);
		killings.SetOwner(this);
		//XXXXX:more LPS Field
	}

	//注意:各个field的变量名尽量不要改,改动后会将无法从DB里读出旧有的数据
	LPSBase base;
	LPSEquip equip;
	LPSBag bags[MAX_PLAYER_BAG];
	LPSPickUp pickup;
	LPSSkills skills;
	LPSSkillFasts skillfasts;
	LPSNpcSet npcsets[MAX_PLAYER_NPCSET];
	LPSRetinueSet rtnusets[MAX_PLAYER_NPCSET];
	LPSExploreMapSet emsets[MAX_PLAYER_EXPLOREMAPSET];
	LPSPersistSet_Agent persistsetsAgent[MAX_PLAYER_PERSISTSET_AGENT];
	LPSPersistSet_AgentS persistsetsAgentS[MAX_PLAYER_PERSISTSET_AGENT_S];
	LPSPersistSet_AgentBrief persistsetsAgentBrief[MAX_PLAYER_PERSISTSET_AGENT_BRIEF];
	LPSPersistSet_LevelAI persistsetsLevelAI[MAX_PLAYER_PERSISTSET_LEVELAI];
	LPSEntrances entrances;
	LPSAbilities abilities;
	LPSArtifacts artifacts;
	LPSItemMemory itemmemory;
	LPSFasts fasts;
	LPSKillings killings;
	LPSMisc misc;
	//XXXXX:more LPS Field

	void ClearDirtyDB()
	{
		bDirtyDB=FALSE;
		tWait=ANIMTICK_INFINITE;
	}
	void SetDirtyDB(AnimTick tWait_)
	{
		bDirtyDB=TRUE;
		verDB++;
		if (tWait_<tWait)
			tWait=tWait_;
	}

	void ClearDirtyClient()
	{
		bDirtyClient=0;
	}
	BOOL IsDirtyClient()
	{
		return bDirtyClient;
	}
	void SetDirtyClient()
	{
		bDirtyClient=1;
	}
	DWORD GetVer()	{		return verDB;	}
	DWORD bDirtyDB:1;
	DWORD bDirtyClient:1;
	LPSVer verDB:30;
	AnimTick tWait;//等多久保存到db里面
};


struct LPSFieldInfo
{
	const char *nm;
	DWORD off;
};
LPSFieldInfo *LPS_GetFieldInfos(DWORD &c);

void LPS_Build(LevelPlayerStates *lps,PlayerClass clss,const char *nm);
void LPS_Clear(LevelPlayerStates *lps);
void LPS_Copy(LevelPlayerStates *lpsTo,LevelPlayerStates *lpsFrom);
void LPS_SetValid(LevelPlayerStates *lps);
BOOL LPS_FindBagSlotForItem(i_math::rect_c &rcSlot,LevelPlayerStates *lps,DWORD iBag,DWORD wSlot,DWORD hSlot);

int LPS_FindFieldIdx(const char *nmField);//返回-1如果找不到
int LPS_FindPickUpIdx();
int LPS_FindBagIdx(DWORD iBag);
int LPS_FindEquipIdx();
int LPS_FindSkillsIdx();
int LPS_FindSkillFastsIdx();
int LPS_FindEntrancesIdx();
int LPS_FindAbilitiesIdx();
int LPS_FindArtifactsIdx();
int LPS_FindItemMemoryIdx();
int LPS_FindFastsIdx();
int LPS_FindMiscIdx();
int LPS_FindKillingsIdx();
//XXXXX:more LPS Field

BOOL LPS_IsPickUp(LevelPlayerStates *lps);

LPSNpcSet *LPS_FindNpcSet(LevelPlayerStates *lps,RecordID idRec);
LPSNpcData *LPS_FindNpcData(LevelPlayerStates *lps,RecordID idRec);
LPSNpcData *LPS_QueryNpcData(LevelPlayerStates *lps,RecordID idRec);

LPSRetinueData *LPS_FindRetinue(LevelPlayerStates *lps,RetinueUID uid);
LPSRetinueData *LPS_QueryRetinue(LevelPlayerStates *lps,RetinueUID uid);
LPSRetinueData *LPS_NewRetinue(LevelPlayerStates *lps,RetinueType tp,RecordID idRec,EquipSetPick iPickedEquipSet,LevelMoveMethod method);
void LPS_EraseRetinue(LevelPlayerStates *lps,RetinueUID uid);

LevelExploreMaps LPS_FindExploreMaps(LevelPlayerStates *lps,RecordID idMap);
LevelExploreMaps LPS_QueryExploreMaps(LevelPlayerStates *lps,RecordID idMap);
LevelExploreMaps LPS_NewExploreMaps(LevelPlayerStates *lps,RecordID idMap,i_math::recti &rcMap);

LevelPersistEntry_Agent *LPS_FindPersistEntry_Agent(LevelPlayerStates *lps,RecordID idMap,LevelGUID guid);
LevelPersistEntry_Agent *LPS_QueryPersistEntry_Agent(LevelPlayerStates *lps,RecordID idMap,LevelGUID guid);
void LPS_SetPersistEntry_Agent(LevelPlayerStates *lps,RecordID idMap,LevelGUID guid,LevelPersistEntry_Agent &entry);
LevelPersistData_Agent*LPS_FindPersistData_Agent(LevelPlayerStates *lps,RecordID idMap);
void LPS_ErasePersistEntry_Agent(LevelPlayerStates *lps,RecordID idMap,LevelGUID guid);

LevelPersistEntry_AgentS *LPS_FindPersistEntry_AgentS(LevelPlayerStates *lps,RecordID idMap,LevelGUID guid);
LevelPersistEntry_AgentS *LPS_QueryPersistEntry_AgentS(LevelPlayerStates *lps,RecordID idMap,LevelGUID guid);
void LPS_SetPersistEntry_AgentS(LevelPlayerStates *lps,RecordID idMap,LevelGUID guid,LevelPersistEntry_AgentS &entry);
LevelPersistData_AgentS*LPS_FindPersistData_AgentS(LevelPlayerStates *lps,RecordID idMap);
void LPS_ErasePersistEntry_AgentS(LevelPlayerStates *lps,RecordID idMap,LevelGUID guid);

LevelAgentBrief *LPS_FindPersistEntry_AgentBrief(LevelPlayerStates *lps,RecordID idMap,LevelGUID guid);
LevelAgentBrief *LPS_QueryPersistEntry_AgentBrief(LevelPlayerStates *lps,RecordID idMap,LevelGUID guid);
void LPS_SetPersistEntry_AgentBrief(LevelPlayerStates *lps,RecordID idMap,LevelGUID guid,LevelAgentBrief&entry);
LevelPersistData_AgentBrief*LPS_FindPersistData_AgentBrief(LevelPlayerStates *lps,RecordID idMap);
void LPS_ErasePersistEntry_AgentBrief(LevelPlayerStates *lps,RecordID idMap,LevelGUID guid);

LevelPersistEntry_LevelAI *LPS_FindPersistEntry_LevelAI(LevelPlayerStates *lps,RecordID idMap,StringID nmAI);
LevelPersistEntry_LevelAI *LPS_QueryPersistEntry_LevelAI(LevelPlayerStates *lps,RecordID idMap,StringID nmAI);
void LPS_SetPersistEntry_LevelAI(LevelPlayerStates *lps,RecordID idMap,StringID nmAI,LevelPersistEntry_LevelAI &entry);
LevelPersistData_LevelAI*LPS_FindPersistData_LevelAI(LevelPlayerStates *lps,RecordID idMap);
void LPS_ErasePersistEntry_LevelAI(LevelPlayerStates *lps,RecordID idMap,StringID nmAI);

LevelSkillGrade LPS_GetSkillGrade(LevelPlayerStates *lps,CLevelRecords *records,RecordID idSkill);
DWORD LPS_GetSkillStack(LevelPlayerStates *lps,CLevelRecords *records,RecordID idSkill);
BOOL LPS_DecSkillStack(LevelPlayerStates *lps,CLevelRecords *records,RecordID idSkill,DWORD nStack);
BOOL LPS_ExistSkill(LevelPlayerStates *lps,CLevelRecords *records,RecordID idSkill);//返回idSkill是否是一个已经掌握的技能
BOOL LPS_AddSkillItem(LevelPlayerStates *lps,CLevelRecords *records,RecordID idSkillItem,DWORD nStack);

DWORD LPS_CalcItemStackCount(LevelPlayerStates *lps,RecordID idItem);
void LPS_RemoveEquipment(LevelPlayerStates *lps,EquipPart part);
EquipPart LPS_EquipPartFromItem(LevelPlayerStates *lps,RecordID idItem);
EquipPart LPS_FindEquipedBow(LevelPlayerStates *lps,CLevelRecords *records);//优先返回ActiveWeapon的那个
RecordID LPS_GetActiveShield(LevelPlayerStates *lps,CLevelRecords *records);

class CLevelAbilities;
void LPS_SaveAbilities(LevelPlayerStates *lps,CLevelAbilities *abilities);
void LPS_LoadAbilities(LevelPlayerStates *lps,CLevelAbilities *abilities);


LevelItemState *LPS_FindArtifact(LevelPlayerStates *lps,RecordID idArtifactItem);
LevelItemState *LPS_FindArtifact(LevelPlayerStates *lps,CLevelRecords *records,LevelArtifactType tp);
BOOL LPS_AddArtifact(LevelPlayerStates *lps,CLevelRecords *records,RecordID idArtifactItem,int nStack);
BOOL LPS_RemoveArtifact(LevelPlayerStates *lps,CLevelRecords *records,LevelArtifactType tpArtifact);
int LPS_DecArtifactStack(LevelPlayerStates *lps,CLevelRecords *records,LevelArtifactType tpArtifact,int nStack);//返回实际减掉的Stack数量

void LPS_AddItemMemory(LevelPlayerStates *lps,RecordID idItem);
BOOL LPS_CheckItemMemory(LevelPlayerStates *lps,RecordID idItem);

BOOL LPS_SetSelectedSkill(LevelPlayerStates *lps,LevelFastTarget &target);
BOOL LPS_GetSelectedSkill(LevelPlayerStates *lps,LevelFastTarget &target);

void LPS_AddKilling(LevelPlayerStates *lps,CLevelObj *lo);
void LPS_IncKillingHonor(LevelPlayerStates *lps,CLevelObj *lo);
