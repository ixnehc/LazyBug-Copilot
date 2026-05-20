#include "stdh.h"

#include "GeomCapsule.h"

HRESULT GeomCapsule::GenerateVerticesIndices(VtxPosNormal *dataVB, DWORD sizeV, void *dataIB, DWORD sizeI/*FVF format*/)
{
	if (!(dataVB&&dataIB))
	{
		return S_FALSE;
	}

#define INDEX_TYPE short

	if (sizeV<_totalVertices*sizeof(VtxPosNormal)||sizeI<_totalIndices*sizeof(INDEX_TYPE))
	{
		return S_FALSE;
	}

	float radius=_dia/2;
	//DWORD numRings=radius/_span+1;



	float rad_v=i_math::Pi/2/(_numRingsOfHemisphere-1);   //´¹Ö±Æ«½Ç

	//float rad_v=i_math::Pi/2/radius/SPAN;
	float rad_h=2*i_math::Pi/_numVerticesPerRing;  //Ë®Æ½Æ«½Ç


	//Vertices Generating
	VtxPosNormal* vertices_normal=(VtxPosNormal*)dataVB;


	DWORD offset=0;

	for (DWORD iRing=0;iRing<_numRingsOfHemisphere;iRing++)
	{
		float r=radius*sin(iRing*rad_v);

		for (DWORD iVertex=0;iVertex<_numVerticesPerRing;iVertex++)
		{

			vertices_normal[_numVerticesPerRing*iRing+iVertex]._pos.x=r*cos(iVertex*rad_h);
			vertices_normal[_numVerticesPerRing*iRing+iVertex]._pos.y=radius*cos(iRing*rad_v);
			vertices_normal[_numVerticesPerRing*iRing+iVertex]._pos.z=r*sin(iVertex*rad_h);

			vertices_normal[_numVerticesPerRing*iRing+iVertex]._pos.y+=_dia*_ratio/2;

			offset++;
		}
	}


	for (DWORD iRing=0;iRing<_numRingsOfHemisphere;iRing++)
	{
		float r=radius*cos(iRing*rad_v);

		for (DWORD iVertex=0;iVertex<_numVerticesPerRing;iVertex++)
		{

			vertices_normal[offset+_numVerticesPerRing*iRing+iVertex]._pos.x=r*cos(iVertex*rad_h);
			vertices_normal[offset+_numVerticesPerRing*iRing+iVertex]._pos.y=-radius*sin(iRing*rad_v);
			vertices_normal[offset+_numVerticesPerRing*iRing+iVertex]._pos.z=r*sin(iVertex*rad_h);

			vertices_normal[offset+_numVerticesPerRing*iRing+iVertex]._pos.y-=_dia*_ratio/2;

		}
	}



	//normals Generating

	for (DWORD iRing=0;iRing<_numRingsOfHemisphere*2;iRing++)
	{
		for (DWORD iVertex=0;iVertex<_numVerticesPerRing;iVertex++)
		{

			if (iRing==0)
			{
				GeomTri tris[3];

				tris[0]._v0=vertices_normal[iRing*_numVerticesPerRing+iVertex]._pos;
				tris[0]._v1=vertices_normal[iRing*_numVerticesPerRing+(iVertex+1)%_numVerticesPerRing]._pos;
				tris[0]._v2=vertices_normal[(iRing+1)*_numVerticesPerRing+(iVertex+1)%_numVerticesPerRing]._pos;

				tris[1]._v0=vertices_normal[iRing*_numVerticesPerRing+iVertex]._pos;
				tris[1]._v1=vertices_normal[(iRing+1)*_numVerticesPerRing+(iVertex+1)%_numVerticesPerRing]._pos;
				tris[1]._v2=vertices_normal[(iRing+1)*_numVerticesPerRing+iVertex]._pos;

				tris[2]._v0=vertices_normal[iRing*_numVerticesPerRing+iVertex]._pos;
				tris[2]._v1=vertices_normal[(iRing+1)*_numVerticesPerRing+iVertex]._pos;
				tris[2]._v2=vertices_normal[iRing*_numVerticesPerRing+(iVertex-1+_numVerticesPerRing)%_numVerticesPerRing]._pos;

				CalculateVertexNormal(&vertices_normal[iRing*_numVerticesPerRing+iVertex]._normal,tris,3);

				continue;
			}

			//if (iRing==numRings-1)
			if	(iRing==_numRingsOfHemisphere*2-1)
			{
				GeomTri tris[3];

				tris[0]._v0=vertices_normal[iRing*_numVerticesPerRing+iVertex]._pos;
				tris[0]._v1=vertices_normal[iRing*_numVerticesPerRing+(iVertex-1+_numVerticesPerRing)%_numVerticesPerRing]._pos;
				tris[0]._v2=vertices_normal[(iRing-1)*_numVerticesPerRing+(iVertex-1+_numVerticesPerRing)%_numVerticesPerRing]._pos;

				tris[1]._v0=vertices_normal[iRing*_numVerticesPerRing+iVertex]._pos;
				tris[1]._v1=vertices_normal[(iRing-1)*_numVerticesPerRing+(iVertex-1+_numVerticesPerRing)%_numVerticesPerRing]._pos;
				tris[1]._v2=vertices_normal[(iRing-1)*_numVerticesPerRing+iVertex]._pos;

				tris[2]._v0=vertices_normal[iRing*_numVerticesPerRing+iVertex]._pos;
				tris[2]._v1=vertices_normal[(iRing-1)*_numVerticesPerRing+iVertex]._pos;
				tris[2]._v2=vertices_normal[iRing*_numVerticesPerRing+(iVertex+1)%_numVerticesPerRing]._pos;

				CalculateVertexNormal(&vertices_normal[iRing*_numVerticesPerRing+iVertex]._normal,tris,3);

				continue;
			}

			GeomTri tris[6];

			tris[0]._v0=vertices_normal[iRing*_numVerticesPerRing+iVertex]._pos;
			tris[0]._v1=vertices_normal[iRing*_numVerticesPerRing+(iVertex-1+_numVerticesPerRing)%_numVerticesPerRing]._pos;
			tris[0]._v2=vertices_normal[(iRing-1)*_numVerticesPerRing+(iVertex-1+_numVerticesPerRing)%_numVerticesPerRing]._pos;

			tris[1]._v0=vertices_normal[iRing*_numVerticesPerRing+iVertex]._pos;
			tris[1]._v1=vertices_normal[(iRing-1)*_numVerticesPerRing+(iVertex-1+_numVerticesPerRing)%_numVerticesPerRing]._pos;
			tris[1]._v2=vertices_normal[(iRing-1)*_numVerticesPerRing+iVertex]._pos;

			tris[2]._v0=vertices_normal[iRing*_numVerticesPerRing+iVertex]._pos;
			tris[2]._v1=vertices_normal[(iRing-1)*_numVerticesPerRing+iVertex]._pos;
			tris[2]._v2=vertices_normal[iRing*_numVerticesPerRing+(iVertex+1)%_numVerticesPerRing]._pos;

			tris[3]._v0=vertices_normal[iRing*_numVerticesPerRing+iVertex]._pos;
			tris[3]._v1=vertices_normal[iRing*_numVerticesPerRing+(iVertex+1)%_numVerticesPerRing]._pos;
			tris[3]._v2=vertices_normal[(iRing+1)*_numVerticesPerRing+(iVertex+1)%_numVerticesPerRing]._pos;

			tris[4]._v0=vertices_normal[iRing*_numVerticesPerRing+iVertex]._pos;
			tris[4]._v1=vertices_normal[(iRing+1)*_numVerticesPerRing+(iVertex+1)%_numVerticesPerRing]._pos;
			tris[4]._v2=vertices_normal[(iRing+1)*_numVerticesPerRing+iVertex]._pos;

			tris[5]._v0=vertices_normal[iRing*_numVerticesPerRing+iVertex]._pos;
			tris[5]._v1=vertices_normal[(iRing+1)*_numVerticesPerRing+iVertex]._pos;
			tris[5]._v2=vertices_normal[iRing*_numVerticesPerRing+(iVertex-1+_numVerticesPerRing)%_numVerticesPerRing]._pos;

			CalculateVertexNormal(&vertices_normal[iRing*_numVerticesPerRing+iVertex]._normal,tris,3);

		}

	}


	//Indices Generating

	unsigned short* indices=(unsigned short*)dataIB;

	DWORD i=0;
	for (WORD iRing=0;iRing<_numRingsOfHemisphere*2-1;iRing++)
	{
		for (WORD iVertex=0;iVertex<_numVerticesPerRing;iVertex++)
		{
			indices[i]=iRing*_numVerticesPerRing+iVertex;
			indices[i+1]=(iRing+1)*_numVerticesPerRing+(iVertex+1)%_numVerticesPerRing;
			indices[i+2]=(iRing+1)*_numVerticesPerRing+iVertex;

			indices[i+3]=(iRing+1)*_numVerticesPerRing+(iVertex+1)%_numVerticesPerRing;
			indices[i+4]=iRing*_numVerticesPerRing+iVertex;
			indices[i+5]=iRing*_numVerticesPerRing+(iVertex+1)%_numVerticesPerRing;

			i+=6;
		}
	}

	return S_OK;
}

