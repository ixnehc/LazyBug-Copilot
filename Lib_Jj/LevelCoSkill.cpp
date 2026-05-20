/********************************************************************
	created:	2013/6/30 
	author:		cxi
	
	purpose:	协同技能的Charge
*********************************************************************/

#include "stdh.h"
#include "LevelCoSkill.h"

#include "LevelRecordUnit.h"

#include "commondefines/general_stl.h"


BOOL CLevelCoSkill::AddCharge(RecordID idSkill,LevelSkillGrade grd,LevelSkillTarget &target)
{
	if (idSkill!=_idSkill)
	{
		int idx=-1;
		VEC_FIND_BY_ELEMENT(_rec->coskills,idSkill,idSkill,idx);
		if (idx==-1)
		{//不支持这个技能的协同
			_idSkill=idSkill;
			_chargeFull=0;
			_charge=0;
		}
		else
		{
			_idSkill=idSkill;
			_chargeFull=(WORD)_rec->coskills[idx].nFullCharge;
			_charge=0;
		}
	}
	if (TRUE)
	{
		if (_chargeFull<=0)
			return FALSE;
		if (_charge<_chargeFull)
			_charge++;

		if (_charge>=_chargeFull)
		{
			_grd=grd;
			_target=target;
			return TRUE;
		}
	}

	return FALSE;
}

RecordID CLevelCoSkill::FetchCharge(RecordID idSkill)
{
	if (_idSkill==RecordID_Invalid)
		return RecordID_Invalid;
	if (_idSkill!=idSkill)
		return RecordID_Invalid;
	if (_chargeFull<=0)
		return RecordID_Invalid;

	if (_charge<_chargeFull)
		return RecordID_Invalid;

	_charge=0;
	return _idSkill;
}
