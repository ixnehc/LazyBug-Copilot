/********************************************************************
	created:	2006/10/13   11:21
	filename: 	e:\IxEngine\Common\resdata\MtrlExtData.cpp
	author:		cxi
	
	purpose:	material data
*********************************************************************/
#include "stdh.h"

#include "MtrlExtData.h"

#include "stringparser/stringparser.h"

#include "datapacket/DataPacket.h"
#include "commondefines/general_stl.h"

#include "strlib/strlib.h"

#include "log/LogDump.h"

#include <assert.h>

static KeyType KTFromGSem(GSemCode code)
{
	switch(code)
	{
		case GSem_Shiness:
		case GSem_ShineStr:
		case GSem_Alpha:
		case GSem_Float:
			return KT_Float;
		case GSem_ColorAlpha:
			return KT_Color;
		case GSem_UVXform:
			return KT_MapCoord;
	}
	return KT_None;
}

//根据一个字符串,解析effect param的信息
BOOL ParseMteEPInfo(MtrlExtData::EPInfo &info,StringID idStr)
{
	if (!StrLib_Get()->IsValid(idStr))
		return FALSE;
	std::string s=StrLib_GetStr(idStr);
	std::vector<std::string>buf;
	SplitStringBy("|",s,&buf);
	if (buf.size()<3)
		return FALSE;
	info.name=buf[0];
	info.nameShow=buf[1];
	info.sem.code=SemCodeFromName(buf[2].c_str());
	if (buf.size()>=4)
		info.sem.constraint=buf[3];

	if (info.sem.code==GSem_Unknown)
		return FALSE;

	info.gvt=GetSemVarType(info.sem);
	info.kt=KTFromGSem(info.sem.code);

	info.nmEP=idStr;
	return TRUE;
}

//注意:这个函数很慢
StringID FindEPEStringID(const char *name)
{
	StringID idGrp=StrLib_Get()->FindGroup("EffectParam定义");
	if (idGrp==StringID_Invalid)
		return StringID_Invalid;

	DWORD c;
	StringID *ids=StrLib_Get()->EnumGroupSubs(idGrp,c);
	for (int i=0;i<c;i++)
	{
		const char *s=StrLib_GetStr(ids[i]);
		char *p=(char*)name;
		char *q=(char *)s;
		while(*p)
		{
			if ((*p)!=(*q))
				break;
			p++;
			q++;
		}
		if ((*p)==0)
			return ids[i];
	}
	return StringID_Invalid;
}


//////////////////////////////////////////////////////////////////////////
//MtrlExtData::EPInfo
void MtrlExtData::EPInfo::Save(CDataPacket &dp)
{
	DP_WriteVar(dp,nmEP);
	dp.Data_WriteString(name);
	dp.Data_WriteString(nameShow);
	DP_WriteVar(dp,gvt);
	DP_WriteVar(dp,sem.code);
	dp.Data_WriteString(sem.constraint);
	DP_WriteVar(dp,kt);
	dp.Data_WriteString(nameDesc);
}

void MtrlExtData::EPInfo::Load(CDataPacket &dp)
{
	DP_ReadVar(dp,nmEP);
	dp.Data_ReadString(name);
	dp.Data_ReadString(nameShow);
	DP_ReadVar(dp,gvt);
	DP_ReadVar(dp,sem.code);
	dp.Data_ReadString(sem.constraint);
	DP_ReadVar(dp,kt);
	dp.Data_ReadString(nameDesc);
}



//////////////////////////////////////////////////////////////////////////
//MtrlExtData
IMPLEMENT_CLASS(MtrlExtData);


MtrlExtData::MtrlExtData()
{
	Zero();
}
MtrlExtData::~MtrlExtData()
{
	Clean();
}

void MtrlExtData::Zero()
{
	bCompiled=TRUE;

	sample.Zero();
}

void MtrlExtData::Clean()
{
	src="";
	feature.Clean();

	sample.Clear();

	Zero();
}

