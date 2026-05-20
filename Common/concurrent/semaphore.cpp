/********************************************************************
	created:	2008/07/09
	created:	9:7:2008   15:56
	filename: 	d:\IxEngine\Common\concurrent\semaphore.cpp
	file path:	d:\IxEngine\Common\concurrent
	file base:	semaphore
	file ext:	cpp
	author:		cxi
	
	purpose:	semaphore implement
*********************************************************************/

#include "stdh.h"
#include "semaphore.h"

Semaphore::Semaphore(DWORD count)
{
	handle=CreateSemaphore(NULL,count,0x7fffffff,NULL);
}

Semaphore::~Semaphore()
{
	if (handle)
		CloseHandle(handle);
}

void Semaphore::Wait()
{
	WaitForSingleObject(handle,INFINITE);
}


void Semaphore::Post()
{
	ReleaseSemaphore(handle,1,NULL);
}
