#pragma once


//bufsize is in DWORD
template<int bufsize>
class Bitset
{
public:
	Bitset()
	{
		resize(bufsize*32);
		resetAll();
	}
	void resize(DWORD nBits)
	{
		_nBits=nBits;
		if (_nBits>bufsize*32)
			_nBits=bufsize*32;
	}
	//set to 1
	void set(DWORD iPos)
	{
		if (iPos>=_nBits)
			return;
		_buf[iPos/32]|=(1<<(iPos%32));
	}
	void setAll()//set all to 1
	{
		memset(_buf,0xff,(_nBits+7)/8);
	}
	void reset(int iPos)//set to 0
	{
		if (iPos>=_nBits)
			return;
		_buf[iPos/32]&=(~(1<<(iPos%32)));
	}
	void resetAll()//set all to 0
	{
		memset(_buf,0,(_nBits+7)/8);
	}

	void setN(DWORD nBits)//将前nBits个位设成1
	{
		if (nBits>_nBits)
			nBits=_nBits;
		memset(_buf,0xff,(nBits+7)/8);
	}

	void resetN(DWORD nBits)//将前nBits个位设成0
	{
		if (nBits>_nBits)
			nBits=_nBits;
		memset(_buf,0,(nBits+7)/8);
	}


	BOOL test(int iPos) const
	{
		if (iPos>=_nBits)
			return FALSE;
		return (_buf[iPos/32]&(1<<(iPos%32)))!=0;
	}
	DWORD size() const
	{
		return _nBits;
	}
	BOOL all() const
	{
		int n=_nBits/32;
		int i;
		for (i=0;i<n;i++)
		{
			if (_buf[i]!=0xffffffff)
				return FALSE;
		}
		DWORD remain=_nBits%32;
		if (remain>0)
		{
			remain=(1<<remain)-1;
			if ((_buf[i]&remain)!=remain)
				return FALSE;
		}
		return TRUE;
	}
	BOOL none() const
	{
		int n=_nBits/32;
		int i;
		for (i=0;i<n;i++)
		{
			if (_buf[i]!=0)
				return FALSE;
		}
		DWORD remain=_nBits%32;
		if (remain>0)
		{
			remain=(1<<remain)-1;
			if ((_buf[i]&remain)!=0)
				return FALSE;
		}
		return TRUE;
	}
	Bitset&operator=(const Bitset&src)
	{
		int n=(src._nBits+31)/32;
		memcpy(_buf,src._buf,n*sizeof(DWORD));
		_nBits=src._nBits;
		return *this;
	}
	Bitset&operator|=(const Bitset&src)
	{
		int n=(_nBits+31)/32;
		for (int i=0;i<n;i++)
			_buf[i]|=src._buf[i];
		return *this;
	}
	Bitset&operator&=(const Bitset&src)
	{
		int n=(_nBits+31)/32;
		for (int i=0;i<n;i++)
			_buf[i]&=src._buf[i];
		return *this;
	}
	//反转
	void inverse()
	{
		int n=(_nBits+31)/32;
		for (int i=0;i<n;i++)
			_buf[i]=~_buf[i];
	}

	BYTE *getdata()	{		return (BYTE*)_buf;	}
	DWORD getdatasize()//in byte
	{
		return bufsize*sizeof(DWORD);
	}
	void setdata(BYTE *data)
	{
		memcpy(_buf,data,getdatasize());
	}

	BOOL assign(DWORD v,DWORD iBit,DWORD nBit)//assign a nBit unsigned value to a data starting from iBit,
	{
		//a very slow implement,just for test
		if(iBit+nBit>=size())
			return FALSE;
		Bitset<1> t;
		memcpy(t.getdata(),&v,sizeof(DWORD));
		for(int i=0;i<nBit;i++)
		{
			if(t.test(i))
				set(iBit+i);
			else
				reset(iBit+i);
		}
		return TRUE;
	}


protected:
	DWORD _nBits;
	DWORD _buf[bufsize];
};

//bufsize is in DWORD
template<int bufsize,BOOL bDef>
class BitsetStack
{
public:
	BitsetStack()
	{
		n=0;
	}
	void push(BOOL b)
	{
		if (b)
			bitset.set(n);
		else
			bitset.reset(n);
		n++;
	}
	void pop()
	{
		if (n>0)
			n--;
	}
	BOOL cur()
	{
		if (n>0)
			return bitset.test(n-1);
		return bDef;
	}
	Bitset<bufsize> bitset;
	WORD n;
};
