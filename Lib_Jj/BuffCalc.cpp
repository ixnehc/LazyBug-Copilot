/********************************************************************
	created:	2012/01/03
	file base:	BuffCalc
	author:		cxi
	
	purpose:	Buff 的计算
*********************************************************************/

#include "stdh.h"

#include "BuffCalc.h"

float CBuffFormular::GetIMS()
{
	float cold=0.0f,ims=0.0f;
	float coldT;
	for (int i=0;i<_buffs->size();i++)
	{
		CBuffFactor*p=(*_buffs)[i];
		if (!p->IsAlive())
			continue;

		coldT=p->GetSlow();
		if (coldT>cold)
			cold=coldT;
		ims+=p->GetIMS();
	}

	float ret=1.0f+ims-cold;
	if (ret<0.1f)
		ret=0.1f;
	return ret;
}

float CBuffFormular::GetIAS()
{
	float cold=0.0f,ias=0.0f;
	float coldT;
	for (int i=0;i<_buffs->size();i++)
	{
		CBuffFactor *p=(*_buffs)[i];
		if (!p->IsAlive())
			continue;

		coldT=p->GetSlow();
		if (coldT>cold)
			cold=coldT;
		ias+=p->GetIAS();
	}

	float ret=1.0f+ias-cold;
	if (ret<0.1f)
		ret=0.1f;
	return ret;
}

