#pragma once

#include "class/class.h"

#include "LevelDefines.h"

struct LevelRecordUnit;

class CLevelCoSkill
{
public:
	DEFINE_CLASS(CLevelCoSkill);
	CLevelCoSkill()
	{
		_idSkill=RecordID_Invalid;
		_grd=LevelSkillGrade_Invalid;

		_charge=0;
		_chargeFull=0;
		_rec=NULL;
	}

	void Init(LevelRecordUnit *rec)	{		_rec=rec;	}

	BOOL AddCharge(RecordID idSkill,LevelSkillGrade grd,LevelSkillTarget &target);//返回Charge有没有满

	RecordID FetchCharge(RecordID idSkill);

protected:
	
	RecordID _idSkill;
	LevelSkillGrade _grd;

	WORD _charge;
	WORD _chargeFull;//满值

	LevelSkillTarget _target;

	LevelRecordUnit *_rec;

};
