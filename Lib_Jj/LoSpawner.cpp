
#include "stdh.h"

#include "Level.h"
#include "LevelRtnus.h"

#include "LoSpawner.h"

#include "LoUnit.h"

#include "Random/Random.h"

#include "LevelRecordRegion.h"

/////////////////////////////////////////////////////////////////////////////
//CLoSpawner

BOOL LopSpawner::CheckCreateChance(CLevel *level,CLevelObjSrc *los0)
{
	if (!bRef)
		return TRUE;//不参考分布信息,必然创建

	LosSpawner *los=(LosSpawner *)los0;
	i_math::matrix43f *mat=&los->GetMat();

	extern AgentDistributeInfo *LevelUtil_FindADI(CLevel *lvl,RecordID idAgent,float x,float z);
	AgentDistributeInfo*adi=LevelUtil_FindADI(level,los->GetRecID(),mat->getTranslationP()->x,mat->getTranslationP()->z);
	if (!adi)
		return FALSE;

	if (!CSysRandom::Roll(adi->rateAppear))
		return FALSE;

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
//CLoSpawner


BOOL CLoSpawner::OnActivate()
{
	CSysRandom rand;

	LosSpawner *los=(LosSpawner *)_src;
	LopSpawner *lop=(LopSpawner *)_param;

	i_math::vector2df center=GetFramePos();
	if (TRUE)
	{
		for (int i=0;i<lop->count;i++)
		{
			float radian=rand.RandRange(0.0f,i_math::Pi*2.0f);
			float radius=rand.RandRange(0.0f,lop->radius);

			LevelPos pos;
			pos.x=center.x+cosf(radian)*radius;
			pos.y=center.y+sinf(radian)*radius;

			if (!_level->GetUnitMgr()->IsWalkable(UnitFindPath_Walkable,pos))
				continue;

			CLoUnit* lo=(CLoUnit*)_level->CreateObj(Class_Ptr2(CLoUnit));

			LevelGrade grd=lop->grdBase;

			if (lop->bRef)
			{
				AgentDistributeInfo *adi=NULL;
				extern AgentDistributeInfo *LevelUtil_FindADI(CLevel *lvl,RecordID idAgent,float x,float z);
				adi=LevelUtil_FindADI(_level,los->GetRecID(),center.x,center.y);
				if (adi)
					grd=adi->grdRef;
			}

			grd=(LevelGrade)CSysRandom::RandVaryUInt((int)grd,lop->grdVary);

			lo->PostCreate(LevelPlayerID_Wild,NULL,los->idUnit,grd,NULL,EquipSetPick_None,pos);

			_level->AddToActives(lo);

			if (TRUE)
			{
				CLevelPlayer *player=_level->GetPlayer((LevelPlayerID)lop->idMaster);
				if (player)
				{
					if (player->GetRtnus())
					player->GetRtnus()->Add_New(lo,FALSE);
				}
			}

			SAFE_RELEASE(lo);
		}
	}

	return TRUE;
}
