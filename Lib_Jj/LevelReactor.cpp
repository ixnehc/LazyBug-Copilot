/********************************************************************
	created:	2012/01/02
	file base:	Buff
	author:		cxi
	
	purpose:	LevelBuff
*********************************************************************/

#include "stdh.h"

#include "LevelBuff.h"

#include "Level.h"

#include "LevelRecordBuff.h"
#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorMem.h"

#include "Log/LogDump.h"

//////////////////////////////////////////////////////////////////////////
//CLevelReactor

void CLevelReactor::Create(LevelReactorParam *param,CLevelReactors *reactors)
{
	_SetParam(param);
	_reactors=reactors;

	_OnCreate();

}



void CLevelReactor::Destroy()
{
	_OnDestroy();

	Zero();

}

CLevelObj*CLevelReactor::_GetOwner()
{
	if (_reactors)
	{
		if (_reactors->_owner)
		{
			return _reactors->_owner->GetOwner();
		}
	}
	return NULL;
}


LevelOSB CLevelReactor::_GetOSB()
{
	if (_reactors)
	{
		if (_reactors->_owner)
		{
			return LevelOSB(_reactors->_owner);
		}
	}
	return LevelOSB();
}





//////////////////////////////////////////////////////////////////////////
//CLevelReactors
void CLevelReactors::Init(CLevelBuff*buff)
{
	_owner=buff;


	//创建所有的reactor
	LevelRecordBuff *rec=buff->GetRec();
	if (rec)
	{
		_LoadReactors(&rec->paramsReactor[0],rec->paramsReactor.size());
	}

}

void CLevelReactors::Clear()
{
	for (int i=0;i<_reactors.size();i++)
	{
		if (_reactors[i])
		{
			_reactors[i]->Destroy();
			Safe_Class_Delete(_reactors[i]);
		}


		SAFE_DESTROY(_reactors[i]);
	}
	_reactors.clear();

	Zero();
}


void CLevelReactors::Update(AnimTick t)
{
	for (int i=0;i<_reactors.size();i++)
	{
		CLevelReactor *reactor=_reactors[i];
		if (reactor)
			reactor->Update(t);
	}
}


void CLevelReactors::HandleEvent(LevelEvent &e)
{
	for (int i=0;i<_reactors.size();i++)
	{
		CLevelReactor *reactor=_reactors[i];
		if (reactor)
		{
			reactor->HandleEvent(e);
			if (e.bHandled)
				return;
		}
	}
}


CLevelReactor *CLevelReactors::FindReactor(CClass *clssBuff)
{
	for (int i=0;i<_reactors.size();i++)
	{
		CLevelReactor *reactor=_reactors[i];
		if (reactor->GetClass()->IsSameWith(clssBuff))
			return reactor;
	}
	return NULL;
}


void CLevelReactors::_LoadReactors(LevelReactorParamEntry *entries,DWORD c)
{
	for (int i=0;i<c;i++)
	{
		LevelReactorParam *param=entries[i].param;
		if (param)
		{
			CClass *clss=param->GetReactorClass();
			if (clss)
			{
				CLevelReactor *reactor=(CLevelReactor *)clss->New();
				reactor->Create(param,this);

				_reactors.push_back(reactor);
			}
		}
	}
}
