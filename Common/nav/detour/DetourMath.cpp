
#include "stdh.h"

#include "DetourMath.h"

dtVec3 operator *(const dtVec3 &v,float s)
{
	return dtVec3(v.x*s,v.y*s,v.z*s);
}

dtVec3 operator *(float s,const dtVec3 &v)
{
	return dtVec3(v.x*s,v.y*s,v.z*s);
}
