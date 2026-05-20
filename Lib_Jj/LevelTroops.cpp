
#include "stdh.h"

#include "Level.h"
#include "LevelIDs.h"

#include "LevelAIContext.h"

#include "LevelTroops.h"

#include "behaviorgraph/BehaviorValue.h"

//////////////////////////////////////////////////////////////////////////
//LevelTroopDesc
const char *LevelTroopDesc::GetBrief(void *param)
{
	FillDescAssist *assist=(FillDescAssist *)param;
	static std::string s;
	s="n/a";
	if (IsValid())
	{
		if (!bEnable)
			FormatString(s,"[Disabled]%s",assist->GetUnitName(idUnit));
		else
		{
			if (nMin==nMax)
				FormatString(s,"%s:%d个",assist->GetUnitName(idUnit),nMin,nMax);
			else
				FormatString(s,"%s:%d~%d个",assist->GetUnitName(idUnit),nMin,nMax);
		}
	}
	return s.c_str();

}


const char *LevelTroopRank_GetDesc(LevelTroopRank rank)
{
	switch(rank)
	{
		case LevelTroopRank_Minion:
			return "Minion";
		case LevelTroopRank_Leader:
			return "Leader";
	}
	return "n/a";
}

const char *LevelTroopRankFlag_GetDesc(LevelTroopRankFlags flags,StringID nmRef)
{
	static std::string s;
	s="";
	if (nmRef==StringID_BhvValInvalidRef)
	{
		if ((flags&LevelTroopRankFlag_All)==LevelTroopRankFlag_All)
			return "全部职级";

		if (flags&LevelTroopRankFlag_Leader)
		{
			if (s!="")
				s+=",";
			s+="Leader";
		}
		if (flags&LevelTroopRankFlag_Minion)
		{
			if (s!="")
				s+=",";
			s+="Minion";
		}
	}
	else
		s=std::string("[")+StrLib_GetStr(nmRef)+"]";

	return s.c_str();
}


//////////////////////////////////////////////////////////////////////////
//TroopCombatContext
void TroopCombatContext::Clear()
{
	for (int i=0;i<detects.size();i++)
	{
		SAFE_RELEASE(detects[i]);
	}
	detects.clear();
}


//////////////////////////////////////////////////////////////////////////
//CLevelTroop
extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);

void CLevelTroop::Init(CLevel *level)
{
	_level=level;
}

void CLevelTroop::Clear()
{
	ClearUnitsAndFrames(TRUE);
	_tcc.Clear();

	Zero();
}

void CLevelTroop::ClearUnitsAndFrames(BOOL bDestroy)
{
	DetachAllUnits(bDestroy);
	_frames.clear();
}


void CLevelTroop::AddFrame(LevelTroopDesc *desc)
{
	LevelTroopFrame frm;
	frm.desc=desc;
	_frames.push_back(frm);
}

void CLevelTroop::FlushDeadFrames()
{
	if (!_level)
		return;

	int i=0;
	int c=_frames.size();
	while(i<c)
	{
		LevelTroopFrame *frm=&_frames[i];
		if (frm->idUnit!=LevelObjID_Invalid)
		{
			if (_CheckDead(frm->idUnit))
			{
				CLevelObj *lo=LevelUtil_GetAliveLo(_level,frm->idUnit);
				if (lo)
				{
					lo->SetTroop(NULL);
					lo->DeferDestroy();
				}

				Swap(_frames[i],_frames[c-1]);
				c--;
				continue;
			}
		}
		else
		{
			Swap(_frames[i],_frames[c-1]);
			c--;
			continue;
		}
		i++;
	}

	_frames.resize(c);
}

LevelTroopFrame *CLevelTroop::GetFrame(DWORD idxFrame)
{
	if (idxFrame<_frames.size())
		return &_frames[idxFrame];
	return NULL;
}

