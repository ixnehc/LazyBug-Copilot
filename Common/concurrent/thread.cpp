
#include "stdh.h"
#include "thread.h"


DWORD ThreadFunc(void* data)
{
	if (CThread* thread = reinterpret_cast<CThread*>(data)) 
		return thread->OnDo();
	return 0;
}


CThread::CThread ()
{
	_handle=NULL;
}

CThread::~CThread()
{
	if(_handle) 
		CloseHandle(_handle);
}

void CThread::Start()
{
	if(_handle)
		return;
	_handle=::CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ThreadFunc, reinterpret_cast<void*>(this),0,NULL);
	::SetThreadPriority(_handle,THREAD_PRIORITY_LOWEST);
// 	::SetThreadPriorityBoost(_handle,TRUE);


}


int CThread::OnDo()
{
	return 0;
}
