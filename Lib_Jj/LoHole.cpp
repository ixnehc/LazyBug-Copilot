
#include "stdh.h"

#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelUtil.h"
#include "LevelRtnus.h"

#include "LoUnit.h"

#include "LoHole.h"

#include "Random/Random.h"

#include "LevelRecords.h"
#include "LevelRecordRegion.h"
#include "LevelRecordAgent.h"
#include "LevelRecordUnit.h"

#include "Buff_Birth.h"


////////////////////////////////////////////////////////////////////////
//LopHole
LopHole::LopHole()
{
	GConstructor();
	HoleSpawnAmount t;
	amnts.push_back(t);
}

LopHole::~LopHole()
{
	GDestructor();

}

BOOL LopHole::CheckCreateChance(CLevel *level,CLevelObjSrc *los)
{
	return TRUE;
	i_math::matrix43f *mat=&los->GetMat();

	extern AgentDistributeInfo *LevelUtil_FindADI(CLevel *lvl,RecordID idAgent,float x,float z);
	AgentDistributeInfo*adi=LevelUtil_FindADI(level,los->GetRecID(),mat->getTranslationP()->x,mat->getTranslationP()->z);
	if (!adi)
		return FALSE;

	if (!CSysRandom::Roll(adi->rateAppear))
		return FALSE;

	return TRUE;
}


////////////////////////////////////////////////////////////////////////
//CLoBugHole

void CLoHole::PostCreate()
{
	CLoAgent::PostCreate();

	LosHole *los=(LosHole*)_src;
	LopHole *lop=(LopHole*)_param;
	LevelRecordAgent *rec=los->GetRecord();

	//根据shape创建Static 的Units
	CUnitMgrNavMesh *unitmgr=GetLevel()->GetUnitMgr();
	i_math::matrix43f *mat=&_src->GetMat();
	LevelObjCircle circle;
	i_math::spheref sph;
	_shape.reserve(los->shape.size());
	_circles.reserve(los->shape.size());
	for (int i=0;i<los->shape.size();i++)
	{
		sph=los->shape[i];
		mat->transformSphere(sph,sph);

		CUnit *unit=unitmgr->CreateUnit(i_math::vector2df(sph.center.x,sph.center.z),0.0f,rec->Radius,0.0f,NULL);
		//UnitCollide
		if (TRUE)
		{
			UnitCollide collide=UnitCollide_Empty;
			extern void UnitCollide_SetStatic(UnitCollide &collide,BOOL bStatic);
			UnitCollide_SetStatic(collide,TRUE);
			unit->SetCollide(collide);
		}
		unit->SetData((void*)this);

		_shape.push_back(unit);

		circle.center.set(sph.center.x,sph.center.z);
		circle.radius=sph.radius;
		_circles.push_back(circle);
	}

	if (TRUE)
	{
		CSysRandom rnd;
		BOOL bIgnore[32];
		for (int i=0;i<los->cats.size();i++)
			bIgnore[i]=(BYTE)!rnd.Roll(los->cats[i].rate);

		_infosBug.resize(los->cats.size());
		VEC_SET(_infosBug,0);

		for (int i=0;i<lop->amnts.size();i++)
		{
			RecordID idUnit=lop->amnts[i].idUnit;
			int idx=0;
			if (idUnit!=RecordID_Invalid)
			{
				VEC_FIND_BY_ELEMENT(los->cats,idUnit,idUnit,idx);
			}
			if (idx<0)
				continue;

			if (idx>=_infosBug.size())
				continue;

			if (bIgnore[idx])
				continue;

//			int n=lop->amnts[i].amount+rnd.RandRange(-lop->amnts[i].variation,lop->amnts[i].variation);
			int n=rnd.RandRangeInt(lop->amnts[i].amountMin,lop->amnts[i].amountMax+1);
			if (n<0)
				n=0;
			_infosBug[idx].count+=n;
			_infosBug[idx].grdBase=lop->amnts[i].grdBase;
			_infosBug[idx].grdVary=lop->amnts[i].grdVary;
		}
	}

	//residable
	if (TRUE)
	{
		i_math::vector3df pos;
		if (los->sitesReside.size()>0)
			pos=los->sitesReside[0].getTranslation();
		_residable.Init(los->GetMat(),pos);
	}

	_grdRef=0;
	if (TRUE)
	{
		AgentDistributeInfo *adi=NULL;
		i_math::matrix43f *mat=&los->GetMat();
		if (mat)
		{
			extern AgentDistributeInfo *LevelUtil_FindADI(CLevel *lvl,RecordID idAgent,float x,float z);
			adi=LevelUtil_FindADI(_level,los->GetRecID(),mat->getTranslationP()->x,mat->getTranslationP()->z);
			if (adi)
				_grdRef=adi->grdRef;
		}
	}
}

