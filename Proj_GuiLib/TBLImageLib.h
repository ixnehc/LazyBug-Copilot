
#pragma once

#include "WorldSystem/ITrrn.h"


#include <map>

class IWorldSystem;
class ITrrnBrushLib;
class IUtilRS;
class CxImage;

class IRenderSystem;
class CTBLImageLib
{
public:
	CTBLImageLib()
	{
		_brlib=NULL;
	}
	~CTBLImageLib()
	{
		Clear();
		SAFE_RELEASE(_brlib);
	}
	void Zero()
	{
		_brlib=NULL;
	}

	void SetBrLib(ITrrnBrushLib *brlib);
	void SetRS(IRenderSystem*pRS,IUtilRS *pUtilRS)	{		_pRS=pRS;	_pUtilRS=pUtilRS;}
	ITrrnBrushLib * GetBrLib()	{		return _brlib;	}
	IRenderSystem*GetRS()	{		return _pRS;	}
	IUtilRS *GetUtilRS()	{		return _pUtilRS;	}
	void Clear();

	void SyncForAll();
	void SyncForTexSet(TTSID idTexSet);

	CxImage *GetTex(TTSID idTexSet,DWORD iTex,BOOL bNS);
	DWORD GetTexCount(TTSID idTexSet);

protected:
	struct TexSet
	{
		TexSet()
		{
			memset(this,0,sizeof(*this));
		}
		void Clear();
		CxImage *images[MAX_BRUSH_TEX];
		CxImage *images_ns[MAX_BRUSH_TEX];
	};
	std::map<TTSID,TexSet> _texsets;

	ITrrnBrushLib *_brlib;
	IRenderSystem*_pRS;
	IUtilRS *_pUtilRS;


};
