/********************************************************************
	created:	2012/03/19
	file base:	RecordsData
	author:		cxi
	
	purpose:	Records ×ÊÔ´µÄdata
*********************************************************************/
#include "stdh.h"

#include "RecordsData.h"

#include "stringparser/stringparser.h"

IMPLEMENT_CLASS(RecordsData);


void RecordsData::Clear()
{
	records.Clear();
}

void RecordsData::Save(CDataPacket &dp)
{
	records.Save(&dp);
}

void RecordsData::Load(CDataPacket &dp)
{
	records.Load(&dp);
}