void CLoHole::OnDestroy()
{
	for (int i=0;i<_shape.size();i++)
		SAFE_DESTROY(_shape[i]);
	_shape.clear();
	_circles.clear();

	_infosBug.clear();

	_residable.Clear();
}


BOOL CLoHole::OnActivate()
{
	CSysRandom rand;


	return TRUE;
}

LevelObjCircle *CLoHole::GetShapeCircles(DWORD &count)
{
	count=_circles.size();
	return _circles.data();
}


void CLoHole::_OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
}


void CLoHole::Update()
{
	int nTotalBugs=0;
	for (int i=0;i<_infosBug.size();i++)
		nTotalBugs+=_infosBug[i].count;

	if (nTotalBugs<=0)
		return;

	float dist=10.0f;

	CLevel *level=GetLevel();

	LevelPos posMe=GetFramePos();
	level->EnumLo(posMe,dist);
	DWORD count;
	CLevelObj **objs=level->GetEnumLo(count);

	CSysRandom rnd;
	LosHole *los=(LosHole*)_src;
	LopHole *lop=(LopHole*)_param;


	CRandom_Gauss rand;

	for (int i=0;i<count;i++)
	{
		CLevelObj *lo=objs[i];
		if (lo->IsPlayer())
		{
			CLevelPlayer *player=LevelUtil_PlayerFromLo(lo);

			LevelEoqPower powerEoq=LevelUtil_CalcEoqPower(lo);
			BOOL bEoq=LevelEoqPower_IsValid(powerEoq);

			HoleSpawnCategory *cat=NULL;

			LevelGrade grd=0;

			if (TRUE)
			{
				int iCategory=10000;
				if (TRUE)
				{
					int idx=CSysRandom::RandRangeInt(0,nTotalBugs);
					for (int i=0;i<_infosBug.size();i++)
					{
						if (idx<_infosBug[i].count)
						{
							iCategory=i;
							_infosBug[i].count--;
							grd=CSysRandom::RandVaryUInt(_infosBug[i].grdBase,_infosBug[i].grdVary);
							break;
						}
						idx-=_infosBug[i].count;
					}
				}
				assert(iCategory<los->cats.size());
				cat=&los->cats[iCategory];
			}

			if (cat)
			{
				if (cat->sites.size()>0)
				{
					int idx=CSysRandom::RandRangeInt<int>(0,cat->sites.size());
					i_math::matrix43f mat=cat->sites[idx];
					mat*=los->GetMat();

					LevelRecordUnit *recUnit=NULL;
					if (recUnit=level->GetRecords()->GetUnit(cat->idUnit))
					{
						BOOL bNeedCreate=FALSE;
						BOOL bRetinue=FALSE;
						LevelPlayerID idPlayer=LevelPlayerID_Wild;
						if (bEoq)
						{
							if (recUnit->demandEoq.bAllowEoqControl)
							{
								LevelEoqPower demand=(LevelEoqPower )rand.rand(recUnit->demandEoq.mean,recUnit->demandEoq.deviation);
								if (powerEoq>demand)
								{
									bRetinue=TRUE;
									bNeedCreate=TRUE;
								}
							}
						}
						else
							bNeedCreate=TRUE;

						if (bNeedCreate)
						{
							CLoUnit* lo=(CLoUnit*)level->CreateObj(Class_Ptr2(CLoUnit));

							LevelPos pos;
							pos.x=mat.getTranslationP()->x;
							pos.y=mat.getTranslationP()->z;


							lo->PostCreate(idPlayer,NULL,cat->idUnit,grd+_grdRef,NULL,EquipSetPick_None,pos);

							level->AddToActives(lo);

							BuffArg_Birth param;
							if (TRUE)
							{
								i_math::xformf xfm;
								xfm.fromMatrix(mat);
								i_math::vector3df euler;
								xfm.rot.toEuler(euler);
								param.eulerX=euler.x;
							}
							level->GetDecider()->MakeBuff(lo,cat->idBirthBuff,
								ANIMTICK_FROM_SECOND(1.0f),&param,FALSE);//1.0为随便填写的值,这个Buff持续的时间应根据record里决定

							if (bRetinue)
							{
								if (player)
								{
									if (player->GetRtnus())
										player->GetRtnus()->Add_New(lo,TRUE);
								}
							}

							SAFE_RELEASE(lo);
						}
					}

				}
			}
			break;
		}
	}
	

}
