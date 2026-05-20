
#include "stdh.h"

#include "sharedPtr.h"

#include "stdio.h"

RefType::RefType()
{
	_oldVer = 0;
	_proxy = NULL;
}
RefType::RefType(const RefType &oth)
{
	_oldVer = 0;
	_proxy = NULL;
}
RefType & RefType::operator =(const RefType &oth)
{
	_oldVer = 0;
	_proxy = NULL;
	return *this;
}
void RefType::Release()
{
	_proxy->Break(this);
}
inline BOOL RefType::IsBreak()//该数据引用是否断开
{
	return (!_proxy);
}
//得到数据的改变
BOOL RefType::FetchChange()
{
	DWORD nVer = _GetVersion();
	if(nVer!=_oldVer){
		_oldVer = nVer;
		return TRUE;
	}
	return FALSE;
}

BOOL RefType::IsChange()
{
	return _GetVersion()!=_oldVer;
}

void RefType::ClearChange()
{
	_oldVer = _GetVersion();
}
void * RefType::GetObj()
{
	//一个正常的引用至少有两个引用计数 Target 和 Ref本身
	if(_proxy)
		return _proxy->_GetObj();
	return NULL;
}

RefType * RefType::GetRef()
{
	if(_proxy)
		return _proxy->GetRef();
	return NULL;
}

BOOL RefType::CommitChange() //提交变化
{
	if(_proxy){
		if(_proxy->CommitChange()){
			_oldVer = _GetVersion();
			return TRUE;
		}
	}
	return FALSE;
}
DWORD RefType::_GetVersion()
{
	DWORD ver = 0;
	if(_proxy){
		ver = _proxy->_GetVersion();
	}
	return ver;
}

//////////////////////////////////////////////////////////////////////////
RefProxy::~RefProxy()
{
}
void RefProxy::Break(RefType * p)
{
	Free(p);
	if(IsFree())
		Class_Delete(this);
}
void RefProxy::OnDataLost() //RefTarget 死亡通知
{
	_tag = NULL;
	if(IsFree())
		Class_Delete(this);
}
RefType * RefProxy::Alloc()
{
	RefType * pRef = NULL;
	//首先利用一个空闲的引用
	if(!_idles.empty()){
		RefType * p = _idles.back();
		_idles.pop_back();
		_Node * n = (_Node*)(p);
		assert(p==&(n->p));
		assert(n->bFreed = true);
		n->bFreed = false;
		pRef = p;
	}
	//创建一个新的
	if(!pRef){
		size_t icur = _refs.size();
		_refs.resize(icur+1);
		_Node & n = _refs.back();
		n.bFreed = false;
		pRef = &(n.p);
	}

	return pRef;
}

void RefProxy::Free(RefType * p)
{
	assert(p);
	_Node * n = (_Node *)(p);
	n->bFreed = true;
	_idles.push_back(p);	
}

BOOL RefProxy::IsFree() //连接的两方都无联系
{
	if(!_tag&&(_idles.size()==_refs.size()))
		return TRUE;
	return FALSE;
}
void RefProxy::_FreeAll()
{
	for(int i = 0;i<_refs.size();i++){
		_Node & n = _refs[i];
		if(!n.bFreed){
			n.bFreed = true;
			_idles.push_back(&(n.p));
		}
	}
	assert(_idles.size()==_refs.size());
}

RefType * RefProxy::GetRef() //会导致引用计数增加
{
	RefType * pRef = NULL;
	if(_tag){
		pRef = Alloc();
		pRef->_oldVer = 0;
		pRef->_proxy = this;
	}
	return pRef;
}
BOOL RefProxy::CommitChange()
{
	if(_tag){
		_tag->OnRefChange();
		return TRUE;
	}
	return  FALSE;
}
void * RefProxy::_GetObj()
{
	if(_tag)
		return _tag->_GetObj();
	return NULL;
}
DWORD RefProxy::_GetVersion()
{
	if(_tag)
		return _tag->_version;
	return 0;
}
//////////////////////////////////////////////////////////////////////////
TargetType::TargetType(const TargetType &oth)
{
	_proxy = NULL;
	_version = 1;
}
TargetType & TargetType::operator =(const TargetType &oth)
{
	_proxy = NULL;
	_version = 1;
	return *this;
}
TargetType::TargetType()
{
	_proxy = Class_New2(RefProxy);
	_proxy->SetTarget(this);
	_version = 1;
	_oldVer = 0;
}
TargetType::~TargetType()
{
	Break();
}
void TargetType::OnRefChange() //提交变化 增加版本
{
	if(_funOnChange.empty()) //异步更新
		_version++;
	else					 //即使更新
		_funOnChange();
}
void TargetType::CommitChange()
{
	_version++;
	_oldVer = _version;
	if(!_funOnChange.empty())
		_funOnChange();
}

BOOL TargetType::FetchChange() //自从上次调用该方法 数据是否有变化
{
	if(_oldVer!=_version){
		_oldVer = _version;
		return TRUE;
	}
	return FALSE;
}
BOOL TargetType::IsChange()
{
	return _version!=_oldVer;
}
void TargetType::ClearChange()
{
	_oldVer = _version;
}
void TargetType::Break()
{
	assert(_proxy);
	_proxy->OnDataLost();
}

RefType * TargetType::GetRef()
{
	return _proxy->GetRef();
}
//////////////////////////////////////////////////////////////////////////
RefPtr::RefPtr()
{
	_ptr = NULL;
}
RefPtr::~RefPtr()
{
	Free();
}
void RefPtr::Free()
{
	if(_ptr){
		_ptr->Release();
		_ptr = NULL;
	}
}
RefPtr::RefPtr(RefType * ref)
{
	if(ref) //有效的引用
		_ptr = ref;
	else
		_ptr = NULL;
}
RefPtr::RefPtr(const RefPtr & oth)
{
	_ptr = NULL;
	if(oth._ptr){
		if(!oth._ptr->IsBreak())
			_ptr = oth._ptr->GetRef(); //得到一个新的引用指向相同的数据源
	}
}
RefPtr & RefPtr::operator= (RefType * ref)
{
	//如果当前指针非空释放当前指针
	Free();

	if(ref&&!(ref->IsBreak())) //有效的引用
		_ptr = ref;
	else
		_ptr = NULL;

	return *this;
}

RefPtr & RefPtr::operator= (const RefPtr & oth)
{
	//如果当前指针非空释放当前指针
	Free();

	if(oth._ptr){
		RefType * ptr = oth._ptr->GetRef(); //得到一个新的引用指向相同的数据源
		_ptr = ptr;
	}

	return *this;
}
BOOL RefPtr::IsBreak()
{
	if(!_ptr)
		return TRUE;

	if(_ptr->IsBreak()){
		Free();
		return TRUE;
	}

	return FALSE;
}
BOOL RefPtr::IsChange()
{
	if(_ptr)
		return _ptr->IsChange();
	return FALSE;
}
BOOL RefPtr::FetchChange()
{
	if(_ptr)
		return _ptr->FetchChange();
	return FALSE;
}
void RefPtr::ClearChange()
{
	if(_ptr)
		_ptr->ClearChange();
}
//////////////////////////////////////////////////////////////////////////







