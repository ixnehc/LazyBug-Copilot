/********************************************************************
	created:	2010/4/19   13:31
	file path:	d:\IxEngine\Interfaces\WorldSystem
	author:		chenxi
	
	purpose:	Anim Tree Ctrl interfaces
*********************************************************************/

#pragma once

#include "anim/animdefines.h"

class IMatrice43;
class IRagdollCtrl
{
public:
	INTERFACE_REFCOUNT;

	virtual BOOL Tick(AnimTick t)=0;
	virtual BOOL Calc(AnimTick t,IMatrice43 *mats)=0;//注意:返回的mats是在世界空间里的
	virtual BOOL Calc(AnimTick t,i_math::matrix43f &mat)=0;

};
