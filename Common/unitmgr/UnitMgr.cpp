
#include "stdh.h"
#include "UnitMgr.h"

#include "math/range.h"

#include "Log/LogFile.h"
#include "timer/profiler.h"

#include "rvo2/RvoSimulator.h"
#include "rvo2/RvoUnit.h"

#include "Unit3DMgr.h"

#include <set>
#include <map>

#define VERIFY_ALIVE(p)	if (p) {if (!(p)->IsAlive())	SAFE_RELEASE(p);}


LogFile g_logUnitMgr("UnitMgr");

//////////////////////////////////////////////////////////////////////////
//UnitCollide

struct UnitCollideInfo
{
	DWORD ally:8;
	DWORD bPlayer:1;
	DWORD layor:8;
	DWORD bStatic:1;//静态场景
	DWORD bGhost:1;
};

//目前同一个Ally中,Player不会碰撞非Player的Unit
void UnitCollide_SetAlly(UnitCollide &collide,DWORD ally)
{
	UnitCollideInfo *info=(UnitCollideInfo *)&collide;
	info->ally=ally;
}

void UnitCollide_SetAlly(CUnit *unit,DWORD ally)
{
	UnitCollide collide=unit->GetCollide();
	UnitCollide_SetAlly(collide,ally);
	unit->SetCollide(collide);
}


void UnitCollide_SetPlayer(UnitCollide &collide,BOOL bPlayer)
{
	UnitCollideInfo *info=(UnitCollideInfo *)&collide;
	info->bPlayer=bPlayer?1:0;
}

void UnitCollide_SetPlayer(CUnit *unit,BOOL bPlayer)
{
	UnitCollide collide=unit->GetCollide();
	UnitCollide_SetPlayer(collide,bPlayer);
	unit->SetCollide(collide);
}

void UnitCollide_SetGhost(UnitCollide &collide,BOOL bGhost)
{
	UnitCollideInfo *info=(UnitCollideInfo *)&collide;
	info->bGhost=bGhost?1:0;
}

void UnitCollide_SetGhost(CUnit *unit,BOOL bGhost)
{
	UnitCollide collide=unit->GetCollide();
	UnitCollide_SetGhost(collide,bGhost);
	unit->SetCollide(collide);
}


//目前同一个Ally中,不同的layor之间不会相互collide
void UnitCollide_SetLayor(UnitCollide &collide,DWORD layor)
{
	UnitCollideInfo *info=(UnitCollideInfo *)&collide;
	info->layor=layor;
}

void UnitCollide_SetLayor(CUnit *unit,DWORD layor)
{
	UnitCollide collide=unit->GetCollide();
	UnitCollide_SetLayor(collide,layor);
	unit->SetCollide(collide);
}



void UnitCollide_SetStatic(UnitCollide &collide,BOOL bStatic)
{
	UnitCollideInfo *info=(UnitCollideInfo *)&collide;
	info->bStatic=bStatic?1:0;
}


BOOL UnitCollide_Check(UnitCollide src,UnitCollide target)
{
	UnitCollideInfo *infoSrc=(UnitCollideInfo *)&src;
	UnitCollideInfo *infoTarget=(UnitCollideInfo *)&target;

	if (infoSrc->bStatic)
		return FALSE;

	if (infoTarget->bStatic)
		return TRUE;

	if (infoSrc->bGhost)
		return FALSE;
	if (infoTarget->bGhost)
		return FALSE;

	if (infoSrc->ally==UnitCollide_AllyToAllPlayer)
	{
		if (infoTarget->bPlayer)
			return FALSE;
	}
	if (infoTarget->ally==UnitCollide_AllyToAllPlayer)
	{
		if (infoSrc->bPlayer)
			return FALSE;
	}

	if (infoSrc->ally!=infoTarget->ally)
		return TRUE;

	if (infoSrc->bPlayer)
		return FALSE;

	if ((infoSrc->layor&infoTarget->layor)==0)
		return FALSE;

	return TRUE;
}



//////////////////////////////////////////////////////////////////////////
//CUnitPath
void CUnitPath::Clear()
{
	_nodes.clear();
	_distsToGo.clear();
	Zero();
}

void CUnitPath::Destroy()
{
	Clear();
	Release();
}

void CUnitPath::BuildDistsToGo()
{
	if (!_distsToGo.empty())
		return;

	_distsToGo.resize(_nodes.size());
	for (int i=_nodes.size()-1;i>=0;i--)
	{
		int iNext=i+1;
		if (iNext>=_nodes.size())
			iNext=_nodes.size()-1;

		_distsToGo[i]=_nodes[i].getDistanceFrom(_nodes[iNext]);
		if (iNext>i)
			_distsToGo[i]+=_distsToGo[iNext];
	}
}

float CUnitPath::GetDistToGo(i_math::vector2df &pos,int iPathNode)
{
	if (iPathNode>=_nodes.size())
		return -1.0f;

	BuildDistsToGo();
	return pos.getDistanceFrom(_nodes[iPathNode])+_distsToGo[iPathNode];
}



void DestroyPathLink(CUnitPath *pathes)
{
	CUnitPath *p=pathes;
	while(p)
	{
		CUnitPath *q=p;
		p=p->_next;
		SAFE_DESTROY(q);
	}
}


//////////////////////////////////////////////////////////////////////////
//CUnitTargetPos
void CUnitTargetPos::Clear()
{
	DestroyPathLink(_toMe);
}

//////////////////////////////////////////////////////////////////////////
//CUnitTarget
void CUnitTarget::VerifyAlive()
{
	VERIFY_ALIVE(_unit);
	VERIFY_ALIVE(_unit3D);
}

void CUnitTarget::Clear()
{
	SAFE_RELEASE(_unit);
	SAFE_RELEASE(_unit3D);
	Safe_Class_Delete(_pos);
	Zero();
}



BOOL CUnitTarget::CheckInRange(CUnit *unitSrc,BOOL bDisableClosestFollow)
{
	if (_bCur)
		return TRUE;

	if (_unit)
	{
		float dist2=(float)(unitSrc->GetPos()-_unit->GetPos()).getLengthSQ();
		float rSum;
		if ((!_bClosestFollow)||(bDisableClosestFollow))
			rSum=unitSrc->_radius+_unit->_radius+_rangeFollow;
		else
			rSum=unitSrc->_radius+_unit->_radius;
		if (rSum*rSum+0.01f>dist2)
			return TRUE;
	}
	if (_unit3D)
	{
		i_math::vector2df posTarget(_unit3D->GetPos().x,_unit3D->GetPos().z);
		float dist2=(float)(unitSrc->GetPos()-posTarget).getLengthSQ();
		float rSum=unitSrc->_radius+0.0f+_rangeFollow;
		if (rSum*rSum+0.01f>dist2)
			return TRUE;
	}

	if (_pos)
	{
		float dist2=(float)(unitSrc->GetPos()-_pos->_pos).getLengthSQ();
		if ((!_bClosestFollow)||(bDisableClosestFollow))
		{
			if (dist2<_rangeFollow*_rangeFollow+0.01f)
				return TRUE;
		}
		else
		{
			if (dist2<0.01f)
				return TRUE;
		}
	}

	return FALSE;
}

BOOL CUnitTarget::CheckInFaceRad(CUnit *unitSrc)
{
	if (_bCur)
		return TRUE;

	i_math::vector2df pos;
	if (!GetPos(pos))
		return FALSE;

	if (unitSrc->GetPos().getDistanceSQFrom(pos)<0.0001f)
		return TRUE;//太近了

	i_math::vector2df dir=pos-unitSrc->GetPos();
	float gap=i_math::get_radian_dist(unitSrc->_face,atan2f(dir.y,dir.x));
	if (gap<=_radFace)
		return TRUE;
	return FALSE;
}

BOOL CUnitTarget::CheckInFaceRange(CUnit *unitSrc)
{
	if (_bCur)
		return TRUE;

	if (_unit)
	{
		float dist2=(float)(unitSrc->GetPos()-_unit->GetPos()).getLengthSQ();
		float rSum=unitSrc->_radius+_unit->_radius+_rangeFace;
		if (rSum*rSum+0.01f>dist2)
			return TRUE;
	}
	if (_unit3D)
	{
		i_math::vector2df posTarget(_unit3D->GetPos().x,_unit3D->GetPos().z);
		float dist2=(float)(unitSrc->GetPos()-posTarget).getLengthSQ();
		float rSum=unitSrc->_radius+0.0f+_rangeFace;
		if (rSum*rSum+0.01f>dist2)
			return TRUE;
	}

	if (_pos)
	{
		float dist2=(float)(unitSrc->GetPos()-_pos->_pos).getLengthSQ();
		if (dist2<_rangeFace*_rangeFace+0.01f)
			return TRUE;
	}

	return FALSE;
}


BOOL CUnitTarget::GetPos(i_math::vector2df &pos)
{
	if (_unit)
	{
		pos=_unit->GetPos();
		return TRUE;
	}
	if (_unit3D)
	{
		pos.set(_unit3D->GetPos().x,_unit3D->GetPos().z);
		return TRUE;
	}
	if (_pos)
	{
		pos=_pos->_pos;
		return TRUE;
	}

	return FALSE;
}



//////////////////////////////////////////////////////////////////////////
//CUnit


void CUnit::_Clear()
{
	if (_bAlive)
	{
		_mgr->GetMap()->RemoveUnit(this);
		Safe_Class_Delete(_target);
		Safe_Class_Delete(_targetPending);
		Safe_Class_Delete(_rot);


		_infoGesture.Clear();

		_ClearPath();
		DestroyPathLink(_toMe);
		Zero();
	}
}

void CUnit::_Destroy()
{
	_Clear();
	Release();
}


