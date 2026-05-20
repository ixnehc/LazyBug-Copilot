/********************************************************************
	created:	2012/10/20 
	author:		cxi
	
	purpose:	Level 几率分配器
*********************************************************************/
#include "stdh.h"

#include "LevelChancer.h"

#include "timer/timer.h"

/////////////////////////////////////////////////////////////////////////////////////
//CLevelChanceData

void CLevelChanceData::Init()
{
	_chSpawner=Class_New2(LevelChanceChannel);
	_nResults=0;
}

void CLevelChanceData::Clear()
{
	Safe_Class_Delete(_chSpawner);
	_nResults=0;
}

LevelChanceHandle CLevelChanceData::Register_Spawn(DWORD wt)
{

	LevelChanceEntry entry;
	entry.wt=wt;
	entry.idxResult=_nResults++;

	_chSpawner->entries.push_back(entry);
	_chSpawner->wtTotal+=wt;

	DWORD idx=_chSpawner->entries.size()-1;
	return idx+1;
}



/////////////////////////////////////////////////////////////////////////////////////
//CLevelChancer

void CLevelChancer::Init(CLevelChanceData *data)
{
	_data=data;
	_results.resize(_data->_nResults);

}

void CLevelChancer::Clear()
{
	_results.clear();
	_data=NULL;
}



void CLevelChancer::_MakeDice(LevelChanceChannel *ch,int nWts)
{
	if (!ch)
		return;

	if (ch->entries.size()<=0)
		return;

	extern int GenPrimeStep();
	int step=GenPrimeStep();
	int start=((int)GetTSC())%ch->entries.size();

	//此处的算法有问题,不能达到权重越大,出现几率越大的效果
	while(nWts>0)
	{
		_results[ch->entries[start].idxResult].chance=1.0f;
		nWts-=(int)ch->entries[start].wt;
		start=(start+step)%ch->entries.size();
	}


}


void CLevelChancer::MakeDice_Spawn(float rate)
{
	if (!_data)
		return;

	int nWts=(int)(rate*(float)_data->_chSpawner->wtTotal);

	_MakeDice(_data->_chSpawner,nWts);

}

void CLevelChancer::MakeDice_Spawn(int nWts)
{
	if (!_data)
		return;

	_MakeDice(_data->_chSpawner,nWts);
}


BOOL CLevelChancer::GetChance(LevelChanceHandle hChance,float &chance)
{
	hChance--;
	chance=0.0f;
	if (hChance>=_results.size())	
		return FALSE;
	chance=_results[hChance].chance;
	return TRUE;
}