HRESULT GeomCapsule::GenerateVerticesIndices(VtxPos *dataVB, DWORD sizeV, void *dataIB, DWORD sizeI/*FVF format*/)
{
	if (!(dataVB&&dataIB))
	{
		return S_FALSE;
	}

#define INDEX_TYPE short

	if (sizeV<_totalVertices*sizeof(VtxPos)||sizeI<_totalIndices*sizeof(INDEX_TYPE))
	{
		return S_FALSE;
	}

	float radius=_dia/2;
	//DWORD numRings=radius/_span+1;



	float rad_v=i_math::Pi/2/(_numRingsOfHemisphere-1);   //´¹Ö±Æ«½Ç

	//float rad_v=i_math::Pi/2/radius/SPAN;
	float rad_h=2*i_math::Pi/_numVerticesPerRing;  //Ë®Æ½Æ«½Ç


	//Vertices Generating

	VtxPos* vertices=(VtxPos*)dataVB;
	


	DWORD offset=0;

	for (DWORD iRing=0;iRing<_numRingsOfHemisphere;iRing++)
	{
		float r=radius*sin(iRing*rad_v);

		for (DWORD iVertex=0;iVertex<_numVerticesPerRing;iVertex++)
		{

			vertices[_numVerticesPerRing*iRing+iVertex].x=r*cos(iVertex*rad_h);
			vertices[_numVerticesPerRing*iRing+iVertex].y=radius*cos(iRing*rad_v);
			vertices[_numVerticesPerRing*iRing+iVertex].z=r*sin(iVertex*rad_h);

			vertices[_numVerticesPerRing*iRing+iVertex].y+=_dia*_ratio/2;

			offset++;
		}
	}


	for (DWORD iRing=0;iRing<_numRingsOfHemisphere;iRing++)
	{
		float r=radius*cos(iRing*rad_v);

		for (DWORD iVertex=0;iVertex<_numVerticesPerRing;iVertex++)
		{
			vertices[offset+_numVerticesPerRing*iRing+iVertex].x=r*cos(iVertex*rad_h);
			vertices[offset+_numVerticesPerRing*iRing+iVertex].y=-radius*sin(iRing*rad_v);
			vertices[offset+_numVerticesPerRing*iRing+iVertex].z=r*sin(iVertex*rad_h);

			vertices[offset+_numVerticesPerRing*iRing+iVertex].y-=_dia*_ratio/2;
		}
	}



	//Indices Generating

	unsigned short* indices=(unsigned short*)dataIB;

	DWORD i=0;
	for (DWORD iRing=0;iRing<_numRingsOfHemisphere*2-1;iRing++)
	{
		for (DWORD iVertex=0;iVertex<_numVerticesPerRing;iVertex++)
		{
			indices[i]=iRing*_numVerticesPerRing+iVertex;
			indices[i+1]=(iRing+1)*_numVerticesPerRing+(iVertex+1)%_numVerticesPerRing;
			indices[i+2]=(iRing+1)*_numVerticesPerRing+iVertex;

			indices[i+3]=(iRing+1)*_numVerticesPerRing+(iVertex+1)%_numVerticesPerRing;
			indices[i+4]=iRing*_numVerticesPerRing+iVertex;
			indices[i+5]=iRing*_numVerticesPerRing+(iVertex+1)%_numVerticesPerRing;

			i+=6;
		}
	}

	return S_OK;
}