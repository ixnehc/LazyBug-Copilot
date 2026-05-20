/********************************************************************
	created:	2007/9/18   16:35
	filename: 	e:\IxEngine\Common\timer\profiler.cpp
	author:		cxi
	
	purpose:	profiler tool

	*********************************************************************/

#include "stdh.h"

#include "profiler.h"
#include "timer.h"

#include <stdio.h>
#include <stdlib.h>

#include <assert.h>

ProfilerMgr *GetProfilerMgr()
{
#ifdef PROFILE_ENABLE
	static ProfilerMgr mgr;
	return &mgr;
#else
	return NULL;
#endif
}

//////////////////////////////////////////////////////////////////////////
//Profiler

Profiler::Profiler(const char *name0)
{
	strcpy(name,name0);
	desc[0]=0;
	bNum=FALSE;

	ref=0;
	ts=0;

	Show(ProfShow_Recent,FALSE);
	Show(ProfShow_Peak,FALSE);

	data[ProfShow_Current].suffix="";
	data[ProfShow_Recent].suffix="(Recent)";
	data[ProfShow_Peak].suffix="(Peak)";

	GetProfilerMgr()->Register(this);
}

void Profiler::Start()
{
	if (ref==0)
		ts=GetTSC();
	ref++;
}

void Profiler::End()
{
	ref--;
	if (ref<=0)
	{
		unsigned __int64 te=GetTSC();

		data[ProfShow_Current].count++;
		data[ProfShow_Current].total+=te-ts;

		ref=0;
	}
}

void Profiler::Update()
{
	_Data *c,*r,*pk;
	c=&data[ProfShow_Current];
	r=&data[ProfShow_Recent];
	pk=&data[ProfShow_Peak];

	if (c->total>pk->total)
		pk->total=c->total;
	if (c->ms>pk->ms)
		pk->ms=c->ms;
	if (c->count>pk->count)
		pk->count=c->count;
	if (c->rate>pk->rate)
		pk->rate=c->rate;

	if (c->count>0)
	{
		r->total=c->total;
		r->ms=c->ms;
		r->count=c->count;
		r->rate=c->rate;
	}
}


//////////////////////////////////////////////////////////////////////////
//ProfilerMgr
ProfilerMgr::ProfilerMgr()
{
	memset(_name,0,sizeof(_name));
	_cBuf=0;
	_top=0;
	_next=NULL;

	_bEnable=TRUE;

	_tickStart=GetTickCount();
	_cycleStart=GetTSC();
}

void ProfilerMgr::SetName(const char *name)
{
	strcpy(_name,name);
}


void ProfilerMgr::Register(Profiler*prof)
{
	assert(_cBuf<ARRAY_SIZE(_buf));
	_buf[_cBuf]=prof;
	_cBuf++;
}

void ProfilerMgr::Link(ProfilerMgr *mgr)
{
	if (mgr==this)
		return;
	ProfilerMgr **p=&_next;
	while(*p)
	{
		if (*p==mgr)
			return;//already existing
		p=&(*p)->_next;
	}
	(*p)=mgr;
}

Profiler *ProfilerMgr::FindProfiler(const char *name)
{
	for (int i=0;i<_cBuf;i++)
	{
		if (strcmp(_buf[i]->name,name)==0)
			return _buf[i];
	}
	if (_next)
		return _next->FindProfiler(name);
	return NULL;
}

//º∆À„√ø∫¡√Îµƒcycle ˝
float ProfilerMgr::CalcCyclePerMS()
{
	return (float)((double)(GetTSC()-_cycleStart)/(double)(GetTickCount()-_tickStart));
}



void ProfilerMgr::Push(Profiler *prof)
{
	assert(_top<ARRAY_SIZE(_stack));

	_stack[_top]=prof;
	_top++;
}

Profiler *ProfilerMgr::Pop()
{
	assert(_top>0);
	_top--;
	return _stack[_top];
}


int ProfilerComp(const void *l,const void *r)
{
	if (strcmp((*(Profiler**)l)->GetName(),(*(Profiler**)r)->GetName())>0)
		return 1;
	return -1;
// 	if ((*(Profiler**)l)->GetTotal()>(*(Profiler**)r)->GetTotal())
// 		return -1;
// 	return 1;
}

