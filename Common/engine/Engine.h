
#pragma once

#include "../math/imath_all.h"

#include "WorldSystem/IAssetSystemDefines.h"
#include "WorldSystem/IEntitySystemDefines.h"
#include "WorldSystem/IMapFileArgs.h"

#include "../strlib/strlib.h"
#include "../config/config.h"


#define MAKE_MAP_PATH(__pWS,__path,__pathFull)															\
if (!IsFullPath(__path))																											\
{																																				\
	__pathFull=__pWS->GetPath(WSPath_Map);																	\
	__pathFull=__pathFull+"\\"+(__path);																				\
}																																				\
else																																			\
	__pathFull=__path;


class IFileSystem;
struct EngineParam
{
	EngineParam()
	{
	}
};
class CEngine
{
public:
	CEngine()
	{
		_pFS=NULL;
	}

	BOOL Init(EngineParam &param);
	void UnInit();

	IFileSystem *GetFS()	{		return _pFS;	}

	CStrLib *GetStrLib()	{		return &_strlib;	}

protected:

	IFileSystem *_pFS;

	CStrLib _strlib;
};

extern CEngine g_Engine;

