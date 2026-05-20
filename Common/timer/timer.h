#pragma once

#define MAKE_YMD(yyyy,mm,dd) ((yyyy)<<16)|((mm)<<8)|(dd)
#define YMD_Y(ymd) ((ymd)>>16)
#define YMD_M(ymd) (((ymd)&0x0000ff00)>>8)
#define YMD_D(ymd) ((ymd)&0x000000ff)

typedef unsigned int YMD;

typedef unsigned __int64 AbsTick;



inline __int64 GetTSC(void) //time stamp count
{
	return __rdtsc();
};

extern unsigned __int64 GetAbsTick();

inline AbsTick AbsTickFromFILETIME(FILETIME ft)
{
	return (*((unsigned __int64*)&ft)) / 10000;// 1000/10
}


FILETIME GetCurFileTime();

extern BOOL IsYearRuen(int yy);
extern BOOL CheckYMDValidity(YMD ymd);


class CTimer
{
public:
	CTimer();

	void Reset(); // resets the timer
	void Start(); // starts the timer
	void Stop();  // stop (or pause) the timer
	void Advance(); // advance the timer by 0.1 seconds
	double GetAbsoluteTime(); // get the absolute system time
	double GetTime(); // get the current time
	double GetElapsedTime(); // get the time that elapsed between GetElapsedTime() calls
	bool IsStopped(); // returns true if timer stopped

protected:
	bool m_bUsingQPF;
	bool m_bTimerStopped;
	LONGLONG m_llQPFTicksPerSec;

	LONGLONG m_llStopTime;
	LONGLONG m_llLastElapsedTime;
	LONGLONG m_llBaseTime;
};


