/********************************************************************
	created:	2006/06/21
	created:	21:6:2006   20:35
	filename: 	d:\IxEngine\Common\timer\timer.cpp
	author:		cxi
	
	purpose:	time-related functions
*********************************************************************/
#include "stdh.h"

#include "timer.h"

#include "assert.h"

#include "time.h"


//Return an absolute time(never overflow),in ms
unsigned __int64 GetAbsTick()
{
	SYSTEMTIME systime;
	GetSystemTime(&systime);
	FILETIME filetime;
	BOOL b = SystemTimeToFileTime(&systime,&filetime);
	assert(b);
	//	Uint64 time = filetime.dwHighDateTime << 32 || filetime.dwLowDateTime;
	return AbsTickFromFILETIME(filetime);
// 	return ( *((unsigned __int64*)&filetime) )/10000;// 1000/10
}

FILETIME GetCurFileTime()
{
	SYSTEMTIME systime;
	GetSystemTime(&systime);
	FILETIME filetime;
	BOOL b = SystemTimeToFileTime(&systime,&filetime);

	return filetime;
}


int g_aDayCount[12]=
{
	31,
	28,
	31,
	30,
	31,
	30,
	31,
	31,
	30,
	31,
	30,
	31,
};


int g_aDayCountRuen[12]=
{
	31,
	29,
	31,
	30,
	31,
	30,
	31,
	31,
	30,
	31,
	30,
	31,
};

BOOL IsYearRuen(int yy)
{
	if ((yy%4)!=0)
		return FALSE;

	if ((yy%400)==0)
		return TRUE;

	if ((yy%100)==0)
		return FALSE;

	return TRUE;
}

BOOL CheckYMDValidity(YMD ymd)//return whether valid
{
	int yy,mm,dd;

	yy=YMD_Y(ymd);
	mm=YMD_M(ymd);
	dd=YMD_D(ymd);

	int *p;
	if (IsYearRuen(yy))
		p=g_aDayCountRuen;
	else
		p=g_aDayCount;

	if ((mm<1)||(mm>12))
		return FALSE;

	if (dd<1)
		return FALSE;

	if (dd>p[mm-1])
		return FALSE;

	return TRUE;
}

CTimer::CTimer()
{
	m_bUsingQPF         = false;
	m_bTimerStopped     = true;
	m_llQPFTicksPerSec  = 0;

	m_llStopTime        = 0;
	m_llLastElapsedTime = 0;
	m_llBaseTime        = 0;

	// Use QueryPerformanceFrequency() to get frequency of timer.  
	LARGE_INTEGER qwTicksPerSec;
	m_bUsingQPF = (bool) (QueryPerformanceFrequency( &qwTicksPerSec ) != 0);
	m_llQPFTicksPerSec = qwTicksPerSec.QuadPart;
}


//--------------------------------------------------------------------------------------
void CTimer::Reset()
{
	if( !m_bUsingQPF )
		return;

	// Get either the current time or the stop time
	LARGE_INTEGER qwTime;
	if( m_llStopTime != 0 )
		qwTime.QuadPart = m_llStopTime;
	else
		QueryPerformanceCounter( &qwTime );

	m_llBaseTime        = qwTime.QuadPart;
	m_llLastElapsedTime = qwTime.QuadPart;
	m_llStopTime        = 0;
	m_bTimerStopped     = FALSE;
}


//--------------------------------------------------------------------------------------
void CTimer::Start()
{
	if( !m_bUsingQPF )
		return;

	// Get the current time
	LARGE_INTEGER qwTime;
	QueryPerformanceCounter( &qwTime );

	if( m_bTimerStopped )
		m_llBaseTime += qwTime.QuadPart - m_llStopTime;
	m_llStopTime = 0;
	m_llLastElapsedTime = qwTime.QuadPart;
	m_bTimerStopped = FALSE;
}


//--------------------------------------------------------------------------------------
void CTimer::Stop()
{
	if( !m_bUsingQPF )
		return;

	if( !m_bTimerStopped )
	{
		// Get either the current time or the stop time
		LARGE_INTEGER qwTime;
		if( m_llStopTime != 0 )
			qwTime.QuadPart = m_llStopTime;
		else
			QueryPerformanceCounter( &qwTime );

		m_llStopTime = qwTime.QuadPart;
		m_llLastElapsedTime = qwTime.QuadPart;
		m_bTimerStopped = TRUE;
	}
}


//--------------------------------------------------------------------------------------
void CTimer::Advance()
{
	if( !m_bUsingQPF )
		return;

	m_llStopTime += m_llQPFTicksPerSec/10;
}


//--------------------------------------------------------------------------------------
double CTimer::GetAbsoluteTime()
{
	if( !m_bUsingQPF )
		return -1.0;

	// Get either the current time or the stop time
	LARGE_INTEGER qwTime;
	if( m_llStopTime != 0 )
		qwTime.QuadPart = m_llStopTime;
	else
		QueryPerformanceCounter( &qwTime );

	double fTime = qwTime.QuadPart / (double) m_llQPFTicksPerSec;

	return fTime;
}


//--------------------------------------------------------------------------------------
double CTimer::GetTime()
{
	if( !m_bUsingQPF )
		return -1.0;

	// Get either the current time or the stop time
	LARGE_INTEGER qwTime;
	if( m_llStopTime != 0 )
		qwTime.QuadPart = m_llStopTime;
	else
		QueryPerformanceCounter( &qwTime );

	double fAppTime = (double) ( qwTime.QuadPart - m_llBaseTime ) / (double) m_llQPFTicksPerSec;

	return fAppTime;
}


//--------------------------------------------------------------------------------------
double CTimer::GetElapsedTime()
{
	if( !m_bUsingQPF )
		return -1.0;

	// Get either the current time or the stop time
	LARGE_INTEGER qwTime;
	if( m_llStopTime != 0 )
		qwTime.QuadPart = m_llStopTime;
	else
		QueryPerformanceCounter( &qwTime );

	double fElapsedTime = (double) ( qwTime.QuadPart - m_llLastElapsedTime ) / (double) m_llQPFTicksPerSec;
	m_llLastElapsedTime = qwTime.QuadPart;

	return fElapsedTime;
}


//--------------------------------------------------------------------------------------
bool CTimer::IsStopped()
{
	return m_bTimerStopped;
}
