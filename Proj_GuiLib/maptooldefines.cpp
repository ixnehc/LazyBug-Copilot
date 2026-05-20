
#include "stdh.h"

Gdiplus::Image* RebuildImage(void* data, DWORD sz)
{
	Gdiplus::Image * pImage = NULL;

	//检查数据
	if(!data||!sz)
		return NULL;

	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE,sz);
	IStream * pStream = NULL;
	if(hMem){
		CreateStreamOnHGlobal(hMem,TRUE,&pStream);
		if(!pStream){
			GlobalFree(hMem);
			return NULL;
		}
	}

	pStream->Write(data,sz,NULL);
	if(data&&sz>0){
		pImage = Gdiplus::Image::FromStream(pStream);
		pStream->Release();
	}

	return pImage;
}
