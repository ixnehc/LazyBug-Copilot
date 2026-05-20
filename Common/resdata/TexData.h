#pragma once

#include <string>
#include <vector>


#include "ResData.h"


class CDataPacket;
struct TexData:public ResData
{
	DECLARE_CLASS(TexData);

	enum TexDataType
	{
		Tex_UNKNOWN=0, 
		Tex_TGA,
		Tex_BMP,
		Tex_DDS,
		Tex_JPG,
	};


	TexData();
	virtual ~TexData();
	void Zero();
	void Clean();


	//Overriding
	virtual 	ResType GetType();
	virtual const char *GetTypeName();
	virtual const char *GetTypeSuffix()	{		return "";	}
	virtual void CalcContent(std::string &s);
	virtual void Save(CDataPacket &dp);
	virtual void Load(CDataPacket &dp);
	virtual void SaveHeader(CDataPacket &dp);
	virtual void LoadHeader(CDataPacket &dp);

	TexDataType GetDataType()	{		return textype;	}
	BYTE *GetDataPtr()	{		return data.data();	}
	DWORD GetDataSize()	{		return data.size();	}

	BOOL IsEmpty()	{		return textype==Tex_UNKNOWN;	}

	TexDataType textype;
	std::vector<BYTE>data;


};

