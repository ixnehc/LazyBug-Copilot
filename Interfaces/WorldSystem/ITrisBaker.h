/********************************************************************
	created:	2011/8/15   9:57
	file path:	e:\IxEngine\Interfaces\WorldSystem
	author:		chenxi
	
	purpose:	tiangles map baking
*********************************************************************/
#pragma once


class CProgress;
class ITrisBaker
{
public:
	INTERFACE_REFCOUNT;

	virtual BOOL Bake(i_math::recti &rc)=0;//rc单位为米

};

