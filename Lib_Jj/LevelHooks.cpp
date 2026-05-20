/********************************************************************
	created:	2012/12/25 
	author:		cxi
	
	purpose:	Level Hook
*********************************************************************/
#include "stdh.h"
#include "LevelHooks.h"
#include "LevelObj.h"
  
BOOL CLevelHooks::Init()
{
	return TRUE;
}

void CLevelHooks::_ClearEntry(Entry *entry)
{
	for (int i=0;i<entry->nodes.size();i++)
		SAFE_RELEASE(entry->nodes[i].lo);

	entry->nodes.clear();
}


void CLevelHooks::Clear()
{
	for (int i=0;i<LevelHookMax;i++)
		_ClearEntry(&_entriesHk[i]);

	memset(_flagsHk,0,sizeof(_flagsHk));
	_registereds.clear();

}

static inline void _InsertNode(std::vector<CLevelHooks::HookNode> &nodes,CLevelObj *lo,DWORD prior)
{
	CLevelHooks::HookNode t;
	t.lo=lo;
	t.prior=prior;

	lo->AddRef();

	CLevelHooks::HookNode *p=nodes.data();
	DWORD sz=nodes.size();
	int i;
	for (i=0;i<sz;i++)
	{
		if (p[i].prior<=prior)
			break;
	}
	if (i<sz)
		nodes.insert(nodes.begin()+i,t);
	else
		nodes.push_back(t);

}


BOOL CLevelHooks::RegisterHook(LevelHookType tpHook,CLevelObj *lo,DWORD prior)
{
	if (tpHook>=LevelHookMax)
		return FALSE;

	if (!_flagsHk[tpHook])
	{
		_registereds.push_back(tpHook);
		_flagsHk[tpHook]=1;
	}

	_InsertNode(_entriesHk[tpHook].nodes,lo,prior);

	return TRUE;
}


//for the given entry,remove the node whose lo is no longer alive
template<typename T_Node>
void _FlushNodes(std::vector<T_Node>&entry)
{
	T_Node*p=entry.data();
	DWORD sz,sz0;
	sz=sz0=entry.size();
	DWORD c=0;
	DWORD i=0;
	while(i<sz)
	{
		if (p[i].lo)
		{
			if (!(p[i].lo)->IsAlive())
			{
				SAFE_RELEASE(p[i].lo);
				i++;
				continue;
			}
		}
		p[c]=p[i];
		c++;
		i++;
	}

	entry.resize(c);
}

static inline void _SendHook(std::vector<CLevelHooks::HookNode>&nodes,LevelHook &hk)
{
	CLevelHooks::HookNode *p=nodes.data();
	DWORD sz=nodes.size();

	for (int i=0;i<sz;i++)
	{
		if (p[i].lo->IsAlive())
			p[i].lo->HandleHook(hk);
	}
}



BOOL CLevelHooks::SendHook(LevelHook&hk)
{
	LevelHookType tp=(LevelHookType)hk.GetType();
	if (tp>=LevelHookMax)
		return FALSE;

	_SendHook(_entriesHk[tp].nodes,hk);
	return TRUE;
}

static inline void _FlushEntry(CLevelHooks::Entry &entry)
{
	_FlushNodes<CLevelHooks::HookNode>(entry.nodes);
}


void CLevelHooks::GarbageCollect(BOOL bFull)
{
	DWORD step=4;

	if (bFull)
		step=_registereds.size();

	if (step>_registereds.size())
		step=_registereds.size();

	for (int i=0;i<step;i++)
		_FlushEntry(_entriesHk[_registereds[(_lastgc+i)%_registereds.size()]]);
}




