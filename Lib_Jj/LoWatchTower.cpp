
#include "stdh.h"

#include "Level.h"

#include "LoUnit.h"

#include "LoWatchTower.h"
#include "Buff_ResideHole.h"

#include "Random/Random.h"

#include "LevelRecords.h"
#include "LevelRecordAgent.h"

#include "LevelEvents.h"


////////////////////////////////////////////////////////////////////////
//LosGoblinWatchTower


////////////////////////////////////////////////////////////////////////
//LopWatchTower


////////////////////////////////////////////////////////////////////////
//CLoGoblinWatchTower

BOOL FindNearestMat(i_math::matrix43f *mats,DWORD nMats,i_math::matrix43f &matBase,LevelPos3D &pos,i_math::matrix43f &matNearest)
{

	i_math::matrix43f matInv=matBase;
	matInv.makeInverse();
	i_math::vector3df pos2;
	matInv.transformVect(pos,pos2);//转到局部空间里比较

	float dist2Min=1000000.0f;
	int iFound=-1;
	for (int i=0;i<nMats;i++)
	{
		float dist2=pos2.getDistanceFromSQ(*mats[i].getTranslationP());
		if (dist2<dist2Min)
		{
			iFound=i;
			dist2Min=dist2;
		}
	}
	if (iFound==-1)
		return FALSE;

	matNearest=mats[iFound];
	matNearest=matNearest*matBase;

	return TRUE;

}

BOOL FindNearestPos(i_math::matrix43f *mats,DWORD nMats,i_math::matrix43f &matBase,LevelPos &pos,LevelPos &posNearest)
{
	i_math::matrix43f matNearest;
	if (FALSE==FindNearestMat(mats,nMats,matBase,LevelPos3D(pos.x,0.0f,pos.y),matNearest))
		return FALSE;
	posNearest.set(matNearest.getTranslationP()->x,matNearest.getTranslationP()->z);
	return TRUE;
}



void BuildShapeCircles(std::vector<LevelObjCircle>&circles,std::vector<i_math::spheref>&shape,i_math::matrix43f *mat)
{
	LevelObjCircle circle;
	i_math::spheref sph;
	circles.reserve(shape.size());
	for (int i=0;i<shape.size();i++)
	{
		sph=shape[i];
		mat->transformSphere(sph,sph);

		circle.center.set(sph.center.x,sph.center.z);
		circle.radius=sph.radius;
		circles.push_back(circle);
	}
}

void BuildShapeUnits(std::vector<CUnit *>&units,CUnitMgrNavMesh *unitmgr,std::vector<i_math::spheref>&shape,i_math::matrix43f *mat,void *owner)
{
	i_math::spheref sph;
	units.reserve(shape.size());
	for (int i=0;i<shape.size();i++)
	{
		sph=shape[i];
		mat->transformSphere(sph,sph);

		CUnit *unit=unitmgr->CreateUnit(i_math::vector2df(sph.center.x,sph.center.z),0.0f,sph.radius,0.0f,NULL);
		//UnitCollide
		if (TRUE)
		{
			UnitCollide collide=UnitCollide_Empty;
			extern void UnitCollide_SetStatic(UnitCollide &collide,BOOL bStatic);
			UnitCollide_SetStatic(collide,TRUE);
			unit->SetCollide(collide);
		}
		unit->SetData(owner);

		units.push_back(unit);
	}
}




void CLoWatchTower::_CreateInitialUnit()
{
	LopWatchTower *lop=GetLop<LopWatchTower>();
	if (!lop)
		return;

	LosWatchTower *los=GetLos<LosWatchTower>();
	if (!los)
		return;

	if (!CSysRandom::Roll(lop->rateUnit))
		return;

	CLevel *level=GetLevel();
	if (level->GetRecords()->GetUnit(los->idUnit)&&level->GetRecords()->GetBuff(los->idResideBuff))
	{
		CLoUnit* lo=(CLoUnit*)level->CreateObj(Class_Ptr2(CLoUnit));

		LevelPos pos;
		i_math::matrix43f mat;
		if (los->sitesStand.size()>0)
			mat=los->sitesStand[0];
		mat*=los->GetMat();

		pos.x=mat.getTranslationP()->x;
		pos.y=mat.getTranslationP()->z;

		LevelGrade grd=(LevelGrade)CSysRandom::RandVaryUInt(lop->grdBase,lop->grdVary);
		lo->PostCreate(LevelPlayerID_Wild,NULL,los->idUnit,grd,NULL,EquipSetPick_None,pos);

		BuffArg_Reside param;
		param.idTarget=GetID();
		level->GetDecider()->MakeBuff(lo,los->idResideBuff,ANIMTICK_INFINITE,&param,FALSE);

		level->AddToActives(lo);

		SAFE_RELEASE(lo);



	}

	
}


