
#pragma once

template <class T,DWORD T_bit>
struct DestroyCache
{
	void Add(T *p)
	{
		if (p->TestBit(T_bit))
		{
			p->Release();
			return;
		}
		p->SetBit(T_bit);
		buf.push_back(p);
	}
	void Flush()
	{
		int i=0;
		while(i<buf.size())
		{
			buf[i]->Destroy();
			buf[i]->ClearBit(T_bit);
			i++;
		}
		buf.clear();
	}
	std::vector<T *>buf;
};

