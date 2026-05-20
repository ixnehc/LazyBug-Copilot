#pragma once

#include <string>
#include <vector>

#include "ResData.h"

#include "../strlib/strlibdefines.h"
#include "../shaderlib/SLHolder.h"
#include "../anim/animbase.h"

#include "MtrlData.h"


class CDataPacket;
struct MtrlExtData:public ResData
{
	DECLARE_CLASS(MtrlExtData);

	struct EPInfo
	{
		EPInfo()
		{
			nmEP=StringID_Invalid;
		}
		StringID nmEP;
		std::string name;
		std::string nameShow;
		GVarType gvt;
		GSem sem;
		KeyType kt;
		std::string nameDesc;

		void Save(CDataPacket &dp);
		void Load(CDataPacket &dp);

		EPInfo &operator =(const EPInfo &src)
		{
			nmEP=src.nmEP;
			name=src.name;
			nameShow=src.nameShow;
			gvt=src.gvt;
			sem.code=src.sem.code;
			sem.constraint=src.sem.constraint;
			kt=src.kt;
			nameDesc=src.nameDesc;
			return *this;
		}

	};


	MtrlExtData();
	virtual ~MtrlExtData();
	void Zero();
	void Clean();


	//Overriding
	virtual 	ResType GetType();
	virtual const char *GetTypeName();
	virtual const char *GetTypeSuffix()	{		return "mte";	}
	virtual void CalcContent(std::string &s);
	virtual void Save(CDataPacket &dp);
	virtual void Load(CDataPacket &dp);
	virtual void SaveHeader(CDataPacket &dp)	{	}
	virtual void LoadHeader(CDataPacket &dp)	{	}

	EPInfo *FindEPInfo(StringID nm);
	void RefreshEPs();

	std::vector<EPInfo>epinfo;
	std::string src;//源代码字符串

	BOOL bCompiled;//表示feature里是否包含编译好的内容
	SLFeature feature;

	MtrlData sample;//用于测试的材质
};


extern BOOL ParseMteEPInfo(MtrlExtData::EPInfo &info,StringID idStr);
