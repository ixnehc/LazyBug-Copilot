/********************************************************************
	created:	2006/8/26   15:38
	filename: 	d:\IxEngine\Proj_GuiLib\ArcBall.cpp
	author:		ixnehc
	
	purpose:	arc ball to control rotation & moving
*********************************************************************/
#include "stdh.h"

#include "ArcBall.h"

#include "RenderSystem/ITools.h"

#include <assert.h>

#pragma warning (disable: 4244)

CCameraController::CCameraController()
{
	_sens=1.0f;
	_up=85.0f;
	_down=-85.0f;
	_vTarget.set(4,4,4);
	_vTargetCur=_vTarget;
	_vDir.set(-1,-1,-1);
	_vDir.normalize();
	_vDir.toAngle();
	_vDirCur=_vDir;
	_bVerInv=FALSE;
	_pt.set(0,0);

	_bBeginned=FALSE;

	_bFocus=FALSE;

}

void CCameraController::SetTarget(vector3df &v)
{
	_vTargetCur=_vTarget=v;
}

void toDirection(i_math::vector3df &v)
{
	f64 fSinH = sinf(v.X*i_math::GRAD_PI2);  // heading
	f64 fCosH = cosf(v.X*i_math::GRAD_PI2);
	f64 fSinP = sinf(v.Y*i_math::GRAD_PI2);  // pitch
	f64 fCosP = cosf(v.Y*i_math::GRAD_PI2);

	v.X =(fCosP*fCosH);
	v.Y = +fSinP;
	v.Z = (fCosP*fSinH);
}

void CCameraController::SetSensitiveRate(f32 rate)
{
	_sens=rate;
}

//Inverse vertically
void CCameraController::SetVerInv(BOOL bInv)
{
	_bVerInv=bInv;
}

//by _up,_down
void CCameraController::_ClampVerDir(vector3df &vDir0)
{
	vDir0.y=clamp_f(vDir0.y,_down,_up);
}

//down,up are both in degree
void CCameraController::SetVerRange(f32 down,f32 up)
{
	_down=clampdown_f(down,-85.0f);
	_up=clampup_f(up,85.0f);
	assert(_up>=_down);

	_ClampVerDir(_vDir);
	_ClampVerDir(_vDirCur);
}

void CCameraController::DragBegin(int x,int y)
{
	_pt.set(x,y);
	_vTarget=_vTargetCur;
	_vDir=_vDirCur;
	_bBeginned=TRUE;
}

void CCameraController::_UpdateTarget(vector3df &vTarget,point2di &ptOff)
{
	vector3df up(0,1,0),vHor,vVer;
	vector3df vDir;
	vDir=_vDirCur;
	vDir.toDirection();
	vHor=up.crossProduct(vDir);
	vHor.normalize();
	vVer=vDir.crossProduct(vHor);
	vVer.normalize();
	vHor*=_sens*0.01f*(f32)ptOff.x;
	vVer*=_sens*0.01f*(f32)ptOff.y;

	vTarget-=vHor;
	vTarget-=vVer;
}

void CCameraController::DragMove(int &x,int &y)
{
	if (!_bBeginned)
		DragBegin(x,y);
	point2di ptOff;
	ptOff.set(x,y);
	ptOff-=_pt;
	ptOff.y=-ptOff.y;
	_vTargetCur=_vTarget;
	_UpdateTarget(_vTargetCur,ptOff);

}


void CCameraController::_UpdateDir(vector3df &vDir,point2di &ptOff)
{
	vDir.x+=0.2f*(f32)ptOff.x;
	vDir.y+=0.2f*(f32)ptOff.y;
	vDir.y=clamp_f(vDir.y,_down,_up);
}

