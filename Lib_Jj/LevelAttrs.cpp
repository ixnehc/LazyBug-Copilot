
#include "stdh.h"

#include "LevelAttrs.h"
#include "LevelOp.h"
#include "LevelPlayerStates.h"

#include "LevelRecordUnit.h"
#include "LevelRecordAgent.h"

#include "datapacket/BitPacket.h"

////////////////////////////////////////////////////////////////////////
//LevelAttrValue
float Lav::Modify(float delta)
{
	if (delta>0)
	{
		if (v+delta>max)
			delta=max-v;
	}
	else
	{
		if (((int)v)+delta<0)
			delta=-v;
	}
	v+=delta;

	ver++;//有变化
	return delta;
}

float Lav::ModifyMax(float delta)
{
	if (delta<0)
	{
		if (max+delta<0)
			delta=-max;
	}
	max+=delta;
	if (delta>0)
		v+=delta;
	if (v>max)
		v=max;

	ver++;//有变化
	return delta;
}


void Lav::MakeMod(float delta,BOOL bInstant,LavMod &mod)
{
	WORD verT=ver;
	delta=Modify(delta);
	mod.ver=verT;
	mod.delta=delta;
	mod.bInstant=bInstant;
	mod.bModMax=0;
}

void Lav::MakeMaxMod(float delta,LavMod &mod)
{
	WORD verT=ver;
	delta=ModifyMax(delta);
	mod.ver=verT;
	mod.delta=delta;
	mod.bInstant=1;//永远是立即模式
	mod.bModMax=1;
}


void Lav::Save(CBitPacket *bp)
{
	bp->Data_WriteSimple(v);
	bp->Data_WriteSimple(max);
	bp->Data_WriteSimple(ver);
}

void Lav::Load(CBitPacket *bp)
{
	bp->Data_ReadSimple(v);
	bp->Data_ReadSimple(max);
	bp->Data_ReadSimple(ver);
}




////////////////////////////////////////////////////////////////////////
//LevelAttr_Base


void LevelAttr_Base::Init(LevelRecordUnit *rec,LevelPlayerStates *lps,LevelGrade grd)
{
	if (!rec)
		return;

	grade=grd;

	if (!lps)
	{
		hp.Reset(rec->HP,rec->HP);
		sp.Reset(rec->SP,rec->SP);
		spFull.Reset(rec->SP,rec->SP);
	}
	else
	{
		hp.Reset((WORD)lps->base.MaxHP,(WORD)lps->base.MaxHP);
		sp.Reset((WORD)lps->base.FullSP,(WORD)lps->base.FullSP);
		spFull.Reset((WORD)lps->base.FullSP,(WORD)lps->base.FullSP);
	}

}

void LevelAttr_Base::Init(LevelRecordAgent *rec)
{
	if (!rec)
		return;
	hp.Reset(rec->HP,rec->HP);
	sp.Reset(rec->SP,rec->SP);
	spFull.Reset(rec->SP,rec->SP);
}



void LevelAttr_Base::WriteFirstSync(CBitPacket *bp,BOOL bPlayer)
{
	hp.Save(bp);
	if (bPlayer)
	{
		bp->Data_WriteSimple(grade);
		bp->Data_WriteSimple(hnr);
		bp->Data_WriteSimple(vita_);
		bp->Data_WriteSimple(worm);
		bp->Data_WriteSimple(str);
		bp->Data_WriteSimple(magic);
		sp.Save(bp);
		spFull.Save(bp);
	}
}

void LevelAttr_Base::ReadFirstSync(CBitPacket *bp,BOOL bPlayer)
{
	hp.Load(bp);
	if (bPlayer)
	{
		bp->Data_ReadSimple(grade);
		bp->Data_ReadSimple(hnr);
		bp->Data_ReadSimple(vita_);
		bp->Data_ReadSimple(worm);
		bp->Data_ReadSimple(str);
		bp->Data_ReadSimple(magic);
		sp.Load(bp);
		spFull.Load(bp);
	}
}



////////////////////////////////////////////////////////////////////////
//LevelAttr_Drop
void LevelAttr_Drop::Init(LevelRecordUnit *rec)
{
	rateGold=rec->rateDrop.rateGold;
	rateGem=rec->rateDrop.rateGem;
}


////////////////////////////////////////////////////////////////////////
//LevelAttr_Resource
void LevelAttr_Resource::WriteFirstSync(CBitPacket *bp)
{
	for (int i=0;i<ARRAY_SIZE(res);i++)
		res[i].Save(bp);
}

void LevelAttr_Resource::Init(LevelPlayerStates *lps)
{
	for (int i=0;i<ARRAY_SIZE(res);i++)
	{
		DWORD *v=lps->base.GetRes((LevelResourceType)(i+1));
		if (v)
			res[i].SetCur_Int((int)*v);
		res[i].SetMax_Int(0xffff);
	}
}

//////////////////////////////////////////////////////////////////////////
//LevelAttr_Temple
void LevelAttr_Temple::Init(LevelPlayerStates *lps)
{
	temples[LevelTemple_Sun-1]=lps->base.templeSun;
	temples[LevelTemple_Moon-1]=lps->base.templeMoon;
	temples[LevelTemple_Fire-1]=lps->base.templeFire;
	temples[LevelTemple_Star-1]=lps->base.templeStar;
	temples[LevelTemple_Sand-1]=lps->base.templeSand;
	temples[LevelTemple_Craft-1]=lps->base.templeCraft;
	//XXXXX:More LevelTempleType
}

void LevelAttr_Temple::WriteSync(CBitPacket *bp)
{
	bp->Data_WriteData(temples,sizeof(temples));
}


////////////////////////////////////////////////////////////////////////
//LevelAttr_MBResource
void LevelAttr_MagicBoard::WriteFirstSync(CBitPacket *bp)
{
	for (int i=1;i<MBRes_ActualMax;i++)
		res[i].Save(bp);
}

void LevelAttr_MagicBoard::Init(LevelPlayerStates *lps)
{
	res[MBRes_Mana].SetCur_Int(0);
	res[MBRes_Mana].SetMax_Int(200);

	res[MBRes_Gold].SetCur_Int(0);
	res[MBRes_Gold].SetMax_Int(0xffff);

	res[MBRes_Crystal].SetCur_Int(0);
	res[MBRes_Crystal].SetMax_Int(0xffff);
}

//////////////////////////////////////////////////////////////////////////
//LevelAttr_SpeedMod
void LevelAttr_SpeedMod::WriteFirstSync(CBitPacket *bp)
{
	bp->Data_WriteSimple(ias);
	bp->Data_WriteSimple(ims);
}


