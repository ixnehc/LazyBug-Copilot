#pragma once

#include <string>
#include <vector>
#include <stack>
#include "../math/vector3d.h"

#include "ResData.h"

#include "behaviorgraph/BehaviorGraphPads.h"

#include "../gds/GObj.h"



struct BehaviorGraphData:public ResData
{
	DECLARE_CLASS(BehaviorGraphData);


	BehaviorGraphData();
	virtual ~BehaviorGraphData();
	virtual void Zero();
	virtual void Clean();

	//Overriding
	virtual 	ResType GetType();
	virtual const char *GetTypeName();
	virtual const char *GetTypeSuffix()	{		return "bgr";	}
	virtual void CalcContent(std::string &s);
	virtual void Save(CDataPacket &dp);
	virtual void Load(CDataPacket &dp);
	virtual void SaveHeader(CDataPacket &dp);
	virtual void LoadHeader(CDataPacket &dp);

	//properties
	CBehaviorGraphPads pads;
};