void CCameraController::DragRotate(int &x,int &y)
{
	if (!_bBeginned)
		DragBegin(x,y);

	point2di ptOff;
	ptOff.set(x,y);

	ptOff-=_pt;

	ptOff.y=-ptOff.y;

	if (!_bFocus)
	{
		_vDirCur=_vDir;
		_UpdateDir(_vDirCur,ptOff);
	}
	else
	{
		if (TRUE)
		{
			//首先我们先求出_posFocus到观察方向的纵平面投影点
			i_math::vector3df nmlPlane;
			EIntersectionRelation3D relation;
			i_math::vector3df center;
			if (TRUE)
			{
				i_math::vector3df dir;
				dir=_vDir;
				dir.toDirection();
				dir.normalize();//视线方向
				nmlPlane=dir.crossProduct(i_math::vector3df(0,1,0));
				i_math::plane3df pl(_vTarget,nmlPlane);

				pl.getProjectionOf(_posFocus,center);
				relation=pl.classifyPointRelation(_posFocus);
			}

			//旋转角度
			_vDirCur=_vDir;
			_UpdateDir(_vDirCur,ptOff);

			//计算旋转的改变
			float radVer,radHor;
			radHor=(_vDirCur.x-_vDir.x)*i_math::GRAD_PI2;
			radVer=(_vDirCur.y-_vDir.y)*i_math::GRAD_PI2;

			//根据旋转的改变,修改camera的位置
			if(TRUE)
			{
				//转到_posFocus为中心的空间里
				center-=_posFocus;
				i_math::vector3df vTarget=_vTarget;
				vTarget-=_posFocus;

				i_math::vector3df axis=-center;
				if (relation==ISREL3D_BACK)
					axis=-axis;
				else
				{
					if (relation==ISREL3D_PLANAR)
						axis=nmlPlane;
				}
				axis.normalize();

				i_math::quatf qHor,qVer;
				qHor.fromAngleAxis(radHor,i_math::vector3df(0,1,0));
				qVer.fromAngleAxis(radVer,axis);

				vTarget=qVer*vTarget;
				vTarget=qHor*vTarget;

				vTarget+=_posFocus;
				_vTargetCur=vTarget;
			}
		}
	}

}

void CCameraController::ZoomIn(int step)
{
	i_math::vector3df dir=_vDirCur;
	dir.toDirection();
	dir.normalize();
	_vTargetCur=_vTargetCur-dir*_sens*0.01f*step;
	_vTarget=_vTargetCur;
	_bBeginned=FALSE;
}

//a negative value to backward
void CCameraController::Forward(int step)
{
	ZoomIn(-(step));
}

//a positive value to shift right,negative value to shift left
void CCameraController::ShiftHor(int step)
{
	i_math::point2di ptOff(-step,0);
	_vTarget=_vTargetCur;
	_UpdateTarget(_vTarget,ptOff);
	_vTargetCur=_vTarget;
	_bBeginned=FALSE;

}

//a positive value to shift up,negative value to shift down
void CCameraController::ShiftVer(int step)
{
	i_math::point2di ptOff(0,-step);
	_vTarget=_vTargetCur;
	_UpdateTarget(_vTarget,ptOff);
	_vTargetCur=_vTarget;
	_bBeginned=FALSE;
}


void CCameraController::UpdateCamera(ICamera *cam)
{   
	vector3df eye,up,hor,dir,at;
	dir=_vDirCur;
	dir.toDirection();
	dir.normalize();
	eye=_vTargetCur;
	up.set(0,1,0);
	hor=up.crossProduct(dir);
	up=dir.crossProduct(hor);

	at=eye+dir;
	cam->SetPosTarget(eye,at,up);
}

void CCameraController::SyncFromCamera(ICamera *cam)
{
	vector3df eye,dir;
	cam->GetEyePos(eye);
	cam->GetEyeDir(dir);

	SetTarget(eye);

	dir.toAngle();
	_vDir=_vDirCur=dir;

}



void CCameraController::SetFocusPos(i_math::vector3df &pos)
{
	if (_bFocus)
	{
		if (_posFocus==pos)
			return;//没有变化
	}
	_bFocus=TRUE;
	_posFocus=pos;
	_bBeginned=FALSE;
}
void CCameraController::ClearFocusPos()
{
	if (!_bFocus)
		return;
	_bFocus=FALSE;
	_bBeginned=FALSE;
}

BOOL CCameraController::ResetFocus()
{
	if (!_bFocus)
		return FALSE;

	i_math::vector3df dir;
	dir=_vDirCur;
	dir.toDirection();
	dir.normalize();
	float dist=(_posFocus-_vTargetCur).dotProduct(dir);
	if (dist<2.0f)
		dist=2.0f;
	_vTarget=_posFocus-dir*dist;
	_vTargetCur=_vTarget;
	_bBeginned=FALSE;
	return TRUE;
}
