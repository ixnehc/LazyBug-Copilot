
#include "stdh.h"

#include "Level.h"
#include "LevelIDs.h"

#include "LevelResPiles.h"

void CLevelResPiles::Init(CLevel *level)
{
	_level=level;
}

void CLevelResPiles::Clear()
{
	_piles.clear();
	Zero();
}

void CLevelResPiles::Deposit(LevelObjID idOwner,LevelResourceType tp,int amount)
{
	for (int i=0;i<_piles.size();i++)
	{
		if ((_piles[i].idOwner==idOwner)&&(_piles[i].tp==tp))
		{
			_piles[i].amount+=amount;
			return;
		}
	}

	LevelResPile pile;
	pile.idOwner=idOwner;
	pile.tp=tp;
	pile.amount=amount;

	_piles.push_back(pile);
}

int CLevelResPiles::GetAmount(LevelObjID idOwner,LevelResourceType tp)
{
	for (int i=0;i<_piles.size();i++)
	{
		if ((_piles[i].idOwner==idOwner)&&(_piles[i].tp==tp))
			return _piles[i].amount;
	}
	return 0;
}


int CLevelResPiles::Fetch(LevelObjID idOwner,LevelResourceType tp,int amount)
{
	for (int i=0;i<_piles.size();i++)
	{
		if (((_piles[i].idOwner==idOwner)||(_piles[i].idOwner==LevelObjID_Invalid))&&(_piles[i].tp==tp))
		{
			if (_piles[i].amount>amount)
			{
				_piles[i].amount-=amount;
				return amount;
			}
			else
			{
				amount=_piles[i].amount;
				_piles[i]=_piles[_piles.size()-1];
				_piles.pop_back();
				return amount;
			}
		}
	}
	return 0;
}