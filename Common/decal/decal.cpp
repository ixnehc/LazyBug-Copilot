#include "stdh.h"

#include "decal.h"


inline float DotProduct(const i_math::vector4df& p, const i_math::vector3df& q)
{
	return (p.x * q.x + p.y * q.y + p.z * q.z + p.w);
}


void CDecal::Reset(const i_math::vector3df& center, const i_math::vector3df& normal, const i_math::vector3df& tangent, float width, float height, float depth)
{
	_center = center;
	_normal = normal;

	_width=width;
	_height=height;

	i_math::vector3df binormal = tangent.crossProduct(normal);
	_tangent=tangent;
	_binormal=binormal;

	// Calculate boundary planes
	float d = center.dotProduct(tangent);
	_plLeft = i_math::vector4df(tangent.x, tangent.y, tangent.z, width * 0.5F - d);
	_plRight = i_math::vector4df(-tangent.x, -tangent.y, -tangent.z, width * 0.5F + d);

	d = center.dotProduct(binormal);
	_plTop= i_math::vector4df(binormal.x, binormal.y, binormal.z, height * 0.5F - d);
	_plBottom = i_math::vector4df(-binormal.x, -binormal.y, -binormal.z, height * 0.5F + d);

	d = center.dotProduct(normal);
	_plFront = i_math::vector4df(-normal.x, -normal.y, -normal.z, depth + d);
	_plBack = i_math::vector4df(normal.x, normal.y, normal.z, depth - d);

	// Begin with empty mesh
	_nResultVertex = 0;
	_nResultTri = 0;

}

void CDecal::ClipMesh(long nTris, const CDecal::Triangle *tri, const BYTE*vertex, const BYTE*normal,const BYTE *uv,DWORD stride)
{
	// Begin with empty mesh
	_nResultVertex = 0;
	_nResultTri = 0;

	_ClipMesh(nTris,tri,vertex,normal,uv,stride);
	// Assign texture mapping coordinates
	float one_over_w = 1.0F / _width;
	float one_over_h = 1.0F / _height;
	for (long a = 0; a < _nResultVertex; a++)
	{
		i_math::vector3df v = _vtxResult[a].pos - _center;
		float s = v.dotProduct(_tangent) * one_over_w + 0.5F;
		float t = v.dotProduct(_binormal) * one_over_h + 0.5F;
		_vtxResult[a].uv0 = i_math::vector2df(s, t);
		_vtxResult[a].binormal=_tangent.crossProduct(_vtxResult[a].normal);
		_vtxResult[a].tangent=_vtxResult[a].normal.crossProduct(_vtxResult[a].binormal);
	}

}


bool CDecal::_AddPolygon(long vertexCount, const i_math::vector3df *vertex, const i_math::vector3df *normal, const i_math::vector2df *uv)
{
	long count = _nResultVertex;
	if (count + vertexCount >= MaxDecalVertices) 
		return (false);

	// Add polygon as a triangle fan
	Triangle *triangle = _triResult + _nResultTri;
	_nResultTri += vertexCount - 2;
	for (long a = 2; a < vertexCount; a++)
	{
		triangle->index[0] = (unsigned short) count;
		triangle->index[1] = (unsigned short) (count + a - 1);
		triangle->index[2] = (unsigned short) (count + a);
		triangle++;
	}

	// Assign vertex colors
// 	float f = 1.0F / (1.0F - DecalEpsilon);
	for (long b = 0; b < vertexCount; b++)
	{
		_vtxResult[count].pos = vertex[b];
		_vtxResult[count].normal = normal[b];
		if (uv)
			_vtxResult[count].uv1=uv[b];
// 		const i_math::vector3df& n = normal[b];
// 		float alpha = (DotProduct(_normal, n) / n.length() - DecalEpsilon) * f;
// 		_colResult[count] = ColorRGBA(1.0F, 1.0F, 1.0F, (alpha > 0.0F) ? alpha : 0.0F);
		count++;
	}

	_nResultVertex = count;
	return (true);
}

void CDecal::_ClipMesh(long triangleCount, const CDecal::Triangle *triangle, 
					   const BYTE*vertex, const BYTE*normal,const BYTE *uv,DWORD stride)
{
	i_math::vector3df		newVertex[9];
	i_math::vector3df		newNormal[9];
	i_math::vector2df		newuv[9];

	// Clip one triangle at a time
	for (long a = 0; a < triangleCount; a++)
	{
		long i1 = triangle->index[0];
		long i2 = triangle->index[1];
		long i3 = triangle->index[2];

		const i_math::vector3df& v1 = *(i_math::vector3df*)&vertex[i1*stride];
		const i_math::vector3df& v2 = *(i_math::vector3df*)&vertex[i2*stride];
		const i_math::vector3df& v3 = *(i_math::vector3df*)&vertex[i3*stride];

		i_math::vector3df cross = (v2 - v1).crossProduct(v3 - v1);
		cross.normalize();
		if (_normal.dotProduct(cross) <-0.05f)
		{
			newVertex[0] = v1;
			newVertex[1] = v2;
			newVertex[2] = v3;

			newNormal[0] = *(i_math::vector3df*)&normal[i1*stride];
			newNormal[1] = *(i_math::vector3df*)&normal[i2*stride];
			newNormal[2] = *(i_math::vector3df*)&normal[i3*stride];

			if (uv)
			{
				newuv[0] = *(i_math::vector2df*)&uv[i1*stride];
				newuv[1] = *(i_math::vector2df*)&uv[i2*stride];
				newuv[2] = *(i_math::vector2df*)&uv[i3*stride];
			}		

			if (uv)
			{
				long count = _ClipPolygon(3, newVertex, newNormal,newuv, newVertex, newNormal,newuv);
				if ((count != 0) && (!_AddPolygon(count, newVertex, newNormal,newuv))) break;
			}
			else
			{
				long count = _ClipPolygon(3, newVertex, newNormal,NULL, newVertex, newNormal,NULL);
				if ((count != 0) && (!_AddPolygon(count, newVertex, newNormal,NULL))) break;
			}
		}

		triangle++;
	}
}

