#include "stdh.h"
#include ".\DataLinkBase.h"

#define DEFAULT_MAX_VALUE  99999999.0f

CLinkInBase::CLinkInBase(void)
{
	m_bFloat = TRUE;
	_link_min = NULL;
	_link_max = NULL;
	_bminlimiter = false;
	_bmaxlimiter = false;
	_range_min = - DEFAULT_MAX_VALUE;
	_range_max =   DEFAULT_MAX_VALUE;
	m_bLock = FALSE;
}
CLinkInBase::~CLinkInBase(void)
{
}
void CLinkInBase::SetLimitMin(ILinkIn * link,bool bLimiter)
{
	_bminlimiter = bLimiter;
	_link_min = (CLinkInBase *)link;
	_OnLimits(); //更新用户表现
}
void CLinkInBase::SetLimitMax(ILinkIn * link,bool bLimiter)
{
	_link_max = (CLinkInBase *)link;
	_bmaxlimiter = bLimiter;
	_OnLimits(); //更新用户表现
}
bool CLinkInBase::SetValue(int value,bool bNotify,bool bCommit)
{
	if(IsLocked())
		return true;

	float fv = float(value);
	
	if(false==_PrepareSet(fv,bNotify,bCommit)){
		_RollBack();
		return false;
	}

	OnSetValue(value,bNotify,bCommit);
	Flush(bNotify,bCommit);

	return true;
}
bool CLinkInBase::SetValue(float value,bool bNotify,bool bCommit)
{
	if(IsLocked())
		return true;

	if(false==_PrepareSet(value,bNotify,bCommit)){
		_RollBack();
		return false;
	}

	OnSetValue(value,bNotify,bCommit);
	Flush(bNotify,bCommit);

	return true;
}

bool CLinkInBase::_PrepareSet(float value,bool bNotify,bool bCommit)
{
	bool bOk = true;

	float minv = _range_min ,maxv = _range_max;

	if(_link_min&&_bminlimiter) //得到最小限制
	{
		float f = _link_min->GetFVal();
		minv = max(minv,f);
	}
	if(_link_max&&_bmaxlimiter) //得到最大限制
	{
		float f = _link_max->GetFVal();
		maxv = min(maxv,f);	
	}

	if(value<minv||value>maxv)
		bOk = false;
	
	//推进最小值 ，使下限限制数据中心小于当前值
	if(_link_min&&(!_bminlimiter)) // _link_min is pushed by me 
	{
		float fv = _link_min->GetFVal();
		if(fv>value)
			bOk = bOk&&_link_min->_PrepareSet(value,bNotify,bCommit);	
	}

	//推进最大值 ，使下限限制数据中心大于当前值
	if(_link_max&&(!_bmaxlimiter))
	{
		float fv = _link_max->GetFVal();
		if(fv<value)
			bOk = bOk&&_link_max->_PrepareSet(value,bNotify,bCommit);
	}
	
	return bOk;
}
BOOL CLinkInBase::_LinkMe(ILinkOut * custCtrl)
{
	if(!custCtrl)
		return FALSE;

	CLinkOutBase * linkOut = (CLinkOutBase *)custCtrl;

	BOOL bFloat = linkOut->IsFloat();
	if(bFloat!=m_bFloat)
		return FALSE;

	for(int i = 0;i<_links_to_me.size();i++)
		if(_links_to_me[i]==linkOut)
			return TRUE;

	_links_to_me.push_back(linkOut);
	
	float value = GetFVal();


	if(m_bFloat)
		linkOut->OnSetValue(value);
	else
		linkOut->OnSetValue(int(value));

	return TRUE;
}

void CLinkInBase::Flush(bool bNotify,bool bCommitChange) //不需要进行数据校验 因为此时得到的数据已经确保正确
{
	float value = GetFVal();
	
	Lock();
	for(int i = 0;i<_links_to_me.size();i++)
	{
		CLinkOutBase * linkOut = (CLinkOutBase *)(_links_to_me[i]);
		if(linkOut->IsLocked())
			continue;

		if(m_bFloat)
			linkOut->OnSetValue(value);
		else
			linkOut->OnSetValue(int(value));
		
		//刷新与其相连的任何一个数据中心
		linkOut->Flush(bNotify,bCommitChange);
	}
	UnLock();
}