void CLoWatchTower::PostCreate()
{
	CLoAgent::PostCreate();

	LosWatchTower *src=(LosWatchTower*)_src;
	LopWatchTower *lop=(LopWatchTower*)_param;

	LevelRecordAgent *rec=GetLos()->GetRecord();
	if (rec)
	{
		_attrBase.Init(rec);
		_attrResists=rec->GetResists();
		_attrEvade=rec->GetEvade();
	}

	//根据shape创建Static 的Units
	CUnitMgrNavMesh *unitmgr=GetLevel()->GetUnitMgr();
	i_math::matrix43f *mat=&_src->GetMat();
	extern void LevelUtil_BuildShapeUnits(std::vector<CUnit *>&units,CUnitMgrNavMesh *unitmgr,std::vector<i_math::spheref>&shape,i_math::matrix43f *mat,CLevelObj*owner);
	LevelUtil_BuildShapeUnits(_shape,unitmgr,src->shape,mat,this);

	BuildShapeCircles(_circles,src->shape,&src->GetMat());

	//residable
	if (TRUE)
	{
		i_math::vector3df pos;
		if (src->sitesStand.size()>0)
			pos=src->sitesStand[0].getTranslation();
		_residable.Init(GetLos()->GetMat(),pos);
	}

	_buffs.Init(this,_level->GetBuffIDPool());

	_CreateInitialUnit();

	//测试用,杀死自己
// 	if (FALSE)
// 	{
// 		LeKill e;
// 		e.loTarget=this;
// 		e.osbSrc=&LevelOSB(this);
// 		e.link.id=LevelOpLinkID_Invalid;
// 		OnEvent(e);
// 	}



}

void CLoWatchTower::OnDestroy()
{
	_buffs.Clear();

	for (int i=0;i<_shape.size();i++)
		SAFE_DESTROY(_shape[i]);
	_shape.clear();
	_circles.clear();

	_residable.Clear();
}


BOOL CLoWatchTower::OnActivate()
{
	CSysRandom rand;


	return TRUE;
}

LevelObjCircle *CLoWatchTower::GetShapeCircles(DWORD &count)
{
	count=_circles.size();
	return _circles.data();
}


void CLoWatchTower::_OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	bContent=TRUE;
	_attrBase.WriteFirstSync(bp,FALSE);
	_buffs.WriteFirstSync(bp);
}


void CLoWatchTower::Update()
{
	_buffs.Update(GetT());
}

BOOL CLoWatchTower::FindEntry(LevelPos &pos,LevelPos &posEntry)
{
	LosWatchTower *los=GetLos<LosWatchTower>();
	if (!los)
		return FALSE;

	return FindNearestPos(&los->sitesEntry[0],los->sitesEntry.size(),los->GetMat(),pos,posEntry);
}


void CLoWatchTower::OnEvent(LevelEvent &e0)
{
	if (e0.GetType()==LET_Kill)
	{
		LeKill &e=(LeKill &)e0;
		if (e.loTarget)
		{
			if (e.loTarget->GetID()==GetID())
			{//自己被杀
				_residable.Enable(FALSE);
				LevelObjID idObj=_residable.GetOccupingObj();
				if (idObj!=LevelObjID_Invalid)
				{
					CLevelObj *lo=GetLevel()->GetIDs()->LoFromID(idObj);
					if (lo)
						lo->HandleEvent(e);
				}
			}
		}
	}
}



