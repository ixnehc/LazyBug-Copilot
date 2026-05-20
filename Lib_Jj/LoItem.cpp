
#include "stdh.h"
#include "Level.h"

#include "LoItem.h"

#include "LevelRecordUnit.h"
#include "LevelRecordItem.h"
#include "LevelRecords.h"

#include "LevelPlayerStates.h"

#include "LevelSkill.h"

#include "Random/Random.h"
#include "timer/profiler.h"

#include "LevelOSB.h"


//////////////////////////////////////////////////////////////////////////
//CLoItem

void CLoItem::PostCreate(LevelItemState *state,LevelPos&pos,LevelOSB &osb,LevelOpLink &link)
{
	_state.CopyFrom(state);

	CLevelRecords *records=_level->GetRecords();
	_rec=records->GetItem(_state.tid);

	_pos=pos;

	if (osb.IsEmpty())
		_opBirth=NewOp<LevelOp_ItemBirth>(link);
	else
		_opBirth=osb.NewOp<LevelOp_ItemBirth>(link);

	_tCreate=_level->GetT_();

}

void CLoItem::OnDestroy()
{
	_state.Clear();

	Safe_Class_Delete(_opBirth);

	Zero();
}

void CLoItem::Update()
{
	if (_opBirth)
	{
		AnimTick tCur=_level->GetT_();
		if (tCur>=_tCreate+ANIMTICK_FROM_SECOND(0.2f))
		{
			Safe_Class_Delete(_opBirth);
		}
	}
}


void CLoItem::WriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	bp->Data_WriteSimpleR(_pos);

	if (_rec)
		bp->Bits_Write(_rec->GetSimpleID(),11);
	else
		bp->Bits_Write(0,11);

	_state.Save(*bp->GetDP());

	if (_opBirth)
	{
		bp->Bit_Write_1();
		_opBirth->GetDesc().Save(bp);
		_opBirth->Save(bp);
	}
	else
		bp->Bit_Write_0();

	bContent=TRUE;
}



