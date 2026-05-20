#pragma once


#include "../math/imath_all.h"
#include "../math/volume.h"
#include "../math/triangle3d.h"

struct SpacialTester
{
	enum TesterType
	{
		None,
		Box,
		Rect,
		Line,
		Sphere,
		Frustum,
	};
	enum Result
	{
		NoTouch=0,//the tester has no touch with the target
		Intersect=1,//the tester intersects with the target
		Contain=2,//the tester contains the target
		ForceDword = 0xffffffff,
	};
	TesterType type;

	i_math::aabbox3df aabb;
	i_math::rectf rc;
	i_math::line3df line;
	i_math::spheref sph;
	i_math::volumeCvxf vol;

	SpacialTester()
	{
		Set(i_math::aabbox3df());
	}
	BOOL Equals(SpacialTester &other);
	void Set(i_math::aabbox3df &aabb0)
	{
		type=Box;
		aabb=aabb0;
	}
	void Set(i_math::rectf &rc0)
	{
		type=Rect;
		rc=rc0;
	}
	void Set(i_math::line3df &line0)
	{
		type=Line;
		line=line0;
	}
	void Set(i_math::spheref &sph0)
	{
		type=Sphere;
		sph=sph0;
	}
	void SetFrustum(i_math::volumeCvxf &vol0)
	{
		type=Frustum;
		vol=vol0;
	}

	BOOL Translate(i_math::vector3df &pos);

	BOOL GetAABB(i_math::aabbox3df &aabb);

	Result Test(i_math::aabbox3df &aabb);
	Result Test(i_math::vector3df &v);
	Result Test(SpacialTester &other);
	Result Test(i_math::vector3df &pos,float radius);
	Result Test(i_math::matrix43f &mat,i_math::aabbox3df &aabb);
	Result Test(i_math::triangle3df &tri);
};




