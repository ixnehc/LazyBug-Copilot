#pragma once

#define PROFILE_ENABLE

enum ProfilerShow
{
	ProfShow_Current=0,
	ProfShow_Recent=1,
	ProfShow_Peak=2,
	ProfShow_Max,
};


struct Profiler
{
	Profiler(const char *name);
	void Start();
	void End();

	const char *GetName()
	{
		return (const char *)name;
	}

	const char *GetDesc()
	{
		return (const char *)desc;
	}

	unsigned __int64 GetTotal()	{	return data[ProfShow_Current].total;	}
	double GetTotalMS()	{		return data[ProfShow_Current].ms;	}
	void Update();

	void Show(ProfilerShow s,BOOL bShow)		{		data[s].bShow=bShow;	}

	void SetNumber(float v)
	{
		bNum=TRUE;
		num=v;
	}


protected:
	int ref;
	char name[32];
	char desc[32];
	BOOL bNum;
	float num;
	unsigned __int64 ts;

	struct _Data
	{
		_Data()
		{
			total=0;
			ms=0;
			count=0;
			rate=0;
		}
		unsigned __int64 total;//total time,in cycle
		double ms;//total time,in ms
		DWORD count;//total count
		double rate;//
		BOOL bShow;
		const char *suffix;
	};
	_Data data[ProfShow_Max];

friend struct ProfilerMgr;
};

struct ProfilerMgr
{
public:
	ProfilerMgr();
	void SetName(const char *name);
	void Register(Profiler*prof);

	void Link(ProfilerMgr *mgr);

	const char *Dump();
	const char *GetDump()	{		return _bufDump;	}
	Profiler *FindProfiler(const char *name);
	float CalcCyclePerMS();//计算每毫秒的cycle数

	void Push(Profiler *prof);
	Profiler *Pop();

	void Enable(BOOL bEnable);
	BOOL IsEnable()	{		return _bEnable;	}


protected:
	void _Sort();
	void _CalcRate(unsigned __int64 full);
	void _CalcMS();
	void _Dump(char *&buf,int &sz);
	void _DumpLine(char *&buf,int &sz,const char *fmtstr,...);
	char _name[64];
	Profiler *_buf[256];
	Profiler *_buf2[256];//sort profiler buffer
	int _cBuf;

	Profiler *_stack[512];
	int _top;

	char _bufDump[2048];

	DWORD _tickStart;
	unsigned __int64 _cycleStart;

	BOOL _bEnable;


	ProfilerMgr*_next;
};

#ifdef PROFILE_ENABLE

//only show the current value
#define ProfilerStart(name)																\
	static Profiler profiler_##name(#name);										\
	profiler_##name.Show(ProfShow_Current,TRUE);						\
	profiler_##name.Show(ProfShow_Recent,FALSE);						\
	profiler_##name.Show(ProfShow_Peak,FALSE);							\
	extern ProfilerMgr *GetProfilerMgr();											\
	if (GetProfilerMgr()->IsEnable())													\
	{																										\
		profiler_##name.Start();																\
		GetProfilerMgr()->Push(&profiler_##name);							\
	}

//only show the peak value 
#define ProfilerStart_Peak(name)														\
	static Profiler profiler_##name(#name);										\
	profiler_##name.Show(ProfShow_Current,FALSE);						\
	profiler_##name.Show(ProfShow_Recent,FALSE);						\
	profiler_##name.Show(ProfShow_Peak,TRUE);							\
	extern ProfilerMgr *GetProfilerMgr();											\
	if (GetProfilerMgr()->IsEnable())													\
	{																										\
		profiler_##name.Start();																\
		GetProfilerMgr()->Push(&profiler_##name);							\
	}

//only show the peak value 
#define ProfilerStart_Recent(name)													\
	static Profiler profiler_##name(#name);										\
	profiler_##name.Show(ProfShow_Current,FALSE);						\
	profiler_##name.Show(ProfShow_Recent,TRUE);							\
	profiler_##name.Show(ProfShow_Peak,FALSE);							\
	extern ProfilerMgr *GetProfilerMgr();											\
	if (GetProfilerMgr()->IsEnable())													\
	{																										\
		profiler_##name.Start();																\
		GetProfilerMgr()->Push(&profiler_##name);							\
	}


//show both the current and peak value
#define ProfilerStart_Full(name)														\
	static Profiler profiler_##name(#name);										\
	profiler_##name.Show(ProfShow_Current,TRUE);						\
	profiler_##name.Show(ProfShow_Recent,TRUE);							\
	profiler_##name.Show(ProfShow_Peak,TRUE);							\
	extern ProfilerMgr *GetProfilerMgr();											\
	if (GetProfilerMgr()->IsEnable())													\
	{																										\
		profiler_##name.Start();																\
		GetProfilerMgr()->Push(&profiler_##name);							\
	}


#define ProfilerEnd()																			\
{																											\
	extern ProfilerMgr *GetProfilerMgr();											\
	if (GetProfilerMgr()->IsEnable())													\
	{																										\
		Profiler *__prof=GetProfilerMgr()->Pop();								\
		if (__prof)	__prof->End();																\
	}																										\
}

#define Profiler_Number(name,v)														\
	static Profiler profiler_##name(#name);										\
	profiler_##name.SetNumber((float)(v));

#define Profiler_String(name,str)														\
	static Profiler profiler_##name(#name);										\
	strcpy((char*)profiler_##name.GetDesc(),(str));

#else

#define ProfilerDefine(name)
#define ProfilerStart(name)
#define ProfilerStart_Recent(name)
#define ProfilerStart_Peak(name)
#define ProfilerStart_Full(name)
#define ProfilerEnd()
#define Profiler_Number(name,v)
#define Profiler_String(name,str)

#endif

#define ProfilerWait(ms)				\
{				\
	__int64 tt=GetTSC()+(__int64)(((float)(ms))*GetProfilerMgr()->CalcCyclePerMS());				\
	while(GetTSC()<tt);				\
}



extern ProfilerMgr *GetProfilerMgr();
