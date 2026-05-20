
#include "stdh.h"

#include "Level.h"

#include "LoUnit.h"

#include "EoChain.h"

#include "LevelRecords.h"

#include "LevelOSB.h"


BIND_EOPARAM(EoChain,EoParamChain);

void EoChain::_OnPostCreate()
{
	_BuildChain();
}

extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);

void EoChain::_BuildChain()
{
	if (_idHost==LevelObjID_Invalid)
		return;

	if (TRUE)
	{
		extern BOOL LevelUtil_TestAnyBuff(CLevelObj *lo,DWORD flagBuff);
		if (LevelUtil_TestAnyBuff(LevelUtil_GetAliveLo(_level,_idHost),BuffFlag_Dead|BuffFlag_Invisible))
			return;
	}

	EoParamChain *param=GetParam<EoParamChain>();

	DWORD c;
	CLevelObj **buf=_DetectRange(_xfmInitial.pos.getXZ(),param->radiusAffect,c);

	Entry e;
	e.lo=LevelUtil_GetAliveLo(_level,_idHost);
	e.id=_idHost;
	e.iStep=0;
	e.parent=-1;

	if (!e.lo)
		return;

	_entries.push_back(e);

	float radiusStep2=param->radiusStep*param->radiusStep;

	int i=0;
	while(i<_entries.size())
	{
		Entry &e=_entries[i];
		int iEntry=i;
		i++;

		if (e.iStep>=param->nSteps)
			break;

		LevelPos posMe=e.lo->GetFramePos();

		for (int k=0;k<param->nBranch;k++)
		{
			CLevelObj *loClosest=NULL;
			int idxClosest=-1;
			float dist2Min=100000000.0f;
			for (int j=0;j<c;j++)
			{
				CLevelObj *loTarget=buf[j];
				if (!loTarget)
					continue;
				if (loTarget->GetID()==_idHost)
					continue;
				LevelPos pos=loTarget->GetFramePos();

				float dist2=posMe.getDistanceSQFrom(pos);
				if (dist2>radiusStep2)
					continue;//太远了

				if (dist2<dist2Min)
				{
					loClosest=loTarget;
					dist2Min=dist2;
					idxClosest=j;
				}
			}

			if (idxClosest>=0)
			{
				Entry eNew;
				eNew.lo=loClosest;
				eNew.id=loClosest->GetID();
				eNew.parent=iEntry;
				eNew.iStep=e.iStep+1;

				_entries.push_back(eNew);

				//交换到最后
				buf[idxClosest]=buf[c-1];
				buf[c-1]=loClosest;
				c--;
			}

			if (c<=0)
				break;
		}
		if (c<=0)
			break;
	}

	_nSteps=0;
	_nDealedEntries=0;
	_tStart=_GetT();
}

void EoChain::_OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	bp->Data_NextWord()=_entries.size();

	for (int i=0;i<_entries.size();i++)
	{
		bContent=TRUE;
		Entry &e=_entries[i];

		bp->Data_WriteSimple(e.id);
		bp->Data_WriteSimple(e.parent);
		bp->Data_WriteSimple(e.iStep);
	}
}


void EoChain::_OnUpdate()
{
	EoParamChain *param=GetParam<EoParamChain>();
	if (!param)
		return;

	AnimTick t=_GetT();

	t+=LEVEL_FRAME_TICK;

	t=ANIMTICK_SAFE_MINUS(t,_tStart);
	int nStep=t/param->dtStep;

	for (int i=_nDealedEntries;i<_entries.size();i++)
	{
		Entry &e=_entries[i];
		if (e.iStep>=nStep)
			break;

		_nDealedEntries++;

		if (param->bIgnoreHostDeal)
		{
			if (e.id==_idHost)
				continue;
		}

		CLevelObj *lo=LevelUtil_GetAliveLo(_level,e.id);
		if (!lo)
			continue;
		DealArg arg;
		arg.link.id=_level->GenOpLinkID();
		arg.link.iSerial=i;
		_MakeDeals(lo,arg);

		_nDealed++;
	}


	if (_level->GetT_()>_tStart+param->dtStep*param->nSteps+ANIMTICK_FROM_SECOND(2.0f))
	{
		DeferDestroy();
	}
}

