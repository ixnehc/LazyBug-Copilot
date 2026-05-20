
#include "stdh.h"

#include ".\pin.h"

#define DEFAULT_MAX_VALUE  99999999.0f

CPinboard::CPinboard(void)
{
	_bFloat = TRUE;
	_link_min = NULL;
	_link_max = NULL;
	_bminlimiter = false;
	_bmaxlimiter = false;
	_range_min = - DEFAULT_MAX_VALUE;
	_range_max =   DEFAULT_MAX_VALUE;
	_bLock = FALSE;
	_bMessageLock = FALSE;
}
CPinboard::~CPinboard(void)
{

}
void CPinboard::Lock()
{
	_bLock = TRUE;
}

void CPinboard::UnLock()
{
	_bLock = FALSE;
}

BOOL CPinboard::SetValue(int value,BOOL bNotify/* = FALSE*/)
{
	//已经相等了
	if(GetIVal()==value)
		return TRUE;

	if(IsLocked())
		return TRUE;
	
	return _CheckAndSetValue(EndChange,float(value),bNotify);//手工设置不再向外发送消息
}
BOOL CPinboard::SetValue(float value,BOOL bNotify/* = FALSE*/)
{
	//已经相等了
	if(GetFVal()==value)
		return TRUE;

	if(IsLocked())
		return TRUE;

	return _CheckAndSetValue(EndChange,value,bNotify);//手工设置不再向外发送消息
}

BOOL CPinboard::_CheckAndSetValue(ChangeState state,float value,BOOL bSendMessage)
{	
	Lock();

	//检验并提交修改
	if(FALSE==_CheckValid(value)){
		UnLock();
		return FALSE;
	}

	if(!_PushLimiter(state,value,bSendMessage)){
		UnLock();	
		return FALSE;
	}
	
	//同步到 Pin
	for(int i = 0;i<_links_to_me.size();i++){
		CPin * pin = _links_to_me[i];
		if(pin->IsLocked())
			continue;
		if(_bFloat)
			pin->OnSetValue(value,state);
		else
			pin->OnSetValue(int(value),state);
	}
	
	if(_bFloat)
		OnSetValue(value,state);
	else
		OnSetValue(int(value),state);

	if(bSendMessage&&(!IsMessageLocked())){
		LockMessage();
		OnNotifyChange(state);		//通知数据发生了改变
		UnLockMessage();
	}

	
	UnLock();

	return TRUE;
}

BOOL CPinboard::_CheckValid(float value)
{
	float minv,maxv; //默认的限制	
	GetLimits(minv,maxv);
	return (value>=minv&&value<=maxv);
}

BOOL CPinboard::_PushLimiter(ChangeState state,float value,BOOL bSendMessage)
{
	//推进最小值 ，使下限限制数据中心小于当前值
	if(_link_min&&(!_bminlimiter)){
		float fv = _link_min->GetFVal();
		if(value<fv){
			if(_link_min->IsLocked()) //避免约束死锁
				return TRUE;

			if(FALSE==_link_min->_CheckAndSetValue(state,value,bSendMessage))//限制器修改失败
				return FALSE;
		}
	}

	//推进最大值 ，使下限限制数据中心大于当前值
	if(_link_max&&(!_bmaxlimiter)){
		float fv = _link_max->GetFVal();
		if(value>fv){
			if(_link_max->IsLocked())  //避免约束死锁
				return TRUE;

			if(FALSE==_link_max->_CheckAndSetValue(state,value,bSendMessage))
				return FALSE;
		}
	}

	return TRUE;
}

int CPinboard::GetIVal()
{
	float v = GetFVal();
	return int(v);
}

void CPinboard::_NotifyLink(CPin * pin)
{	
	if(pin){
		pin->OnSetLimits(_range_min,_range_max);

		if(_bFloat)
			pin->OnSetValue(GetFVal(),EndChange);
		else
			pin->OnSetValue(GetIVal(),EndChange);

		int i = 0;
		for(;i<_links_to_me.size();i++){
			if(_links_to_me[i]==pin)
				break;
		}

		if(i>=_links_to_me.size())
			_links_to_me.push_back(pin);
	}
}