void CUnit::Destroy()
{
	if (_mirror)
	{
		RvoSimulator *sim=_mgr->GetMirror();
		if (sim)
			sim->removeUnit(_mirror);
		_mirror=NULL;
	}
	_Destroy();
}


void CUnit::_ClearPath()
{
	if (_path)
	{
		if (!_path->_bDirectPath)
		{
			SAFE_RELEASE(_path);
		}
		else
		{
			SAFE_DESTROY(_path);
		}
	}
}

void CUnit::Reset()
{
	Safe_Class_Delete(_target);
	Safe_Class_Delete(_targetPending);
	_ClearPath();
	_SetStage(UnitStage_NotMove);
	_nStucks=0;
	_radLastAvoid=-1.0f;
}


void CUnit::Reset(i_math::vector2df &pos,float face)
{
	if (_pos.getDistanceFrom(pos)>2.0f)
	{
		int v=0;
		v++;
	}
	_pos=pos;
	_face=face;
	assert(!isnan(_face));

	Reset();

	if (_tile)//在一个UnitMap里
		_mgr->UpdateUnitPos(this);
}

UnitSession CUnit::RequestNoTarget()
{
	if (TRUE)
	{
		CUnitTarget *targetToCheck=_target;
		if (_targetPending)
			targetToCheck=_targetPending;
		if (targetToCheck&&targetToCheck->IsCur())
			return _sessionPending;//没有区别
	}
	Safe_Class_Delete(_targetPending);
	_targetPending=Class_New2(CUnitTarget);
	_targetPending->_bCur=TRUE;

	_sessionPending++;
	return _sessionPending;
}

UnitSession CUnit::RequestTarget(CUnit *target,float range,BOOL bClosestFollow,BOOL bNoStopMoveWhenInRange)
{
	if (TRUE)
	{
		CUnitTarget *targetToCheck=_target;
		if (_targetPending)
			targetToCheck=_targetPending;
		if (targetToCheck)
		{
			if ((targetToCheck->_unit==target)&&
				(targetToCheck->_rangeFollow==range)&&
				(targetToCheck->_bClosestFollow==bClosestFollow)&&
				(targetToCheck->_bNoStopMoveWhenInRange==bNoStopMoveWhenInRange))
				return _sessionPending;//没有区别
		}
	}
	Safe_Class_Delete(_targetPending);
	_targetPending=Class_New2(CUnitTarget);

	SAFE_REPLACE(_targetPending->_unit,target);
	_targetPending->_rangeFollow=range;
	_targetPending->_bClosestFollow=bClosestFollow;
	_targetPending->_bNoStopMoveWhenInRange=bNoStopMoveWhenInRange;

	_sessionPending++;
	return _sessionPending;
}

UnitSession CUnit::RequestTarget3D(CUnit3D *target,float range,BOOL bNoStopMoveWhenInRange)
{
	if (TRUE)
	{
		CUnitTarget *targetToCheck=_target;
		if (_targetPending)
			targetToCheck=_targetPending;
		if (targetToCheck)
		{
			if ((targetToCheck->_unit3D==target)&&
				(targetToCheck->_rangeFollow==range)&&
				(targetToCheck->_bNoStopMoveWhenInRange==bNoStopMoveWhenInRange))
				return _sessionPending;//没有区别
		}
	}

	Safe_Class_Delete(_targetPending);
	_targetPending=Class_New2(CUnitTarget);

	SAFE_REPLACE(_targetPending->_unit3D,target);
	_targetPending->_rangeFollow=range;
	_targetPending->_bNoStopMoveWhenInRange=bNoStopMoveWhenInRange;

	_sessionPending++;
	return _sessionPending;
}

UnitSession CUnit::RequestTargetPos(i_math::vector2df &pos,float range,BOOL bClosestFollow,BOOL bNoStopMoveWhenInRange,BOOL bFindPathOnUnwalkable)
{
	if (TRUE)
	{
		CUnitTarget *targetToCheck=_target;
		if (_targetPending)
			targetToCheck=_targetPending;

		if (targetToCheck)
		{
			if ((targetToCheck->_pos)&&
				(targetToCheck->_pos->_pos==pos)&&
				(targetToCheck->_rangeFollow==range)&&
				(targetToCheck->_bClosestFollow==bClosestFollow)&&
				(targetToCheck->_bFindPathOnUnwalkable==bFindPathOnUnwalkable)&&
				(targetToCheck->_bNoStopMoveWhenInRange==bNoStopMoveWhenInRange))
				return _sessionPending;//没有区别
		}
	}

	Safe_Class_Delete(_targetPending);
	_targetPending=Class_New2(CUnitTarget);

	CUnitTargetPos *target=Class_New2(CUnitTargetPos);
	target->_pos=pos;

	_targetPending->_pos=target;
	_targetPending->_rangeFollow=range;
	_targetPending->_bClosestFollow=bClosestFollow;
	_targetPending->_bNoStopMoveWhenInRange=bNoStopMoveWhenInRange;
	_targetPending->_bFindPathOnUnwalkable=bFindPathOnUnwalkable;

	_sessionPending++;
	return _sessionPending;
}

UnitSession CUnit::RequestFacing(float range,float rad)
{
	if (!_targetPending)
		return _sessionPending;

	_targetPending->_bFace=1;
	_targetPending->_rangeFace=range;
	_targetPending->_radFace=rad;

	_sessionPending++;
	return _sessionPending;
}

BOOL CheckPathNeedRecalc(i_math::vector2df &posSrc,i_math::vector2df &posTarget,CUnitPath *path)
{
	float dist2=(float)(posTarget-posSrc).getLengthSQ();
	float dist2Move=(float)(path->_target-posTarget).getLengthSQ();

	if (dist2<=0.0001f)
		dist2=0.0001f;
	if (dist2Move/dist2>0.09f)
		return TRUE;
	return FALSE;
}

static inline BOOL CheckPathNeedRecalc(CUnit *src,CUnitTarget *target,CUnitPath *path)
{
	if (!path)
		return TRUE;
	if (!target)
		return TRUE;
	if (target->IsCur())
		return TRUE;

	i_math::vector2df posTarget;
	if (!target->GetPos(posTarget))
	{
		assert(FALSE);
		return TRUE;
	}

	return CheckPathNeedRecalc(src->GetPos(),posTarget,path);
}

BOOL CUnit::_CheckInRange()
{
	if (_target)
		return _target->CheckInRange(this,FALSE);

	return FALSE;
}

BOOL CUnit::_CheckInRange_NoClosestFollow()
{
	if (_target)
		return _target->CheckInRange(this,TRUE);

	return FALSE;
}

BOOL CUnit::_CheckInFaceRad()
{
	if (_target)
		return _target->CheckInFaceRad(this);
	return FALSE;
}

BOOL CUnit::_CheckInFaceRange()
{
	if (_target)
		return _target->CheckInFaceRange(this);
	return FALSE;
}



float CUnit::GetDistToGo()//返回离终点还有多远的距离
{
// 	if (_stage==UnitStage_NotMove)
// 		return -1.0f;
	if (!_path)
		return -1.0f;

	return _path->GetDistToGo(_pos,_iPathNode);
}

BOOL CUnit::_MakePathToTarget()
{
	_closest=100000.0f;
	_tClosest=0xffffffff;

	if (!_target)
		return FALSE;

	i_math::vector2df posTarget;
	if (!_target->GetPos(posTarget))
		return FALSE;

	float tolerancePathFind=0.0f;
	if (_target->_unit)
		tolerancePathFind=_target->_rangeFollow+_target->_unit->_radius+_radius;
	if (_target->_unit3D)
		tolerancePathFind=_target->_rangeFollow+_radius;
	if (_target->_pos)
		tolerancePathFind=_target->_rangeFollow+_radius;

	BOOL bFindPathOnUnwalkable=_target->_bFindPathOnUnwalkable;

	if (bFindPathOnUnwalkable||(TRUE==_mgr->StaticObstacleTest(_tpPathFind,_pos,posTarget)))
	{
		//有阻挡,我们必须搜索一条路径
		CUnitPath **toMe=NULL;
		if (_target->_unit)
			toMe=&_target->_unit->_toMe;
		if (_target->_unit3D)
			toMe=&_target->_unit3D->_toMe;
		if (_target->_pos)
			toMe=&_target->_pos->_toMe;

		//在已有的路径中找到一条可用的路径
		if (!bFindPathOnUnwalkable)
		{
			i_math::vector2df v1,v2;
			CUnitPath *p=*toMe;
			while(p)
			{
				DWORD sz=p->_nodes.size();

				//判断目标点和路径的终点是否差距过大
				if (_target)
				{
					if(CheckPathNeedRecalc(this,_target,p))
					{
						p=p->_next;
						continue;
					}
				}

				//起始第一段的向量
				v1=p->_nodes[0]-p->_nodes[1];
				v1.normalize();

				v2=_pos-p->_nodes[1];
				v2.normalize();

				if (v1.dotProduct(v2)>=0.866)//在+/- 30度之间
				{
					if (!_mgr->StaticObstacleTest(_tpPathFind,_pos,p->_nodes[1]))
					{
						_path=p;
						_path->AddRef();
						_bEscape=0;
						break;
					}
				}

				p=p->_next;
			}
		}

		if (!_path)
		{
			CUnitPath *path=Class_New2(CUnitPath);
			path->_bAlive=1;

			if (!bFindPathOnUnwalkable)
			{
				BOOL bEscape;
				if (FALSE==_mgr->FindPath(_tpPathFind,_pos,posTarget,tolerancePathFind,path->_nodes,bEscape))
				{
					_mgr->AddFailPath(_pos,posTarget);
					Class_Delete(path);
					return FALSE;
				}
				else
				{
					path->_target=posTarget;//记录下目的点

					//连接到toMe上
					path->_next=(*toMe);
					(*toMe)=path;
					path->AddRef();

					//将这条路径作为当前路径
					_path=path;
					_path->AddRef();
					_bEscape=bEscape?1:0;
				}
			}
			else
			{
				if (FALSE==_mgr->FindPathOnUnwalkable(_tpPathFind,_pos,posTarget,path->_nodes))
				{
					_mgr->AddFailPath(_pos,posTarget);
					Class_Delete(path);
					return FALSE;
				}
				else
				{
					path->_target=posTarget;//记录下目的点
					path->_bOnUnwalkable=1;

					//将这条路径作为当前路径
					_path=path;
					_path->AddRef();
					_bEscape=0;
				}
			}
		}
	}
	else
	{
		//没有阻挡,我们新建一个直接到达的路径
		CUnitPath *path=Class_New2(CUnitPath);
		path->_bAlive=1;
		path->_bDirectPath=1;
		path->_nodes.push_back(_pos);
		path->_nodes.push_back(posTarget);
		path->_target=posTarget;//记录下目的点

		_path=path;
		_path->AddRef();
		_bEscape=0;
	}

	return TRUE;

}

