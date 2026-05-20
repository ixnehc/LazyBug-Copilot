#pragma once

#include <string>

#include <vector>

#include "ResDataDefines.h"

#include "../class/class.h"

class CDataPacket;
struct ResData
{
//Overidable
	virtual CClass *GetClass()=0;
	virtual 	ResType GetType()=0;
	virtual const char *GetTypeName()=0;
	virtual const char *GetTypeSuffix()=0;//该种资源的后缀
	virtual void Save(CDataPacket &dp)=0;
	virtual void Load(CDataPacket &dp)=0;

	//Note:ResData could be loaded only the header data ,which is small and could be used
	//to access some brief information.
	virtual void SaveHeader(CDataPacket &dp)=0;
	virtual void LoadHeader(CDataPacket &dp)=0;

	virtual void CalcContent(std::string &s)		{}

	virtual void CollectRefs(std::vector<std::string>&buf)	{		return;	}

//Operations
	const char *GetContent();//A string to describe the content of this res data
	BOOL Copy(ResData &src);
	ResData *Clone();
	BOOL Equal(ResData &other);
	void SaveData(std::vector<BYTE>&data);
	void LoadData(std::vector<BYTE>&data);
	void SaveHeaderData(std::vector<BYTE>&data);
	void LoadHeaderData(std::vector<BYTE>&data);

	ResData(){}
	virtual ~ResData(){}

	std::string content;

};

extern ResData *ResData_New(ResType tp);
extern void ResData_Delete(ResData *p);
extern ResData *ResData_Clone(ResData *p);

