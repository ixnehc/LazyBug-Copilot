#pragma once

#include "GuiLib.h"

#include <string>
#include <vector>

class CxImage;
class IUtilRS;
class ITexture;
class IRenderSystem;
struct TexData;
//Note: the tex data is assumed containing a 1-frame tex
CxImage *ImageFromTexData(IRenderSystem*pRS,TexData *td,CxImage *container=NULL);

CxImage *ImageFromBMP(UINT idBitmap,CxImage *container=NULL);

CxImage *ImageFromTGA(UINT idTga,CxImage *container=NULL);

CxImage *ImageFromTex(ITexture*tex,CxImage *container=NULL);



GuiLib_Api void SaveImage(DWORD *col,DWORD w,DWORD h,const char *fn);
GuiLib_Api DWORD LoadImage(DWORD *col,DWORD &w,DWORD &h,const char *fn);
GuiLib_Api DWORD LoadImageTga(DWORD *col,DWORD &w,DWORD &h,const char *fn);

GuiLib_Api void SaveAliasePattern(DWORD base);
GuiLib_Api void LoadAliasePattern(DWORD base,const char *fn);
