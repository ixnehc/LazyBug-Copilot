#pragma once

#include "../strlib/strlib.h"

#include "../records/recordsdefine.h"

#include "BehaviorDefines.h"

class CRecord;
struct FillDescAssist
{
	virtual const char *GetUnitName(RecordID idRec)=0;
	virtual const char *GetSkillName(RecordID idRec)=0;
	virtual const char *GetItemName(RecordID idRec)=0;
	virtual const char *GetAgentName(RecordID idRec)=0;
	virtual const char *GetGestureName(RecordID idRec)=0;
	virtual const char *GetBuffName(RecordID idRec)=0;
	virtual const char *GetMagicTileName(RecordID idRec)=0;
	virtual const char *GetMapName(RecordID idMap)=0;
	virtual const char *GetResName(RecordID idMap)=0;
	virtual const char *GetEoName(RecordID idMap)=0;

	virtual CRecord *GetClonedRec_Buff(RecordID idBuff)=0;//注意返回的CRecord* 有一个引用计数

	virtual const char *GetStr(StringID nm)=0;
	virtual float CalcHeight(const char *str)=0;
	virtual BehaviorMemType GetMemType(StringID nm)=0;
};