void CLinkInBase::_RollBack(CLinkOutBase * custCtrl/* = NULL*/)
{
	if(custCtrl){ //为LinkOut提供的恢复
		float f = GetFVal();
		custCtrl->OnSetValue(f);
	}
	else{ //自身发生改变 提供的恢复
		float f = GetFVal();
		float minV,maxV;
		GetLimits(minV,maxV);
		f = i_math::clamp_f(f,minV,maxV);
		Flush(false,true);
	}
}
void CLinkInBase::_OnLimits()
{
	float minV,maxV;
	GetLimits(minV,maxV);
	
	OnSetLimits(minV,maxV);
	for(int i = 0;i<_links_to_me.size();i++){
		CLinkOutBase * pLinkOut = _links_to_me[i];
		pLinkOut->OnSetLimits(minV,maxV);
	}
}

void CLinkInBase::SetLimits(float minValue,float maxValue) //设置上下限时不想向发出数据改变通知
{
	assert(minValue<=maxValue);
	_range_min = minValue;
	_range_max = maxValue;

	float value = GetFVal();
	value = i_math::clamp_f(value,_range_min,_range_max);
	OnSetValue(value,false,false);

	Flush(false,true); //确保此时没有任何CLinkOutBase处于锁定状态

	_OnLimits(); //更新用户表现
}

void CLinkInBase::GetLimits(float & minValue,float &maxValue)
{
	minValue = _range_min;
	maxValue = _range_max;

	if(_link_min&&_bminlimiter)
		minValue = max(_range_min,_link_min->GetFVal());
	if(_link_max&&_bmaxlimiter)
		maxValue = min(_range_max,_link_max->GetFVal());
}
void CLinkInBase::Enable(BOOL bOnoff)
{
	for(int i = 0;i<_links_to_me.size();i++){
		CLinkOutBase * link = (CLinkOutBase *)(_links_to_me[i]);
		link->Enable(bOnoff);
	}
	OnEnable(bOnoff);
}
//////////////////////////////////////////////////////////////////////////
CLinkOutBase::CLinkOutBase()
{
	m_bFloat = TRUE;
	m_bLock = FALSE;
	_range_min = - DEFAULT_MAX_VALUE;
	_range_max =   DEFAULT_MAX_VALUE;
}
CLinkOutBase::~CLinkOutBase()
{

}
bool CLinkOutBase::SetValue(int value,bool bNotify,bool bCommit)
{
	if(false== _PrepareSet(float(value))) //提交给LinkIn校验 ，校验不成功会被设置为 LinkIn的当前值
		return false;

	OnSetValue(value);
	Flush(bNotify,bCommit);

	return true;
}
bool CLinkOutBase::SetValue(float value,bool bNotify,bool bCommit)
{
	if(false== _PrepareSet(value))
		return false;

	OnSetValue(value);
	Flush(bNotify,bCommit);

	return true;
}
BOOL CLinkOutBase::LinkTo(ILinkIn * custCtrl)
{
	assert(custCtrl);
	int i = 0;
	for(;i< _links_to_other.size();i++)
		if(_links_to_other[i] == custCtrl)
			break;

	if(FALSE==((CLinkInBase *)custCtrl)->_LinkMe(this))
		return FALSE;

	if(i>=_links_to_other.size())
		_links_to_other.push_back(((CLinkInBase *)custCtrl));

	return TRUE;
}
void CLinkOutBase::Flush(bool bNotify,bool bCommit)
{
	float value = GetFVal();  //不需要检验
	
	Lock();
	for(int i = 0;i<_links_to_other.size();i++)
	{
		ILinkIn * linkIn = _links_to_other[i];
		if(m_bFloat)
			((CLinkInBase *)linkIn)->SetValue(value,bNotify,bCommit); //SetValue会进行校验
		else
			((CLinkOutBase *)linkIn)->SetValue(int(value),bNotify,bCommit);
	}
	UnLock();
}

bool CLinkOutBase::_PrepareSet(float value)
{
	for(int i = 0;i<_links_to_other.size();i++)
	{
		CLinkInBase * link = (CLinkInBase *)(_links_to_other[i]);
		if(false==link->_PrepareSet(value,false,false)){
			link->_RollBack(this); //将值回滚到正确值	
			return false;
		}
	}

	return true;
}
void CLinkOutBase::Enable(BOOL bOnoff)
{
	OnEnable(bOnoff);
}
