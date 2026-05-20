#include "3dcDecode.h"
#include "stdh.h"
#include <emmintrin.h>

#pragma  warning(disable:4244)
#pragma  warning(disable:4101)

void _fillU8V8Data(unsigned char * const  block, unsigned char * outPointer,int w);
void _fillU8V8DataElement(unsigned char * const  block, unsigned char * outPointer,int w);
void _fillR5G6B58Data(unsigned char * const  block, unsigned char * outPointer,int w);
void _obtainR5G6B58Data(unsigned char * const  block, unsigned char * collectData);
WORD  _convertToR5G6B5(unsigned char * color);
void  Convert3DcToU8V8(const unsigned char *inBuf,void *outBuf,int w,int h)
{
	const  int loopX=h/4,loopY=w/4;
	const  int iPitchUV=w*8;
	const  int iPitch3dc=w*4;
	int i,j;
	for(i=0;i<loopX;i++)
		for(j=0;j<loopY;j++)
			_fillU8V8Data((unsigned char*)inBuf+i*iPitch3dc+16*j,(unsigned char *)outBuf+i*iPitchUV+j*8,w);
}
void _fillU8V8Data(unsigned char * const block, unsigned char * outPointer,int w)
{
	_fillU8V8DataElement(block+8,outPointer,w);  //the X element.
	_fillU8V8DataElement(block,outPointer+1,w);    //the Y element.
}
void _fillU8V8DataElement(unsigned char * const  block, unsigned char * outPointer,int w)
{
	const unsigned char max=block[0];
	const unsigned char min=block[1];
	const int iPitch=w*2;
	UINT64 index;
	index=*(UINT64*)(block+2);
	unsigned char curIndex,curVal,curLoc;
	int i=0;
	for(;i<16;i++)
	{
		UINT64 curIndex=index&0x7;
		if(curIndex==0)
			curVal=max;
		else if(curIndex==1)
			curVal=min;
		else
			curVal=((8-curIndex)*max+(curIndex-1)*min)/7;
		curLoc=i%4;
		*(outPointer+(i/4)*iPitch+curLoc*2)=curVal-128;
		index=index>>3;
	}
}

void  Convert3DcToR5G6B5(const unsigned char *inBuf,void *outBuf,int w,int h)
{
	const  int loopX=h/4,loopY=w/4;
	const  int iPitchR5G6B5=w*8;
	const  int iPitch3dc=w*4;
	int i,j;
	for(i=0;i<loopX;i++)
		for(j=0;j<loopY;j++)
			_fillR5G6B58Data((unsigned char*)inBuf+i*iPitch3dc+16*j,(unsigned char *)outBuf+i*iPitchR5G6B5+j*8,w);
}
void _fillR5G6B58Data(unsigned char * const  block, unsigned char * outPointer,int w)
{
		unsigned char  collectData[32];
		WORD color;
		int i=0,curLoc;
		const int iPitch=w*2;

		_obtainR5G6B58Data(block+8,collectData);
		_obtainR5G6B58Data(block,collectData+1);
		
		for(;i<16;i++)
		{
			color=_convertToR5G6B5(collectData+2*i);
			curLoc=i%4;
			*(WORD*)(outPointer+(i/4)*iPitch+2*curLoc)=color;
		}

}
WORD  _convertToR5G6B5(unsigned char * color)
{	 
	return ((color[0]>>3)<<11|(color[1]>>2)<<5|0);
}
void _obtainR5G6B58Data(unsigned char * const  block, unsigned char * collectData)
{
	const unsigned char max=block[0];
	const unsigned char min=block[1];
	UINT64 index;
	index=*(UINT64*)(block+2);
	unsigned char curIndex,curVal,curLoc;
	int i=0;
	for(;i<16;i++)
	{
		UINT64 curIndex=index&0x7;
		if(curIndex==0)
			curVal=max;
		else if(curIndex==1)
			curVal=min;
		else
			curVal=((8-curIndex)*max+(curIndex-1)*min)/7;
		collectData[2*i]=curVal;
		index=index>>3;
	}
}