BOOL CUnit::_SupportStartStage()
{
	if (_pace)
	{
		if (_pace->bSupportStart)
		{
			assert((_pace->startFw.GetDur()>0)&&(_pace->startRot.GetDur()>0));
			if ((_pace->startFw.GetDur()>0)&&(_pace->startRot.GetDur()>0))
				return TRUE;
		}
	}
	return FALSE;
}

BOOL CUnit::_SupportStopStage()
{
	if (_pace)
	{
		if (_pace->bSupportStop)
		{
			assert(_pace->stop.GetDur()>0);
			if (_pace->stop.GetDur()>0)
				return TRUE;
		}
	}
	return FALSE;
}


void CUnit::_StartMove()
{
	if (!_path)
		return;
	if ((_stage==UnitStage_Abort)||(!_SupportStartStage()))
		_SetStage(UnitStage_Move);
	else
	{
		float from,to;
		from=_face;

		i_math::vector2df &v2=_path->_nodes[1];
		i_math::vector2df dir=v2-_pos;
		to=atan2f(dir.y,dir.x);
		assert(!isnan(to));

		float gap=i_math::get_radian_dist(from,to);

		assert(ANIMTICK_TO_SECOND(_pace->startFw.GetEndTick())>0.0f);
		assert(ANIMTICK_TO_SECOND(_pace->startRot.GetEndTick())>0.0f);
		if (gap>_pace->angleStartRot*i_math::GRAD_PI2)
		{
			_SetStage(UnitStage_StartRot);
			_rot->speed=gap/(ANIMTICK_TO_SECOND(_pace->startRot.GetEndTick()));
			assert(!isnan(_rot->speed));
		}
		else
		{
			_SetStage(UnitStage_StartFw);
			_rot->speed=gap/(ANIMTICK_TO_SECOND(_pace->startFw.GetEndTick()));
			assert(!isnan(_rot->speed));
		}

		_rot->from=from;
		_rot->to=to;
	}

	_tInStage=0.0f;
	_closest=100000.0f;
	_tClosest=0xffffffff;

	_nStucks=0;
	_iPathNode=1;//往第1个路点前进
}

BOOL CUnit::IsStopFinished()
{
	if (_stage==UnitStage_Stop)
	{
		assert(_pace);
		return (_tInStage>=ANIMTICK_TO_SECOND(_pace->stop.GetEndTick()));
	}

	return FALSE;
}

BOOL CUnit::IsStartFinished()
{
	if (_stage==UnitStage_StartFw)
	{
		assert(_pace);
		return (_tInStage+_mgr->GetDt()/2.0f>=ANIMTICK_TO_SECOND(_pace->startFw.GetEndTick()));
	}
	if (_stage==UnitStage_StartRot)
	{
		assert(_pace);
		return (_tInStage+_mgr->GetDt()/2.0f>=ANIMTICK_TO_SECOND(_pace->startRot.GetEndTick()));
	}
	return FALSE;
}


BOOL CUnit::IsClosestFollow()
{
	if (_target)
		return _target->_bClosestFollow;
	return FALSE;
}



void CUnit::_StopMove()
{
	if (!_SupportStopStage())
		FinalizeReach();
	else
	{
		BOOL bNeedStop=TRUE;
		if (_target)
		{
			BOOL bInRange=_CheckInRange_NoClosestFollow();
			if (bInRange)
			{
				if (_target->_bNoStopMoveWhenInRange)
						bNeedStop=FALSE;
			}
		}
	
		if (bNeedStop)
			_SetStage(UnitStage_Stop);
		else
			FinalizeReach();
	}
	_tInStage=0.0f;

	_nStucks=0;
}

void CUnit::FinalizeReach()
{
	if (_CheckInRange_NoClosestFollow())
	{
		if (_target)
		{
			if (_target->_bFace)
			{
				if (_target->CheckInFaceRad(this))
				{
					if (_target->CheckInFaceRange(this))
						_SetStage(UnitStage_Faced);
					else
					{
						_SetStage(UnitStage_Abort);
						_reasonAbort=UnitAbortReason_OutOfFacingRange;
					}
				}
				else
				{
					i_math::vector2df posTarget;
					if (_target->GetPos(posTarget))
					{
						_SetStage(UnitStage_RotateOnSpot);
						_rot->from=_face;
						i_math::vector2df dir=posTarget-_pos;
						_rot->to=atan2f(dir.y,dir.x);
						assert(!isnan(_rot->to));

						_rot->speed=100000000.0f;
						if (_pace)
						{
							float gap=i_math::get_radian_dist(_rot->from,_rot->to);
							if (_pace->durROS>0)
								_rot->speed=gap/_pace->durROS;
						}
						assert(!isnan(_rot->speed));
					}
					else
					{
						_SetStage(UnitStage_Abort);
						_reasonAbort=UnitAbortReason_LoseTarget;
					}
				}
			}
			else
				_SetStage(UnitStage_Reached);
		}
		else
			_SetStage(UnitStage_Reached);
	}
	else
	{
		_SetStage(UnitStage_Abort);
		_reasonAbort=UnitAbortReason_CannotReach;
	}

	_nStucks=0;

}




void CUnit::_SetStage(UnitStage stage)
{
	if (_stage==stage)
		return;
	if ((stage!=UnitStage_StartFw)&&(stage!=UnitStage_StartRot)&&(stage!=UnitStage_RotateOnSpot))
	{
		Safe_Class_Delete(_rot);
	}
	else
	{
		_rot=Class_New2(UnitRot);
	}

	if ((stage!=UnitStage_StartFw)&&(stage!=UnitStage_StartRot)&&(stage!=UnitStage_Move))
		_ClearPath();

	if (stage==UnitStage_Reached||stage==UnitStage_Abort||stage==UnitStage_Faced)
	{
		Safe_Class_Delete(_target);
	}
	_stage=stage;
	_reasonAbort=UnitAbortReason_None;
	_tInStage=0.0f;
}

BOOL CUnit::_CanAcceptPendingTarget()
{
	if ((_stage==UnitStage_StartFw)||(_stage==UnitStage_StartRot)||(_stage==UnitStage_Stop)||(_stage==UnitStage_RotateOnSpot))
		return FALSE;
	return TRUE;
}



