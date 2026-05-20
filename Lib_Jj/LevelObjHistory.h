#pragma once

#include "LevelEventSrc.h"

class CLevelObjHistory:public CLevelEventSrc
{
public:
	CLevelObjHistory()
	{
		_t=0;
		_dur=ANIMTICK_FROM_SECOND(0.2f);
	}

	void SetDur(AnimTick dur)
	{
		_dur=dur;
	}

	void Update(AnimTick dt)
	{
		_t+=dt;
	}

	void Add(LevelObjID id)
	{
		CLevelEventSrc::Add(LET_None,id,_t);
	}
	BOOL Exist(LevelObjID id)
	{
		AnimTick tAfter=_t;
		tAfter=ANIMTICK_SAFE_MINUS(_t,_dur);
		return CLevelEventSrc::Exist(LET_None,id,tAfter);
	}

protected:
	AnimTick _t;
	AnimTick _dur;
};

