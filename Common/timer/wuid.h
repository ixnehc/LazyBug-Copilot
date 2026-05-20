#pragma once

#include "timer/timer.h"


typedef unsigned __int64 WUID;


inline WUID GenWUID()
{
	return (GetAbsTick()<<32)|(GetTSC()&0xffffffff);
}
