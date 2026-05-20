#pragma once

#include <string>
#include <vector>


#include "ResData.h"


class CDataPacket;
struct SoundData:public ResData
{
	DECLARE_CLASS(SoundData);

	SoundData();
	virtual ~SoundData();
	void Zero();
	void Clean();


	//Overriding
	virtual 	ResType GetType();
	virtual const char *GetTypeName();
	virtual const char *GetTypeSuffix()	{		return "";	}
	virtual void CalcContent(std::string &s);
	virtual void Save(CDataPacket &dp){}
	virtual void Load(CDataPacket &dp){}
	virtual void SaveHeader(CDataPacket &dp){}
	virtual void LoadHeader(CDataPacket &dp){}



};

