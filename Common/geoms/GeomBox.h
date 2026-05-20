/********************************************************************
	created:	2011/09/15
	created:	15:9:2011   11:59
	filename: 	d:\IxEngine\Proj_Demo_Capsule_Pan\GeomBox.h
	file path:	d:\IxEngine\Proj_Demo_Capsule_Pan
	file base:	GeomBox
	file ext:	h
	Coder:		Pan
	
	purpose:	
*********************************************************************/

#pragma once



#include "GeomCommon.h"


class GeomBox
{
public:

#define NUMFACES 12
#define NUMVTX 8

	GeomBox(float height=1.0f,float length=1.0f,float depth=1.0f)
	{
		_height=height;
		_length=length;
		_depth=depth;
	}

	void SetHeight(float height=1.0f){_height=height;}
	float GetHeight()const{return _height;} 

	void SetLength(float length=1.0f){_length=length;}
	float GetLength()const{return _length;}

	void SetDepth(float depth=1.0f){_depth=depth;}
	float GetDepth()const{return _depth;}

	//generate pos
	HRESULT GenerateVerticesIndices(VtxPos *dataVb, DWORD sizeVb/*in bytes*/,  void *dataIb, DWORD sizeIb/*in bytes*/);
	
	// generate pos and normal
	HRESULT GenerateVerticesIndices(VtxPosNormal* dataVb,DWORD sizeVb/*in bytes*/,  void *dataIb, DWORD sizeIb/*in bytes*/);


	DWORD GetTotalVertices(bool normal=false) const {return normal? NUMFACES*3:NUMVTX;}
	DWORD GetTotalIndices() const  {return NUMFACES*3;}


private:

	float _height,_length,_depth;
};