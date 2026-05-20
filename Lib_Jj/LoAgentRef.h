
#pragma once

#include "LevelDefines.h"

struct LoAgentRef
{
	LoAgentRef()
	{
		GConstructor();
		Zero(TRUE);
	}

	BEGIN_GOBJ( LoAgentRef, 1 );
	END_GOBJ();

	void Zero(BOOL bIntuitive)
	{
		idMap=0;
		guid=0;
		idAgent=0;
	}

	void Clear()
	{
		Zero(FALSE);
	}

	void Copy(LoAgentRef *src)
	{
		idMap=src->idMap;
		idAgent=src->idAgent;
		guid=src->guid;
	}

	void Save(CDataPacket &dp)
	{
		dp.Data_NextByte()=1;//version
		dp.Data_WriteSimple(idMap);
		dp.Data_WriteSimple(idAgent);
		dp.Data_WriteSimple(guid);
	}

	BOOL Load(CDataPacket &dp)
	{
		BYTE ver=dp.Data_NextByte();//version
		dp.Data_ReadSimple(idMap);
		dp.Data_ReadSimple(idAgent);
		dp.Data_ReadSimple(guid);

		return TRUE;
	}

	void SaveDelta(CDataPacket &dp,LoAgentRef*pRef)
	{
		Save(dp);
	}

	void LoadDelta(CDataPacket &dp,std::vector<void*>*ptrsDelta)
	{
		Load(dp);
		if (ptrsDelta)
			ptrsDelta->push_back(this);
	}


	BOOL IsValid()
	{
		return (idMap!=RecordID_Invalid)&&(idAgent!=RecordID_Invalid)&&(guid!=0);
	}


	RecordID idMap;
	LevelGUID guid;
	RecordID idAgent;
};