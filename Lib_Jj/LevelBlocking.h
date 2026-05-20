#pragma once

#include "class/class.h"

#include "LevelDefines.h"

struct LevelRecordUnit;
class CLevelObj;
class CLevelBlocking
{
public:
	DEFINE_CLASS(CLevelBlocking);
	CLevelBlocking()
	{
		Zero();
	}
	void Zero()
	{
		_bActive=FALSE;
		_tActivated=ANIMTICK_INFINITE;
		_tRecentBlock=ANIMTICK_INFINITE;
		_owner=NULL;
	}

	void Init(CLevelObj *owner)	{ _owner=owner;	}
	void Clear()	{		Zero();	}

	void Activate();
	void Deactiveate()	{		_bActive=FALSE;	}
	BOOL IsActive()	{		return _bActive;	}
	AnimTick GetActivatedTime()	{		return _tActivated;	}

	BOOL CanBlock(LevelPos dir,AnimTick t);
	void AddBlock(LevelPos dir,AnimTick t);


protected:
	CLevelObj *_owner;

	BOOL _bActive;
	AnimTick _tActivated;
	AnimTick _tRecentBlock;
	LevelPos _dirRecentBlock;

};
