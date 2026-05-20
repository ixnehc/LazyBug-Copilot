#pragma  once
/*
*									low-------------------------------high
* inBuf data formate D3DFMT_ATIN2  maxY minY Y0---Y15 maxX minX X0----X15
* outBuf data formate  D3DFMT_V8U8
* w the width of image
* h the height of image
*/
extern void  Convert3DcToU8V8(const unsigned char *inBuf,void *outBuf,int w,int h);

/*
*									low-------------------------------high
* inBuf data formate D3DFMT_ATIN2  maxY minY Y0---Y15 maxX minX X0----X15
* inBuf data formate  D3DFMT_R5G6B5
* w the width of image
* h the height of image
* bRGB if the data in inbuf put away in the sequence of R G B ,bRGB will be TRUE ,else FALSE.
*/
extern void  Convert3DcToR5G6B5(const unsigned char *inBuf,void *outBuf,int w,int h);