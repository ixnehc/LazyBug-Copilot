#pragma once

#include "LevelDefines.h"

#include "class/class.h"

#include "sparsearray/sparsearray2D.h"

#define LEVELEVENTMAP_TILE_LEN (4.0f)//单位为米

#define EVENT_DATA_BUFFER_SIZE 4096


struct LevelEventQueue
{
	LevelEventQueue()
	{
		iFrame=0;
		flags=0;
		ePlayerKilling=eUnitKilling=0;
	}

	void Clear()
	{
		events.clear();
		flags=0;
		ePlayerKilling=eUnitKilling=0;
	}

	void Add(LevelEvent *e)
	{
		events.push_back(e);
	}
	void Add(LevelEventMapFlag flag)
	{
		flags|=flag;
	}
	void AddPlayerKilling(LevelPlayerID idPlayer)	{		ePlayerKilling|=(1<<idPlayer);	}
	void AddUnitKilling(LevelPlayerID idPlayer)	{		eUnitKilling|=(1<<idPlayer);	}

	DWORD iFrame;
	std::vector<LevelEvent*>events;
	LevelPlayerMask ePlayerKilling;
	LevelPlayerMask eUnitKilling;
	LevelEventMapFlag flags;
};

struct LevelEventTile
{
	LevelEventQueue qu[2];
};

struct EventData
{
	EventData()
	{
		Clear();
	}
	void Clear()
	{
		sz=0;
	}
	BYTE *Add(BYTE *p,DWORD n)
	{
		if (sz+n>EVENT_DATA_BUFFER_SIZE)
			return NULL;
		BYTE *ret=&buf[sz];
		memcpy(ret,p,n);
		sz+=n;
		return ret;
	}

	BYTE buf[EVENT_DATA_BUFFER_SIZE];
	DWORD sz;

};

struct LevelSignals
{
	struct Signal
	{
		Signal()
		{
			nm=StringID_Invalid;
			idSender=LevelObjID_Invalid;
			idLo=LevelObjID_Invalid;
			radius2=0.0f;
		}
		LevelObjID idSender;
		StringID nm;
		LevelObjID idLo;//发给谁,如果idLo不为空,则center/radius2都无效
		LevelPos center;
		float radius2;
	};

	LevelSignals()
	{
		Clear();
	}
	void Clear()
	{
		sz=0;
	}


	BOOL Add(StringID nm,LevelPos &center,float radius,LevelObjID idSender)
	{
		if (sz>=ARRAY_SIZE(buf))
			return FALSE;
		Signal*sig=&buf[sz];
		sig->nm=nm;
		sig->idLo=LevelObjID_Invalid;
		sig->center=center;
		sig->radius2=radius*radius;
		sig->idSender=idSender;
		sz++;
		return TRUE;
	}

	BOOL Add(StringID nm,LevelObjID idLo,LevelObjID idSender)
	{
		if (sz>=ARRAY_SIZE(buf))
			return FALSE;
		Signal*sig=&buf[sz];
		sig->nm=nm;
		sig->idLo=idLo;
		sig->idSender=idSender;
		sz++;
		return TRUE;
	}


	BOOL Test(StringID nm,LevelObjID idLo,LevelPos &pos)
	{
		return Find(nm,idLo,pos)!=NULL;
	}

	Signal *Find(StringID nm,LevelObjID idLo,LevelPos &pos)
	{
		for (int i=0;i<sz;i++)
		{
			Signal *sig=&buf[i];
			if (sig->nm!=nm)
				continue;
			if (sig->idLo!=LevelObjID_Invalid)
			{
				if (idLo==sig->idLo)
					return sig;
			}
			else
			{
				if (sig->center.getDistanceSQFrom(pos)<sig->radius2)
					return sig;
			}
		}
		return NULL;
	}
	void Get(LevelObjID idLo,LevelPos &pos,std::vector<Signal *> &bufRet)
	{
		bufRet.clear();
		for (int i=0;i<sz;i++)
		{
			Signal *sig=&buf[i];
			if (sig->idLo!=LevelObjID_Invalid)
			{
				if (idLo==sig->idLo)
					bufRet.push_back(sig);
			}
			else
			{
				if (sig->center.getDistanceSQFrom(pos)<sig->radius2)
					bufRet.push_back(sig);
			}
		}
	}

	Signal buf[64];
	DWORD sz;

};




class CLevel;
class CLevelObj;
class CLevelEventMap
{
public:
	CLevelEventMap()
	{
		Zero();
	}
	void Zero()
	{
		_iFrame=1;
		_flip=1;
	}
	void Create();
	void Destroy();

	DWORD GetFrame()	{		return _iFrame;	}

	void Update()
	{
		_iFrame++;
		_flip=1-_flip;
		_events[_flip].Clear();
		_signals[_flip].Clear();
	}

	template <typename T>
	void AddEvent(T&e,LevelPos &pos,float radius)
	{
		LevelEvent *pe=(LevelEvent *)_events[_flip].Add(&e,sizeof(e));
		if (!pe)
			return;

		_AddEvent(pe,pos,radius);
	}
	void AddEventFlag(LevelEventMapFlag flag,LevelPos &pos,float radius);

	void AddPlayerKilling(LevelPlayerID idPlayer,LevelPos &pos,float radius);
	void AddUnitKilling(LevelPlayerID idPlayer,LevelPos &pos,float radius);

	void AddSignal(StringID nm,LevelPos &pos,float radius,LevelObjID idSender);
	void AddSignal(StringID nm,LevelObjID idLo,LevelObjID idSender);
	BOOL TestSignal(StringID nm,LevelObjID idLo,LevelPos &pos);
	LevelSignals::Signal *FindSignal(StringID nm,LevelObjID idLo,LevelPos &pos);
	LevelSignals::Signal **GetSignals(LevelObjID idLo,LevelPos &pos,DWORD &nSignals);

	LevelEvent **GetEvents(LevelPos &pos,DWORD &count);
	LevelEventMapFlag GetEventFlags(LevelPos &pos);
	LevelPlayerMask GetPlayerKilling(LevelPos &pos);
	LevelPlayerMask GetUnitKilling(LevelPos &pos);
	LevelEventQueue *GetEventQueue(LevelPos &pos)	{		return _GetEventQueue(pos);	}

	void GarbageCollect();

public:
	void _AddEvent(LevelEvent *e,LevelPos &pos,float radius);
	LevelEventQueue *_GetEventQueue(LevelPos &pos);

	DWORD _iFrame;
	DWORD _flip;

	SparseArray2D<LevelEventTile,8,0,FALSE> _mp;
	EventData _events[2];
	LevelSignals _signals[2];

	std::vector<LevelSignals::Signal *> _temp;
};
