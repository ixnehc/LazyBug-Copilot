/********************************************************************
	created:	2008/07/11   11:21
	filename: 	e:\IxEngine\Common\resdata\SoundData.cpp
	author:		cxi
	
	purpose:	Sound data
*********************************************************************/

#include "stdh.h"

#include "SoundData.h"

#include "stringparser/stringparser.h"
 
   
#include "datapacket/DataPacket.h"

#include <assert.h>


//////////////////////////////////////////////////////////////////////////
//SoundData
IMPLEMENT_CLASS(SoundData);


SoundData::SoundData()
{
	Zero();
}
SoundData::~SoundData()
{
	Clean();
}

void SoundData::Zero()
{
}

void SoundData::Clean()
{
}

ResType SoundData::GetType()
{
	return Res_Sound;
}
const char *SoundData::GetTypeName()
{
	return "Sound";
}

void SoundData::CalcContent(std::string &s)
{
	s="n/a";
}

