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

public:
	void Read(void* buf, unsigned long size)
	{
		if (size > _size)
			size = _size;
		memcpy(buf, _data, size);
		_data += size;
		_size -= size;
	}

private:
	char* _data;
	unsigned long _size;
};

template <unsigned long Size>
class COutDataStream
{
public:
	COutDataStream() : _cursor(0)
	{
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
		if ((_cursor + sizeof(char)) <= Size)
		{
			*((char *)(_data + _cursor)) = val;
			_cursor += sizeof(char);
		}
		return (*this);
	}
	COutDataStream& operator << (unsigned short val)
	{
		if ((_cursor + sizeof(unsigned short)) <= Size)
		{
			*((unsigned short *)(_data + _cursor)) = val;
			_cursor += sizeof(unsigned short);
		}
		return (*this);
	}
	COutDataStream& operator << (unsigned int val)
	{
		if ((_cursor + sizeof(unsigned int)) <= Size)
		{
			*((unsigned int *)(_data + _cursor)) = val;
			_cursor += sizeof(unsigned int);
		}
		return (*this);
	}
	COutDataStream& operator << (unsigned long val)
	{
		if ((_cursor + sizeof(unsigned long)) <= Size)
		{
			*((unsigned long *)(_data + _cursor)) = val;
			_cursor += sizeof(unsigned long);
		}
		return (*this);
	}
	COutDataStream& operator << (LONG64 val)
	{
		if ((_cursor + sizeof(LONG64)) <= Size)
		{
			*((LONG64 *)(_data + _cursor)) = val;
			_cursor += sizeof(LONG64);
		}
		return (*this);
	}

public:
	void Write(void* buf, unsigned long count)
	{
		if ((_cursor + count) <= Size)
		{
			memcpy((_data + _cursor), buf, count);
			_cursor += count;
		}
	}

private:
	char _data[Size];
	unsigned long _cursor;
};

#define DEFAULT_OUTDATASTREAM_SIZE	512
typedef COutDataStream<512> DefaultOutDataStream;
#endif