void CLevelTroop::DetachAllUnits(BOOL bDestroy)
{
	if (!_level)
		return;

	std::deque<LevelTroopFrame>::iterator it;
	for (it=_frames.begin();it!=_frames.end();it++)
	{
		LevelTroopFrame *frm=&(*it);
		if (frm->idUnit!=LevelObjID_Invalid)
		{
			CLevelObj *lo=LevelUtil_GetAliveLo(_level,frm->idUnit);
			if (lo)
			{
				lo->SetTroop(NULL);
				if (bDestroy)
					lo->DeferDestroy();
			}
			frm->idUnit=LevelObjID_Invalid;
		}
	}
}

void CLevelTroop::DetachUnit(LevelObjID idUnit)
{
	if (!_level)
		return;

	std::deque<LevelTroopFrame>::iterator it;
	for (it=_frames.begin();it!=_frames.end();it++)
	{
		LevelTroopFrame *frm=&(*it);
		if (frm->idUnit==idUnit)
		{
			CLevelObj *lo=LevelUtil_GetAliveLo(_level,frm->idUnit);
			if (lo)
			{
				lo->SetTroop(NULL);
// 				lo->DeferDestroy();
			}
			frm->idUnit=LevelObjID_Invalid;

			if (!frm->desc)
				_frames.erase(it);
			break;
		}
	}
}

BOOL CLevelTroop::_CheckDead(LevelObjID idUnit)
{
	extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
	CLevelObj *lo=LevelUtil_GetAliveLo(_level,idUnit);
	if (!lo)
		return TRUE;
	if (lo->GetType()!=LevelObjType_Unit)
		return FALSE;

	extern BOOL LevelUtil_CheckDead(CLevelObj *lo);
	return LevelUtil_CheckDead(lo);
}


void CLevelTroop::FlushDeadUnits()
{
	if (!_level)
		return;

	std::deque<LevelTroopFrame>::iterator it;
	for (it=_frames.begin();it!=_frames.end();it++)
	{
		LevelTroopFrame *frm=&(*it);
		if (frm->idUnit!=LevelObjID_Invalid)
		{
			if (!_CheckDead(frm->idUnit))
				continue;

			CLevelObj *lo=LevelUtil_GetAliveLo(_level,frm->idUnit);
			if (lo)
			{
				lo->SetTroop(NULL);
				lo->DeferDestroy();
			}
			frm->idUnit=LevelObjID_Invalid;
		}
	}
}

BOOL CLevelTroop::IsAllDead(LevelTroopRankFlags  flagsRank)
{
	if (!_level)
		return TRUE;

	std::deque<LevelTroopFrame>::iterator it;
	for (it=_frames.begin();it!=_frames.end();it++)
	{
		LevelTroopFrame *frm=&(*it);
		if (frm->idUnit!=LevelObjID_Invalid)
		{
			if (frm->CheckRank(flagsRank))
			{
				if (!_CheckDead(frm->idUnit))
					return FALSE;
			}
		}
	}
	
	return TRUE;
}

BOOL CLevelTroop::CheckHealthRatio_AnyBelow(LevelTroopRankFlags flagsRank,float ratio)
{
	if (!_level)
		return TRUE;

	std::deque<LevelTroopFrame>::iterator it;
	for (it=_frames.begin();it!=_frames.end();it++)
	{
		LevelTroopFrame *frm=&(*it);
		if (frm->idUnit!=LevelObjID_Invalid)
		{
			if (frm->CheckRank(flagsRank))
			{
				extern float LevelUtil_GetHealthRatio(CLevel *level,LevelObjID id);
				float r=LevelUtil_GetHealthRatio(_level,frm->idUnit);
				if (r<ratio)
					return TRUE;
			}
		}
	}

	return TRUE;
}


BOOL CLevelTroop::AddUnit(LevelTroopDesc *desc,LevelObjID idUnit)
{
	LevelTroopFrame frm;
	frm.desc=desc;
	frm.idUnit=idUnit;
	if (TRUE)
	{
		CLevelObj *lo=LevelUtil_GetAliveLo(_level,idUnit);
		if (lo)
			lo->SetTroop(this);
	}
	_frames.push_back(frm);

	return TRUE;
}

