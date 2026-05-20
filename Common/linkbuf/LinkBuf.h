#pragma once

template<typename T,int T_bufsize>
class CLinkBuf
{
public:
	template<typename T,int T_bufsize>
	struct Iter
	{
		Iter()
		{
			memset(this,0,sizeof(*this));
		}
		BOOL IsEnd()
		{
			return idx>=idxEnd;
		}

		Iter&operator++()
		{
			idx++;
			if (idx<T_bufsize)
				ptr++;
			else
			{
				iNode++;
				idx=0;
				if (iNode>=owner->_nodes.size())
				{
					idxEnd=0;
					ptr=NULL;
				}
				else
				{
					if (!owner->_nodes[iNode])
						owner->_nodes[iNode]=new Node<T,T_bufsize>;
					ptr=owner->_nodes[iNode]->buf;
					idxEnd=owner->_sz-iNode*T_bufsize;
				}
			}
			return *this;
		}
		Iter operator++(int)
		{
			Iter _Tmp = *this;
			++*this;
			return (_Tmp);
		}

		T &operator*()
		{
			return *ptr;
		}

		T *ptr;
		CLinkBuf *owner;
		DWORD iNode;
		DWORD idx;//在node中的位置
		DWORD idxEnd;
	};

	typedef Iter<T,T_bufsize> iterator;

	template<typename T,int T_bufsize>
	struct Node
	{
		T buf[T_bufsize];
	};


	~CLinkBuf()
	{
		Clear();

		for (DWORD i=0;i<_nodes.size();i++)
		{
			SAFE_DELETE(_nodes[i]);
		}
		_nodes.clear();
	}

	void Resize(DWORD sz)
	{
		DWORD szNode=(sz+T_bufsize-1)/T_bufsize;
// 		DWORD szOld=_nodes.size();
// 		for (DWORD i=szNode;i<szOld;i++)
// 		{
// 			SAFE_DELETE(_nodes[i]);
// 		}
		if (szNode>_nodes.size())
			_nodes.resize(szNode);
		_sz=sz;
	}
	DWORD Size()
	{
		return _sz;
	}
	void Clear()
	{
		Resize(0);
	}

	iterator GetIter(DWORD idx)
	{
		iterator it;
		if (idx>=_sz)
			return it;//空的

		DWORD iNode=idx/T_bufsize;
		if (!_nodes[iNode])
			_nodes[iNode]=new Node<T,T_bufsize>;
		it.iNode=iNode;
		it.idx=idx%T_bufsize;
		it.ptr=_nodes[iNode]->buf+it.idx;
		it.idxEnd=_sz-iNode*T_bufsize;
		it.owner=this;

		return it;
	}

	//不作校验
	T &GetAt(DWORD idx)
	{
		DWORD iNode=idx/T_bufsize;
		if (!_nodes[iNode])
			_nodes[iNode]=new Node<T,T_bufsize>;

		return _nodes[iNode]->buf[idx%T_bufsize];
	}

public://take it as protected
	std::vector<Node<T,T_bufsize>*>_nodes;
	DWORD _sz;



};
