/********************************************************************
	created:	2008/07/11   11:21
	filename: 	e:\IxEngine\Common\resdata\TexData.cpp
	author:		cxi
	
	purpose:	texture data
*********************************************************************/

#include "stdh.h"

#include "TexData.h"

#include "stringparser/stringparser.h"


#include "datapacket/DataPacket.h"

#include <assert.h>


//////////////////////////////////////////////////////////////////////////
//TexData::Layor



//////////////////////////////////////////////////////////////////////////
//TexData
IMPLEMENT_CLASS(TexData);


//ver 1 --> ver 2: Layor::ShaderBlend::alpharef, Layor::reserve
#define MTRLDATA_VER 5

TexData::TexData()
{
	Zero();
}
TexData::~TexData()
{
	Clean();
}

void TexData::Zero()
{
	textype=Tex_UNKNOWN;
}

void TexData::Clean()
{
	data.clear();

	Zero();
}

ResType TexData::GetType()
{
	return Res_Texture;
}
const char *TexData::GetTypeName()
{
	return "Texture";
}

void TexData::CalcContent(std::string &s)
{
	s="n/a";
}


void TexData::Save(CDataPacket &dp)
{
	dp.Data_NextInt()=textype;
	DP_WriteVector(dp,data);
}

void TexData::Load(CDataPacket &dp)
{
	textype=(TexDataType)dp.Data_NextInt();
	DP_ReadVector(dp,data);
}

void TexData::SaveHeader(CDataPacket &dp)
{
}

void TexData::LoadHeader(CDataPacket &dp)
{
}


