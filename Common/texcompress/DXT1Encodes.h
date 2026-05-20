#pragma once
/*
** inBuf data formate  D3DFMT_A8B8G8R8
* outBuf data formate D3DFMT_DXT1
* w the width of image
* h the height of image
*/
extern void  CompressBmpToDXT1(const BYTE *inBuf,void *outBuf,int w,int h);

//inBuf格式是565格式
extern void CompressBmp565ToDXT1(const BYTE *inBuf,void *outBuf,int w,int h);