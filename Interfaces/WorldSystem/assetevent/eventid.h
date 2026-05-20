/********************************************************************
	created:	2008/1/21   9:46
	file path:	d:\IxEngine\Interfaces\WorldSystem
	author:		cxi
	
	purpose:	event base
*********************************************************************/

#pragma once




//All the events id
enum AstEventID
{
	ID_ENone=0,

//Hook Event ID

	ID_HkSetWorldCenter,
	ID_HkGetWorldCenter,
	ID_HkSaveToMap,
	ID_HkReloadMap,
	ID_HkGetTerrain,
	ID_HkGetForestEditor,
	ID_HKGetWaterEditor,
	ID_HkGetGlobalEvn,
	ID_HkSetCtrlOp,
	ID_HkGetCamera,
	ID_HkGetMapPath,
	ID_HkChangeMap,
	ID_HkPostChangeMap,

	MAX_HOOKEVENT_ID,

//General Event ID
	ID_EVENT_START=2048,

	ID_ETest,

	MAX_EVENT_ID,
};


