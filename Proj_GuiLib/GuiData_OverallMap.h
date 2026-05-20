#pragma once

#include "GuiLib.h"

#include "class/class.h"

#include "editor/editor.h"

#include "WorldSystem/IWorldSystemInterfaces.h"
#include "WorldSystem/IAssetSystemDefines.h"

struct MiniMapField
{
	DEFINE_CLASS(MiniMapField);

	DWORD len;
	std::vector<DWORD>data;

};

struct OutlineMapField
{
	DEFINE_CLASS(OutlineMapField);

	DWORD len;
	std::vector<DWORD>data;
};

class IMapFile;

struct GuiLib_Api GuiData_OverallMap:public GeData
{
	virtual const char *GetName()	{		return "overallmap";	}
	GuiData_OverallMap()
	{
		Zero();
	}
	void Zero()
	{
		ptOff.set(0,0);
		scale = 1.0f;
		pImage = NULL;
		bDrawImage=FALSE;
		mf=NULL;
	}
	void Clear();
	void Reset(IMapFile *mf);

	std::vector<i_math::pos2di> fldSels;
	i_math::pos2df ptOff;
	float scale;

	void* pImage;		// pointer type [Image *]
	BOOL bDrawImage; //记录是否绘制了简略图

	DWORD *GetFieldRawMiniMap(i_math::pos2di &ptFld,DWORD &w,DWORD &h);
	void DiscardFieldRawMiniMapCache(i_math::pos2di &ptFld);

	DWORD*GetFieldOutlineMap(i_math::pos2di &ptFld,DWORD &w,DWORD &h);
	void DiscardFieldOutlineMap(i_math::pos2di &ptFld);

	IMapFile *mf;
	std::map<i_math::pos2di,MiniMapField*> fldsRawMiniMap;
	std::map<i_math::pos2di,OutlineMapField*> fldsOutlineMap;


};