long CDecal::_ClipPolygon(long vertexCount, const i_math::vector3df *vertex, const i_math::vector3df *normal, 
						  const i_math::vector2df *uv,i_math::vector3df *newVertex, i_math::vector3df *newNormal,i_math::vector2df *newuv) const
{
	i_math::vector3df		tempVertex[9];
	i_math::vector3df		tempNormal[9];
	i_math::vector2df		tempuv0[9];

	i_math::vector2df *tempuv=NULL;
	if (uv)
		tempuv=tempuv0;

	// Clip against all six planes
	long count = _ClipPolygonAgainstPlane(_plLeft, vertexCount, vertex, normal, uv,tempVertex, tempNormal,tempuv);
	if (count != 0)
	{
		count = _ClipPolygonAgainstPlane(_plRight, count, tempVertex, tempNormal,tempuv, newVertex, newNormal,newuv);
		if (count != 0)
		{
			count = _ClipPolygonAgainstPlane(_plBottom, count, newVertex, newNormal, newuv,tempVertex, tempNormal,tempuv);
			if (count != 0)
			{
				count = _ClipPolygonAgainstPlane(_plTop, count, tempVertex, tempNormal, tempuv,newVertex, newNormal,newuv);
				if (count != 0)
				{
					count = _ClipPolygonAgainstPlane(_plBack, count, newVertex, newNormal, newuv,tempVertex, tempNormal,tempuv);
					if (count != 0)
					{
						count = _ClipPolygonAgainstPlane(_plFront, count, tempVertex, tempNormal,tempuv, newVertex, newNormal,newuv);
					}
				}
			}
		}
	}

	return (count);
}

long CDecal::_ClipPolygonAgainstPlane(const i_math::vector4df& plane, long vertexCount, const i_math::vector3df *vertex, const i_math::vector3df *normal, const i_math::vector2df *uv, 
										i_math::vector3df *newVertex, i_math::vector3df *newNormal, i_math::vector2df *newuv)
{
	bool	negative[10];

	// Classify vertices
	long negativeCount = 0;
	for (long a = 0; a < vertexCount; a++)
	{
		bool neg = (DotProduct(plane, vertex[a]) < 0.0F);
		negative[a] = neg;
		negativeCount += neg;
	}

	// Discard this polygon if it's completely culled
	if (negativeCount == vertexCount) return (0);

	long count = 0;
	for (long b = 0; b < vertexCount; b++)
	{
		// c is the index of the previous vertex
		long c = (b != 0) ? b - 1 : vertexCount - 1;

		if (negative[b])
		{
			if (!negative[c])
			{
				// Current vertex is on negative side of plane,
				// but previous vertex is on positive side.
				const i_math::vector3df& v1 = vertex[c];
				const i_math::vector3df& v2 = vertex[b];
				float t = DotProduct(plane, v1) / (plane.x * (v1.x - v2.x) + plane.y * (v1.y - v2.y) + plane.z * (v1.z - v2.z));
				newVertex[count] = v1 * (1.0F - t) + v2 * t;

				const i_math::vector3df& n1 = normal[c];
				const i_math::vector3df& n2 = normal[b];
				newNormal[count] = n1 * (1.0F - t) + n2 * t;

				if (uv)
				{
					const i_math::vector2df& uv1 = uv[c];
					const i_math::vector2df& uv2 = uv[b];
					newuv[count] = uv1 * (1.0F - t) + uv2 * t;
				}
				count++;
			}
		}
		else
		{
			if (negative[c])
			{
				// Current vertex is on positive side of plane,
				// but previous vertex is on negative side.
				const i_math::vector3df& v1 = vertex[b];
				const i_math::vector3df& v2 = vertex[c];
				float t = DotProduct(plane, v1) / (plane.x * (v1.x - v2.x) + plane.y * (v1.y - v2.y) + plane.z * (v1.z - v2.z));
				newVertex[count] = v1 * (1.0F - t) + v2 * t;

				const i_math::vector3df& n1 = normal[b];
				const i_math::vector3df& n2 = normal[c];
				newNormal[count] = n1 * (1.0F - t) + n2 * t;

				if (uv)
				{
					const i_math::vector2df& uv1 = uv[b];
					const i_math::vector2df& uv2 = uv[c];
					newuv[count] = uv1 * (1.0F - t) + uv2 * t;
				}

				count++;
			}

			// Include current vertex
			newVertex[count] = vertex[b];
			newNormal[count] = normal[b];
			if (uv)
				newuv[count]=uv[b];
			count++;
		}
	}

	// Return number of vertices in clipped polygon
	return (count);
}

