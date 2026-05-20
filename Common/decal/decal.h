  #pragma once

#include "../vertexfmt/vertexfmt.h"


#define MaxDecalVertices 4096

#define DecalEpsilon 0.25f


class CDecal
{
public:
	struct Triangle
	{
		unsigned short	index[3];
	};

	CDecal()
	{
		_width=0;
		_height=0;
		_nResultTri=0;
		_nResultVertex=0;
	}

	void Reset(const i_math::vector3df& center, const i_math::vector3df& normal, const i_math::vector3df& tangent, float width, float height, float depth);

	void ClipMesh(long triangleCount, const Triangle *triangle, const BYTE*vertex, const BYTE*normal,const BYTE *uv,DWORD stride);

	Triangle *GetResultTriangle(DWORD &c)
	{
		c=_nResultTri;
		return _triResult;
	}
	VtxDecal *GetResultVtx(DWORD &c)
	{
		c=_nResultVertex;
		return _vtxResult;
	}


protected:

	i_math::vector3df		_center;
	i_math::vector3df		_normal;
	i_math::vector3df		_tangent;
	i_math::vector3df		_binormal;
	float _width;
	float _height;

	i_math::vector4df		_plLeft;
	i_math::vector4df		_plRight;
	i_math::vector4df		_plBottom;
	i_math::vector4df		_plTop;
	i_math::vector4df		_plFront;
	i_math::vector4df		_plBack;

	long		_nResultVertex;
	long		_nResultTri;

	VtxDecal _vtxResult[MaxDecalVertices];
	Triangle	_triResult[MaxDecalVertices];

	void _ClipMesh(long triangleCount, const Triangle *triangle, const BYTE*vertex, const BYTE*normal,const BYTE *uv,DWORD stride);
	bool _AddPolygon(long vertexCount, const i_math::vector3df *vertex, const i_math::vector3df *normal,const i_math::vector2df *uv);
	long _ClipPolygon(long vertexCount, const i_math::vector3df *vertex, const i_math::vector3df *normal,const i_math::vector2df *uv, 
							i_math::vector3df *newVertex, i_math::vector3df *newNormal,i_math::vector2df *newuv) const;
	static long _ClipPolygonAgainstPlane(const i_math::vector4df& plane, long vertexCount, const i_math::vector3df *vertex, 
						const i_math::vector3df *normal,const i_math::vector2df *uv,
						i_math::vector3df *newVertex, i_math::vector3df *newNormal,i_math::vector2df *newuv);

};
