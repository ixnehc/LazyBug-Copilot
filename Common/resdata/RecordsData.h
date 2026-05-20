#pragma once

#include <string>
#include <vector>

#include "ResData.h"

#include "datapacket/DataPacket.h"
#include "../class/class.h"

#include "../records/records.h"


struct RecordsData:public ResData
{
	DECLARE_CLASS(RecordsData);
	void Clear();
	virtual 	ResType GetType()	{		return Res_Records;	}
	virtual const char *GetTypeName()	{		return "Records";	}
	virtual const char *GetTypeSuffix()	{		return "rcs";	}

	virtual void Save(CDataPacket &dp);
	virtual void Load(CDataPacket &dp);

	virtual void SaveHeader(CDataPacket &dp)
	{
	}
	virtual void LoadHeader(CDataPacket &dp)
	{
	}

	virtual WORD GetVer()	{		return 1;	};

	CRecords records;
};

