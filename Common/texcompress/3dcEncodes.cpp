#include "stdh.h"
#include "3dcEncodes.h"
#include <emmintrin.h>
#include "bitset/bitset.h"
#include <stdlib.h>

#pragma  warning(disable:4244)
//get split cell buffer.
void _ExtractBlock(const BYTE *buffer,int w,BYTE *block);
void _ExtractColorInch(const BYTE *block,float *inch);
void _Fill3dcData(const BYTE * block,const float *inchX,const float *inchY,BYTE *outBuf);
void _FillIndice(const BYTE *block,const float *inchX,const float *inchY,BYTE * index);
void  CompressBmpTo3Dc(const BYTE *inBuf,void *outBuf,int w,int h)
{
	int i=0,j=0;
	float inchX[8];
	float inchY[8];
	BYTE block[64];
	const int loopW=w/4;
	const int loopH=h/4;
	const int widthBitLen=w*4;
	const int outBufSize=w*h;
	memset(outBuf,0,outBufSize);
	for(;i<loopH;i++)
		for(j=0;j<loopW;j++)
		{ 
			_ExtractBlock(inBuf+i*64*loopW+16*j,w,block);
			_ExtractColorInch(block+2,inchX);
			_ExtractColorInch(block+1,inchY);
			_Fill3dcData(block,inchX,inchY,(BYTE*)outBuf+i*16*loopW+j*16);
		}
}
void _FillIndice(const BYTE *block,const float *inchX,const float *inchY,BYTE * index)
{
	
	int i=0;
	int j=0;
	float minXDist,minYDist,iDist;
	DWORD iIndexX=0,iIndexY=0;
	float bX,bY;
	DWORD iIndexMergeX[16],iIndexMergeY[16];
	DWORD  indexBlock;

	memset(iIndexMergeX,0,sizeof(iIndexMergeX));
	memset(iIndexMergeY,0,sizeof(iIndexMergeY));
	for(i=0;i<16;i++)
	{
		minXDist=255;
		minYDist=255;
		bX=block[4*i+2];
		bY=block[4*i+1];
		for(j=0;j<8;j++)
		{
			iDist=abs(bX-inchX[j]);
			if(iDist<minXDist) 
			{
				iIndexX=j;
				minXDist=iDist;
			}
			iDist=abs(bY-inchY[j]);
			if(iDist<minYDist) 
			{
				iIndexY=j;
				minYDist=iDist;
			}
		}
		iIndexMergeX[i]=iIndexX;
		iIndexMergeY[i]=iIndexY;
	}

	BYTE  maxX=inchX[0],maxY=inchY[0],minX=inchX[1],minY=inchY[1];

	if(TRUE)
	{
		//0-DWORD
		indexBlock=0;
		indexBlock|=iIndexMergeX[15]<<29;
		indexBlock|=iIndexMergeX[14]<<26;
		indexBlock|=iIndexMergeX[13]<<23;
		indexBlock|=iIndexMergeX[12]<<20;
		indexBlock|=iIndexMergeX[11]<<17;
		indexBlock|=iIndexMergeX[10]<<14;
		indexBlock|=iIndexMergeX[9]<<11;
		indexBlock|=iIndexMergeX[8]<<8;
		indexBlock|=iIndexMergeX[7]<<5;
		indexBlock|=iIndexMergeX[6]<<2;
		indexBlock|=iIndexMergeX[5]>>1;
		*(DWORD*)(index+12)=indexBlock;

		//1-DWORD
		indexBlock=0;
		indexBlock|=iIndexMergeX[5]<<31;
		indexBlock|=iIndexMergeX[4]<<28;
		indexBlock|=iIndexMergeX[3]<<25;
		indexBlock|=iIndexMergeX[2]<<22;
		indexBlock|=iIndexMergeX[1]<<19;
		indexBlock|=iIndexMergeX[0]<<16;
		*(DWORD*)(index+8)=indexBlock;
		index[9]=minX;
		index[8]=maxX;
	  
		//2-DWORD
		indexBlock=0;
		indexBlock|=iIndexMergeY[15]<<29;
		indexBlock|=iIndexMergeY[14]<<26;
		indexBlock|=iIndexMergeY[13]<<23;
		indexBlock|=iIndexMergeY[12]<<20;
		indexBlock|=iIndexMergeY[11]<<17;
		indexBlock|=iIndexMergeY[10]<<14;
		indexBlock|=iIndexMergeY[9]<<11;
		indexBlock|=iIndexMergeY[8]<<8;
		indexBlock|=iIndexMergeY[7]<<5;
		indexBlock|=iIndexMergeY[6]<<2;
		indexBlock|=iIndexMergeY[5]>>1;
		*(DWORD*)(index+4)=indexBlock;

		//3-DWORD
		indexBlock=0;
		indexBlock|=iIndexMergeY[5]<<31;
		indexBlock|=iIndexMergeY[4]<<28;
		indexBlock|=iIndexMergeY[3]<<25;
		indexBlock|=iIndexMergeY[2]<<22;
		indexBlock|=iIndexMergeY[1]<<19;
		indexBlock|=iIndexMergeY[0]<<16;
		*(DWORD*)(index)=indexBlock;
		index[1]=minY;
		index[0]=maxY;

	}
}

void _Fill3dcData(const BYTE * block,const float *inchX,const float *inchY,BYTE *outBuf)
{
	_FillIndice(block,inchX,inchY,outBuf);
}
void _ExtractBlock(const BYTE *buffer,int w,BYTE *block)
{
	int i=0;
	for(;i<4;i++)
		memcpy(block+i*16,buffer+i*4*w,16);
}
void _ExtractColorInch(const BYTE *block,float *inch)
{
	BYTE i=0,_sub;
	float cur,step;
	BYTE maxColor=*block;
	BYTE minColor=*block;
	for(;i<16;i++)
	{	
		if(*(block+4*i)>maxColor)
			maxColor=*(block+4*i);
		if(*(block+4*i)<minColor)
			minColor=*(block+4*i);
	}
	_sub=maxColor-minColor;
	step=(float)_sub/7;  
	inch[0]=maxColor;
	inch[1]=minColor;
	inch[2]=maxColor-step;
	i=3;
	for(;i<8;i++)
	{	
		cur=inch[i-1]-step;
		inch[i]=cur;	   
	}
}

