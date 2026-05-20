
#pragma once

#include "tbbinc.h"

#include "semaphore.h"
#include "mutexes.h"

#include <deque>


//×èÈû¶ÓÁÐ
template <typename T>
class CBlockQueue
{

public:

	typedef QueueMutex MutexType;
	CBlockQueue()
	{
	}

	virtual ~CBlockQueue()
	{
		MutexType::scoped_lock lock(_mu);
		while(!_buf.empty())
			_buf.pop_front();
	}

	/** This adds an object to tail and notifies blocked processed. */
	void PushBack(T data)
	{
		MutexType::scoped_lock lock(_mu);
		_buf.push_back(data);
		_smph.Post();
	}

	void PushFront(T data)
	{
		MutexType::scoped_lock lock(_mu);
		_buf.push_front(data);
		_smph.Post();
	}

	/** This returns the next object from the queue or blocks if it is empty */
	T Pop()
	{
		while(1)
		{
			_smph.Wait();

			if (TRUE)
			{
				MutexType::scoped_lock lock(_mu);
				if (_buf.size()>0)
				{
					T tmp = _buf.front();
					_buf.pop_front();
					return tmp;
				}
				_smphFlush.Post();
			}
		}

	}

	/** This returns true if the queue isn't empty. */
	bool IsAvailable()
	{
		MutexType::scoped_lock lock(_mu);
		return !_buf.empty();
	}

	int GetSize()
	{
		MutexType::scoped_lock lock(_mu);
		return _buf.size();
	}

	void Flush()
	{
		if (TRUE)
		{
			MutexType::scoped_lock lock(_mu);
			_smph.Post();
		}

		_smphFlush.Wait();
	}

protected:
	std::deque<T> _buf;
	MutexType _mu;
	Semaphore _smph;

	Semaphore _smphFlush;

};




template <typename T,int T_ClusterSize=8>
class CBlockQueueEx
{

public:

	typedef QueueMutex MutexType;
	CBlockQueueEx()
	{
	}

	virtual ~CBlockQueueEx()
	{
		MutexType::scoped_lock lock(_mu);
		while(!_buf.empty())
			_buf.pop_front();
	}

	/** This adds an object to tail and notifies blocked processed. */
	void PushBack(T data)
	{
		if (_in.idx>=T_ClusterSize)
		{
			_in.count=_in.idx;
			_in.idx=0;
			MutexType::scoped_lock lock(_mu);
			_buf.push_back(_in);
			_smph.Post();
		}
		_in.buf[_in.idx]=data;
		_in.idx++;
		return;
	}

	void FinalizePush()
	{
		if (_in.idx==0)
			return;

		_in.count=_in.idx;
		_in.idx=0;

		MutexType::scoped_lock lock(_mu);
		_buf.push_back(_in);
		_smph.Post();
	}

	/** This returns the next object from the queue or blocks if it is empty */
	T Pop()
	{
		if (_out.idx<_out.count)
		{
			_out.idx++;
			return _out.buf[_out.idx-1];
		}
		while(1)
		{
			_smph.Wait();

			if (TRUE)
			{
				MutexType::scoped_lock lock(_mu);
				if (_buf.size()>0)
				{
					_out= _buf.front();
					_buf.pop_front();
					_out.idx=1;
					return _out.buf[0];
				}
				_smphFlush.Post();
			}
		}
	}

	void Flush()
	{
		FinalizePush();
		if (TRUE)
		{
			MutexType::scoped_lock lock(_mu);
			_smph.Post();
		}

		_smphFlush.Wait();
	}

protected:
	struct _Cluster
	{
		T buf[T_ClusterSize];
		DWORD count;
		DWORD idx;
	};
	std::deque<_Cluster> _buf;
	MutexType _mu;
	Semaphore _smph;

	Semaphore _smphFlush;

	_Cluster _in;
	_Cluster _out;

};
