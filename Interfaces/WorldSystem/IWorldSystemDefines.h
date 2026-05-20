/********************************************************************
	created:	2008/1/16   12:32
	filename: 	e:\IxEngine\Interfaces\WorldSystem\IWorldSystemDefines.h
	author:		cxi
	
	purpose:	exposed world system defines for the world system user
*********************************************************************/

#pragma once
#include "anim/animdefines.h"

class IFileSystem;
class IRenderSystem;
class IUtilRS;
class IKinectDevice;

//////////////////////////////////////////////////////////////////////////
//WorldSystem main


class CConfig;
struct WorldSystemInit
{
	WorldSystemInit()
	{
		pFS=NULL;
		pRS=NULL;
		pUtilRS=NULL;
		pKD=NULL;
		pathMap=pathTrrnBrushLib=pathProtoLib=pathSptLib=pathStringLib=pathHwCursor="";
		cfg=NULL;
	}
	IFileSystem *pFS;
	IRenderSystem *pRS;
	IUtilRS *pUtilRS;
	IKinectDevice *pKD;
	std::string pathDataRoot;
	std::string pathMap;
	std::string pathTrrnBrushLib;
	std::string pathTrrnBrushLibCache;
	std::string pathProtoLib;
	std::string pathSptLib;
	std::string pathSpgLib;
	std::string pathStringLib;
	std::string pathHwCursor;
	std::string pathBrushLib;

	CConfig *cfg;
};

enum WorldSystemPath
{
	WSPath_DataRoot,
	WSPath_Map,
	WSPath_TrrnBrushLib,
	WSPath_TrrnBrushLibCache,
	WSPath_ProtoLib,
	WSPath_StringLib,
	WSPath_HwCursor,//Ó²¼þÊó±ê
	WSPath_BrushLib,
	WSPath_Max,
};




