
#include "stdh.h"

#include "commondefines/general_stl.h"

#include "CentipedeRope.h"

#include "unitmgr/UnitMgrNavMesh.h"

//////////////////////////////////////////////////////////////////////////
//CCentipedeRope::Twist
void CCentipedeRope::Twist::Init(Mass* mass1, Mass* mass2,CentipedeRopeProp *prop)
{
	this->prop= prop;               // Set The stiffness
	this->mass1 = mass1;                     // Set mass1
	this->mass2 = mass2;                     // Set mass2
}

//////////////////////////////////////////////////////////////////////////
//CCentipedeRope
void CCentipedeRope::Init(int numOfMasses,CentipedeRopeProp &prop,CUnitMgrNavMesh *unitmgr)
{
	CRope::Init(numOfMasses,(RopeProp &)prop,i_math::vector3df(0,0,0));

	for (int i=1;i<numOfMasses;i++)
		_masses[i]->bStatic=0;

	_twists.resize(numOfMasses - 1);
	for (int a = 0; a < numOfMasses - 1; ++a)              
	{
		_twists[a].Init(_masses[a], _masses[a + 1],&prop);
// 		_twists[a].yaw=-10.0f*i_math::GRAD_PI2;
// 		_twists[a].yaw=-1.0f*i_math::GRAD_PI2*(float)a;
	}

	_units.resize(numOfMasses);
	VEC_SET(_units,0);

	_faceRoot=0.0f;

	_unitmgr=unitmgr;
	_prop=&prop;
}

void CCentipedeRope::SetTwists(LevelFaceYaw *yaws)
{
	for (int a = 0; a < _twists.size(); ++a)
		_twists[a].yaw=yaws[a];
}


void CCentipedeRope::Clear()
{
	CRope::Clear();

	_twists.clear();
}

void CCentipedeRope::SetUnit(int idx,CUnit *unit)
{
	if (((DWORD)idx)<_units.size())
		_units[idx]=unit;
}

void CCentipedeRope::Break(int idx)
{
	if (idx<=0)
		return;
	int idxSpring=idx-1;
	if(idxSpring<_springs.size())
		_springs[idxSpring]->bBroken=TRUE;
	_masses[idx]->bStatic=1;
}


BOOL CCentipedeRope::_CalcSegRepulsion(CUnit *unitMe,CUnit *unitNb,CUnit *unit,i_math::vector3df &force)
{
	i_math::vector2df posProj;
	i_math::line2df line;
	line.start=unitMe->GetPos();

	if (unitNb)
	{
		line.end=unitNb->GetPos();

		float r;
		line.getProjection(unit->GetPos(),r);
		if ((r<=1.0f)&&(r>=0.0f))
		{
			posProj=line.start+(line.end-line.start)*r;
			force.setXZ(posProj-unit->GetPos());

			float dist=force.getLength();
			if (dist>0.001f)
			{
				force/=dist;
				dist=unitMe->GetRadius()+unit->GetRadius()-dist;
				if (dist<0.0f)
					dist=0.0f;
				force*=dist;
				force*=_prop->repulsionObstacle;//*(float)i/(float)_masses.size();
				return TRUE;
			}
		}
	}
	return FALSE;
}

BOOL CCentipedeRope::_IsRopeUnit(CUnit *unit)
{
	CLevelObj *lo=(CLevelObj *)unit->GetData();
	if (!lo)
		return FALSE;

	if (lo->GetType()!=LevelObjType_Unit)
		return FALSE;

	extern CLevelBuff *LevelUtil_FindBuffByRecordID(CLevelObj *lo,RecordID idBuff);
	if (!LevelUtil_FindBuffByRecordID(lo,_prop->idRopeUnitBuff))
		return FALSE;

	return TRUE;
}

