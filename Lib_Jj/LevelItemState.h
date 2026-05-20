#pragma once

#include "class/class.h"
#include "records/records.h"
#include "LevelDefines.h"

#define MAX_ITEM_BUFF (16)
#define MAX_ITEM_EMBED (12)

typedef BYTE ItemBuffType;//A EItemBuffType value


//Item上的附加属性
struct ItemBuff
{
	ItemBuff()
	{
		tp=0;
	}
	ItemBuffType tp;//A EItemBuffType value

	void Set_AddMaxHP(int delta);
	void Set_AddFullSP(int delta);
	void Set_AddPhysDef(int delta);
	void Set_AddPhysDef_Base(int delta);
	void Set_AddPhysDef_Rate(int delta);//以百分点为单位
	void Set_AddMoveSpeed(int delta);
	void Set_AddAttackSpeed(int delta);
	void Set_AddAttackSpeedBow(int delta);
	void Set_AddFireResist(int delta);
	void Set_AddElecResist(int delta);
	void Set_AddColdResist(int delta);
	void Set_AddPoisonResist(int delta);
	void Set_AddPhysDmg_Bow(int delta);
	void Set_AddPhysDmgRate_Bow(int delta);

	//1,2,4字节参数各一个
	union
	{
		BYTE b;
		char c;
	};
	union
	{
		WORD w;
		short sh;
	};
};

//内嵌物
struct EmbedItemState
{
	EmbedItemState()
	{
		Zero();
	}
	void Zero()
	{
		tid=RecordID_Invalid;
		nBuffs=0;
	}
	void Clear()
	{
		Zero();
	}
	void CopyFrom(EmbedItemState *src)
	{
		tid=src->tid;
		nBuffs=src->nBuffs;
		idxSocket=src->idxSocket;
		memcpy(buffs,src->buffs,nBuffs*sizeof(ItemBuff));
	}
	void Save(CDataPacket &dp)
	{
		DP_WriteVar(dp,tid);
		DP_WriteVar(dp,idxSocket);
		DP_WriteVar(dp,nBuffs);
		dp.Data_WriteData(buffs,nBuffs*sizeof(ItemBuff));
	}
	void Load(CDataPacket &dp)
	{
		DP_ReadVar(dp,tid);
		DP_ReadVar(dp,idxSocket);
		DP_ReadVar(dp,nBuffs);
		dp.Data_ReadData(buffs,nBuffs*sizeof(ItemBuff));
	}
	RecordID tid;

	BYTE idxSocket;//嵌在第几个Socket
	WORD nBuffs;//有几个Buff
	ItemBuff buffs[MAX_ITEM_BUFF];
};

//内嵌物集合
struct ItemEmbeds
{
	DEFINE_CLASS(ItemEmbeds);
	ItemEmbeds()
	{
		Zero();
	}
	~ItemEmbeds()
	{
		Clear();
	}

	void Zero()
	{
		count=0;
	}

	void Clear()
	{
		Zero();
	}
	void CopyFrom(ItemEmbeds*src)
	{
		count=src->count;
		for (int i=0;i<count;i++)
			items[i].CopyFrom(&src->items[i]);
	}
	void Save(CDataPacket &dp)
	{
		DP_WriteVar(dp,count);
		for (int i=0;i<count;i++)
			items[i].Save(dp);
	}
	void Load(CDataPacket &dp)
	{
		DP_ReadVar(dp,count);
		for (int i=0;i<count;i++)
			items[i].Load(dp);
	}

	DWORD count;
	EmbedItemState items[MAX_ITEM_EMBED];
};

//Item
struct LevelItemState
{
	LevelItemState()
	{
		Zero();
	}

	void Zero()
	{
		tid=RecordID_Invalid;
		nBuffs=0;
		nStack=0;
		embeds=NULL;
		grd=0;
	}

	void Clear()
	{
		Safe_Class_Delete(embeds);
		Zero();
	}

	BOOL IsValid()	{		return tid!=RecordID_Invalid;	}

	void CopyFrom(LevelItemState *src)
	{
		tid=src->tid;
		nStack=src->nStack;
		nBuffs=src->nBuffs;
		memcpy(buffs,src->buffs,nBuffs*sizeof(ItemBuff));

		grd=src->grd;

		if (src->embeds)
		{
			if (!embeds)
				embeds=Class_New2(ItemEmbeds);
			embeds->CopyFrom(src->embeds);
		}
		else
		{
			if (embeds)
				embeds->Clear();
			Safe_Class_Delete(embeds);
		}
	}

	void Save(CDataPacket &dp)
	{
		DP_WriteVar(dp,tid);
		DP_WriteVar(dp,nStack);
		DP_WriteVar(dp,nBuffs);
		dp.Data_WriteData(buffs,nBuffs*sizeof(ItemBuff));

		DP_WriteVar(dp,grd);

		if (embeds)
		{
			dp.Data_NextByte()=1;
			embeds->Save(dp);
		}
		else
			dp.Data_NextByte()=0;
	}
	void Load(CDataPacket &dp)
	{
		Clear();

		DP_ReadVar(dp,tid);
		DP_ReadVar(dp,nStack);

		DP_ReadVar(dp,nBuffs);
		dp.Data_ReadData(buffs,nBuffs*sizeof(ItemBuff));

		DP_ReadVar(dp,grd);


		if (dp.Data_NextByte())
		{
			embeds=Class_New2(ItemEmbeds);
			embeds->Load(dp);
		}
	}

	void Swap(LevelItemState *other)
	{
		::Swap(tid,other->tid);
		::Swap(nStack,other->nStack);
		::Swap(nBuffs,other->nBuffs);
		::Swap(grd,other->grd);

		ItemBuff buffsT[MAX_ITEM_BUFF];
		memcpy(buffsT,buffs,MAX_ITEM_BUFF*sizeof(ItemBuff));
		memcpy(buffs,other->buffs,MAX_ITEM_BUFF*sizeof(ItemBuff));
		memcpy(other->buffs,buffsT,MAX_ITEM_BUFF*sizeof(ItemBuff));

		::Swap(embeds,other->embeds);
	}

	BOOL AddItemBuff(ItemBuff &buff);

	RecordID tid;//type id

	WORD nStack;//堆叠个数

	WORD nBuffs;//有几个Buff
	ItemBuff buffs[MAX_ITEM_BUFF];

	ItemEmbeds *embeds;

	LevelGrade grd;
};
