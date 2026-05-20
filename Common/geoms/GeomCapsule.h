/********************************************************************
	created:	2011/09/13
	created:	13:9:2011   15:31
	filename: 	d:\IxEngine\Proj_Demo_Capsule_Pan\GeomCapsule.h
	file path:	d:\IxEngine\Proj_Demo_Capsule_Pan
	file base:	GeomCapsule
	file ext:	h
	Coder:		Pan
	
	purpose:	
*********************************************************************/
#pragma  once

#include "GeomCommon.h"


class GeomCapsule
{
public:

	//dia: dia of Capsule
	//ratio: height of Capsule/dia of Capsule
	GeomCapsule(float dia=2.0f,float ratio=2.0f)
	{
		_dia=dia;
		_ratio=ratio;


		//_span=SPAN_DEFAULT;
		_numVerticesPerRing=NUM_VERTICES_PER_RING_DEFAULT;
		_numRingsOfHemisphere=NUM_RINGS_OF_HEMISPHERE_DEFAULT;

		//_totalVertices=_numVerticesPerRing*(_dia/2/_span+1)*2;
		_totalVertices=_numVerticesPerRing*_numRingsOfHemisphere*2;
		_totalIndices=6*(_totalVertices-_numVerticesPerRing);
	}



	//Span between rings,you can regard it as LOD, the smaller value, the more detail
	/*void SetSpan(float span=SPAN_DEFAULT)
	{
		_span=span;

		_totalVertices=_numVerticesPerRing*(_dia/2/_span+1)*2;
		_totalIndices=6*(_totalVertices-_numVerticesPerRing);
	}*/

	//num of vertices per ring,you can regard it as LOD, the greater value,the more detail
	void SetNumVerticesPerRing(DWORD num=NUM_VERTICES_PER_RING_DEFAULT)
	{
		_numVerticesPerRing=num;

		_totalVertices=_numVerticesPerRing*_numRingsOfHemisphere*2;
		_totalIndices=6*(_totalVertices-_numVerticesPerRing);

	}

	//num of rings of a hemisphere,you can regard it as LOD, the greater value,the more detail
	void SetNumRingsOfHemisphere(DWORD num=NUM_RINGS_OF_HEMISPHERE_DEFAULT)
	{
		_numRingsOfHemisphere=num;

		//_totalVertices=_numVerticesPerRing*(_dia/2/_span+1)*2;
		_totalVertices=_numVerticesPerRing*_numRingsOfHemisphere*2;
		_totalIndices=6*(_totalVertices-_numVerticesPerRing);

	}

	// generate pos and normal
	HRESULT GenerateVerticesIndices(VtxPosNormal *dataVB, DWORD sizeV/*in bytes*/,  void *dataIB, DWORD sizeI/*in bytes*/);

	// generate pos
	HRESULT GenerateVerticesIndices(VtxPos* dataVB,DWORD sizeV/*in bytes*/,  void *dataIB, DWORD sizeI/*in bytes*/);

	DWORD GetTotalVertices() const {return _totalVertices;}
	DWORD GetTotalIndices() const  {return _totalIndices;}

private:

	float _dia,_ratio;
	DWORD _totalVertices,_totalIndices;

	//float _span;
	DWORD _numVerticesPerRing,_numRingsOfHemisphere;
};