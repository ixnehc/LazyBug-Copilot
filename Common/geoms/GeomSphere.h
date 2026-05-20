/********************************************************************
	created:	2011/09/15
	created:	15:9:2011   16:44
	filename: 	d:\IxEngine\Proj_Demo_Capsule_Pan\GeomSphere.h
	file path:	d:\IxEngine\Proj_Demo_Capsule_Pan
	file base:	GeomSphere
	file ext:	h
	Coder:		Pan
	
	purpose:	
*********************************************************************/
#pragma once

#include "GeomCommon.h"


class GeomSphere
{
public:
	GeomSphere(float radius=1.0f)
	{
		_radius=radius;

		//_span=SPAN_DEFAULT;
		_numVerticesPerRing=NUM_VERTICES_PER_RING_DEFAULT;
		_numRingsOfHemisphere=NUM_RINGS_OF_HEMISPHERE_DEFAULT;

		_totalVertices=_numVerticesPerRing*(_numRingsOfHemisphere*2-1);
		_totalIndices=6*(_totalVertices-_numVerticesPerRing);
	}


	//Span between rings,you can regard it as LOD, the smaller value, the more detail
	/*void SetSpan(float span=0.1f)
	{
		_span=span;

		_totalVertices=_numVerticesPerRing*(_radius*2/_span+1);
		_totalIndices=6*(_totalVertices-_numVerticesPerRing);
	}*/

	//num of vertices per ring,you can regard it as LOD, the greater value,the more detail
	void SetNumVerticesPerRing(WORD num=NUM_VERTICES_PER_RING_DEFAULT)
	{
		_numVerticesPerRing=num;

		_totalVertices=_numVerticesPerRing*(_numRingsOfHemisphere*2-1);
		_totalIndices=6*(_totalVertices-_numVerticesPerRing);

	}

	//num of rings of a hemisphere,you can regard it as LOD, the greater value,the more detail
	void SetNumRingsOfHemisphere(WORD num=NUM_RINGS_OF_HEMISPHERE_DEFAULT)
	{
		_numRingsOfHemisphere=num;

		_totalVertices=_numVerticesPerRing*(_numRingsOfHemisphere*2-1);
		_totalIndices=6*(_totalVertices-_numVerticesPerRing);
	}


	// generate pos and normal
	HRESULT GenerateVerticesIndices(VtxPosNormal *dataVB, DWORD sizeV/*in bytes*/,  void *dataIB, DWORD sizeI/*in bytes*/);
	
	// generate pos
	HRESULT GenerateVerticesIndices(VtxPos* dataVB,DWORD sizeV/*in bytes*/,  void *dataIB, DWORD sizeI/*in bytes*/);

	DWORD GetTotalVertices() const {return _totalVertices;}
	DWORD GetTotalIndices() const  {return _totalIndices;}

private:

	float _radius;
	DWORD _totalVertices,_totalIndices;

	//float _span;
	WORD _numVerticesPerRing,_numRingsOfHemisphere;
};