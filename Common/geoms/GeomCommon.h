/********************************************************************
	created:	2011/09/15
	created:	15:9:2011   17:13
	filename: 	d:\IxEngine\Proj_Demo_Capsule_Pan\GeomCommon.h
	file path:	d:\IxEngine\Proj_Demo_Capsule_Pan
	file base:	GeomCommon
	file ext:	h
	Coder:		Pan
	
	purpose:	
*********************************************************************/
#pragma  once


#include "vertexfmt/vertexfmt.h"


#define IN
#define OUT

#define SPAN_DEFAULT 0.1f
#define NUM_VERTICES_PER_RING_DEFAULT 50
#define NUM_RINGS_OF_HEMISPHERE_DEFAULT 10

struct GeomTri
{
	i_math::vector3df _v0,_v1,_v2;
};

HRESULT CalculateFaceNormal(OUT i_math::vector3df* normal,IN GeomTri* tri);



//listTri:triangle faces which the vertex is associated;
//num: num of triangles

HRESULT CalculateVertexNormal(i_math::vector3df *normal, GeomTri *tris, DWORD numTris);


//void ForTrisOfVertex(OUT,DWORD indexOfVertex,void* indexBuf,DWORD numIndices);