
#pragma once

#include "../class/class.h"

#include <deque>

#include "assert.h"

#include "../fastdelegate/FastDelegate.h"

// 用于数据共享，一个数据源 多个地方使用 只有数据源才能更该数据 
// 其他的地方只会使用数据 通过GetObj得到的数据指针不能保存 只能该次使用 RefType不再使用时Release

class RefType;
class RefTarget;

class RefProxy;
class RefPtr;

class RefType
{
	//试图通过构造或和赋值都将得到无效的引用	
private:
	RefType(const RefType &oth);
	RefType & operator =(const RefType &oth);
public:	
	RefType(void);
	virtual ~RefType(){}

	void Release();
	inline BOOL IsBreak();  //该数据引用是否断开

	BOOL FetchChange();
	BOOL IsChange();
	void ClearChange();
	BOOL CommitChange(); //提交变化

	void * GetObj();
	RefType * GetRef();

protected:
	DWORD _GetVersion();

	friend class TargetType;
	friend class RefPtr;
	friend class RefProxy;
protected:
	RefProxy * _proxy;
	DWORD   _oldVer;
};

//////////////////////////////////////////////////////////////////////////
class  RefProxy
{
public:
	DEFINE_CLASS(RefProxy);
	RefProxy(){_tag = NULL;}
	~RefProxy();
	void Break(RefType * p);
	RefType * Alloc();
	void Free(RefType * p);
	BOOL IsFree();
	
	void SetTarget(TargetType * tag){_tag = tag;}
	void OnDataLost();

	BOOL CommitChange();
	void * _GetObj();
	RefType * GetRef();
	DWORD _GetVersion();

	struct _Node{
		_Node(){bFreed = false;}
		_Node(const _Node & n){
			memcpy(&p,&(n.p),sizeof(RefType));			
			bFreed = n.bFreed;
		}
		_Node & operator = (const _Node & n){
			memcpy(&p,&(n.p),sizeof(RefType));			
			bFreed = n.bFreed;
			return *this;
		}
		RefType  p;
		bool bFreed;
	};

protected:
	void _FreeAll();
private:
	std::deque<_Node> _refs;
	std::vector<RefType *> _idles;
	TargetType * _tag;
};

//////////////////////////////////////////////////////////////////////////
class TargetType
{
private:
	TargetType(const TargetType &oth);
	TargetType & operator =(const TargetType &oth);

public:	
	TargetType();
	~TargetType();

	virtual void * _GetObj() = 0;
	void OnRefChange(); //提交变化 增加版本
	void CommitChange();
	BOOL FetchChange(); //自从上次调用该方法 数据是否有变化
	BOOL IsChange();
	void ClearChange();
	void Break();

	//同步模式下 设置 处理数据改变时 立即响应的函数
	typedef fastdelegate::FastDelegate0<void> FunOnChange;
	template <class T_proxy>void SetCallBack( T_proxy * pThis, 
											  void(T_proxy:: * fun)(void) ){
		_funOnChange.bind(pThis,fun);
	}

	RefType * GetRef();
	
	friend class  RefProxy;
protected:
	DWORD _version;
	DWORD _oldVer;
	FunOnChange _funOnChange;
	RefProxy * _proxy;
};

//////////////////////////////////////////////////////////////////////////
class RefPtr
{
public:
	RefPtr();
	RefPtr(RefType * ref);
	virtual ~RefPtr();

	RefPtr(const RefPtr & oth);
	RefPtr & operator= (RefType * ref);
	RefPtr & operator= (const RefPtr & oth);

	BOOL IsBreak();
	BOOL IsChange();
	BOOL FetchChange();
	void ClearChange();
	void Free();
protected:
	RefType * _ptr;
};

//////////////////////////////////////////////////////////////////////////
template<class TObj>
class SharedData :public TargetType			//以数据块的方式分享数据
{
public:
	virtual void * _GetObj(){return &_obj;}
	virtual TObj & GetObj(){return _obj;}
private:
	TObj  _obj;
};

template<class TObj>
class SharedPtr:public TargetType		  //以数据指针的方式分享数据
{
public:
	SharedPtr(){_pObj = NULL;}
	SharedPtr(TObj * pObj){_pObj = pObj;}
	virtual void * _GetObj(){return _pObj;}
	TObj * GetObj(){return _pObj;}
	void SetObj(TObj * pObj){
		_pObj = pObj;
		_version++;
	}
private:
	TObj * _pObj;
};

//////////////////////////////////////////////////////////////////////////
template< class TObj>
class RefConPtr :public RefPtr		//分发的数据为只读的
{
public:
	RefConPtr(){}
	RefConPtr(RefType * p)
	:RefPtr(p){}
	const TObj * GetObj()
	{
		void * obj = NULL;
		if(_ptr)
			return (TObj *)(_ptr->GetObj());
		return NULL;
	}
};


/************************************************************************/
/*RefType 数据引用的抽象 通过它能找到被分享的数据
/*TargetType 共享数据的分发器 通过它将数据以RefType的形式分发除去 并保持与RefType的交流
/*数据链：它的一端连接RefType 另一端连接RefTarget
/*RefTarget :为RefType连接的另一端 ，当连接有效时连接的另一端TargetType ,但是有时候TargetType先与RefType
/*死亡，或TargetType中断数据共享，都将主动与RefType断开联系，这时候由RefPool代为接管来伺候 RefType的死亡。
/*RefPool:提供一个TargetType的数据引用分配服务。
/*如果RefPool 服务的对象 RefTarget 和 RefType 都已经死亡 
/*此时 RefPoolManager 将回收RefPool 为新的 RefTarget服务。


/*危险的行为 如果RefType 已经无效 但是不去释放自己的连接 ，导致为其提供服务的RefPool始终处于僵死的状态
/*解决方法：通知RefType的拥有者 尽快释放无效的引用
/************************************************************************/