BOOL CPinboard::SetLimits(float minValue,float maxValue) //设置上下限时不想向发出数据改变通知
{
	if(minValue>maxValue)
		return FALSE;
	
	_range_min = minValue;
	_range_max = maxValue;

	OnSetLimits(_range_min,_range_max);
	
	for(int i = 0;i<_links_to_me.size();i++){
		CPin * pin = _links_to_me[i];
		pin->OnSetLimits(_range_min,_range_max);
	}

	return _AdjustOnLimitChange();
}

BOOL CPinboard::SetLimitMin(CPinboard * link,bool bLimiter)
{
	_link_min = (CPinboard *)link;
	_bminlimiter = bLimiter;
	return _AdjustOnLimitChange();
}

BOOL CPinboard::SetLimitMax(CPinboard * link,bool bLimiter)
{
	_link_max = (CPinboard *)link;
	_bmaxlimiter = bLimiter;
	return _AdjustOnLimitChange();
}
BOOL CPinboard::_AdjustOnLimitChange()
{
	float oldv = GetFVal();
	float maxv,minv;
	GetLimits(minv,maxv);
	
	if(maxv<minv)
		return FALSE;
	
	float newv = i_math::clamp_f(oldv,minv,maxv);
	
	return _CheckAndSetValue(EndChange,newv,FALSE);
}

void CPinboard::GetLimits(float & minValue,float &maxValue)
{
	minValue = _range_min;
	maxValue = _range_max;

	if(_link_min&&_bminlimiter)
			minValue = max(_range_min,_link_min->GetFVal());

	if(_link_max&&_bmaxlimiter)
			maxValue = min(_range_max,_link_max->GetFVal());
}

void CPinboard::Enable(BOOL bOnoff)
{
	for(int i = 0;i<_links_to_me.size();i++){
		CPin * pin = _links_to_me[i];
		pin->OnEnable(bOnoff);
	}
	OnEnable(bOnoff);
}
void CPinboard::NotifyBeginChange()
{
	_NotifyChange(PreChange);
}
void CPinboard::NotifyOnChange()
{
	_NotifyChange(OnChange);
}
void CPinboard::NotifyEndChange()
{
	_NotifyChange(EndChange);
}
void CPinboard::_NotifyChange(ChangeState state)
{
	float v = GetFVal();
	if(!_CheckAndSetValue(state,v,TRUE)){
		float minv,maxv;
		GetLimits(minv,maxv);
		v = i_math::clamp_f(v,minv,maxv);
		if(_bFloat)
			OnSetValue(v,state);
		else
			OnSetValue(int(v),state);
		_CheckAndSetValue(state,v,TRUE);
	}
}
//////////////////////////////////////////////////////////////////////////
CPin::CPin()
{
	_bFloat = TRUE;
	_bLock = FALSE;
}

CPin::~CPin()
{

}
int CPin::GetIVal()
{
	float v = GetFVal();
	return int(v);
}
void CPin::OnSetValue(int value,ChangeState state)
{
	OnSetValue(float(value),state);
}
void CPin::LinkTo(CPinboard * pinboard)
{	
	if(pinboard){
		int i = 0;
		for(;i< _pinborads.size();i++)
			if(_pinborads[i] == pinboard)
				break;

		if(i>=_pinborads.size()){
			pinboard->_NotifyLink(this);
			_pinborads.push_back(pinboard);
		}
	}
}

void CPin::NotifyBeginChange()
{
	_NotifyChange(PreChange);
}

void CPin::NotifyOnChange()
{
	_NotifyChange(OnChange);
}

void CPin::NotifyEndChange()
{
	_NotifyChange(EndChange);
}

void CPin::_NotifyChange(ChangeState state)
{
	float v = GetFVal();

	if(_pinborads.empty())
		return;

	Lock();
	int i = 0;
	for(;i<_pinborads.size();i++){
		CPinboard * pinboard = _pinborads[i];
		if(FALSE==pinboard->_CheckAndSetValue(state,v,TRUE))
			break;
	}

	//数据设置失败 恢复到原值
	if(i<_pinborads.size()){
		//得到旧值
		float minv,maxv;
		_pinborads[0]->GetLimits(minv,maxv);
		float climpv = i_math::clamp_f(v,minv,maxv);

		//将自己解锁 使自己能被恢复到原来的值
		UnLock(); 
		for(int k =0;k<=i;k++){
			CPinboard * pinboard = _pinborads[k];
			BOOL bOk = pinboard->_CheckAndSetValue(state,climpv,TRUE);
		}		
	}

	UnLock();
}


