
/********************************************************************
	created:	2012/11/19 
	author:		cxi
	
	purpose:	session 
*********************************************************************/
#include "stdh.h"

#include "BehaviorGraphData.h"

#include "datapacket/DataPacket.h"

#include "stringparser/stringparser.h"

#include <assert.h>    
 
#pragma warning(disable:4018)

 
//////////////////////////////////////////////////////////////////////////
//BehaviorGraphData

IMPLEMENT_CLASS(BehaviorGraphData);

BehaviorGraphData::BehaviorGraphData()
{
	Zero();
}
BehaviorGraphData::~BehaviorGraphData()
{
	Clean();
}

void BehaviorGraphData::Zero()
{
}

void BehaviorGraphData::Clean()
{
	pads.Clear();
	Zero();
}


ResType BehaviorGraphData::GetType()
{
	return Res_BehaviorGraph;
}
const char *BehaviorGraphData::GetTypeName()
{
	return "BehaviorGraph";
}


#define BGDATA_VER 3
void BehaviorGraphData::Save(CDataPacket &dp)
{
	dp.Data_NextWord()=BGDATA_VER;

	StringID nm=pads.GetName();
	dp.Data_WriteSimple(nm);

	pads.Save(dp);
}

void BehaviorGraphData::SaveHeader(CDataPacket &dp)
{
}

void BehaviorGraphData::Load(CDataPacket &dp)
{
	WORD ver=dp.Data_NextWord();
	if (ver>=3)
	{
		StringID nm;
		dp.Data_ReadSimple(nm);
	}
	pads.Clear();
	pads.Load(dp);
}

void BehaviorGraphData::LoadHeader(CDataPacket &dp)
{
}

void BehaviorGraphData::CalcContent(std::string &s)
{
	s="";
}
