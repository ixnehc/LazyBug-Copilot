#include "stdh.h"
#include "GeomCommon.h"


HRESULT CalculateFaceNormal(OUT i_math::vector3df* normal,IN GeomTri* tri)
{
	if (tri==NULL)
	{
		return S_FALSE;
	}

	i_math::vector3df normalFace;
	//normalFace=(tri->_v1-tri->_v0).crossProduct(tri->_v2-tri->_v1);
	normalFace=(tri->_v1-tri->_v0).crossProduct(tri->_v2-tri->_v0);
	normalFace.normalize();
	*normal=normalFace;
	return S_OK;
}


HRESULT CalculateVertexNormal(i_math::vector3df *normal, GeomTri *tris, DWORD numTris)
{
	if (tris==NULL)
	{
		return S_FALSE;
	}

	if (normal==NULL)
	{
		return S_FALSE;
	}

	i_math::vector3df normalVertex;

	for (int iTri=0;iTri<numTris;iTri++)
	{
		i_math::vector3df normalFace;
		CalculateFaceNormal(&normalFace,&tris[iTri]);
		normalVertex+=normalFace;
	}

	normalVertex.normalize();

	*normal=normalVertex;

	return S_OK;
}