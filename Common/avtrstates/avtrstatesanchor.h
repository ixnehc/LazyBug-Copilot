
#pragma once


#include "../ref/ref.h"


class CAvtrStates;
struct StateChange
{
	DEFINE_CLASS(StateChange)

	AvtrStateMask old;
	AvtrStateMask cur;//switchÒÔºóµÄ×´Ì¬
	AvtrCmdFlags cmds;
	CAvtrStates *avs;
};


struct AvtrStatesMonitor
{
	DEFINE_REF();
	void Clear()
	{
		BreakRef();
	}
	virtual void NotifyChange(StateChange &change)	{	}
};

struct AvtrStatesMonitorEntry
{
	DEFINE_CLASS(AvtrStatesMonitorEntry);
	Ref *refMonitor;

	AvtrStatesMonitorEntry*next;
};


struct AvtrStatesAnchor
{

	AvtrStatesAnchor()
	{
		entries=NULL;
	}

	void Clear()
	{
		AvtrStatesMonitorEntry *e=entries;
		while(e)
		{
			SAFE_RELEASE(e->refMonitor);
			AvtrStatesMonitorEntry *next=e->next;
			Class_Delete(e);
			e=next;
		}
		entries=NULL;

	}

	void Register(AvtrStatesMonitor *mon)
	{
		AvtrStatesMonitorEntry *entry=Class_New2(AvtrStatesMonitorEntry);

		entry->refMonitor=mon->ObtainRef();
		entry->next=entries;
		entries=entry;
	}
	void UnRegister(AvtrStatesMonitor *mon)
	{
		AvtrStatesMonitorEntry**e=&entries;

		Ref *refMon=mon->GetRef();

		while(*e)
		{
			AvtrStatesMonitorEntry**p=e;
			e=&(*e)->next;

			if ((*p)->refMonitor==refMon)
			{
				SAFE_RELEASE((*p)->refMonitor);
				AvtrStatesMonitorEntry*tp=(*p);
				(*p)=(*p)->next;
				Class_Delete(tp);

				break;
			}
		}
	};

	void NotifyChange(StateChange &change)
	{
		Ref*notifies[64];//big enough
		DWORD nNotifies=0;

		AvtrStatesMonitorEntry**e=&entries;
		while(*e)
		{
			assert((*e)->refMonitor);

			if (!(*e)->refMonitor->GetStuff())
			{
				SAFE_RELEASE((*e)->refMonitor);
				AvtrStatesMonitorEntry*tp=(*e);
				(*e)=(*e)->next;
				Class_Delete(tp);
			}
			else
			{
				(*e)->refMonitor->AddRef();
				if (nNotifies<ARRAY_SIZE(notifies)-2)
					notifies[nNotifies++]=(*e)->refMonitor;
				else
					assert(FALSE);
				e=&(*e)->next;
			}
		}

		for (int i=0;i<nNotifies;i++)
		{
			AvtrStatesMonitor*mon=(AvtrStatesMonitor*)notifies[i]->GetStuff();
			if (mon)
				mon->NotifyChange(change);
			SAFE_RELEASE(notifies[i]);
		}

	}

	AvtrStatesMonitorEntry *entries;
};

