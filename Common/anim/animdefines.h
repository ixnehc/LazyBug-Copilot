#pragma once

#define ANIMTICK_PER_SECOND 4800 //same as 3dsmax
#define ANIMTICK_FROM_SECOND(s) (AnimTick)(((float)ANIMTICK_PER_SECOND)*(float)(s))
#define ANIMTICK_TO_SECOND(tick) (((float)(tick))/(float)ANIMTICK_PER_SECOND)

#define ANIMTICK_SAFE_MINUS(t1,t2) ((t1)>(t2)?((t1)-(t2)):0)

typedef DWORD AnimTick;
#define ANIMTICK_INFINITE (0xffffffff)
#define ANIMTICK_CURRENT (0xfffffffe)
#define ANIMTICK_MAX (0xfffffffd)