void CUnit::UpdateStage(float dt0)
{
	float dt=dt0;
	if (_infoGesture.IsValid())
		return;

	float faceLast=_face;

	_dist=0.0f;

	float rotlimit=0.0f;//这一帧可以最多转多少弧度

	BOOL bNewPath=FALSE;

	//更新target
	if (TRUE)
	{
		if (_CanAcceptPendingTarget())
		{
			if (_targetPending)
			{
				Safe_Class_Delete(_target);
				_target=_targetPending;
				_targetPending=NULL;

				_session=_sessionPending;

				_radLastAvoid=-1.0f;//Target切换,标记为不在绕
			}
		}

		if (_target)
		{
			_target->VerifyAlive();
			if (_target->IsEmpty())
			{
				Safe_Class_Delete(_target);
			}
		}
	}

	if (_path)
	{
		if (!_path->IsAlive())
		{
			SAFE_RELEASE(_path);
		}
	}

	if ((_stage==UnitStage_NotMove)||
		(_stage==UnitStage_Reached)||
		(_stage==UnitStage_Faced)||
		(_stage==UnitStage_Abort)||
		(_stage==_UnitStage_Blocked))
	{
		_tInStage+=dt;

		if (_target&&(!_target->IsCur()))
		{
			if (!_CheckInRange())
			{
				if (_MakePathToTarget())
				{
					_StartMove();
					bNewPath=TRUE;
				}
			}
			else
			{
				_SetStage(_UnitStage_Reaching);
				FinalizeReach();
			}
		}
	}

	BOOL bFacingMovingDir=FALSE;
	float faceMoving=_face;
	if (IsStart())
	{
		assert(_pace);

		float tInStartStageLast=_tInStage;
		_tInStage+=dt;
		dt=0.0f;
		rotlimit=1000000.0f;//没有限制

		_face=_rot->from;
		i_math::rotate_limited(_face,_rot->to,_tInStage*_rot->speed);
		assert(!isnan(_face));
		faceMoving=_rot->to;

		_dir.x=cosf(_rot->to);
		_dir.y=sinf(_rot->to);

		if (_stage==UnitStage_StartFw)
		{
			_dist=_pace->startFw.GetFloat(ANIMTICK_FROM_SECOND(_tInStage));
			_dist-=_pace->startFw.GetFloat(ANIMTICK_FROM_SECOND(tInStartStageLast));
		}
		else
		{
			_dist=_pace->startRot.GetFloat(ANIMTICK_FROM_SECOND(_tInStage));
			_dist-=_pace->startRot.GetFloat(ANIMTICK_FROM_SECOND(tInStartStageLast));
		}
		if (_dist<0.0f)
			_dist=0.0f;
	}

	if (_stage==UnitStage_Move)
	{
		if (_target)
		{
			_target->VerifyAlive();
			if (_target->IsEmpty())
			{
				Safe_Class_Delete(_target);
			}
		}

		if (!_target)
		{
			_StopMove();
		}
		else
		{
			if (CheckPathNeedRecalc(this,_target,_path))
			{
				_ClearPath();

				bNewPath=_MakePathToTarget();
				_iPathNode=1;
			}

			if (!_path)
				_StopMove();
			else
			{
				rotlimit=1000000.0f;
				if (_pace)
				{
					if (_pace->rotlimit>0.0f)
						rotlimit=_pace->rotlimit*dt;
				}
				float tInMoveStageLast=_tInStage;
				_tInStage+=dt;
				dt=0.0f;

				i_math::vector2df dir;
				if (_bEscape||bNewPath)
				{
					i_math::vector2df &v2=_path->_nodes[_iPathNode];
					dir=v2-_pos;
					float length=(float)dir.getLength();
					if (length>0.0f)
						dir/=length;
					else
						dir.set(0,0);
				}
				else
				{
					if (_iPathNode+1<_path->_nodes.size())
					{//目的地不是最后一个路点
						float d1,d2;
						i_math::vector2df &v1=_path->_nodes[_iPathNode-1];
						i_math::vector2df &v2=_path->_nodes[_iPathNode];
						i_math::vector2df &v3=_path->_nodes[_iPathNode+1];
						d1=(_pos.x-v1.x)*(v2.y-v1.y)-(_pos.y-v1.y)*(v2.x-v1.x);
						d2=(v3.x-v1.x)*(v2.y-v1.y)-(v3.y-v1.y)*(v2.x-v1.x);

						if ((d1==0.0f)||//当前点在第一段所在的直线上
							(d1*d2>0.0f))//当前点和v3在第一段所在直线的同一侧
							dir=v2-_pos;
						else
							dir=v2-v1;
						dir.safe_normalize();
					}
					else
					{//目的地是最后一个路点
						i_math::vector2df &v1=_path->_nodes[_iPathNode-1];
						i_math::vector2df &v2=_path->_nodes[_iPathNode];
						dir=v2-v1;
						dir.normalize();
						i_math::vector2df dirTarget=v2-_pos;
						float dist=(float)dirTarget.getLength();
						if (dist>_speed*(_tInStage-tInMoveStageLast)*1.5f)
						{
							dirTarget/=dist;
							if (dir.dotProduct(dirTarget)<0.87f)//夹角大于30度
								dir=dirTarget;
						}
						else
						{
							if (dist>0.001f)
								dir=dirTarget/dist;
							else
								dir.set(0,0);
						}
					}
				}

				float rad=atan2f(dir.y,dir.x);
				faceMoving=rad;

// 				if (_pace&&_pace->bSupportStop&&i_math::get_radian_dist(rad,_face)>135.0f*i_math::GRAD_PI2)
// 					_StopMove();//太大夹角
// 				else
				{
					if (_iPathNode+1>=_path->_nodes.size())
					{
						float distToEnd=_pos.getDistanceFrom(_path->_nodes[_iPathNode]);
						float rate=i_math::clamp_f(1.0f-distToEnd/4.0f,0.0f,1.0f);
						rotlimit=i_math::lerp(rotlimit,rotlimit*4.0f,rate);
					}


					bFacingMovingDir=i_math::rotate_limited(_face,rad,rotlimit*i_math::GRAD_PI2);
					assert(!isnan(_face));
					_dir.x=cosf(_face);
					_dir.y=sinf(_face);
					if ((_bEscape)&&(!bFacingMovingDir))//Escape的话,必须要转到方向后才能移动
						_dist=0.0f;
					else
					{
						_dist=_speed*(_tInStage-tInMoveStageLast);
						if (TRUE)
						{
							float radGap=i_math::get_radian_dist(_face,rad);
							float scale=(i_math::Pi-radGap)/i_math::Pi;

							_dist=0.5f*_dist*(1-scale)+_dist*scale;
						}
					}
				}
			}
		}
	}

	float tInStopStageLast=0.0f;
	if (_stage==UnitStage_Stop)
	{
		assert(_pace);

		tInStopStageLast=_tInStage;
		_tInStage+=dt;
		_dir.x=cosf(_face);
		_dir.y=sinf(_face);

		_dist=_pace->stop.GetFloat(ANIMTICK_FROM_SECOND(_tInStage));
		_dist-=_pace->stop.GetFloat(ANIMTICK_FROM_SECOND(tInStopStageLast));
		if (_dist<0.0f)
			_dist=0.0f;
	}

	//测试这个Step(_pos开始沿_dir走_dist)是否能够走到,
	//不能走到的话尝试绕一下
	if (IsMoving())
	{
		BOOL bBlocked=FALSE;
		i_math::vector2df posHit;
		i_math::vector2df target;
		if ((!_bEscape)&&(!IsPathOnUnwalkable()))
		{
			target=_pos+_dir*_dist;

			if (_mgr->StaticRayCast(_tpPathFind,_pos,target,posHit))
			{//无法直接走到
				
				BOOL bOnPath=FALSE;
				if (_stage!=UnitStage_Stop)
				{
					if (_path)//检查是不是准确的在路径上移动
					{
						i_math::vector2df &vTo=_path->_nodes[_iPathNode];
						i_math::vector2df &vFrom=_path->_nodes[_iPathNode-1];
						i_math::vector2df dirPath=vTo-vFrom;

						if (!dirPath.equalsZero())
						{
							dirPath.normalize();
							if (dirPath.dotProduct(_dir)>0.998f)
							{
								i_math::line2df line;
								line.start=vFrom;
								line.end=vTo;

								if (line.isPointOnMe(_pos))
								{
									bOnPath=TRUE;
									i_math::vector2df dirToPath=vTo-_pos;
									float distToPath=dirToPath.getLength();
									if (_dist>distToPath)
										_dist=distToPath;
								}

							}
						}
					}
				}

				if (!bOnPath)//在路径上的话,我们相信可以走到
				{
					//不在路径上

					//尝试转到MovingDir上
					BOOL bUseMovingDir=FALSE;
					if (_stage==UnitStage_Move)
					{
						if (!bFacingMovingDir)
						{
							_face=faceMoving;
							assert(!isnan(_face));
							_dir.x=cosf(_face);
							_dir.y=sinf(_face);
							target=_pos+_dir*_dist;
							if (!_mgr->StaticRayCast(_tpPathFind,_pos,target,posHit))
								bUseMovingDir=TRUE;
						}
					}

					if (!bUseMovingDir)
					{
						BOOL bNeedAvoid=FALSE;
						if (_stage==UnitStage_Move)
								bNeedAvoid=TRUE;
						else
						{
							if (IsStart())
								bNeedAvoid=TRUE;
						}

						if (bNeedAvoid)
						{
							//我们向两侧尝试一下
							i_math::vector2df dir2;

							bBlocked=TRUE;
							for (int k=0;k<4;k++)
							{
								float rad=20.0f*((float)k)*i_math::GRAD_PI2;
								dir2.x = _dir.x*cosf(rad)+ _dir.y*sinf(rad);
								dir2.y= -_dir.x*sinf(rad)+ _dir.y*cosf(rad);
								target=_pos+dir2*_dist;
								if (_mgr->StaticObstacleTest(_tpPathFind,_pos,target))
								{
									dir2.x = _dir.x*cosf(rad)+ _dir.y*-sinf(rad);
									dir2.y= -_dir.x*-sinf(rad)+ _dir.y*cosf(rad);
									target=_pos+dir2*_dist;
									if (_mgr->StaticObstacleTest(_tpPathFind,_pos,target))
									{
										_dist/=2.0f;
										continue;
									}
									else
										_dir=dir2;
								}
								else
									_dir=dir2;	
								bBlocked=FALSE;
								break;
							}
						}
						else
						{
							//暂时卡一下
							_dist=(_pos-posHit).getLength();
						}
					}
				}
			}
		}

		if (bBlocked)
		{
			if (_stage==UnitStage_Move)
			{
				_SetStage(_UnitStage_Blocked);
				_radLastAvoid=-1.0f;
			}
			else
				_dist=(_pos-posHit).getLength();
		}
		else
		{
			if (_stage!=UnitStage_Stop)
			{
				if (_path)
				{
					if (_iPathNode+1<_path->_nodes.size())
					{
						//判断是否可以以下一个路点为目标了
						i_math::vector2df &v1=_path->_nodes[_iPathNode-1];
						i_math::vector2df &v2=_path->_nodes[_iPathNode];
						i_math::vector2df &v3=_path->_nodes[_iPathNode+1];
						target=_pos+_dir*_dist;

						//判断target和v1是不是在(v2-v3)的不同侧
						float d1=(target.x-v2.x)*(v3.y-v2.y)-(target.y-v2.y)*(v3.x-v2.x);
						float d2=(v1.x-v2.x)*(v3.y-v2.y)-(v1.y-v2.y)*(v3.x-v2.x);

						if (d1*d2<=0.0f)
							_iPathNode++;
						else
						{
							if (!_mgr->StaticObstacleTest(_tpPathFind,target,_path->_nodes[_iPathNode+1]))
								_iPathNode++;
						}
					}
				}
			}
		}
	}

	if (_stage==UnitStage_Move)
	{
		if (_pace&&(_pace->rotlimitBigTurn>0.0f)&&(i_math::get_radian_dist(faceLast,_face)/dt0>_pace->rotlimitBigTurn*i_math::GRAD_PI2))
		{
			_face=faceLast;
			assert(!isnan(_face));
			_SetStage(_UnitStage_BigTurnInterrupted);
			_radLastAvoid=-1.0f;
		}
	}

	if (_stage==UnitStage_RotateOnSpot)
	{
		_tInStage+=dt;
		dt=0.0f;

		_dist=0.0f;
		_dir.set(0.0f,0.0f);

		_face=_rot->from;
		i_math::rotate_limited(_face,_rot->to,_tInStage*_rot->speed);
		assert(!isnan(_face));

		float gap=i_math::get_radian_dist(_rot->from,_rot->to);
		float dur=gap/_rot->speed;
		if (_pace)
			dur+=_pace->durROSWait;

		BOOL bReached=FALSE;
		if (_tInStage>=dur)
			bReached=TRUE;

		if (bReached)
		{
			if (_target)
			{
				if (_target->CheckInFaceRad(this))
				{
					if (_target->CheckInFaceRange(this))
						_SetStage(UnitStage_Faced);
					else
					{
						_SetStage(UnitStage_Abort);
						_reasonAbort=UnitAbortReason_OutOfFacingRange;
					}
				}
				else
				{
					_SetStage(UnitStage_Abort);
					_reasonAbort=UnitAbortReason_OutOfFacing;
				}
			}
			else
			{
				_SetStage(UnitStage_Abort);
				_reasonAbort=UnitAbortReason_LoseTarget;
			}
		}

	}

}

