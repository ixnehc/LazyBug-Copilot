/********************************************************************
	created:	13:4:2009   15:32
	filename: 	d:\IxEngine\Interfaces\GameLogic\IGameSystem.h
	author:		chenxi
	
	purpose:	game logic interfaces
*********************************************************************/
#pragma once
#include "IGameSystemDefines.h"

struct EntitySystemState;
class IGameSystem
{
public:
	virtual BOOL Init(EntitySystemState *ss)=0;
	virtual void UnInit()=0;
	virtual void DoClock()=0;
};
