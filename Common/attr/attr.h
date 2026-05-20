
#pragma once

typedef BYTE AttrPriority;

struct AttrNodeBase
{
	virtual CClass *GetClass()=0;
	int AddRef(){return ++ref;}
	int Release()
	{
		ref--;
		if (ref<=0)
		{
			_Destroy();
			Class_Delete(this);
			return 0;
		}
		return ref;
	}

	void Destroy()
	{
		_Destroy();
		Release();
	}
	BOOL IsAlive()	{		return bAlive;	}

	void _Destroy()
	{
		if (bAlive)
		{
			if (pOwnerDirty)
				*pOwnerDirty=1;
			bAlive=0;
		}
	}

	short ref;
	BYTE bAlive:1;
	AttrPriority priority;
	AttrNodeBase *next;
	BYTE *pOwnerDirty;//指向ower的Dirty标志
};

template <typename T>
struct AttrNode:public AttrNodeBase
{
	T v;
};


template <typename T,typename T_Node>
struct AttrMod
{
	AttrMod()
	{
		nodes=NULL;
	}

	void Clear()
	{
		while(nodes)
		{
			AttrNodeBase *next=nodes->next;
			nodes->Destroy();
			nodes=(T_Node*)next;
		}
	}

	AttrNodeBase *Add(T v,AttrPriority priority)//返回值带一个引用计数
	{
		T_Node *node=Class_New2(T_Node);
		node->priority=priority;
		node->bAlive=1;
		node->ref=2;//一个是自己的,一个是返回的
		node->next=NULL;
		node->pOwnerDirty=NULL;

		node->v=v;

		T_Node **pp=&nodes;

		while(1)
		{
			if ((*pp)==NULL)
			{
				(*pp)=node;
				break;
			}

			if ((*pp)->priority<=priority)
			{//插在这里
				node->next=(*pp);
				(*pp)=node;
				break;
			}

			pp=(T_Node**)&((*pp)->next);
		}

		return node;
	}

	BOOL IsValid()
	{
		while(nodes)
		{
			if (!nodes->IsAlive())
			{
				AttrNodeBase *next=nodes->next;
				nodes->Release();
				nodes=(T_Node*)next;
			}
			else
				break;
		}
		return nodes!=NULL;
	}

	BOOL GetValue(T &v)
	{
		while(nodes)
		{
			if (!nodes->IsAlive())
			{
				AttrNodeBase *next=nodes->next;
				nodes->Release();
				nodes=(T_Node*)next;
			}
			else
				break;
		}
		if (nodes)
		{
			v=nodes->v;
			return TRUE;
		}
		return FALSE;
	}
	T_Node *nodes;
};


template <typename T,typename T_Node>
struct AttrSum
{
	AttrSum()
	{
		nodes=NULL;
		bDirty=0;
		sum=(T)0;
	}

	void Clear()
	{
		while(nodes)
		{
			AttrNodeBase *next=nodes->next;
			nodes->Destroy();
			nodes=(T_Node*)next;
		}
	}


	AttrNodeBase *Add(T v)//返回值带一个引用计数
	{
		T_Node *node=Class_New2(T_Node);
		node->priority=0;
		node->bAlive=1;
		node->ref=2;
		node->next=NULL;
		node->pOwnerDirty=&bDirty;

		node->v=v;

		sum+=v;

		node->next=nodes;
		nodes=node;

		return node;
	}

	T GetValue()
	{
		if(bDirty)
		{
			sum=(T)0;
			T_Node**pp=&nodes;
			while(*pp)
			{
				if ((*pp)->IsAlive())
				{
					sum+=(*pp)->v;
					pp=(T_Node**)&(*pp)->next;
				}
				else
				{
					T_Node *t=(*pp);
					pp=(T_Node**)&(*pp)->next;
					t->Release();
				}
			}

			bDirty=0;
		}
		return sum;
	}
	T_Node *nodes;
	T sum;
	BYTE bDirty;
};

template <typename T,typename T_Node>
struct AttrAddOn:public AttrSum<T,T_Node>
{
	AttrAddOn()
	{
		base=(T)0;
	}

	void SetBase(T v)
	{
		base=v;
	}

	T GetValue()
	{
		return base+AttrSum<T,T_Node>::GetValue();
	}

	T base;
};

//AttrNodes
struct AttrNodeFloat:public AttrNode<float>
{
	DEFINE_CLASS(AttrNodeFloat);
};


struct AttrNodeWord:public AttrNode<WORD>
{
	DEFINE_CLASS(AttrNodeWord);
};

struct AttrNodeShort:public AttrNode<short>
{
	DEFINE_CLASS(AttrNodeShort);
};


//Attrs
struct AttrMod_Float:public AttrMod<float,AttrNodeFloat>
{
	
};

struct AttrSum_Float:public AttrSum<float,AttrNodeFloat>
{

};


struct AttrMod_Word:public AttrMod<WORD,AttrNodeWord>
{

};

struct AttrSum_Word:public AttrSum<WORD,AttrNodeWord>
{

};

struct AttrAddOn_Word:public AttrAddOn<WORD,AttrNodeWord>
{

};

struct AttrAddOn_Float:public AttrAddOn<float,AttrNodeFloat>
{

};
