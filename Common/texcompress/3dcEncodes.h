#pragma  once
/*
* inBuf data formate  B8G8R8A8		 low-------------------------------high
* outBuf data formate D3DFMT_ATIN2  maxY minY Y0---Y15 maxX minX X0----X15
* w the width of image
* h the height of image
*/
extern void  CompressBmpTo3Dc(const BYTE *inBuf,void *outBuf,int w,int h);
