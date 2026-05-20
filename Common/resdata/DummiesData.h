#pragma  once

#include "ResData.h"
#include "../math/matrix43.h"
#include "MeshData.h"
#include "ResDataDefines.h"
#include <vector>

#include "../class/class.h"

#define MAX_DUMMYNAMELEN 32

struct  Dummy:public DummyInfo
{
	char name[MAX_DUMMYNAMELEN];
	Dummy & operator=(const Dummy & src)
	{
		idxBone=src.idxBone;
		matOff=src.matOff;
		bt = src.bt;
		matOffInv = src.matOffInv;

		memcpy(data,src.data,sizeof(data));

		strncpy(name,src.name,MAX_DUMMYNAMELEN);
		return *this;
	}
};

struct DummiesData :public ResData 
{
	DECLARE_CLASS(DummiesData);
	//Overriding 
	virtual 	ResType GetType();
	virtual const char *GetTypeName();
	virtual const char *GetTypeSuffix()	{		return "dms";	}
	virtual void Save(CDataPacket &dp);
	virtual void Load(CDataPacket &dp);
	//Note:ResData could be loaded only the header data ,which is small and could be used
	//to access some brief information.
	virtual void SaveHeader(CDataPacket &dp);
	virtual void LoadHeader(CDataPacket &dp);
	virtual void CalcContent(std::string &s);
	
	DWORD GetVersion() {return 2;}
	// dummies data.
	SkeletonInfo  skeletonInfo;	
	std::vector<Dummy> dummies;
};