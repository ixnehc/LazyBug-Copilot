

#ifndef __COLLISION_DETECTION_H_INCLUDED__
#define __COLLISION_DETECTION_H_INCLUDED__

#include <math.h>
#include "irrTypes.h"
#include "vector3d.h"
#include "plane3d.h"
#include "triangle3d.h"

	
namespace irr_base
{		
		
		//// Inputs: plane origin, plane normal, ray origin ray vector.
		//// NOTE: both vectors are assumed to be normalize	
		extern  double intersect(vector3df &pOrigin, vector3df& pNormal,vector3df& vSource ,vector3df &vDir);
	
		// rO: origin of ray  rV: direction of ray
		// sO: origin of sphere , sR: radius of spher		
		extern double intersectSphere(vector3df& vOrigin,vector3df& vRay, vector3df& vCenter, double sR);
		
	//	triangle3df listTriangles[ctTriangles];
		
		extern bool collisionDetection(vector3df& vSource,vector3df& vDirection );
	
} // end namespace irr_base

#endif