BOOL CUnit::CanSetPace()
{
	if ((_stage==UnitStage_NotMove)||
		(_stage==UnitStage_Move)||
		(_stage==UnitStage_Reached)||
		(_stage==UnitStage_Faced)||
		(_stage==UnitStage_Abort))
		return TRUE;
	return FALSE;
}

BOOL CUnit::SetPace(UnitPace *pace)
{
	if (CanSetPace())
	{
		_pace=pace;
		return TRUE;
	}
	return FALSE;
}


void CUnit::_UpdateMirrorCollide()
{
	if (_mirror)
	{
		UnitCollideInfo *info=(UnitCollideInfo *)&_collide;
		_mirror->setGhostCollide(info->bGhost);
	}
}


//////////////////////////////////////////////////////////////////////////
//CUnitMgr::CircumRanges
void CUnitMgr::CircumRanges::Add(float low,float hi,CUnitBase *unit)
{
	Range nr;//new range
	nr.low=low;
	nr.hi=hi;
	nr.unitLow=nr.unitHi=unit;
	DWORD c=0;
	for (int i=0;i<_nRanges;i++)
	{
		Range *p=&_ranges[i];

		if (p->low>nr.hi)
		{
			memmove(&_ranges[c+1],&_ranges[i],(_nRanges-i)*sizeof(Range));
			_ranges[c]=nr;
			_nRanges=c+1+(_nRanges-i);
			return;
		}

		if (p->hi<nr.low)
		{
			_ranges[c]=_ranges[i];
			c++;
			continue;
		}

		//有重叠的部分,将*p merge到 nr中去
		if (p->hi>nr.hi)
		{
			nr.hi=p->hi;
			nr.unitHi=p->unitHi;
		}
		if (p->low<nr.low)
		{
			nr.low=p->low;
			nr.unitLow=p->unitLow;
		}
	}

	_ranges[c]=nr;
	_nRanges=c+1;
}



//////////////////////////////////////////////////////////////////////////
//CUnitMgr

void CUnitMgr::Create(i_math::recti &rcMap)
{
	_mp.Create(rcMap);
}

void CUnitMgr::Destroy()
{
	_ClearUnits();

	_mp.Destroy();

}


CUnit* CUnitMgr::CreateUnit(i_math::vector2df &pos,float face,float radius,float speed,UnitPace *pace,BOOL bAllowMirror,const char *nm)
{
	CUnit *p=Class_New2(CUnit);
	p->_mgr=this;

	p->_nm=nm;

	p->_pos=pos;
	p->_face=face;
	assert(!isnan(p->_face));
	p->_radius=radius;
	p->_speed=speed;
	p->_pace=pace;

	p->_bAlive=1;
	p->_solveflag=_solveflag;
	p->_expandflag=_expandflag;
	p->_sortflag=_sortflag;

	p->AddRef();

	_units.push_back(p);

	p->AddRef();

	_mp.AddUnit(p);

	if (bAllowMirror)
	{
		if (_mirror)
			p->_mirror=_mirror->addUnit(pos,0.0f,0,0.01f,0.01f,radius,0.0f,FALSE);
	}

	return p;
}

void CUnitMgr::_ClearUnits()
{
	for (int i=0;i<_units.size();i++)
	{
		_units[i]->Destroy();
	}
	_units.clear();
}


inline BOOL TestUnitCollide(CUnit *src,CUnit *target)
{
	float dist=intersectSphereBySphere(src->GetPos(),src->_radius,src->_dir,target->GetPos(),target->_radius);
	if (dist>=0.0f)
	{
		if (dist<src->_dist)
			return TRUE;
	}
	return FALSE;
}

inline i_math::vector2df&FastRadianToDir(float radian)
{
	static BOOL bInit=FALSE;
	static i_math::vector2df dirs[360];
	if (!bInit)
	{
		for (int i=0;i<360;i++)
		{
			float radian=(float)i*i_math::Pi*2.0f/360.0f;
			dirs[i].set(cosf(radian),sinf(radian));
		}
		bInit=TRUE;
	}

	int idx=(int)(radian*360.0f/(2.0f*i_math::Pi));
	if (idx>=360)
		idx=360-1;

	return dirs[idx];
}


float CalcScaleForUnitPair(CUnit *unitSrc,CUnit *unitTarget)
{
	UnitCollide collideSrc,collideTarget;
	collideSrc=unitSrc->GetCollide();
	collideTarget=unitTarget->GetCollide();
	UnitCollideInfo *infoSrc=(UnitCollideInfo *)&collideSrc;
	UnitCollideInfo *infoTarget=(UnitCollideInfo *)&collideTarget;
	if (infoSrc->ally==infoTarget->ally)
		return 0.8f;
	return 1.0f;
}

BOOL CUnitMgr::CheckUnitColliding(CUnit *unit,i_math::vector2df &posTarget,float rateShrink,i_math::vector2df *posColliding,CUnit **unitColliding)
{
	i_math::vector2df posSrc=unit->GetPos();
	i_math::rectf rc;
	rc.set(posSrc.x,posSrc.y,posTarget.x,posTarget.y);
	rc.repair();
	rc.inflate(unit->GetRadius());
	_mp.Enum(rc);

	i_math::vector2df dir=posTarget-posSrc;
	if (dir.getLengthSQ()<0.01f*0.01f)
		return FALSE;
	float dist2Min=dir.getLengthSQ();
	dir.safe_normalize();
	BOOL bFound=FALSE;

	rateShrink=i_math::clamp_f(rateShrink,0.0f,1.0f);

	DWORD c;
	CUnitBase **units=(CUnitBase **)_mp.GetEnums(c);
	for (int i=0;i<c;i++)
	{
		CUnit *unit2=(CUnit *)units[i];

// 		if (!unit2->IsMoving())
// 			continue;

		if (unit2==unit)
			continue;

		if (!UnitCollide_Check(unit->GetCollide(),unit2->GetCollide()))
			continue;

		float scale=CalcScaleForUnitPair(unit,unit2);

		float dist=intersectSphereBySphere(posSrc,
			unit->GetRadius()*scale*rateShrink,dir,unit2->GetPos(),unit2->GetRadius()*scale*rateShrink);//rateShrink用于略过边缘colliding
		if (dist>=0.0f)
		{
			float dist=intersectSphereBySphere(posSrc,unit->GetRadius()*scale,dir,unit2->GetPos(),unit2->GetRadius()*scale);
			if (dist*dist<dist2Min)
			{
				if (posColliding)
					*posColliding=posSrc+dir*dist;
				if (unitColliding)
					*unitColliding=unit2;
				bFound=TRUE;
				dist2Min=dist*dist;
			}
		}
	}

	return bFound;
}

DWORD CUnitMgr::CheckUnitCollidings(CUnit *unit,i_math::vector2df &posTarget,float rateShrink,float *&bufDist,i_math::vector2df *&bufPos,CUnit **&bufUnits)
{
	_collidingTempPos.clear();
	_collidingTempUnits.clear();
	_collidingTempDists.clear();
	bufPos=NULL;
	bufUnits=NULL;
	bufDist=NULL;

	i_math::vector2df posSrc=unit->GetPos();
	i_math::rectf rc;
	rc.set(posSrc.x,posSrc.y,posTarget.x,posTarget.y);
	rc.repair();
	rc.inflate(unit->GetRadius());
	_mp.Enum(rc);

	i_math::vector2df dir=posTarget-posSrc;
	if (dir.getLengthSQ()<0.01f*0.01f)
		return 0;
	float dist2Min=dir.getLengthSQ();
	dir.safe_normalize();
	BOOL bFound=FALSE;

	rateShrink=i_math::clamp_f(rateShrink,0.0f,1.0f);

	DWORD c;
	CUnitBase **units=(CUnitBase **)_mp.GetEnums(c);
	for (int i=0;i<c;i++)
	{
		CUnit *unit2=(CUnit *)units[i];

		// 		if (!unit2->IsMoving())
		// 			continue;

		if (unit2==unit)
			continue;

		if (!UnitCollide_Check(unit->GetCollide(),unit2->GetCollide()))
			continue;

		float scale=CalcScaleForUnitPair(unit,unit2);

		float dist=intersectSphereBySphere(posSrc,
			unit->GetRadius()*scale*rateShrink,dir,unit2->GetPos(),unit2->GetRadius()*scale*rateShrink);//rateShrink用于略过边缘colliding
		if (dist>=0.0f)
		{
			float dist=intersectSphereBySphere(posSrc,unit->GetRadius()*scale,dir,unit2->GetPos(),unit2->GetRadius()*scale);//用于求得真实的距离(无视rateShrink)
			if (dist*dist<dist2Min)
			{
				_collidingTempDists.push_back(dist);
				_collidingTempPos.push_back(posSrc+dir*dist);
				_collidingTempUnits.push_back(unit2);
			}
		}
	}

	//按距离远近排序
	if (_collidingTempDists.size()>1)
	{
		for (int i=0;i<_collidingTempDists.size();i++)
		for (int j=i+1;j<_collidingTempDists.size();j++)
		{
			if (_collidingTempDists[i]>_collidingTempDists[j])
			{
				Swap(_collidingTempDists[i],_collidingTempDists[j]);
				Swap(_collidingTempPos[i],_collidingTempPos[j]);
				Swap(_collidingTempUnits[i],_collidingTempUnits[j]);
			}
		}
	}

	bufDist=_collidingTempDists.data();
	bufPos=_collidingTempPos.data();
	bufUnits=_collidingTempUnits.data();
	return _collidingTempPos.size();
}


