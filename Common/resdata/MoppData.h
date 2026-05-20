#pragma once

#include <string>
#include <vector>
#include <stack>
#include "../math/vector3d.h"

#include "ResData.h"



struct MoppData:public ResData
{
	DECLARE_CLASS(MoppData);


	MoppData();
	virtual ~MoppData();
	virtual void Zero();
	virtual void Clean();

	//Overriding
	virtual 	ResType GetType();
	virtual const char *GetTypeName();
	virtual const char *GetTypeSuffix()	{		return "mpp";	}
	virtual void CalcContent(std::string &s);
	virtual void Save(CDataPacket &dp);
	virtual void Load(CDataPacket &dp);
	virtual void SaveHeader(CDataPacket &dp);
	virtual void LoadHeader(CDataPacket &dp);

	void CalcAABB();

	//properties
	
	i_math::aabbox3df aabb;
	std::vector<i_math::vector3df>vertices;
	std::vector<WORD>indices;
	BYTE tpData;
	std::vector<BYTE>data;
	DWORD buildtypeHk;
	i_math::vector4df offsetHk;

};