/********************************************************************
	created:	2011/6/15   10:06
	file path:	e:\IxEngine\Interfaces\WorldSystem
	author:		chenxi
	
	purpose:	dynamic local light interface
*********************************************************************/
#pragma once

struct DynLgtInfo
{
	enum Type
	{
		None,
		Point,
		Spot,
		Sight,
	};
	Type tp;
	i_math::vector3df col;
	float str;//Ç¿¶È
	i_math::matrix43f mat;
	float radius;
	i_math::vector3df dir;
	float inner,outter;
};

struct ZoneRgn;
class IDynamicLocalLight
{
public:
	INTERFACE_REFCOUNT;
	virtual BOOL GetAabb(i_math::aabbox3df &aabb,AnimTick t)=0;

	virtual DynLgtInfo::Type GetType()=0;

	virtual DynLgtInfo *GetInfo(AnimTick t)=0;

protected:

};
