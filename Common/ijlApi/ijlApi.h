#pragma once

#pragma comment(lib, "ijl15.lib")


//dest/szDest should be a big enough buffer
BOOL IJL_Encode565(BYTE *dest,DWORD &szDest,WORD *src,DWORD w,DWORD h,int quality=75);

//if dest is NULL,in w/h will returned the dest buffer w/h 
BOOL IJL_Decode565(WORD *dest,DWORD &w,DWORD &h,BYTE *src,DWORD szSrc);

BOOL IJL_ReadImageInfo(DWORD &w,DWORD &h,DWORD &channel,const char * name);
BOOL IJL_ReadImageInfo(DWORD &w,DWORD &h,DWORD &channel,BYTE *data,DWORD szData);