void CUnitMgr::UpdateUnitPos(CUnit *unit)
{
	if (unit->_mirror)
		unit->_mirror->Pos()=unit->GetPos();
	_mp.UpdateUnit(unit);
}

void CUnitMgr::_UpdateUnitStage(CUnit *unit,float dt)
{
	unit->UpdateStage(dt);
	if (unit->GetStage()==_UnitStage_Blocked)
	{
		unit->UpdateStage(dt);
		if (unit->GetStage()==_UnitStage_Blocked)
			unit->StopMove();
	}
	if (unit->GetStage()==_UnitStage_BigTurnInterrupted)
	{
		unit->StopMove();
		unit->UpdateStage(dt);
	}
	assert(unit->GetStage()!=_UnitStage_Reaching);
}

void CUnitMgr::_UpdateUnit(CUnit *unit,float dt)
{
	i_math::vector2df dirToTarget;

	//根据target的位置,限制移动的距离
	if (unit->_target)
	{
		float dist=0.0f;
		if (unit->_target->_unit)
		{
			BOOL bCollide=UnitCollide_Check(unit->GetCollide(),unit->_target->_unit->GetCollide());

			float v;
			if (bCollide)
				v=unit->_target->_unit->_radius+unit->_radius+unit->_dist;
			else
				v=unit->_dist;
			dirToTarget=(unit->_target->_unit->_pos-unit->_pos);
			dist=(float)dirToTarget.getLength();
			if (v>dist)
			{
				if (bCollide)
					unit->_dist=dist-(unit->_target->_unit->_radius+unit->_radius);
				else
					unit->_dist=dist;
				if (unit->_dist<0.0f)
					unit->_dist=0.0f;
			}
		}
		if (unit->_target->_pos)
		{
			dirToTarget=(unit->_target->_pos->_pos-unit->_pos);
			dist=(float)dirToTarget.getLength();
			if (unit->_dist>dist)
				unit->_dist=dist;
		}

		if (dist>=0.0001f)
			dirToTarget/=dist;
		else
			dirToTarget.set(0,0);

		//更新离目标的最近距离
		if (TRUE)
		{
			BOOL bFinalNode=FALSE;
			if (unit->_path)
			{
				if (unit->_iPathNode==unit->_path->_nodes.size()-1)
					bFinalNode=TRUE;
			}

			if (bFinalNode)
			{
				if (dist<unit->_closest)
				{
					unit->_closest=dist;
					unit->_tClosest=_t;
				}
			}
			else
			{
				unit->_closest=100000.0f;
				unit->_tClosest=0xffffffff;
			}
		}
	}

	_circums.clear();

	//找到所有这个Unit有可能会碰到的Unit
	float range;
	range=unit->_radius+unit->_dist+UNIT_MAX_RADIUS;
	_mp.Enum(unit,range);

	DWORD c;
	CUnitBase **units=(CUnitBase **)_mp.GetEnums(c);
	for (int i=0;i<c;i++)
	{
		CUnit *unit2=(CUnit *)units[i];

		if (unit2==unit)
			continue;//不考虑自己 

		if (FALSE==UnitCollide_Check(unit->GetCollide(),unit2->GetCollide()))
			continue;//不考虑不需要碰撞的unit


		if (unit->_target)
		{
			if (unit2==unit->_target->_unit)
				continue;//不考虑目的Unit
		}

		i_math::vector2df dir=unit2->_pos-unit->_pos;
		float scale=CalcScaleForUnitPair(unit,unit2);
		float v=unit->_radius*scale+unit2->_radius*scale+unit->_dist;

		if (v*v<(float)dir.getLengthSQ())
			continue;//在范围之外,不予考虑

		_circums.push_back(unit2);
	}

	if ((unit->GetStage()==UnitStage_Move)||(unit->IsStart())||(unit->GetStage()==UnitStage_Stop))
	{
		if (_circums.size()>0)
		{
			float radian;//unit移动方向的角度
			radian=atan2f(unit->_dir.y,unit->_dir.x);

			_ranges.Clear();

			for (int j=0;j<_circums.size();j++)
			{
				CUnit *unitOther=_circums[j];
				i_math::vector2df dir=unitOther->_pos-unit->_pos;
				float dist2Other=(float)dir.getLength();

				float scale=CalcScaleForUnitPair(unit,unitOther);
				float rSum=unit->_radius*scale+unitOther->_radius*scale;

				//unitOther相对于unit的角度
				float radianOff=atan2f(dir.y,dir.x)-radian;
				radianOff=i_math::normalize_radian(radianOff);

				float scatter;
				if (rSum>dist2Other)
				{
					//embedded
					scatter=i_math::Pi/2.0f;
				}
				else
					scatter=asin(rSum/dist2Other);

				_ranges.Add(radianOff-scatter,radianOff+scatter,unitOther);
			}

			CircumRanges::Range *range=_ranges.FindRange(0.0f);
			if (range)
			{
				//前行的方向上有阻挡
				BOOL bNeedAvoid=TRUE;

				if (unit->GetStage()==UnitStage_Move)
				{
					if (unit->IsClosestFollow())
					{
						if (unit->_CheckInRange_NoClosestFollow())
						{

							float distToMove=unit->_dist;
							for (int j=0;j<_circums.size();j++)
							{
								CUnit *unitOther=_circums[j];
								float scale=CalcScaleForUnitPair(unit,unitOther);

								float radius=unit->GetRadius()*scale;
								float radiusOther=unitOther->GetRadius()*scale;

								extern f32 intersectSphere2D(i_math::vector2df& vOrigin,i_math::vector2df& vRay, i_math::vector2df& vCenter, f32 sR);
								float dist=intersectSphere2D(unit->_pos,unit->_dir,unitOther->_pos,radius+radiusOther);
								if (dist>=0.0f)
								{
									if (dist<distToMove)
										distToMove=dist;
								}
							}

							//									unit->_dir.set(0,0);
							unit->_dist=distToMove;
							unit->_nStucks=0;
							unit->_radLastAvoid=-1.0f;//标记为不在绕

							unit->SetStage(_UnitStage_Reaching);

							bNeedAvoid=FALSE;//已经Reached,不用绕了
						}
					}
				}

				if (bNeedAvoid)
				{
					float rLow,rHi;
					rLow=radian+range->low;
					rHi=radian+range->hi;

					BOOL bStuck=FALSE;//是否被困住(所有方向都被围住)
					if (range->IsFull())
						bStuck=TRUE;

					float radianAvoid;
					CUnit *unitAvoid;//要绕开哪个unit
					if (!bStuck)
					{
						//选择一个方向进行绕路
						BOOL bLow=TRUE;
						if (unit->_radLastAvoid<0.0f)
						{//上次没有在绕
							if (fabsf(range->hi)>fabsf(range->low))
							{
								radianAvoid=i_math::wrap_radian(rLow);
								unitAvoid=(CUnit*)range->unitLow;
							}
							else
							{
								radianAvoid=i_math::wrap_radian(rHi);
								unitAvoid=(CUnit*)range->unitHi;
								bLow=FALSE;
							}
						}
						else
						{
							//选择一个和上次的选择更接近的方向
							if (i_math::get_radian_dist(rLow,unit->_radLastAvoid)<i_math::get_radian_dist(rHi,unit->_radLastAvoid))
							{
								radianAvoid=i_math::wrap_radian(rLow);
								unitAvoid=(CUnit*)range->unitLow;
							}
							else
							{
								radianAvoid=i_math::wrap_radian(rHi);
								unitAvoid=(CUnit*)range->unitHi;
								bLow=FALSE;
							}
						}

						//对要绕的路,测试一下是否能走通
						if ((!unit->_bEscape)&&(!unit->IsPathOnUnwalkable()))
						{
							i_math::vector2df *dir=&FastRadianToDir(radianAvoid);
							i_math::vector2df target=unit->_pos+(*dir)*unit->_dist;

							if (StaticObstacleTest(unit->_tpPathFind,unit->_pos,target))
							{//走不通,换另一个方向试试
								bLow=!bLow;
								if (!bLow)
								{
									radianAvoid=i_math::wrap_radian(rHi);
									unitAvoid=(CUnit*)range->unitHi;
								}
								else
								{
									radianAvoid=i_math::wrap_radian(rLow);
									unitAvoid=(CUnit*)range->unitLow;
								}

								dir=&FastRadianToDir(radianAvoid);
								target=unit->_pos+(*dir)*unit->_dist;

								if (StaticObstacleTest(unit->_tpPathFind,unit->_pos,target))
									bStuck=TRUE;
							}
						}
					}

					if (!bStuck)
					{
						unit->_nStucks=0;
						unit->_radLastAvoid=radianAvoid;
						i_math::vector2df *dir=&FastRadianToDir(unit->_radLastAvoid);

						BOOL bHold=FALSE;
						if (unitAvoid->_solveflag==_solveflag)
						{//unitAvoid已经处理过了(已经移动过了)
							//查看一下我们要移动的方向,和要绕过的unit(unitAvoid)移动的方向是否太一致了
							if (unitAvoid->IsMoving())
							{
								float d1=(unitAvoid->_dir.x)*(unit->_dir.y)-(unitAvoid->_dir.y)*(unit->_dir.x);//unitAvoid在目的方向的哪一侧
								float d2=(dir->x)*(unit->_dir.y)-(dir->y)*(unit->_dir.x);//移动的方向在目的方向的哪一侧

								if (d1*d2>0.0f)
								{//在同一侧的话
									bHold=TRUE;
									unit->_dir.set(0,0);
									unit->_dist=0.0f;
									unit->_radLastAvoid=-1.0f;//标记为不在绕
								}
							}
						}

						if (!bHold)
						{
							unit->_dir=*dir;

							if ((!unit->_pace)||(unit->_pace->rotlimitAvoid<=0.0f))
								unit->_face=radianAvoid;
							else
								i_math::rotate_limited(unit->_face,radianAvoid,unit->_pace->rotlimitAvoid*i_math::GRAD_PI2*dt);
							assert(!isnan(unit->_face));
						}
					}
					else
					{
						if (unit->_nStucks<7)
							unit->_nStucks++;
						unit->_radLastAvoid=-1.0f;//标记为没有在绕
						unit->_dir.set(0,0);
						unit->_dist=0.0f;
					}
				}
				else
					unit->_nStucks=0;
			}
			else
			{
				//有circum,但没阻碍
				unit->_radLastAvoid=-1.0f;//标记为没有在绕
				unit->_nStucks=0;
			}
		}
		else
		{
			//没有circum
			unit->_radLastAvoid=-1.0f;//标记为没有在绕
			unit->_nStucks=0;
		}

		if (unit->_nStucks>1)
		{
			if (unit->GetStage()==UnitStage_Move)
				unit->SetStage(_UnitStage_Reaching);
		}

		if (unit->IsStart())
		{
			if (unit->IsStartFinished())
			{
				unit->_face=unit->_rot->to;//转到最终值
				assert(!isnan(unit->_face));
				unit->SetStage(UnitStage_Move);
			}
		}

		if (unit->GetStage()==UnitStage_Stop)
		{
			if (unit->IsStopFinished())
				unit->FinalizeReach();
		}

		//检查是否走到
		if (unit->GetStage()==UnitStage_Move)
		{
			if (unit->_path)
			{
				if (unit->_tClosest!=0xffffffff)
				{
					//基于这样的考虑:
					//如果这个unit在某个时刻t达到了最接近的位置,那么我们认为在t之后经过一段
					//时间(delta)的移动,这个unit可以被认为足够接近目标点了,而这段时间(delta)和那个最接近的
					//距离成正比
					DWORD tIntend;
					float speed=unit->_speed;
					if (speed<=0.01f)
						speed=0.01f;
					tIntend=unit->_tClosest+(DWORD)((unit->_closest*i_math::Pi*1.5f/speed)*1000.0f);
					if (_t>tIntend)
					{
						unit->_dist=0.0f;
						unit->SetStage(_UnitStage_Reaching);
					}
				}
			}
		}
	}

	if (unit->_dist>0.0f)
		unit->_pos+=unit->_dir*unit->_dist;

	UpdateUnitPos(unit);//更新在UnitMap里的位置
	if (unit->_bEscape)
	{
		if (IsWalkable(unit->_tpPathFind,unit->_pos))
			unit->_bEscape=0;
	}

	unit->_solveflag=_solveflag;
}

