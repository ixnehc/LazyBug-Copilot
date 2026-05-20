#pragma once

#include "GuiLib.h"

#include "math/vector3d.h"
#include "math/pos2d.h"
#include "math/iTypes.h"

class ICamera;
class ILight;

class GuiLib_Api CCameraController
{
public:
	CCameraController();

	virtual void SetTarget(i_math::vector3df &v);

	virtual void SetSensitiveRate(i_math::f32 rate);
	void SetVerInv(BOOL bInv);//Inverse vertically
	void SetVerRange(i_math::f32 down,i_math::f32 up);//down,up are both in degree

	void SetFocusPos(i_math::vector3df &pos);
	void ClearFocusPos();
	BOOL ResetFocus();//将camera的位置对齐到focus pos上,如果有的话


	virtual void DragBegin(int x,int y);
	virtual void DragMove(int &x,int &y);//[IN/OUT]x,y
	virtual void DragRotate(int &x,int &y);//[IN/OUT]x,y

	virtual void ZoomIn(int step);//a negative value to zoom out

	virtual void Forward(int step);//a negative value to backward
	virtual void ShiftHor(int step);//a positive value to shift right,negative value to shift left
	virtual void ShiftVer(int step);//a positive value to shift up,negative value to shift down




	virtual void UpdateCamera(ICamera *cam);
	virtual void SyncFromCamera(ICamera *cam);

protected:
	void _ClampVerDir(i_math::vector3df &vDir);//by _up,_down
	void _UpdateTarget(i_math::vector3df &vTarget,i_math::point2di &ptOff);
	void _UpdateDir(i_math::vector3df &vDir,i_math::point2di &ptOff);
	BOOL _bVerInv;
	i_math::point2di _pt;
	i_math::vector3df _vTarget;
	i_math::vector3df _vTargetCur;
	i_math::vector3df _vDir;//from src to target,in angle
	i_math::vector3df _vDirCur;//from src to target,in angle
	i_math::f32 _sens;//sensitivity
	i_math::f32 _up,_down;//vertical angle limit,in degree
	BOOL _bBeginned;

	BOOL _bFocus;
	i_math::vector3df _posFocus;

};