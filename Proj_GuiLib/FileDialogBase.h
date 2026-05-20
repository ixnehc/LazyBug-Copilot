#pragma once

#include "GuiLib.h"

const char * FD_BrowseTex(BOOL bOpen,const char * pathRoot=NULL);
class CxImage;
BOOL FD_BrowseImage(BOOL bOpen,CxImage *image);

class IRenderSystem;
GuiLib_Api const char * FD_BrowseTex2(IRenderSystem* pRS, const char * pathRoot, BOOL bAllowPartSelect,const char *lastsel);
GuiLib_Api const char * FD_BrowseResource(DWORD resfilter = 0,const char *pathDef ="");//如果cancel了,返回""
GuiLib_Api const char * FD_BrowseProto(BOOL bOpen,const char * pathRoot);
