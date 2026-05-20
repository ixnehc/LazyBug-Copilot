/********************************************************************
	created:	2006/8/3   16:15
	filename: 	e:\IxEngine\Common\resdata\DtrData.cpp
	author:		cxi
	
	purpose:	mesh resource data
*********************************************************************/
#include "stdh.h"


#include "datapacket/DataPacket.h"

#include "stringparser/stringparser.h"

#include "DtrData.h"

#include <map>
#include <set>

#include <unordered_set>

#include <assert.h>

#pragma warning(disable:4018)


#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#endif    

#define DTRDATA_VER 2


////////////////////////////////////////////////////////////////////////
//DtrPieces

void DtrPiecesData::Save(CDataPacket &dp)
{
	DP_WriteVar(dp,iParent);
	DP_WriteVar(dp,iParentSub);

	DP_WriteVar(dp,aabb);

	DP_WriteVector(dp,vertices);
	DP_WriteVector(dp,indices);
	DP_WriteVar(dp,nParts);
	DP_WriteVector(dp,primsParts);

	DP_WriteVector(dp,verticesShp);
	DP_WriteVector(dp,indicesShp);
	DP_WriteVar(dp, tpData);
	DP_WriteVector(dp,data);
	DP_WriteVector(dp,pieces);

}

void DtrPiecesData::Load(CDataPacket &dp,DWORD ver)
{
	DP_ReadVar(dp,iParent);
	DP_ReadVar(dp,iParentSub);

	DP_ReadVar(dp,aabb);

	DP_ReadVector(dp,vertices);
	DP_ReadVector(dp,indices);
	DP_ReadVar(dp,nParts);
	DP_ReadVector(dp,primsParts);

	DP_ReadVector(dp,verticesShp);
	DP_ReadVector(dp,indicesShp);
	if (ver <= 1)
		tpData = 0;
	else
	{
		DP_ReadVar(dp, tpData);
	}

	DP_ReadVector(dp,data);

	if (ver <= 1)
	{
		std::vector<DtrPieceInfo_old> piecesOld;
		DP_ReadVector(dp, piecesOld);

		pieces.resize(piecesOld.size());
		for (int i = 0;i < piecesOld.size();i++)
			pieces[i].FromOld(piecesOld[i]);
	}
	else
	{
		DP_ReadVector(dp, pieces);
	}
}



//////////////////////////////////////////////////////////////////////////
//DtrData

IMPLEMENT_CLASS(DtrData);

DtrData::DtrData()
{
	Zero();
}
DtrData::~DtrData()
{
	Clean();
}

void DtrData::Zero()
{


}

void DtrData::Clean()
{
	for (int i=0;i<piecesAll.size();i++)
		piecesAll[i].Clear();
	piecesAll.clear();
}


ResType DtrData::GetType()
{
	return Res_Dtr;
}

const char *DtrData::GetTypeName()
{
	return "Destructable";
}



void DtrData::Save(CDataPacket &dp)
{
	dp.Data_NextDword()=DTRDATA_VER;

	dp.Data_NextDword()=piecesAll.size();

	for (int i=0;i<piecesAll.size();i++)
		piecesAll[i].Save(dp);
}

void DtrData::LoadHeader(CDataPacket &dp)
{
}

void DtrData::SaveHeader(CDataPacket &dp)
{
}

void DtrData::Load(CDataPacket &dp)
{
	Clean();

	DWORD ver=dp.Data_NextDword();

	DWORD sz=dp.Data_NextDword();

	piecesAll.resize(sz);
	for (int i=0;i<piecesAll.size();i++)
		piecesAll[i].Load(dp,ver);
}


