#ifndef __Win32ThreadSupports_H__
#define __Win32ThreadSupports_H__

class CWin32Lock
{
public:
	CWin32Lock()
	{
		InitializeCriticalSection(&_cs);
	}
	~CWin32Lock()
	{
		DeleteCriticalSection(&_cs);
	}

public:
	void Lock()
	{
		EnterCriticalSection(&_cs);
	}
	void UnLock()
	{
		LeaveCriticalSection(&_cs);
	}

private:
	CRITICAL_SECTION _cs;
};

class CWin32Event
{
public:
	CWin32Event() : _hEvent(NULL)
	{
	}
	~CWin32Event()
	{
		if (_hEvent)
			CloseHandle(_hEvent);
	}

public:
	BOOL Create(BOOL bInitialState = FALSE)
	{
		_hEvent = CreateEvent(NULL, TRUE, bInitialState, NULL);
		return (_hEvent != NULL);
	}
	void Destroy()
	{
		if (_hEvent)
		{
			CloseHandle(_hEvent);
			_hEvent = NULL;
		}
	}

public:
	void Wait(DWORD dwTimeout = INFINITE)
	{
		WaitForSingleObject(_hEvent, dwTimeout);
	}
	void Set()
	{
		SetEvent(_hEvent);
	}
	void Reset()
	{
		ResetEvent(_hEvent);
	}

private:
	HANDLE _hEvent;
};

class CWin32Thread
{
public:
	CWin32Thread() : _hThread(NULL)
	{
	}
	~CWin32Thread()
	{
		Stop();
	}

public:
	BOOL Start(LPTHREAD_START_ROUTINE lpThreadProc, LPVOID lpParam)
	{
		DWORD dwThreadId;
		_hThread = CreateThread(NULL, 0, lpThreadProc, lpParam, 0, &dwThreadId);
		return (_hThread != NULL);
	}
	void Stop()
	{
		if (_hThread)
		{
			if (WAIT_OBJECT_0 != WaitForSingleObject(_hThread, 60000))
			{
				TerminateThread(_hThread, 0);
			}

			CloseHandle(_hThread);
			_hThread = NULL;
		}
	}
	void Suspend()
	{
		SuspendThread(_hThread);
	}
	void Resume()
	{
		ResumeThread(_hThread);
	}

private:
	HANDLE _hThread;
};
#endif