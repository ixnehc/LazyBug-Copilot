/********************************************************************
	created:	2006/8/3   16:15
	filename: 	e:\IxEngine\Common\resdata\AnimTreeData.cpp
	author:		cxi
	
	purpose:	mesh resource data
*********************************************************************/
#include "stdh.h"

#include "AnimTreeData.h"

#include "datapacket/DataPacket.h"

#include "stringparser/stringparser.h"

#include "AnimTreePads.h"

#include <assert.h>

#pragma warning(disable:4018)


//////////////////////////////////////////////////////////////////////////
//AnimTreeData

IMPLEMENT_CLASS(AnimTreeData);

AnimTreeData::AnimTreeData()
{
	Zero();
}
AnimTreeData::~AnimTreeData()
{
	Clean();
}

void AnimTreeData::Zero()
{
}

void AnimTreeData::Clean()
{
	pads.Clear();
	preview.GClear();
	Zero();
}


ResType AnimTreeData::GetType()
{
	return Res_AnimTree;
}
const char *AnimTreeData::GetTypeName()
{
	return "AnimTree";
}


#define ANIMTREEDATA_VER 2
void AnimTreeData::Save(CDataPacket &dp)
{
	dp.Data_NextWord()=ANIMTREEDATA_VER;

	pads.Save(dp);

	params.GSave(dp);
	preview.GSave(dp);
}

void AnimTreeData::SaveHeader(CDataPacket &dp)
{
}

void AnimTreeData::Load(CDataPacket &dp)
{
	WORD ver=dp.Data_NextWord();
	pads.Clear();
	if (ver==1)
	{
		BOOL bLongPadID;
		pads.CLinkPads::Load(dp,bLongPadID);
	}
	else
		pads.Load(dp);

	params.GLoad(dp);
	preview.GLoad(dp);
}

void AnimTreeData::LoadHeader(CDataPacket &dp)
{
}

void AnimTreeData::CalcContent(std::string &s)
{
	s="";
}

void AnimTreeData::CollectRefs(std::vector<std::string>&buf)
{
	for (int i=0;i<preview.models.size();i++)
	{
		AT_Model *model=&preview.models[i];
		if (!model->mesh.empty())
			buf.push_back(model->mesh);
		if (!model->mtrl.empty())
			buf.push_back(model->mtrl);
	}

	if (!preview.anim.empty())
		buf.push_back(preview.anim);

	DWORD c=pads.GetPadCount();
	for (int i=0;i<c;i++)
	{
		CAnimTreePad *pad=(CAnimTreePad *)pads.GetPad(i);
		pad->CollectRefs(buf);
	}
}
