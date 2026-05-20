/********************************************************************
	created:	2008/1/22   13:32
	file path:	d:\IxEngine\Interfaces\WorldSystem
	author:		cxi
	
	purpose:	implement for the events used in worldsystem._dll(core package)
*********************************************************************/

#include "stdh.h"

#include "../assetevent/eventid.h"

#include "../IAssetEventer.h"


//////////////////////////////////////////////////////////////////////////
//Hook Event
IMPLEMENT_EVENT(HkSetWorldCenter);
IMPLEMENT_EVENT(HkGetWorldCenter);
IMPLEMENT_EVENT(HkSaveToMap);
IMPLEMENT_EVENT(HkReloadMap);
IMPLEMENT_EVENT(HkGetTerrain);
IMPLEMENT_EVENT(HkGetForestEditor);
IMPLEMENT_EVENT(HKGetWaterEditor);
IMPLEMENT_EVENT(HkSetCtrlOp);
IMPLEMENT_EVENT(HkGetCamera);
IMPLEMENT_EVENT(HkGetMapPath);
IMPLEMENT_EVENT(HkChangeMap);
IMPLEMENT_EVENT(HkPostChangeMap);



//////////////////////////////////////////////////////////////////////////
//General Events

IMPLEMENT_EVENT(ETest);
