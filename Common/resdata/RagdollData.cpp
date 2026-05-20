/********************************************************************
	created:	2008/07/11   11:21
	filename: 	e:\IxEngine\Common\resdata\RagdollData.cpp
	author:		cxi
	
	purpose:	Sound data
*********************************************************************/

#include "stdh.h"

#include "RagdollData.h"

#include "stringparser/stringparser.h"
 
   
#include "datapacket/DataPacket.h"

#include <assert.h>


//////////////////////////////////////////////////////////////////////////
//RagdollData
IMPLEMENT_CLASS(RagdollData);


RagdollData::RagdollData()
{
	Zero();
}
RagdollData::~RagdollData()
{
	Clean();
}

void RagdollData::Zero()
{
}

void RagdollData::Clean()
{
}

ResType RagdollData::GetType()
{
	return Res_Sound;
}
const char *RagdollData::GetTypeName()
{
	return "Ragdoll";
}

void RagdollData::CalcContent(std::string &s)
{
	s="n/a";
}

