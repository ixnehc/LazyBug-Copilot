/********************************************************************
	created:	2007/5/25   16:29
	filename: 	e:\IxEngine\Common\geosphere\gsphere.cpp
	author:		cxi
	
	purpose:	geodestic sphere creating. This file is copied from 3dsmax sdk's 
					gsphere sample,modified by cxi
*********************************************************************/

#include "stdh.h"

#include "../math/vector3d.h"
#include "../math/quaternion.h"

#include "gsphere.h"


// CONSTRUCTING THE MESH:

// To construct a geodesic sphere, we take a tetrahedron, subdivide each face into
// segs^2 faces, and project the vertices onto the sphere of the correct radius.

// This subdivision produces 3 kinds of vertices: 4 "corner" vertices, which are the
// original tetrahedral vertices; "edge" vertices, those that lie on the tetrahedron's
// edges, and "face" vertices.  There are 6 edges with (segs-1) verts on each, and
// 4 faces with (segs-1)*(segs-2)/2 verts.

// We construct these vertices in this order: the first four are the corner vertices.
// Then we use spherical interpolation to place edge vertices along each edge.
// Finally, we use the same interpolation to produce face vertices between the edge
// vertices.


// Assumed in the following function: the vertices have the same radius, or
// distance from the origin, and they have nontrivial cross product.

void SphericalInterpolate(std::vector<i_math::vector3df> &vertices,int v1, int v2,int num)
{
	int i;
	float theta, theta1, theta2, sn, cs, rad;
	i_math::vector3df a, b, c;

	if (num<2) 
		return;

	a=vertices[v1];
	b=vertices[v2];
	rad=a.dotProduct(a);

	cs=a.dotProduct(b)/rad;
	cs=i_math::clamp_f(cs,-1.0f,1.0f);
	theta=(float)acos(cs);
	sn=sinf(theta);

	for (i=1; i<num; i++)
	{
		theta1=(theta*(float)i)/(float)num;
		theta2=(theta*(float)(num-i))/(float)num;
		c=(a*sinf(theta2)+b*sinf(theta1))/sn;
		vertices.push_back(c);
	}
}

//vertice count corresponding to nSeg:
//7:492
//6:362
//5:252
//4:162
//3:92
//vertice count= (nSet^2)*10+2
//
//angleRange should be between -1 to 1

void GenScatteringDir(std::vector<i_math::vector3df>&dirs,DWORD nSeg,
					  i_math::vector3df &dirBase,float angleRange)
{
	dirs.clear();

	std::vector<i_math::vector3df>vertices;
	int i,face;


	// First 12 icosahedral vertices
	vertices.push_back(i_math::vector3df(0,1,0));
	float subz=sqrtf(.2f);
	float subrad=2*subz;
	for (face=0; face<5; face++) 
	{
		float theta=2.0f*i_math::Pi*(float)face/5.0f;
		float sn=sinf(theta);
		float cs=cosf(theta);
		vertices.push_back(i_math::vector3df(subrad*cs,subz,subrad*sn));
	}
	for (face=0; face<5; face++) 
	{
		float theta=i_math::Pi*(2.0f*(float)face+1.0f)/5.0f;
		float sn=sinf(theta);
		float cs=cosf(theta);
		vertices.push_back(i_math::vector3df(subrad*cs,-subz,subrad*sn));
	}
	vertices.push_back(i_math::vector3df(0,-1,0));

	if (TRUE)//Adjust the dir before interpolating
	{
		float angle;
		angle=dirBase.dotProduct(i_math::vector3df(0,1,0));
		if (fabsf(angle)<0.9f)
		{//只有当dirBase与(0,1,0)的夹角足够大时才需要调整
			i_math::quatf qu;
			qu.from2Vector(i_math::vector3df(0,1,0),dirBase);

			for (i=0;i<vertices.size();i++)
				vertices[i]=qu*vertices[i];
		}
	}


	// Edge vertices: 6*5*(segs-1) of these.
	for (face=0; face<5; face++) SphericalInterpolate(vertices,0,face+1,nSeg);
	for (face=0; face<5; face++) SphericalInterpolate(vertices,face+1,(face+1)%5+1,nSeg);
	for (face=0; face<5; face++) SphericalInterpolate(vertices,face+1,face+6,nSeg);
	for (face=0; face<5; face++) SphericalInterpolate(vertices,face+1,(face+4)%5+6,nSeg);
	for (face=0; face<5; face++) SphericalInterpolate(vertices,face+6,(face+1)%5+6,nSeg);
	for (face=0; face<5; face++) SphericalInterpolate(vertices,11, face+6,nSeg);
	
	// Face vertices: 4 rows of 5 faces each.
	for (face=0; face<5; face++) 
	for (i=1; i<nSeg-1; i++) 
		SphericalInterpolate(vertices,12+face*(nSeg-1)+i,12+((face+1)%5)*(nSeg-1)+i,i+1);
	for (face=0; face<5; face++) 
	for (i=1; i<nSeg-1; i++) 
		SphericalInterpolate(vertices,12+(face+15)*(nSeg-1)+i,12+(face+10)*(nSeg-1)+i,i+1);
	for (face=0; face<5; face++)
	for (i=1; i<nSeg-1; i++) 
		SphericalInterpolate(vertices,12+((face+1)%5+15)*(nSeg-1)+nSeg-2-i,12+(face+10)*(nSeg-1)+nSeg-2-i,i+1);
	for (face=0; face<5; face++)
	for (i=1; i<nSeg-1; i++)
		SphericalInterpolate (vertices,12+((face+1)%5+25)*(nSeg-1)+i,12+(face+25)*(nSeg-1)+i,i+1);

	if (angleRange<=-1.0f)
	{
		dirs=vertices;
		return;
	}

	for (int i=0;i<vertices.size();i++)
	{
		float dt=i_math::clamp_f(vertices[i].dotProduct(dirBase),-1,1);

		if (dt<angleRange)
			continue;

		dirs.push_back(vertices[i]);
 	}
}
