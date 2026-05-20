
#include "stdh.h"
#include "LevelEventMap.h"

#include "commondefines/general_stl.h"

#include "timer/profiler.h"

#include "Level.h"
#include "LevelObj.h"


void CLevelEventMap::Create()
{

}

void CLevelEventMap::Destroy()
{
	_mp.DiscardAll();
	_mp.Reset();

	for (int i=0;i<2;i++)
		_events[i].Clear();

	Zero();
}

#define BEGIN_ADD_EVENT()																												\
i_math::recti rcTile;																												\
i_math::rectf rcRange;																												\
rcRange.set(pos.x,pos.y,pos.x,pos.y);																												\
rcRange.inflate(radius,radius,radius,radius);																												\
rcTile.Left()=(int)(rcRange.Left()/LEVELEVENTMAP_TILE_LEN);																												\
rcTile.Top()=(int)(rcRange.Top()/LEVELEVENTMAP_TILE_LEN);																												\
rcTile.Right()=(int)(rcRange.Right()/LEVELEVENTMAP_TILE_LEN)+1;																												\
rcTile.Bottom()=(int)(rcRange.Bottom()/LEVELEVENTMAP_TILE_LEN)+1;																												\
for (int j=rcTile.Top();j<rcTile.Bottom();j++)																												\
for (int i=rcTile.Left();i<rcTile.Right();i++)																												\
{																												\
	LevelEventTile *tile=_mp.Obtain(i,j);																												\
	LevelEventQueue *qu=&tile->qu[_flip];																												\
	if (qu->iFrame!=_iFrame)																												\
	{																												\
		qu->Clear();																												\
		qu->iFrame=_iFrame;																												\
	}

#define END_ADD_EVENT()																												\
}


void CLevelEventMap::_AddEvent(LevelEvent *e,LevelPos &pos,float radius)
{
	BEGIN_ADD_EVENT();
		qu->events.push_back(e);
	END_ADD_EVENT();
}

void CLevelEventMap::AddEventFlag(LevelEventMapFlag flag,LevelPos &pos,float radius)
{
	BEGIN_ADD_EVENT();
	qu->Add(flag);
	END_ADD_EVENT();
}

void CLevelEventMap::AddPlayerKilling(LevelPlayerID idPlayer,LevelPos &pos,float radius)
{
	BEGIN_ADD_EVENT();
		qu->AddPlayerKilling(idPlayer);
	END_ADD_EVENT();
}
void CLevelEventMap::AddUnitKilling(LevelPlayerID idPlayer,LevelPos &pos,float radius)
{
	BEGIN_ADD_EVENT();
	qu->AddUnitKilling(idPlayer);
	END_ADD_EVENT();
}


LevelEventQueue *CLevelEventMap::_GetEventQueue(LevelPos &pos)
{
	int xTile=(int)floor(pos.x/LEVELEVENTMAP_TILE_LEN);
	int yTile=(int)floor(pos.y/LEVELEVENTMAP_TILE_LEN);

	LevelEventTile *tile=_mp.Get(xTile,yTile);
	if (!tile)
		return NULL;

	LevelEventQueue *qu=&tile->qu[1-_flip];
	if (qu->iFrame!=_iFrame-1)
	{
		qu->Clear();
		qu->iFrame=_iFrame-1;
		return NULL;
	}
	return qu;
}


LevelEvent **CLevelEventMap::GetEvents(LevelPos &pos,DWORD &count)
{
	count=0;

	LevelEventQueue *qu=_GetEventQueue(pos);
	if (!qu)
		return NULL;

	count=qu->events.size();
	return &qu->events[0];
}

LevelEventMapFlag CLevelEventMap::GetEventFlags(LevelPos &pos)
{
	LevelEventQueue *qu=_GetEventQueue(pos);
	if (!qu)
		return NULL;

	return qu->flags;
}

LevelPlayerMask CLevelEventMap::GetPlayerKilling(LevelPos &pos)
{
	LevelEventQueue *qu=_GetEventQueue(pos);
	if (!qu)
		return 0;

	return qu->ePlayerKilling;
}

LevelPlayerMask CLevelEventMap::GetUnitKilling(LevelPos &pos)
{
	LevelEventQueue *qu=_GetEventQueue(pos);
	if (!qu)
		return 0;

	return qu->eUnitKilling;
}

void CLevelEventMap::AddSignal(StringID nm,LevelPos &pos,float radius,LevelObjID idSender)
{
	_signals[_flip].Add(nm,pos,radius,idSender);
}

void CLevelEventMap::AddSignal(StringID nm,LevelObjID idLo,LevelObjID idSender)
{
	_signals[_flip].Add(nm,idLo,idSender);
}

BOOL CLevelEventMap::TestSignal(StringID nm,LevelObjID idLo,LevelPos &pos)
{
	return _signals[1-_flip].Test(nm,idLo,pos);
}

LevelSignals::Signal *CLevelEventMap::FindSignal(StringID nm,LevelObjID idLo,LevelPos &pos)
{
	return _signals[1-_flip].Find(nm,idLo,pos);
}

LevelSignals::Signal **CLevelEventMap::GetSignals(LevelObjID idLo,LevelPos &pos,DWORD &nSignals)
{
	_signals[1-_flip].Get(idLo,pos,_temp);
	nSignals=_temp.size();
	return _temp.data();
}
