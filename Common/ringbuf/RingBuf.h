#pragma once

template<typename T,int T_bufsize>
class CRingBuf
{
public:
	CRingBuf()
	{
		Zero();
	}
	~CRingBuf()
	{
		Clear();
	}

	void Zero()
	{
		_head=0;
		_count=0;
	}
	void Clear()
	{
		Zero();
	}

	int GetCapacity()
	{
		return T_bufsize;
	}

	int GetCount()
	{
		return _count;
	}

	T &GetAt(int idx)
	{
		return _buf[(_head+idx)%T_bufsize];
	}

	void PushBack(T &v)
	{
		if (_count<T_bufsize)
		{
			_buf[(_head+_count)%T_bufsize]=v;
			_count++;
		}
		else
		{
			_buf[_head]=v;
			_head++;
			_head=_head%T_bufsize;
		}
	}



public://take it as protected
	int _head;
	int _count;

	T _buf[T_bufsize];



};
