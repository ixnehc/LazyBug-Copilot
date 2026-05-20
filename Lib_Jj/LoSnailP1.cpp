
#include "stdh.h"

#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelUtil.h"

#include "LoUnit.h"

#include "LoSnailP1.h"

#include "LevelRecords.h"

#include "timer/profiler.h"


////////////////////////////////////////////////////////////////////////
//CLoSnailP1

void CLoSnailP1::PostCreate()
{
	CLoAgent::PostCreate();

	LopSnailP1 *lop=(LopSnailP1*)_param;
	LosSnailP1 *los=(LosSnailP1*)_src;
}

void CLoSnailP1::OnDestroy()
{
	SAFE_RELEASE(_loSnailUnit);
}


BOOL CLoSnailP1::OnActivate()
{
	_level->RegisterUniqueObj(LevelUniqueObj_SnailP1,this);

	return TRUE;
}

void CLoSnailP1::OnDeactivate()
{
	_level->UnregisterUniqueObj(LevelUniqueObj_SnailP1,this);
}


void CLoSnailP1::Update()
{
	LopSnailP1 *lop=(LopSnailP1*)_param;
}


void CLoSnailP1::_OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	LopSnailP1 *lop=(LopSnailP1*)_param;

}

void CLoSnailP1::_OnWriteSyncL(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	LopSnailP1 *lop=(LopSnailP1*)_param;

}


void CLoSnailP1::_OnPostWriteSync()
{
}


void CLoSnailP1::BreakTongue()
{
	if (_tTongueBroken!=ANIMTICK_INFINITE)
		return;//已经Break了
	_tTongueBroken=_level->GetT_();
}

BOOL CLoSnailP1::IsTongueBrokenForAWhile(AnimTick dur)
{
	if (_tTongueBroken==ANIMTICK_INFINITE)
		return FALSE;
	AnimTick tCur=_level->GetT_();
	if (_tTongueBroken+dur<=tCur)
		return TRUE;
	return FALSE;
}
