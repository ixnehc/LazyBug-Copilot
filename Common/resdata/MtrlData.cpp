/********************************************************************
	created:	2006/10/13   11:21
	filename: 	e:\IxEngine\Common\resdata\MtrlData.cpp
	author:		cxi
	
	purpose:	material data
*********************************************************************/
#include "stdh.h"

#include "MtrlData.h"

#include "stringparser/stringparser.h"


#include "datapacket/DataPacket.h"

#include <assert.h>


//////////////////////////////////////////////////////////////////////////
//MtrlData::Layor



//////////////////////////////////////////////////////////////////////////
//MtrlData
IMPLEMENT_CLASS(MtrlData);


#define MTRLDATA_VER 12

MtrlData::MtrlData()
{
	Zero();
}
MtrlData::~MtrlData()
{
	Clear();
}

void MtrlData::Zero()
{
	lods.clear();
}

void MtrlData::Clear()
{
	for (int i=0;i<lods.size();i++)
		lods[i].Clear();

	Zero();
}

ResType MtrlData::GetType()
{
	return Res_Mtrl;
}
const char *MtrlData::GetTypeName()
{
	return "Material";
}


void MtrlData::CalcContent(std::string &s)
{
	s="n/a,yet";
}

static void Mtrl_SaveLod(CDataPacket &dp,MtrlData::Lod*lod)
{
	dp.Data_WriteString(lod->slib);
	dp.Data_WriteString(lod->features);
	dp.Data_WriteString(lod->unifeature);
	dp.Data_WriteString(lod->mte);

	dp.Data_NextDword()=lod->fps.size();
	for (int i=0;i<lod->fps.size();i++)
	{
		FeatureParamA &fp=lod->fps[i];
		dp.Data_WriteData(fp.ep_name,sizeof(fp.ep_name));
		fp.var.Save(dp);

		//动画信息
		dp.Data_NextByte()=fp.at;
		dp.Data_NextByte()=fp.bMte;
		fp.vs.Save(dp);
		dp.Data_WriteString(fp.pathAnim.c_str());
	}

	DP_WriteVar(dp,lod->demand);

	DP_WriteVar(dp,lod->state);
}

static void Mtrl_LoadLod(CDataPacket &dp,MtrlData::Lod *lod,DWORD ver)
{
	dp.Data_ReadString(lod->slib);
	dp.Data_ReadString(lod->features);
	dp.Data_ReadString(lod->unifeature);
	if (ver>=10)
		dp.Data_ReadString(lod->mte);

	lod->fps.resize(dp.Data_NextDword());
	for (int i=0;i<lod->fps.size();i++)
	{
		FeatureParamA &fp=lod->fps[i];
		dp.Data_ReadData(fp.ep_name,sizeof(fp.ep_name));
		if (ver==5)
			dp.Data_MarchData(8);//旧版本ep_name是32字节,现在是24字节,所以要多前进8个字节
		fp.var.Load(dp);

		if (ver>5)
		{
			//动画信息
			fp.at=dp.Data_NextByte();
			if (ver>=11)
				fp.bMte=dp.Data_NextByte();
			if (ver==6)
				fp.vs.KeySet::LoadOld(dp);
			if (ver==7)
				fp.vs.KeySet::Load_(dp);
			if (ver==8)
				fp.vs.GLoad(dp);
			if (ver>=9)
				fp.vs.Load(dp);
			dp.Data_ReadString(fp.pathAnim);
		}
	}

	if (ver>=MTRLDATA_VER)
		DP_ReadVar(dp,lod->demand);
	else
		dp.Data_NextDword();

	DP_ReadVar(dp,lod->state);
}

void MtrlData::Save(CDataPacket &dp)
{
	dp.Data_NextDword()=MTRLDATA_VER;
	dp.Data_NextDword()=lods.size();
	for (int k=0;k<lods.size();k++)
		Mtrl_SaveLod(dp,&lods[k]);
}

void MtrlData::Load(CDataPacket &dp)
{
	Clear();
	DWORD ver=dp.Data_NextDword();
	if (ver<6)
	{
		DWORD t=dp.Data_NextDword();
	}
	lods.resize(dp.Data_NextDword());
	for (int k=0;k<lods.size();k++)
		Mtrl_LoadLod(dp,&lods[k],ver);
}

void MtrlData::SaveHeader(CDataPacket &dp)
{
	dp.Data_NextDword()=MTRLDATA_VER;
}

void MtrlData::LoadHeader(CDataPacket &dp)
{
	DWORD ver=dp.Data_NextDword();
}

void MtrlData::CollectRefs(std::vector<std::string>&buf)
{
	for (int i=0;i<lods.size();i++)
	{
		Lod *lod=&lods[i];
		if (!lod->mte.empty())
			buf.push_back(lod->mte);   

		for (int j=0;j<lod->fps.size();j++)
		{
			FeatureParamA *fp=&lod->fps[j];
			if (fp->at==FeatureParamA::Anim_Res)
			{
				if (!fp->pathAnim.empty())
					buf.push_back(fp->pathAnim);
			}

			if (fp->var.Type()==GVT_String)   
			{//我们认为它是一个贴图路径   
				if (!fp->var.Str().empty())
					buf.push_back(fp->var.Str());
			}
		}
	}
}