void CUnitMgr::_FinalizeBlocks()
{
	for (int i=0;i<_blocks.size();i++)
	{
		CUnit *unit=_blocks[i];
		if (unit->IsStart())
		{
			if (unit->IsStartFinished())
			{
				unit->_face=unit->_rot->to;//转到最终值
				assert(!isnan(unit->_face));
				unit->SetStage(UnitStage_Move);
			}
		}
		if (unit->GetStage()==UnitStage_Stop)
		{
			if (unit->IsStopFinished())
				unit->FinalizeReach();
		}
	}

	_blocks.clear();
}

void CUnitMgr::_FinalizeReaching(CUnit *unit)
{
	if (unit->GetStage()==_UnitStage_Reaching)
		unit->StopMove();
	else
	{
		if (unit->GetStage()==UnitStage_Move)
		{
			if (unit->_CheckInRange())
				unit->StopMove();
		}
	}
}



void CUnitMgr::UpdateSingle(CUnit *unit,float dt)
{
	_UpdateUnitStage(unit,dt);

	unit->_expandflag=_expandflag;
	unit->_solveflag=_solveflag;

	unit->_solveflag=1-_solveflag;
	_UpdateUnit(unit,dt);

	_FinalizeBlocks();
	_FinalizeReaching(unit);
}


void CUnitMgr::Update(float dt)
{
	_dt=dt;
	_t+=(DWORD)(dt*1000.0f);
	DWORD cDead=0;
	for (int i=0;i<_units.size();i++)
	{
		CUnit *unit=_units[i];
		if (!unit->IsAlive())
		{
			cDead++;
			continue;
		}

		if (unit->GetGesture())
		{
			unit->_infoGesture.UpdateState(unit,dt);
			UpdateUnitPos(unit);//更新在UnitMap里的位置
			continue;
		}

		_UpdateUnitStage(unit,dt);
	}

	if (cDead>_units.size()/4)
		_bNeedFlushDead=TRUE;

	//进行真正的移动(同时处理碰撞的情况)

	//更新这一次Update的的Dirty标志的值
	_expandflag=1-_expandflag;//sort的dirty标志反一反
	_sortflag=1-_sortflag;//sort的dirty标志反一反
	_solveflag=1-_solveflag;//solve的dirty标志反一反

	//首先我们先对各个unit进行排序,以决定移动的顺序
	if (TRUE)
	{
		_stack=_units;

		while(_stack.size()>0)
		{
			CUnit *unit=_stack[_stack.size()-1];
			_stack.pop_back();

			if (!unit->IsAlive())
				continue;

			if (unit->GetGesture())
				continue;

			if (unit->_sortflag==_sortflag)
				continue;//已经在_sort队列中了

			if (unit->_dist<=0.0f)
			{
				if ((unit->IsStart())||(unit->GetStage()==UnitStage_Stop))
					_blocks.push_back(unit);
				unit->_expandflag=_expandflag;
				unit->_sortflag=_sortflag;
				unit->_solveflag=_solveflag;
				continue;
			}

			if (!unit->IsMoving())
			{
				unit->_expandflag=_expandflag;
				unit->_sortflag=_sortflag;
				unit->_solveflag=_solveflag;
				continue;
			}

			//加入排序好的队列中
			if (unit->_expandflag==_expandflag)
			{//已经展开了
				unit->_sortflag=_sortflag;
				_sorts.push_back(unit);
				continue;
			}

			//找到这个unit有可能会碰到的units,并压到堆栈上(这个过程称为展开(expand))
			if (TRUE)
			{
				_stack.push_back(unit);
				unit->_expandflag=_expandflag;//标记为展开了

				float range;
				range=unit->_radius+unit->_dist+UNIT_MAX_RADIUS;
				_mp.Enum(unit,range);

				DWORD c;
				CUnitBase **units=(CUnitBase **)_mp.GetEnums(c);
				for (int i=0;i<c;i++)
				{
					CUnit *unit2=(CUnit *)units[i];

					if (unit2->_sortflag==_sortflag)
						continue;

					if (!unit2->IsMoving())
						continue;

					if (TestUnitCollide(unit,unit2))
						_stack.push_back(unit2);
				}
			}
		}
	}

	//然后依序处理各个Unit
	for (int i=0;i<_sorts.size();i++)
	{
		CUnit *unit=_sorts[i];
		_UpdateUnit(unit,dt);
	}

	_FinalizeBlocks();

	//走到与否的判断
	for (int i=0;i<_sorts.size();i++)
	{
		CUnit *unit=_sorts[i];
		_FinalizeReaching(unit);
	}

	_sorts.clear();

	_GarbageCollect();
}

BOOL CUnitMgr::TestUnitPath(CUnit *unit,CUnit *unitTarget)
{
// 	i_math::vector2df pos=unit->_pos;
// 
// 	CUnit *unitWork=CreateUnit(unit->_pos,unit->_radius,unit->_speed);
// 
// 	_pathTemp.clear();
// 	BOOL bEscape;
// 	if (FALSE==FindPath(unit->_tpPathFind,unit->_pos,unitTarget->_pos,_pathTemp,bEscape))
// 		return FALSE;
// 
	return FALSE;


}

void CUnitMgr::_GarbageCollect()
{
	//先清除掉not alive的units
	if (_bNeedFlushDead)
	{
		DWORD c=0;
		for (int i=0;i<_units.size();i++)
		{
			if (_units[i]->IsAlive())
				_units[c++]=_units[i];
			else
				SAFE_RELEASE(_units[i]);
		}
		_units.resize(c);

		_bNeedFlushDead=FALSE;
	}

	if (_units.size()<=0)
		return;

	DWORD nStep=_units.size()/4+1;//至少有一个step

	if (nStep>_units.size())
		nStep=_units.size();

	for (int i=0;i<nStep;i++)
	{
		int idx=(_idxGC+i)%_units.size();
		CUnit *unit=_units[idx];
		if (!unit->IsAlive())
			continue;

		//清除unit的_toMe中没有被用到的路径
		if (TRUE)
		{
			CUnitPath **p=&unit->_toMe;
			while(*p)
			{
				if ((*p)->GetRef()==1)
				{//只有一个引用计数,没有别人引用它了
					CUnitPath *next=(*p)->_next;
					CUnitPath *path=(*p);
					SAFE_DESTROY(path);

					(*p)=next;
					continue;
				}
				p=&(*p)->_next;
			}
		}
	}

	_idxGC=(_idxGC+nStep)%_units.size();

}

