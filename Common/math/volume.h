#pragma once

#include "iMath.h"
#include "vector3d.h"
#include "plane3d.h"
#include "matrix44.h"
#include "aabbox3d.h"

#include <vector>

#define MAX_VOLUME_PLANES 32

namespace i_math
{

template <class T>
class volumeCvx//Convex volume
{
public:

	volumeCvx()
	{
		nPlanes=0;
	}

	bool isEmpty()
	{
		return nPlanes==0;
	}

	void clear()
	{
		nPlanes=0;
	}

	bool addPlane(i_math::plane3d<T> &pl)
	{
		if (nPlanes>=MAX_VOLUME_PLANES)
			return false;

		planes[nPlanes]=pl;
		nPlanes++;

		return true;
	}

	inline volumeCvx<T>& operator+=(const vector3d<T>& point)
	{
		for (DWORD i=0;i<nPlanes;i++)
			planes[i]+=point;
		return *this;
	}

	bool operator==(const volumeCvx<T>& other) const 
	{
		if (nPlanes!=other.nPlanes)
			return false;
		for (DWORD i=0;i<nPlanes;i++)
		{
			if (!(planes[i]==other.planes[i]))
				return false;
		}
		return true;
	}




	//check whether the aabb is outside or inside or clipping this volume
	//return ISREL3D_FRONT if outside,ISREL3D_BACK if inside,ISREL3D_CLIPPED if clipping
	EIntersectionRelation3D classifyAABB(aabbox3d<T> &aabb)
	{
		EIntersectionRelation3D r=ISREL3D_BACK;
		for (int i=0;i<(int)nPlanes;i++)
		{
			EIntersectionRelation3D r2=aabb.classifyPlaneRelation(planes[i]);

			if (r2==ISREL3D_FRONT)
				return ISREL3D_FRONT;
			if (r2==ISREL3D_CLIPPED)
				r=ISREL3D_CLIPPED;
		}
		return r;
	}

	EIntersectionRelation3D classifyPoint(i_math::vector3df &pos)
	{
		EIntersectionRelation3D r=ISREL3D_BACK;
		for (int i=0;i<(int)nPlanes;i++)
		{
			EIntersectionRelation3D r2=planes[i].classifyPointRelation(pos);

			if (r2==ISREL3D_FRONT)
				return ISREL3D_FRONT;
			if (r2==ISREL3D_PLANAR)
				r=ISREL3D_PLANAR;
		}
		return r;
	}


	void fromViewProj(matrix44<T> &viewproj)
	{
		nPlanes=6;

		matrix44<T> &m=viewproj;
		_makeFrustumPlane(m.m02,m.m12,m.m22,m.m32,planes[0]);//near
		_makeFrustumPlane(m.m03-m.m02,m.m13-m.m12,m.m23-m.m22,m.m33-m.m32,planes[1]);//far
		_makeFrustumPlane(m.m03+m.m00,m.m13+m.m10,m.m23+m.m20,m.m33+m.m30,planes[2]);//left
		_makeFrustumPlane(m.m03-m.m00,m.m13-m.m10,m.m23-m.m20,m.m33-m.m30,planes[3]);//right
		_makeFrustumPlane(m.m03-m.m01,m.m13-m.m11,m.m23-m.m21,m.m33-m.m31,planes[4]);//top
		_makeFrustumPlane(m.m03+m.m01,m.m13+m.m11,m.m23+m.m21,m.m33+m.m31,planes[5]);//bottom
	}

	plane3d<T> planes[MAX_VOLUME_PLANES];//NOTE:each plane should face to outside
	DWORD nPlanes;

protected:
	bool _makeFrustumPlane(T a,T b,T c,T d,plane3d<T> &plane)
	{
		f64 length2= a*a+b*b+c*c;
		if(length2> ROUNDING_ERROR*ROUNDING_ERROR)
		{
			T invlength= (T)(1.0f / sqrt(length2));
			plane.setPlane(vector3d<T>(-a*invlength,-b*invlength,-c* invlength),d*invlength);
			return true;
		}
		else
			return false;
	}
};

	typedef volumeCvx<f32> volumeCvxf;
	typedef volumeCvx<f64> volumeCvxd;

} 

