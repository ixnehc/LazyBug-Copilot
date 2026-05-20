
#include "stdh.h"

#include "commondefines/general_stl.h"

#include "LevelPoem.h"

PoemAwards *CLevelPoem::FindPoemAwards(StringID nm)
{
	CLevelAbilityInitial *upgrade=_GetInitialUpgrade();
	if (upgrade)
	{
		CLevelPoemInitial *upgradePoem=(CLevelPoemInitial *)upgrade;
		
		int idx;
		VEC_FIND_BY_ELEMENT(upgradePoem->_awards,nm,nm,idx);
		if (idx>=0)
			return &upgradePoem->_awards[idx];
	}
	return NULL;
}
