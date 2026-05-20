/********************************************************************
	created:	2013/5/29 
	author:		cxi
	
	purpose:	GA功能:创建单位
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "BgnTroop_Build.h"
   
#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoGeneralAgent.h"

#include "LoUnit.h"
#include "LevelRecordUnit.h"


#include "Log/LogDump.h"
#include "Random/Random.h"


//////////////////////////////////////////////////////////////////////////
//BuildTroopParam
const char *BuildTroopParam::GetBrief(void *param)
{
	static std::string s;
	static std::string ss;

	s="";
	for (int i=0;i<entries.size();i++)
	{
		if (entries[i].IsValid()&&entries[i].IsEnabled())
		{
			if (s!="")
				s+=",";
			AppendFmtString(s,"%s",entries[i].GetBrief(param));
		}
	}
	if (s=="")
		return "n/a";
	FormatString(ss,"[总分%d]%s",(int)scoreTotal,s.c_str());
	return ss.c_str();
}


////////////////////////////////////////////////////////////////////////
//CBgnGA_BuildTroop
BIND_BGN_CLASS(CBgnTroop_Build,CBgpTroop_Build);

void CBgnTroop_Build::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpTroop_Build*pad=_GetPad<CBgpTroop_Build>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	BuildTroopParam *param=&pad->_param;

	CLevelTroop *troop=_ObtainTroop(pad->_troop);
	if (!troop)
	{
		_SetResult(A_Fail);
		return;
	}

	if (!pad->_bForceRecreate)
	{
		if (!troop->IsEmpty())
		{
			_SetResult(A_Ok);
			return;
		}
	}

	if (param)
	{
		LevelBehaviorContext *ctx=_GetCtx();

		if (param)
		{
			troop->ClearUnitsAndFrames(TRUE);

			std::map<LevelTroopDesc *,int>counts;
			std::vector<LevelTroopDesc*> candidates;

			float scoreTotal=param->scoreTotal;
			for (int i=0;i<param->entries.size();i++)
			{
				LevelTroopDesc *entry=&param->entries[i];
				if(entry->idUnit==RecordID_Invalid)
					continue;
				if (!entry->bEnable)
					continue;
				LevelRecordUnit *rec=level->GetRecords()->GetUnit(entry->idUnit);
				if (!rec)
					continue;
				counts[entry]=entry->nMin;
				
				scoreTotal-=rec->score*entry->nMin;
			}

			while(scoreTotal>0.0f)
			{
				candidates.clear();
				for (int i=0;i<param->entries.size();i++)
				{
					LevelTroopDesc *entry=&param->entries[i];
					std::map<LevelTroopDesc *,int>::iterator it=counts.find(entry);
					if (it==counts.end())
						continue;
					if ((*it).second>=entry->nMax)
						continue;
					if (level->GetRecords()->GetUnit(entry->idUnit)->score>scoreTotal)
						continue;
					candidates.push_back(entry);
				}

				if (candidates.size()<0)
					break;

				LevelTroopDesc *entrySel=CSysRandom::RollWeighted<LevelTroopDesc>(candidates);
				if (!entrySel)
					break;

				counts[entrySel]++;
				scoreTotal-=level->GetRecords()->GetUnit(entrySel->idUnit)->score;
			}

			std::map<LevelTroopDesc *,int>::iterator it;
			for (it=counts.begin();it!=counts.end();it++)
			{
				LevelTroopDesc *entry=(*it).first;
				int count=(*it).second;
				if (pad->_countOverride>0)
					count=pad->_countOverride;
				for (int j=0;j<count;j++)
					troop->AddFrame(entry);
			}
		}
	}

	_OutputOk(outputs,1,"结束");
}
