#pragma once

#include <string>
#include <vector>


#include "ResData.h"


class CDataPacket;
struct RagdollData:public ResData
{
	DECLARE_CLASS(RagdollData);

	RagdollData();
	virtual ~RagdollData();
	void Zero();
	void Clean();


	//Overriding
	virtual 	ResType GetType();
	virtual const char *GetTypeName();
	virtual const char *GetTypeSuffix()	{		return "rgd";	}
	virtual void CalcContent(std::string &s);
	virtual void Save(CDataPacket &dp){}
	virtual void Load(CDataPacket &dp){}
	virtual void SaveHeader(CDataPacket &dp){}
	virtual void LoadHeader(CDataPacket &dp){}



};

