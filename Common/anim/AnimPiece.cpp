/********************************************************************
	created:	2006/8/15   13:06
	filename: 	e:\IxEngine\Common\anim\AnimPiece.cpp
	author:		cxi
	
	purpose:	AnimPiece,animation info indexing to KeySet
*********************************************************************/
#include "stdh.h"

#include "AnimPiece.h" 

#include "datapacket/DataPacket.h"

//////////////////////////////////////////////////////////////////////////
//AnimPiece

AnimPiece::AnimPiece()
{
	name=StringID_Invalid;
	tStart=0;
	memset(params,0,sizeof(params));
	tEnd=ANIMTICK_INFINITE;
}

AnimPiece &AnimPiece::operator=(const AnimPiece &src)
{
	name=src.name;
	tStart=src.tStart;
	tEnd=src.tEnd;
	events=src.events;
	zones=src.zones;
	return *this;
}

void AnimPiece::SaveHeader(CDataPacket &dp)
{
	DP_WriteVar(dp,name);
	dp.Data_NextDword()=tStart;
	dp.Data_NextDword()=tEnd;
}

void AnimPiece::LoadHeader(CDataPacket &dp)
{
	DP_ReadVar(dp,name);
	tStart=dp.Data_NextDword();
	tEnd=dp.Data_NextDword();
}

void AnimPiece::SaveZones(CDataPacket &dp)
{
	dp.Data_NextWord()=(WORD)zones.size();
	for (int i=0;i<zones.size();i++)
	{
		AnimEventZone &zone=zones[i];
		dp.Data_WriteSimple(zone.name);
		dp.Data_WriteSimple(zone.t);
		dp.Data_NextByte()=zone.tp;
		switch(zone.tp)
		{
			case AnimEventZone::Fan:
			{
				DP_WriteVector(dp,zone.keysFan);
				break;
			}
		}
	}
}

void AnimPiece::LoadZones(CDataPacket &dp)
{
	zones.resize(dp.Data_NextWord());
	for (int i=0;i<zones.size();i++)
	{
		AnimEventZone &zone=zones[i];
		dp.Data_ReadSimple(zone.name);
		dp.Data_ReadSimple(zone.t);
		zone.tp=(AnimEventZone::Type)dp.Data_NextByte();
		switch(zone.tp)
		{
			case AnimEventZone::Fan:
			{
				DP_ReadVector(dp,zone.keysFan);
				break;
			}
		}
	}
}


void AnimPiece::Save(CDataPacket &dp)
{
	SaveHeader(dp);
	DP_WriteVector(dp,events);

	SaveZones(dp);

	dp.Data_NextFloat() = params[0];
	dp.Data_NextFloat() = params[1];

	dp.Data_NextDword() = iStart;
	dp.Data_NextDword() = iEnd;
}

void AnimPiece::Load(CDataPacket &dp,DWORD ver)
{
	LoadHeader(dp);
	DP_ReadVector(dp,events);
	if (ver>1)
		LoadZones(dp);
	
	params[0] = dp.Data_NextFloat();
	params[1] = dp.Data_NextFloat();

	iStart = dp.Data_NextDword();
	iEnd = dp.Data_NextDword();
}


AnimEventZone *AnimPiece::FindZone(AnimEvent &e)
{
	for (int i=0;i<zones.size();i++)
	{
		if ((zones[i].name==e.name)&&(zones[i].t==e.tEvent))
			return &zones[i];
	}
	return NULL;
}