void DP_WritePathLink(CDataPacket &dp,CUnitPath *pathes)
{
	CUnitPath *p=pathes;
	while(p)
	{
		DP_WriteVar(dp,p);
		p=p->_next;
	}
	DP_WriteVar(dp,p);//A NULL value
}

void DP_WriteUnitTarget(CDataPacket &dp,CUnitTarget *target)
{
	if (target)
	{
		dp.Data_NextByte()=1;

		DP_WriteVar(dp,target->_rangeFollow);
		dp.Data_NextByte()=(BYTE)target->_bClosestFollow;
		dp.Data_NextByte()=(BYTE)target->_bNoStopMoveWhenInRange;
		dp.Data_NextByte()=(BYTE)target->_bFace;
		DP_WriteVar(dp,target->_rangeFace);
		DP_WriteVar(dp,target->_radFace);

		if (target->_unit)
		{
			dp.Data_NextByte()=1;
			dp.Data_WriteString(target->_unit->_nm.c_str());
		}
		else
			dp.Data_NextByte()=0;

		if (target->_pos)
		{
			dp.Data_NextByte()=1;
			DP_WriteVar(dp,target->_pos->_pos);
			DP_WritePathLink(dp,target->_pos->_toMe);
		}
		else
			dp.Data_NextByte()=0;
	}
	else
		dp.Data_NextByte()=0;
}

void CUnitMgr::Dump(CDataPacket &dp)
{
	DP_WriteVar(dp,_expandflag);
	DP_WriteVar(dp,_sortflag);
	DP_WriteVar(dp,_solveflag);

	_GarbageCollect();


	//保存一些需要被引用的数据(CUnitPath)
	if (TRUE)
	{
		std::set<CUnitPath*> pathes;

		for (int i=0;i<_units.size();i++)
		{
			if (_units[i]->_target)
			{
				if (_units[i]->_target->_pos)
				{
					CUnitPath*p=_units[i]->_target->_pos->_toMe;
					while(p)
					{
						pathes.insert(p);
						p=p->_next;
					}
				}
			}
			if (_units[i]->_path)
				pathes.insert(_units[i]->_path);
			if (_units[i]->_toMe)
			{
				CUnitPath*p=_units[i]->_toMe;
				while(p)
				{
					pathes.insert(p);
					p=p->_next;
				}
			}
		}

		if (TRUE)
		{
			dp.Data_NextDword()=pathes.size();
			std::set<CUnitPath*>::iterator it;
			for (it=pathes.begin();it!=pathes.end();it++)
			{
				CUnitPath*p=(*it);
				DP_WriteVar(dp,p);

				DP_WriteVector(dp,p->_nodes);
				DP_WriteVar(dp,p->_flags);
				DP_WriteVar(dp,p->_target);
			}
		}

	}

	dp.Data_NextDword()=_units.size();
	for (int i=0;i<_units.size();i++)
	{
		CUnit *unit=_units[i];
		dp.Data_WriteString(unit->_nm.c_str());
	}

	for (int i=0;i<_units.size();i++)
	{
		CUnit *unit=_units[i];

		DP_WriteVar(dp,unit->_radius);
		DP_WriteVar(dp,unit->_speed);
		DP_WriteVar(dp,unit->_pace);
		DP_WriteVar(dp,unit->_collide);
		DP_WriteVar(dp,unit->_pos);
		DP_WriteVar(dp,unit->_face);
		DP_WriteVar(dp,unit->_htFloating);
		DP_WriteVar(dp,unit->_closest);
		DP_WriteVar(dp,unit->_tClosest);
		DP_WriteVar(dp,unit->_dir);
		DP_WriteVar(dp,unit->_dist);
		DP_WriteVar(dp,unit->_radLastAvoid);
		DP_WriteVar(dp,unit->_flags);
		DP_WriteVar(dp,unit->_path);
		DP_WritePathLink(dp,unit->_toMe);
		DP_WriteUnitTarget(dp,unit->_target);
		DP_WriteUnitTarget(dp,unit->_targetPending);

		if (unit->_rot)
		{
			dp.Data_NextByte()=1;
			DP_WriteVar(dp,*unit->_rot);
		}
		else
			dp.Data_NextByte()=0;
	}

	dp.Data_NextDword()=0;//结束标志

}

void DP_ReadPathLink(CDataPacket &dp,CUnitPath *&pathes,
									std::map<CUnitPath*,CUnitPath*>&mapPathes)
{
	CUnitPath **p=&pathes;
	CUnitPath *key=NULL;
	while(1)
	{
		DP_ReadVar(dp,key);
		if (!key)
			break;
		std::map<CUnitPath*,CUnitPath*>::iterator it=mapPathes.find(key);
		if (it==mapPathes.end())
		{
			assert(false);
			break; 
		}
		(*p)=(*it).second;
		(*p)->AddRef();
		p=&((*p)->_next);
	}
	*p=NULL;
}

void DP_ReadUnitTarget(CDataPacket &dp,CUnitTarget *&target,std::map<CUnitPath*,CUnitPath*>&mapPathes,CUnitMgr *unitmgr)
{
	std::string s;
	if (dp.Data_NextByte()==1)
	{
		target=Class_New2(CUnitTarget);
		DP_ReadVar(dp,target->_rangeFollow);
		target->_bClosestFollow=dp.Data_NextByte();
		target->_bNoStopMoveWhenInRange=dp.Data_NextByte();
		target->_bFace=dp.Data_NextByte();
		DP_ReadVar(dp,target->_rangeFace);
		DP_ReadVar(dp,target->_radFace);

		if (dp.Data_NextByte()==1)
		{
			dp.Data_ReadString(s);
			target->_unit=unitmgr->FindUnit(s.c_str());
			SAFE_ADDREF(target->_unit);
		}
		if (dp.Data_NextByte()==1)
		{
			target->_pos=Class_New2(CUnitTargetPos);
			DP_ReadVar(dp,target->_pos->_pos);
			DP_ReadPathLink(dp,target->_pos->_toMe,mapPathes);
		}
	}
}


void CUnitMgr::Restore(CDataPacket &dp)
{
	_ClearUnits();

	DP_ReadVar(dp,_expandflag);
	DP_ReadVar(dp,_sortflag);
	DP_ReadVar(dp,_solveflag);

	//载入CUnitTargetPos/CUnitPath
	std::map<CUnitTargetPos*,CUnitTargetPos*> targets;
	std::map<CUnitPath*,CUnitPath*> pathes;
	if (TRUE)
	{
		if (TRUE)
		{
			DWORD sz=dp.Data_NextDword();
			for (int i=0;i<sz;i++)
			{
				CUnitPath *pathKey;
				DP_ReadVar(dp,pathKey);

				CUnitPath *path=Class_New2(CUnitPath);
				path->AddRef();
				pathes[pathKey]=path;

				DP_ReadVector(dp,path->_nodes);
				DP_ReadVar(dp,path->_flags);
				DP_ReadVar(dp,path->_target);

			}
		}
	}

	//载入Unit
	DWORD sz=dp.Data_NextDword();
	_units.resize(sz);

	for (int i=0;i<_units.size();i++)
	{
		CUnit *unit=Class_New2(CUnit);
		unit->AddRef();
		unit->_mgr=this;

		dp.Data_ReadString(unit->_nm);
		_units[i]=unit;
	}

	for (int i=0;i<_units.size();i++)
	{
		CUnit *unit=_units[i];

		DP_ReadVar(dp,unit->_radius);
		DP_ReadVar(dp,unit->_speed);
		DP_ReadVar(dp,unit->_pace);
		DP_ReadVar(dp,unit->_collide);
		DP_ReadVar(dp,unit->_pos);
		DP_ReadVar(dp,unit->_face);
		DP_ReadVar(dp,unit->_htFloating);
		DP_ReadVar(dp,unit->_closest);
		DP_ReadVar(dp,unit->_tClosest);
		DP_ReadVar(dp,unit->_dir);
		DP_ReadVar(dp,unit->_dist);
		DP_ReadVar(dp,unit->_radLastAvoid);
		DP_ReadVar(dp,unit->_flags);

		DP_ReadVar(dp,unit->_path);
		DP_ReadPathLink(dp,unit->_toMe,pathes);

		std::map<CUnitPath*,CUnitPath*>::iterator it2=pathes.find(unit->_path);
		if (it2!=pathes.end())
		{
			unit->_path=(*it2).second;
			unit->_path->AddRef();
		}

		DP_ReadUnitTarget(dp,unit->_target,pathes,this);
		DP_ReadUnitTarget(dp,unit->_targetPending,pathes,this);

		if (dp.Data_NextByte()==1)
		{
			unit->_rot=Class_New2(UnitRot);
			DP_ReadVar(dp,*unit->_rot);
		}


		_mp.AddUnit(unit);
	}

	//清除掉临时建立的CUnitPath 的hashmap
	if (TRUE)
	{
		if (TRUE)
		{
			std::map<CUnitPath*,CUnitPath*>::iterator it;
			for (it=pathes.begin();it!=pathes.end();it++)
			{
				CUnitPath*p=(*it).second;
				SAFE_RELEASE(p);
			}
			pathes.clear();
		}
	}

}

