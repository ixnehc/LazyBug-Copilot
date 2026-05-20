
#pragma once

class ILink
{
public:
	virtual int GetIVal() = 0;
	virtual float GetFVal() = 0;
	
	virtual bool SetValue(int value,bool bNotify,bool bCommit = true) = 0;
	virtual bool SetValue(float value,bool bNotify,bool bCommit = true) = 0;
	virtual void Enable(BOOL bOnoff) = 0;

	virtual void SetType(BOOL bFloat) = 0;
	virtual BOOL IsFloat() = 0;
};

class ILinkOut;
class ILinkIn;
class ILinkIn : public ILink
{
public:
	virtual void SetLimits(float minValue,float maxValue) = 0;
	virtual void SetLimitMin(ILinkIn * link,bool bLimiter) = 0;
	virtual void SetLimitMax(ILinkIn * link,bool bLimiter) = 0;
	virtual void GetLimits(float & minValue,float &maxValue) = 0;
};

class ILinkOut: public ILink
{
public:
	virtual BOOL LinkTo(ILinkIn * custCtrl) = 0;
};

