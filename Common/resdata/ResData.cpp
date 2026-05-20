/********************************************************************
	created:	2006/8/3   16:16
	filename: 	e:\IxEngine\Common\resdata\ResData.cpp
	author:		cxi
	
	purpose:	base class for all resource data
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "resdata.h"

#include "datapacket/DataPacket.h"

#include "meshdata.h"
#include "AnimData.h"
#include "MtrlData.h"
#include "DummiesData.h"
#include "SptData.h"
#include "TexData.h"
#include "MoppData.h"
#include "SpgData.h"
#include "AnimTreeData.h"
#include "MtrlExtData.h"
#include "SoundData.h"
#include "RecordsData.h"
#include "RagdollData.h"
#include "DtrData.h"
#include "BehaviorGraphData.h"

//XXXXX:more res type


ResData *ResData_New(ResType tp)
{
	switch (tp)
	{
	case Res_Mesh:
		return Class_New(MeshData);
	case ResA_XForm:
		return Class_New(XFormData);
	case Res_Mtrl:
		return Class_New(MtrlData);
	case ResA_MtrlColor:
		return Class_New(MtrlColorData);
	case ResA_MapCoord:
		return Class_New(MapCoordData);
	case Res_Dummies:
		return Class_New(DummiesData);
	case Res_Spt:
		return Class_New(SptData);
	case Res_Texture:
		return Class_New(TexData);
	case Res_Mopp:
		return Class_New(MoppData);
	case Res_Spg:
		return Class_New(SpgData);
	case Res_AnimTree:
		return Class_New(AnimTreeData);
	case ResA_Bones2:
		return Class_New(BonesData2);
	case Res_MtrlExt:
		return Class_New(MtrlExtData);
	case Res_Sound:
		return Class_New(SoundData);
	case Res_Records:
		return Class_New(RecordsData);
	case Res_Ragdoll:
		return Class_New(RagdollData);
	case Res_Dtr:
		return Class_New(DtrData);
	case Res_BehaviorGraph:
		return Class_New(BehaviorGraphData);
	//XXXXX:more res type
	}
	assert(FALSE);
	return NULL;
}


void ResData_Delete(ResData *p)
{
	Safe_Class_Delete(p);
}

ResData *ResData_Clone(ResData *p)
{
	if (!p)
		return NULL;
	return p->Clone();
}




//A string to describe the content of this res data
const char *ResData::GetContent()
{
	if (content=="")
		CalcContent(content);
	return content.c_str();
}

BOOL ResData::Copy(ResData &src)
{
	if (GetType()!=src.GetType())
		return FALSE;
	std::vector<BYTE>temp;
	CDataPacket dp;
	src.Save(dp);
	temp.resize(dp.GetDataSize());
	dp.SetDataBufferPointer(temp.data());
	src.Save(dp);
	dp.Reset();
	Load(dp);

	content="";
	return TRUE;
}

ResData *ResData::Clone()
{
	ResData *ret=(ResData *)GetClass()->New();
	ret->Copy(*this);
	return ret;
}

BOOL ResData::Equal(ResData &other)
{
	std::vector<BYTE>buf,buf2;
	SaveData(buf);
	other.SaveData(buf2);
	BOOL bSame;
	VEC_COMPARE(buf,buf2,bSame);
	return bSame;
}


void ResData::SaveData(std::vector<BYTE>&data)
{
	DP_BeginSave(dp,data);
		Save(dp);
	DP_EndSave();
}

void ResData::LoadData(std::vector<BYTE>&data)
{
	CDataPacket dp;
	dp.SetDataBufferPointer(data.data());
	Load(dp);
}

void ResData::SaveHeaderData(std::vector<BYTE>&data)
{
	DP_BeginSave(dp,data);
		SaveHeader(dp);
	DP_EndSave();
}
void ResData::LoadHeaderData(std::vector<BYTE>&data)
{
	CDataPacket dp;
	dp.SetDataBufferPointer(data.data());
	LoadHeader(dp);
}

//KeyType和ResType的映射关系
KeyType KeyTypeFromResType(ResType t)
{
	switch(t)
	{
	case ResA_XForm:
		return KT_XForm;
	case ResA_MtrlColor:
		return KT_Color;
	case ResA_MapCoord:
		return KT_MapCoord;
		//XXXXX:More AnimType

	default:
		return KT_None;
	}
}

ResType ResTypeFromKeyType(KeyType kt)
{
	switch(kt)
	{
		case KT_XForm:
			return ResA_XForm;
		case KT_Color:
			return ResA_MtrlColor;
		case KT_MapCoord:
			return ResA_MapCoord;
	}
	return Res_None;
}

