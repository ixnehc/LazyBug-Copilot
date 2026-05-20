
#include "stdh.h"

#include "LevelAgentBrief.h"

#include "datapacket/DataPacket.h"

//////////////////////////////////////////////////////////////////////////
//Brief
void LevelAgentBrief::Brief::Save(CDataPacket &dp)
{
	dp.Data_WriteSimple(tp);
	if (tp!=None)
	{
		dp.Data_WriteSimpleR(rcOnIconTex);
		dp.Data_WriteSimpleR(ptIconAnchor);
		dp.Data_WriteSimple(tip);
	}
}

void LevelAgentBrief::Brief::Load(CDataPacket &dp)
{
	dp.Data_ReadSimple(tp);
	if (tp!=None)
	{
		dp.Data_ReadSimple(rcOnIconTex);
		dp.Data_ReadSimple(ptIconAnchor);
		dp.Data_ReadSimple(tip);
	}
}

BOOL LevelAgentBrief::Brief::Equals(Brief &other)
{
	if (tp!=other.tp)
		return FALSE;
	if (tp==None)
		return TRUE;

	if (rcOnIconTex!=other.rcOnIconTex)
		return FALSE;
	if (ptIconAnchor!=other.ptIconAnchor)
		return FALSE;
	if (tip!=other.tip)
		return FALSE;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//TreasureInfos

void LevelAgentBrief::TreasureInfos::Save(CDataPacket &dp)
{
	dp.Data_EncodeDword(nEntries);
	for (int i=0;i<nEntries;i++)
		dp.Data_WriteSimpleR(entries[i]);
}

void LevelAgentBrief::TreasureInfos::Load(CDataPacket &dp)
{
	nEntries=dp.Data_DecodeDword();
	for (int i=0;i<nEntries;i++)
		dp.Data_ReadSimple(entries[i]);
}



//////////////////////////////////////////////////////////////////////////
//LevelAgentBrief

void LevelAgentBrief::Save(CDataPacket &dp)
{
	dp.Data_NextByte()=1;//ver
	dp.Data_WriteSimpleR(pos);
	dp.Data_WriteSimpleR(idAgent);

	initial.Save(dp);
	cur.Save(dp);
	infosTreasure.Save(dp);
}

void LevelAgentBrief::Load(CDataPacket &dp)
{
	BYTE ver=dp.Data_NextByte();
	dp.Data_ReadSimple(pos);
	dp.Data_ReadSimple(idAgent);

	initial.Load(dp);
	cur.Load(dp);
	infosTreasure.Load(dp);
}

void LevelAgentBrief::ClearCur()
{
	cur.tp=None;
}
