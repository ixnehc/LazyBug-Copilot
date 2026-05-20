/********************************************************************
	created:	2006/8/3   16:15
	filename: 	e:\IxEngine\Common\resdata\MoppData.cpp
	author:		cxi
	
	purpose:	mesh resource data
*********************************************************************/
#include "stdh.h"

#include "MoppData.h"

#include "datapacket/DataPacket.h"

#include "stringparser/stringparser.h"

#include <assert.h>

#pragma warning(disable:4018)

//////////////////////////////////////////////////////////////////////////
//MoppData

IMPLEMENT_CLASS(MoppData);

MoppData::MoppData()
{
	Zero();
}
MoppData::~MoppData()
{
	Clean();
}

void MoppData::Zero()
{
	tpData = 0;
	aabb.resetInvalid();
}

void MoppData::Clean()
{
	vertices.clear();
	indices.clear();
	data.clear();

	Zero();
}


ResType MoppData::GetType()
{
	return Res_Mopp;
}
const char *MoppData::GetTypeName()
{
	return "Mopp";
}



void MoppData::CalcAABB()
{
	BOOL bFirst=TRUE;

	aabb.resetInvalid();

	for (int i=0;i<vertices.size();i++)
		aabb.addInternalPoint(vertices[i]);
}


#define MOPPDATA_VER 2
void MoppData::Save(CDataPacket &dp)
{
	dp.Data_NextDword()=MOPPDATA_VER;

	DP_WriteVar(dp,aabb);
	DP_WriteVector(dp,vertices);
	DP_WriteVector(dp,indices);
	DP_WriteVar(dp, tpData);
	DP_WriteVector(dp,data);
	dp.Data_NextDword()=buildtypeHk;
	DP_WriteVar(dp,offsetHk);

}

void MoppData::SaveHeader(CDataPacket &dp)
{
}

void MoppData::Load(CDataPacket &dp)
{
	Clean();

	DWORD ver=dp.Data_NextDword();

	DP_ReadVar(dp,aabb);
	DP_ReadVector(dp,vertices);
	DP_ReadVector(dp,indices);
	if (ver <= 1)
		tpData = 0;
	else
		DP_ReadVar(dp, tpData);
	DP_ReadVector(dp,data);
	buildtypeHk=dp.Data_NextDword();
	DP_ReadVar(dp,offsetHk);
}

void MoppData::LoadHeader(CDataPacket &dp)
{
	Clean();
}

void MoppData::CalcContent(std::string &s)
{
	s="";
	AppendFmtString(s,
		"Mopp Data Content:\r\n");

	AppendFmtString(s,	
		"    %d vertice,%d indice(%d primitives),%d bytes of data size\r\n",
		vertices.size(),indices.size(),indices.size()/3,data.size());

}
