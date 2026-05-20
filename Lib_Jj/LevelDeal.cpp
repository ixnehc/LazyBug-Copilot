#include "stdh.h"
#include "commondefines/general.h"
#include "LevelDeal.h"
#include "LevelOSB.h"

#include "Random/Random.h"

#include "behaviorgraph/BehaviorMem.h"

CLevelDeal *CLevelDeal::Clone()
{
	CLevelDeal *p=(CLevelDeal *)GetClass()->New();
	p->GetGObj()->Copy(GetGObj());
	return p;
}


BIND_DEAL(Deal_Null);

static StringID GetDealTargetVarName()
{
	static StringID nm=StringID_Invalid;
	if (nm==StringID_Invalid)
		nm=StrLib_Get()->FindStr(0,"v_DealTargetID","行为图内存变量名称");
	return nm;
}


static BOOL Validate(CLevelObj *lo,StringID nmRelay,CLevelObj *loTarget)
{
	if (nmRelay==StringID_Invalid)
		return TRUE;

	if (lo)
	{
		CLevelBehavior *behavior=lo->GetBehaviorAI();
		if (behavior)
		{
			StringID nmVar=GetDealTargetVarName();
			CBehaviorMem *mem=behavior->GetMem(0);
			if (loTarget)
				mem->SetID(nmVar,BehaviorMemType_ObjID,loTarget->GetID());
			AResult result=behavior->StartRelay(nmRelay);
			if (loTarget)
				mem->SetID(nmVar,BehaviorMemType_ObjID,LevelObjID_Invalid);
			if (A_Ok==result)
				return TRUE;
		}
	}
	return FALSE;
}


void MakeDeals(std::vector<DealEntry> &deals,LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)
{
	for (int i=0;i<deals.size();i++)
	{
		DealEntry *e=&deals[i];

		if (CSysRandom::Roll(e->chance))
		{
			if (e->deal)
			{
				if (Validate(osbSrc.GetRootOwner(),e->nmValidatorSrc,loTarget))
					e->deal->Make(osbSrc,loTarget,arg,result);
			}
		}
	}
}

void MakeDeals(std::vector<DealEntry> &deals,LevelOSB &osbSrc,LevelPos3D &pos,DealArg&arg,DealResult *result)
{
	for (int i=0;i<deals.size();i++)
	{
		DealEntry *e=&deals[i];

		if (CSysRandom::Roll(e->chance))
		{
			if (e->deal)
			{
				if (Validate(osbSrc.GetRootOwner(),e->nmValidatorSrc,NULL))
					e->deal->Make(osbSrc,pos,arg,result);
			}
		}
	}
}