void CCentipedeRope::Solve(float dt)
{
	for (int a = 0; a < _masses.size(); ++a)
	{
		_masses[a]->force.setZero();
	}

	for (int a = 0; a < _springs.size(); ++a)
		_springs[a]->Solve(dt);

	for (int a = 0; a < _masses.size(); ++a)                // Start A Loop To Apply Forces Which Are Common For All Masses
	{
		// The air friction
		_masses[a]->ApplyForce(-_masses[a]->vel * _frictionAir);
	}

	//Twist
	if (TRUE)
	{
		LevelFace faceBase=_faceRoot;
		for (int i=0;i<_twists.size();i++)
		{
			Twist &twist=_twists[i];

			if (!(twist.mass1&&twist.mass2))
				continue;

			LevelFace faceTarget=faceBase;
			LevelFaceApplyYaw(faceTarget,twist.yaw);

			LevelFace faceCur=LevelFaceFromDir_XZ(twist.mass2->pos.convert<float>()-twist.mass1->pos.convert<float>());
			faceBase=faceCur;

			LevelPos dirTwist;
			if (TRUE)
			{
				LevelFace faceMove=faceCur;
				LevelFaceApplyYaw(faceMove,i_math::Pi/2.0f);
				dirTwist=LevelFaceToDir(faceMove);
			}

			LevelFaceYaw yaw=LevelFaceCalcYaw(faceTarget,faceCur);

			i_math::vector3df force;
			force.setXZ(dirTwist*(-yaw)*twist.prop->stiffnessTwist);

			twist.mass2->ApplyForce(force.convert<double>());
			twist.mass1->ApplyForce(-force.convert<double>()*0.9);
		}
	}

	//边界
// 	if (FALSE)
// 	{
// 		for (int i = 0; i < _masses.size(); i++)
// 		{
// 			Mass *mass=_masses[i];
// 			if (mass->bStatic)
// 				continue;
// 
// 			if (!_unitmgr->IsWalkable(UnitFindPath_Walkable,mass->pos.getXZ()))
// 			{
// 				LevelPos pos=mass->pos.getXZ();
// 				_unitmgr->ToClosestWalkable(UnitFindPath_Walkable,pos);
// 
// 				i_math::vector3df force;
// 				force.setXZ(pos-mass->pos.getXZ());
// 
// 				force*=_prop->repulsionObstacle;
// 
// 				mass->ApplyForce(force);
// 			}
// 		}
// 	}

	//Unit碰撞
	if (TRUE)
	{
		CUnitMap *mp=_unitmgr->GetMap();
		for (int i = 0; i < _masses.size(); i++)
		{
			Mass *mass=_masses[i];
			if (mass->bStatic)
				continue;

			CUnit *unitMe=_units[i];
			if (!unitMe)
				continue;

			CUnit *unitPrev=NULL,*unitNext=NULL;
			if (i>0)
				unitPrev=_units[i-1];
			if (i<_masses.size()-1)
				unitNext=_units[i+1];

			mp->Enum(unitMe,unitMe->GetRadius());

			DWORD c;
			CUnitBase **units=(CUnitBase **)mp->GetEnums(c);
			for (int j=0;j<c;j++)
			{
				CUnit *unit=(CUnit *)units[j];
				if ((unit==unitMe)||(unit==unitPrev)||(unit==unitNext))
					continue;

				BOOL bRopeUnit=_IsRopeUnit(unit);
				
				float dist2=unit->GetPos().getDistanceSQFrom(unitMe->GetPos());
				float sumRadius=unit->GetRadius()+unitMe->GetRadius();
				if (dist2<sumRadius*sumRadius)
				{
					i_math::vector3df force;

//					if (!bRopeUnit)
					{
						if (_CalcSegRepulsion(unitMe,unitPrev,unit,force))
						{
							mass->ApplyForce(force.convert<double>());
							continue;
						}

						if (_CalcSegRepulsion(unitMe,unitNext,unit,force))
						{
							mass->ApplyForce(force.convert<double>());
							continue;
						}
					}

					float dist=sqrtf(dist2);

					force.setXZ(unitMe->GetPos()-unit->GetPos());
					force.setLength(sumRadius-dist);

					force*=_prop->repulsionObstacle;//*(float)i/(float)_masses.size();
					mass->ApplyForce(force.convert<double>());
				}
			}
		}
	}
}

void CCentipedeRope::Simulate(float dt)
{
	for (int a = 0; a < _masses.size(); ++a)
	{
		Mass *mass=_masses[a];
		if (mass->bStatic)
			continue;

		i_math::vector3dd posOld=mass->pos;
		mass->Simulate(dt); 

		if (!_unitmgr->IsWalkable(UnitFindPath_Walkable,mass->pos.convert<float>().getXZ()))
		{
			LevelPos pos=mass->pos.convert<float>().getXZ();
			_unitmgr->ToClosestWalkable(UnitFindPath_Walkable,pos);
			mass->pos.x=(double)pos.x;
			mass->pos.z=(double)pos.y;
		}

		CUnitMap *mp=_unitmgr->GetMap();
		CUnit *unitMe=_units[a];

		if (FALSE)
		if (unitMe)
		{
			CUnit *unitPrev=NULL,*unitNext=NULL;
			if (a>0)
				unitPrev=_units[a-1];
			if (a<_masses.size()-1)
				unitNext=_units[a+1];

			mp->Enum(mass->pos.convert<float>().getXZ(),unitMe->GetRadius());

			BOOL bStuck=FALSE;

			DWORD c;
			CUnitBase **units=(CUnitBase **)mp->GetEnums(c);
			for (int j=0;j<c;j++)
			{
				CUnit *unit=(CUnit *)units[j];
				if ((unit==unitMe)||(unit==unitPrev)||(unit==unitNext))
					continue;

				float dist2=unit->GetPos().getDistanceSQFrom(mass->pos.convert<float>().getXZ());
				float dist2Old=unit->GetPos().getDistanceSQFrom(posOld.convert<float>().getXZ());
				float sumRadius=unit->GetRadius()+unitMe->GetRadius();
				if (dist2<sumRadius*sumRadius)
				{
					if (dist2<dist2Old)
					{
						bStuck=TRUE;
						break;
					}
				}
			}

			if (bStuck)
				mass->pos=posOld;
		}

		if (unitMe)
		{
			unitMe->_pos=mass->pos.convert<float>().getXZ();
			mp->UpdateUnit(unitMe);
		}
	}

}