BOOL CLevelTroop::AddUnit(LevelTroopRank rank,LevelObjID idUnit)
{
	LevelTroopFrame frm;
	frm.rank=rank;
	frm.idUnit=idUnit;
	if (TRUE)
	{
		CLevelObj *lo=LevelUtil_GetAliveLo(_level,idUnit);
		if (lo)
			lo->SetTroop(this);
	}
	_frames.push_back(frm);

	return TRUE;
}


BOOL CLevelTroop::FillUnit(LevelTroopDesc *desc,LevelObjID idUnit)
{
	for (int i=0;i<_frames.size();i++)
	{
		LevelTroopFrame *frm=&_frames[i];
		if ((frm->desc==desc)&&(frm->idUnit==LevelObjID_Invalid))
		{//找到了一个匹配的编制
			frm->idUnit=idUnit;

			if (TRUE)
			{
				CLevelObj *lo=LevelUtil_GetAliveLo(_level,idUnit);
				if (lo)
					lo->SetTroop(this);
			}

			return TRUE;
		}
	}
	return FALSE;
}


void CLevelTroop::SetCmdToUnits(LevelTroopRankFlags flagsRank,StringID idCmd)
{
	if (!_level)
		return;

	std::deque<LevelTroopFrame>::iterator it;
	for (it=_frames.begin();it!=_frames.end();it++)
	{
		LevelTroopFrame *frm=&(*it);
		if (frm->idUnit!=LevelObjID_Invalid)
		{
			if (frm->CheckRank(flagsRank))
			{
				CLevelObj *lo=LevelUtil_GetAliveLo(_level,frm->idUnit);
				if (lo)
					lo->SetAICmd(idCmd);
			}
		}
	}
}

void CLevelTroop::DiscardCmdFromUnits(LevelTroopRankFlags flagsRank,StringID idCmd)
{
	if (!_level)
		return;

	std::deque<LevelTroopFrame>::iterator it;
	for (it=_frames.begin();it!=_frames.end();it++)
	{
		LevelTroopFrame *frm=&(*it);
		if (frm->idUnit!=LevelObjID_Invalid)
		{
			if (frm->CheckRank(flagsRank))
			{
				CLevelObj *lo=LevelUtil_GetAliveLo(_level,frm->idUnit);
				if (lo)
				{
					if (lo->GetAICmd()==idCmd)
						lo->SetAICmd(StringID_Invalid);
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//CLevelTroops

void CLevelTroops::Init(CLevel *level)
{
	_level=level;

}

void CLevelTroops::Clear()
{
	std::unordered_map<StringID,CLevelTroop*>::iterator it;
	for (it=_buf.begin();it!=_buf.end();it++)
	{
		CLevelTroop *troop=(*it).second;
		if (troop)
			troop->Clear();
		Safe_Class_Delete(troop);
	}
	_buf.clear();
	Zero();
}

CLevelTroop *CLevelTroops::Obtain(StringID nm)
{
	if (nm==StringID_Invalid)
		return NULL;
	std::unordered_map<StringID,CLevelTroop*>::iterator it=_buf.find(nm);
	if (it!=_buf.end())
		return (*it).second;
	CLevelTroop *troop=Class_New2(CLevelTroop);
	troop->Init(_level);
	_buf[nm]=troop;
	return troop;
}

CLevelTroop *CLevelTroops::Get(StringID nm)
{
	if (nm==StringID_Invalid)
		return NULL;
	std::unordered_map<StringID,CLevelTroop*>::iterator it=_buf.find(nm);
	if (it!=_buf.end())
		return (*it).second;
	return NULL;
}

void CLevelTroops::Remove(StringID nm)
{
	if (nm==StringID_Invalid)
		return;
	std::unordered_map<StringID,CLevelTroop*>::iterator it=_buf.find(nm);
	if (it!=_buf.end())
	{
		CLevelTroop *troop=(*it).second;
		if (troop)
			troop->Clear();
		Safe_Class_Delete(troop);
		_buf.erase(it);
	}
}

CLevelTroop *CLevelTroops::GetFirst()
{
	if (_buf.size()>0)
		return (*_buf.begin()).second;
	return NULL;
}
