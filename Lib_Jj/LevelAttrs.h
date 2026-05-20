
#pragma once

#include "class/class.h"

#include "anim/animdefines.h"

#include "LevelDefines.h"
#include "MagicBoardDefines.h"

#include "LevelAttrs_DamageAttr.h"

struct LavMod;
struct Lav//Level Attr Value
{
	Lav()
	{
		memset(this,0,sizeof(*this));
	}

	void Reset(WORD v_,WORD max_)
	{
		v=(float)v_;
		max=(float)max_;
		ver=0;
	}
	float Modify(float delta);//修改值
	float ModifyMax(float delta);//修改最大值,并且修改当前值(如果delta>0,当前值也要加delta,如果delta<0,则当前值不减,但会clamp到max)
	void MakeMod(float delta,BOOL bInstant,LavMod &mod);//修改值并且生成一个LavMod的数据包,用来传给客户端
	void MakeMaxMod(float delta,LavMod &mod);//修改值并且生成一个LavMod的数据包,用来传给客户端

	float GetRatio()	{		return max>0?((float)v)/((float)max):0.0f;	}


	void Save(CBitPacket *bp);
	void Load(CBitPacket *bp);

	void SetCur_Int(int v_)	{		v=(float)v_;	}
	void SetMax_Int(int v_)	{		max=(float)v_;	}
	void SetCur_Float(float v_)	{		v=v_;	}
	void SetMax_Float(float v_)	{		max=v_;	}
	int GetCur_Int()
	{
		return FloatToNearestInt(v);
	}
	int GetMax_Int()
	{
		return FloatToNearestInt(max);
	}
	float GetCur_Float()
	{
		return v;
	}
	float GetMax_Float()
	{
		return max;
	}

	WORD ver;
protected:
	float v;
	float max;
};



struct LevelRecordUnit;
struct LevelRecordAgent;
struct LevelPlayerStates;
struct LevelAttr_Base
{
	LevelAttr_Base()
	{
		grade=1;
		hpRecover=0;
		spRecover=0;
		str=0;
		magic=0;
		vita_=0;
		worm=0;
		hnr=0;
	}

	void Init(LevelRecordUnit *rec,LevelPlayerStates *lps,LevelGrade grd);
	void Init(LevelRecordAgent*rec);

	void WriteFirstSync(CBitPacket *bp,BOOL bPlayer);
	void ReadFirstSync(CBitPacket *bp,BOOL bPlayer);

	Lav hp;
	Lav sp;
	Lav spFull;

	LevelGrade grade;//等级

	BYTE hpRecover;
	BYTE spRecover;

	WORD str;
	WORD magic;

	BYTE vita_;
	WORD worm;

	DWORD hnr;

	LevelPain pain;

};

struct LevelAttr_BaseMod
{
	LevelAttr_BaseMod()
	{
		Zero();
	}
	void Zero()
	{
		hnrAdd=0;
		hpAdd=spAdd=0.0f;
		hpRecoverAdd=0;
	}
	DWORD hnrAdd;
	float hpAdd;
	float spAdd;
	int hpRecoverAdd;
};


struct LevelAttr_SpeedMod
{
	DEFINE_CLASS(LevelAttr_SpeedMod)
	LevelAttr_SpeedMod()
	{
		Zero();
	}
	void CopyFrom(LevelAttr_SpeedMod &src)
	{
		ias=src.ias;
		ims=src.ims;
	}
	void Zero()
	{
		ias=0;
		ims=0;
	}

	void WriteFirstSync(CBitPacket *bp);


	short ias;//攻击速度加成,单位为百分点
	short ims;//移动速度加成单位为百分点
};

struct LevelAttr_Drop
{
	LevelAttr_Drop()
	{
		memset(this,0,sizeof(*this));
	}
	void Init(LevelRecordUnit *rec);
	float rateGold;
	float rateGem;
};

struct LevelPlayerStates;
struct LevelAttr_Resource
{
	DEFINE_CLASS(LevelAttr_Resource);

	void Init(LevelPlayerStates *lps);

	void WriteFirstSync(CBitPacket *bp);

	Lav *Get(LevelResourceType tpRes)
	{
		if ((tpRes>=LevelRes_Max)||(tpRes<=LevelResource_None))
			return NULL;
		return &res[((int)tpRes)-1];
	}

	void CopyFrom(LevelAttr_Resource &src)
	{
		memcpy(&res[0],&src.res[0],sizeof(res));
	}

protected:
	Lav res[LevelRes_Max-1];

};

struct LevelAttr_Temple
{
	DEFINE_CLASS(LevelAttr_Temple);

	void Init(LevelPlayerStates *lps);

	void WriteSync(CBitPacket *bp);

	DWORD GetCount(LevelTempleType tpTemple)
	{
		if ((tpTemple>=LevelTemple_Max)||(tpTemple<=LevelTemple_None))
			return 0;
		return temples[((int)tpTemple)-1].GetAltarCount();
	}

	int Test(LevelTempleType tpTemple,DWORD iAltar)
	{
		if ((tpTemple>=LevelTemple_Max)||(tpTemple<=LevelTemple_None))
			return 0;
		return temples[((int)tpTemple)-1].TestAltar(iAltar);
	}

	void Set(LevelTempleType tpTemple,DWORD iAltar)
	{
		if ((tpTemple>=LevelTemple_Max)||(tpTemple<=LevelTemple_None))
			return;
		(temples[((int)tpTemple)-1]).SetAltar(iAltar);
	}

	void CopyFrom(LevelAttr_Temple &src)
	{
		memcpy(&temples[0],&src.temples[0],sizeof(temples));
	}

protected:
	LevelTempleInfo temples[LevelTemple_Max-1];

};

//Magic Board Resource
struct LevelAttr_MagicBoard
{
	DEFINE_CLASS(LevelAttr_MagicBoard);

	void Init(LevelPlayerStates *lps);

	void WriteFirstSync(CBitPacket *bp);

	void CopyFrom(LevelAttr_MagicBoard &src)
	{
		memcpy(&res[0],&src.res[0],sizeof(res));
	}

	Lav res[MBRes_ActualMax];

};