void ProfilerMgr::_Sort()
{
	//sort all the profiler
	memcpy(_buf2,_buf,sizeof(_buf));
	qsort(_buf2,_cBuf,sizeof(Profiler*),ProfilerComp);
}

void ProfilerMgr::_CalcRate(unsigned __int64 full)
{
	if (full==0)
	{
		for (int i=0;i<_cBuf;i++)
			_buf2[i]->data[ProfShow_Current].rate=0.0f;
	}
	else
	{
		for (int i=0;i<_cBuf;i++)
			_buf2[i]->data[ProfShow_Current].rate=(double)_buf2[i]->data[ProfShow_Current].total/(double)full;
	}
}

void ProfilerMgr::_CalcMS()
{
	DWORD tick=GetTickCount();
	unsigned __int64 cycle=GetTSC();

	double r=(double)(tick-_tickStart)/(double)(cycle-_cycleStart);
	for (int i=0;i<_cBuf;i++)
		_buf2[i]->data[ProfShow_Current].ms=r*(double)_buf2[i]->data[ProfShow_Current].total;
}



void ProfilerMgr::_DumpLine(char *&buf,int &sz,const char *fmtstr,...)
{
	if (sz<=2)
		return;
	va_list args;

	va_start(args,fmtstr);
	int nSize = _vsnprintf(buf,sz-2, fmtstr,args);//-2 to reserve place for "\r\n"
	va_end(args);

	if (nSize==-1)
		nSize=sz-2;
	buf+=nSize;
	memcpy(buf,"\r\n",2);
	buf+=2;
	nSize+=2;
	sz-=nSize;
}



void ProfilerMgr::_Dump(char *&buf,int &sz)
{
	if (_cBuf<=0)
		return;

	//note: the format has the best looking when using font Œ¢»Ì—≈∫⁄/size 12
	_DumpLine(buf,sz," [%s]",_name);
	_DumpLine(buf,sz,			"		ms            count        name");
	for (int i=0;i<_cBuf;i++)
	{
		Profiler *p=_buf2[i];
		if (p->bNum)
		{
			_DumpLine(buf,sz,		"	[ %10.03f ]			%s",p->num,p->name);
			continue;
		}
		if (p->desc[0])
		{
			_DumpLine(buf,sz,		"	[ %s ]			%s",p->desc,p->name);
			continue;
		}

		p->Update();
		for (int j=0;j<ProfShow_Max;j++)
		{
			Profiler::_Data *q=&p->data[j];
			if (q->bShow)
			{
				_DumpLine(buf,sz,		"	%10.03f		%8d		%s%s",
					(float)q->ms,q->count,p->name,q->suffix);
			}
			if (j==ProfShow_Current)
			{
				q->count=0;
				q->total=0;
			}
		}

	}
}

const char *ProfilerMgr::Dump()
{
	//sort by total time
	ProfilerMgr *p=this;
	while(p)
	{
		p->_Sort();
		p=p->_next;
	}

	//get the max of all the profilers' total 
	unsigned __int64 full=0;
	p=this;
	while(p)
	{
		if (p->_cBuf>0)
		{
			if (p->_buf2[0]->GetTotal()>full)
				full=p->_buf2[0]->GetTotal();
		}
		p=p->_next;
	}

	//Calc the rate
	p=this;
	while(p)
	{
		p->_CalcRate(full);
		p->_CalcMS();
		p=p->_next;
	}


	//dump
	p=this;
	char *buf=_bufDump;
	int c=sizeof(_bufDump)-1;
	while(p)
	{
		p->_Dump(buf,c);
		p=p->_next;
	}
	_bufDump[sizeof(_bufDump)-1]=0;

	if (_bufDump[0]==0)
		strcpy(_bufDump,"[no profile info available]");

	return _bufDump;

}

void ProfilerMgr::Enable(BOOL bEnable)
{
	_bEnable=bEnable;
	if (_next)
		_next->Enable(bEnable);
}