ResType MtrlExtData::GetType()
{
	return Res_MtrlExt;
}
const char *MtrlExtData::GetTypeName()
{
	return "MtrlExt";
}

void MtrlExtData::CalcContent(std::string &s)
{
	s="n/a,yet";
}

#define MTRLEXTDATA_VER_4 4

void MtrlExtData::Save(CDataPacket &dp)
{
	dp.Data_NextDword()=MTRLEXTDATA_VER_4;

	dp.Data_WriteString(src.c_str());

	dp.Data_NextInt()=bCompiled;
	if (bCompiled)
		feature.Save(dp);

	dp.Data_NextDword()=epinfo.size();
	for (int i=0;i<epinfo.size();i++)
		epinfo[i].Save(dp);

	sample.Save(dp);
}

void MtrlExtData::Load(CDataPacket &dp)
{
	Clean();
	DWORD ver=dp.Data_NextDword();

	if (ver<3)
	{
		StringID name;
		DP_ReadVar(dp,name);
	}

	dp.Data_ReadString(src);

	bCompiled=dp.Data_NextInt();
	if (bCompiled)
		feature.Load(dp);

	epinfo.resize(dp.Data_NextDword());
	for (int i=0;i<epinfo.size();i++)
		epinfo[i].Load(dp);

	if (ver<MTRLEXTDATA_VER_4)
	{
		if (ver>=2)
		{
			MtrlDemand demand;
			DP_ReadVar(dp,demand);
		}
	}

	sample.Load(dp);
}

void MtrlExtData::RefreshEPs()
{
	epinfo.clear();

	if (!bCompiled)
		return;

	std::vector<EPInfo>infos;

	if (TRUE)
	{
		StringID idGrp=StrLib_Get()->FindGroup("EffectParam定义");
		if (idGrp!=StringID_Invalid)
		{
			DWORD c;
			StringID *candi=StrLib_Get()->EnumGroupSubs(idGrp,c);
			infos.resize(c);
			int n=0;
			for (int i=0;i<c;i++)
			{
				if (ParseMteEPInfo(infos[n],candi[i]))
					n++;
				else
				{
					LOG_DUMP_1P("MtrlExtEdit",Log_Error,"发现无效的[EffectParam定义]字符串:\"%s\"",candi[i]);
				}
			}
			infos.resize(n);
		}
	}

	//feature里的effectparam
	for (int i=0;i<feature.vars.size();i++)
	{
		ShaderVar *var=&feature.vars[i];
		if (var->category!=SVC_EffectParam)
			continue;

		int idx;
		VEC_FIND_BY_ELEMENT(infos,name,var->name,idx);
		if (idx==-1)
		{
			LOG_DUMP_1P("MtrlExtEdit",Log_Warning,"发现未定义的EffectParam:\"%s\"",var->name.c_str());
			continue;
		}

		EPInfo *info=&infos[idx];

		//检查类型是否正确:
		GVarType gvt=GVT_None;
		if (var->type=="float")
			gvt=GVT_F;
		if (var->type=="float2")
			gvt=GVT_Fx2;
		if (var->type=="float3")
			gvt=GVT_Fx3;
		if (var->type=="float4")
			gvt=GVT_Fx4;
		if (var->type=="texture")
			gvt=GVT_String;
		if (var->type=="int")
			gvt=GVT_S;
		if (var->type=="int4")
			gvt=GVT_Sx4;

		if ((gvt==GVT_None)||(gvt!=info->gvt))
		{
			LOG_DUMP_1P("MtrlExtEdit",Log_Warning,"发现无效的EffectParam参数类型:\"%s\"",var->type.c_str());
			continue;
		}

		epinfo.push_back(*info);
	}

}

MtrlExtData::EPInfo *MtrlExtData::FindEPInfo(StringID nm)
{
	int idx;
	VEC_FIND_BY_ELEMENT(epinfo,nmEP,nm,idx);
	if (idx==-1)
		return NULL;
	return &epinfo[idx];
}
