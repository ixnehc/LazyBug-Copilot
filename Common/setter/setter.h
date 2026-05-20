#pragma once

template <typename T>
struct Setter
{
	Setter(T &v,T bNew)
	{
		pv=&v;
		bOld=v;
		v=bNew;
	}
	~Setter()
	{
		*pv=bOld;
	}
	T bNew,bOld;
	T *pv;
};

typedef Setter<BOOL> BoolSetter;
