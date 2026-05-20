#pragma once

typedef DWORD HookPriority;

#define HookPriorityLow(v) (1+(v))
#define HookPriorityMedium(v) (100+(v))
#define HookPriorityHi(v) (200+(v))


template<typename T_clss,typename T_dlgt>
struct HookNode
{
	T_clss *owner;
	T_dlgt func;
	DWORD prior;
};

template<typename T_clss,typename T_dlgt,DWORD szEntry>
class CHooks
{
public:
	typedef HookNode<T_clss,T_dlgt> Node;
	void Reset()
	{
		for (int i=0;i<szEntry;i++)
			_entries[i].clear();
	}
	BOOL Register(DWORD iEntry,T_clss *owner,T_dlgt func,HookPriority prior)
	{
		Node t;
		t.owner=p;
		t.func=func;
		t.prior=prior;

		std::vector<Node> &entry=_entries[iEntry];

		Node *p=&entry[0];
		DWORD sz=entry.size();
		for (int i=0;i<sz;i++)
		{
			if (p->prior<=prior)
				break;
		}
		if (i<sz)
			entry.insert(entry.begin()+i,t);
		else
			entry.push_back(t);

		return TRUE;
	}

protected:
	std::vector<Node> _entries[szEntry];
};
