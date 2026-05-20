#pragma once

#include "editor/editor.h"

//World Editor Events
#define WEE_GetSelAssetClass 1


class IWorldSystem;
class IMapFile2;
class ITrrnBrushLib;
class ITrrnMap;
class IAssetSystem;
struct WEditorEnv:public EditorEnv
{
	WEditorEnv()
	{
		memset(this,0,sizeof(*this));
	}

	IWorldSystem *pWS;
	IMapFile2 *mapfile;
	ITrrnBrushLib *brlib;
	ITrrnMap *trrnmap;
	IAssetSystem *pAS;
};
