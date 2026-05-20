
/************************************************************************/
/*
e:\IxEngine\Common\resdata\DummiesData.cpp
author: star
purpose: a type of ResData for object mounting.
date: 2008-1-4
*/
/************************************************************************/
#include <stdh.h>
#include "DummiesData.h"
#include "../stringparser/stringparser.h"
#include "../datapacket/DataPacket.h"

IMPLEMENT_CLASS(DummiesData);

ResType DummiesData::GetType()
{
	return Res_Dummies;
}
const char *DummiesData::GetTypeName()
{
	 return "Dummies"; 
}
void DummiesData::Save(CDataPacket &dp)
{
	dp.Data_NextDword() = GetVersion();
	DP_WriteVector(dp,skeletonInfo);
	DP_WriteVector(dp,dummies);
}
void DummiesData::Load(CDataPacket &dp)
{
	DWORD ver=dp.Data_NextDword();
	if(ver==GetVersion())
	{
		DP_ReadVector(dp,skeletonInfo);
		DP_ReadVector(dp,dummies);
	}
}
void DummiesData::SaveHeader(CDataPacket &dp)
{
	dp.Data_NextDword() = GetVersion();
}
void DummiesData::LoadHeader(CDataPacket &dp)
{
	DWORD ver = dp.Data_NextDword();
}
void DummiesData::CalcContent(std::string &s)
{
	s="";
	AppendFmtString(s,"Dummies Data Content:\r\n");
	AppendFmtString(s," Skeleton bones:%d \r\n "
						" Dummies: %d\r\n",
					skeletonInfo.size(),dummies.size());	
}

