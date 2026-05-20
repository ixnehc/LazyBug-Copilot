
#include "stdh.h"

#include "Level.h"

#include "LoSpawner.h"

#include "LoSPStone.h"

#include "Random/Random.h"

#include "LevelRecords.h"


BOOL CLoSPStone::OnActivate()
{
	CSysRandom rand;


	return TRUE;
}


void CLoSPStone::_OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	bp->Bit_Write(_bGathered);
	bContent=TRUE;
}

int CLoSPStone::GetSPAmount()
{
	return ((LopSPStone*)_param)->amount;
}


void CLoSPStone::Invoke(CLevelObj *loFrom)
{
	if (_bGathered)
		return;

	LevelRecordSkill *recSkill=GetLevel()->GetRecords()->GetSkill(((LosSPStone*)_src)->Skill);

	if (recSkill)
	{
		LevelSkillTarget target;
		target.SetObjID(loFrom->GetID());

		CLevelSkillDriver driver;
		driver.Init(this);
		driver.Start(LevelSkillType(((LosSPStone*)_src)->Skill),target,FALSE,ClientSkillID_Invalid,LevelSkillGrade_Invalid,NULL);
		driver.Clear();
	}
	

}
