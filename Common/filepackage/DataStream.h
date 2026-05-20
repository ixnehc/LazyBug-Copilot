#ifndef __DataStream_H__
#define __DataStream_H__

class CInDataStream
{
public:
	explicit CInDataStream(const char* data, unsigned long size) : _size(size)
	{
		_data = const_cast<char*>(data);
	}

	~CInDataStream() {}

public:
	void SetData(const char* data, int size)
	{
		_data = const_cast<char*>(data);
		_size = size;
	}
	int GetSize() const
	{
		return _size;
	}

public:
	CInDataStream& operator >> (char& val)
	{
		if (_size >= sizeof(char))
		{
			val = *_data;
			_data += sizeof(char);
			_size -= sizeof(char);
		}
		return (*this);
	}
	CInDataStream& operator >> (unsigned char& val)
	{
		if (_size >= sizeof(unsigned char))
		{
			val = *_data;
			_data += sizeof(unsigned char);
			_size -= sizeof(unsigned char);
		}
		return (*this);
	}
	CInDataStream& operator >> (unsigned short& val)
	{
		if (_size >= sizeof(unsigned short))
		{
			val = *((unsigned short*) _data);
			_data += sizeof(unsigned short);
			_size -= sizeof(unsigned short);
		}
		return (*this);
	}
	CInDataStream& operator >> (int& val)
	{
		if (_size >= sizeof(int))
		{
			val = *((int*) _data);
			_data += sizeof(int);
			_size -= sizeof(int);
		}
		return (*this);
	}
	CInDataStream& operator >> (unsigned int& val)
	{
		if (_size >= sizeof(unsigned int))
		{
			val = *((unsigned int*) _data);
			_data += sizeof(unsigned int);
			_size -= sizeof(unsigned int);
		}
		return (*this);
	}
	CInDataStream& operator >> (unsigned long& val)
	{
		if (_size >= sizeof(unsigned long))
		{
			val = *((unsigned long*) _data);
			_data += sizeof(unsigned long);
			_size -= sizeof(unsigned long);
		}
		return (*this);
	}
	CInDataStream& operator >> (LONG64& val)
	{
		if (_size >= sizeof(LONG64))
		{
			val = *((LONG64*) _data);
			_data += sizeof(LONG64);
			_size -= sizeof(LONG64);
		}
		return (*this);
	}
	CInDataStream& operator >> (ULONG64& val)
	{
		if (_size >= sizeof(ULONG64))
		{
			val = *((ULONG64*) _data);
			_data += sizeof(ULONG64);
			_size -= sizeof(ULONG64);
		}
		return (*this);
	}

public:
	void Read(void* buf, unsigned long size)
	{
		if (size > _size)
			size = _size;
		memcpy(buf, _data, size);
		_data += size;
		_size -= size;
	}

	void Skip(unsigned long offset)
	{
		_size -= offset;
		_data += offset;
	}

private:
	char* _data;
	unsigned long _size;
};

class COutDataStream
{
public:
	COutDataStream() : _cursor(0)
	{
		const int DEFAULT_STREAM_SIZE = 512;
		_data = new char[DEFAULT_STREAM_SIZE];
		_size = DEFAULT_STREAM_SIZE;
	}

	~COutDataStream()
	{
		delete []_data;
	}

public:
	BOOL Create(unsigned long size)
	{
		if (_size < size)
		{
			Destroy();

			_data = new char[size];
			if (_data)
				_size = size;
			return (_data != NULL);
		}
		else
			_cursor = 0;
		return TRUE;
	}
	void Destroy()
	{
		if (_data)
		{
			delete []_data;
			_data = NULL;
			_size = 0;
			_cursor = 0;
		}
	}

public:
	const char* GetData() const
	{
		return _data;
	}
	unsigned long GetSize() const
	{
		return _cursor;
	}
	void Clear()
	{
		_cursor = 0;
	}

public:
	COutDataStream& operator << (char val)
	{
		if ((_cursor + sizeof(char)) <= _size)
		{
			*((char *)(_data + _cursor)) = val;
			_cursor += sizeof(char);
		}
		return (*this);
	}
	COutDataStream& operator << (unsigned char val)
	{
		if ((_cursor + sizeof(unsigned char)) <= _size)
		{
			*((unsigned char *)(_data + _cursor)) = val;
			_cursor += sizeof(unsigned char);
		}
		return (*this);
	}
	COutDataStream& operator << (unsigned short val)
	{
		if ((_cursor + sizeof(unsigned short)) <= _size)
		{
			*((unsigned short *)(_data + _cursor)) = val;
			_cursor += sizeof(unsigned short);
		}
		return (*this);
	}
	COutDataStream& operator << (int val)
	{
		if ((_cursor + sizeof(int)) <= _size)
		{
			*((int *)(_data + _cursor)) = val;
			_cursor += sizeof(int);
		}
		return (*this);
	}
	COutDataStream& operator << (unsigned int val)
	{
		if ((_cursor + sizeof(unsigned int)) <= _size)
		{
			*((unsigned int *)(_data + _cursor)) = val;
			_cursor += sizeof(unsigned int);
		}
		return (*this);
	}
	COutDataStream& operator << (unsigned long val)
	{
		if ((_cursor + sizeof(unsigned long)) <= _size)
		{
			*((unsigned long *)(_data + _cursor)) = val;
			_cursor += sizeof(unsigned long);
		}
		return (*this);
	}
	COutDataStream& operator << (LONG64 val)
	{
		if ((_cursor + sizeof(LONG64)) <= _size)
		{
			*((LONG64 *)(_data + _cursor)) = val;
			_cursor += sizeof(LONG64);
		}
		return (*this);
	}
	COutDataStream& operator << (ULONG64 val)
	{
		if ((_cursor + sizeof(ULONG64)) <= _size)
		{
			*((ULONG64 *)(_data + _cursor)) = val;
			_cursor += sizeof(ULONG64);
		}
		return (*this);
	}

public:
	void Write(void* buf, unsigned long count)
	{
		if (count > 0 && ((_cursor + count) <= _size))
		{
			memcpy((_data + _cursor), buf, count);
			_cursor += count;
		}
	}

private:
	char* _data;
	unsigned long _size;
	unsigned long _cursor;
};
#endif