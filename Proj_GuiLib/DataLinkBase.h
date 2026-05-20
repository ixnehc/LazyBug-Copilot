#pragma once

#include "DataLink.h"

class CLinkInBase;
class CLinkOutBase;

// 非数据中心不向系统外发布数据改变
class CLinkOutBase:public ILinkOut
{
public:
	CLinkOutBase();
	virtual ~CLinkOutBase();
	virtual int GetIVal() = 0;
	virtual float GetFVal() = 0;

	virtual bool SetValue(int value,bool bNotify,bool bCommit = true);
	virtual bool SetValue(float value,bool bNotify,bool bCommit = true);
	virtual void Enable(BOOL bOnoff);

	virtual void OnSetValue(int value) = 0;
	virtual void OnSetValue(float value) = 0;
	virtual void OnSetLimits(float minValue,float maxValue){}
	virtual void OnEnable(BOOL bOnoff) = 0;

	virtual void SetType(BOOL bFloat){m_bFloat = bFloat;}
	virtual BOOL IsFloat(){ return m_bFloat;}

	virtual BOOL LinkTo(ILinkIn * custCtrl);

protected:
	bool _PrepareSet(float value);
	void Flush(bool bNotify,bool bCommitChange);
	void Lock(){m_bLock = TRUE;}
	void UnLock(){m_bLock = FALSE;}
	BOOL IsLocked(){return m_bLock;}

	friend CLinkInBase;
protected:
	BOOL m_bLock;
	BOOL m_bFloat;
	float _range_min,_range_max;
	std::vector<CLinkInBase *> _links_to_other;
};

class CLinkInBase :public ILinkIn
{
public:
	CLinkInBase(void);
	~CLinkInBase(void);
	
	virtual int GetIVal() = 0;
	virtual float GetFVal() = 0;

	virtual bool SetValue(int value,bool bNotify,bool bCommit = true);
	virtual bool SetValue(float value,bool bNotify,bool bCommit = true);
	virtual void Enable(BOOL bOnoff);

	virtual void OnSetValue(int value,bool bNotify,bool bCommit) = 0;
	virtual void OnSetValue(float value,bool bNotify,bool bCommit) = 0; 
	virtual void OnSetLimits(float minValue,float maxValue){}
	virtual void OnEnable(BOOL bOnoff) = 0;

	virtual void SetType(BOOL bFloat){m_bFloat = bFloat;}
	virtual BOOL IsFloat(){return m_bFloat;}
	
	virtual void SetLimits(float minValue,float maxValue);
	virtual void GetLimits(float & minValue,float &maxValue);
	virtual void SetLimitMin(ILinkIn * link,bool bLimiter);
	virtual void SetLimitMax(ILinkIn * link,bool bLimiter);

protected:
	void Lock(){m_bLock = TRUE;}
	void UnLock(){m_bLock = FALSE;}
	BOOL IsLocked(){return m_bLock;}
	void Flush(bool bNotify,bool bCommitChange);

	bool _PrepareSet(float value,bool bNotify,bool bCommit);
	BOOL _LinkMe(ILinkOut * custCtrl) ;
	void _RollBack(CLinkOutBase * custCtrl = NULL);
	void _OnLimits();

	friend class CLinkOutBase;

	BOOL m_bFloat;
	float _range_min,_range_max;
	
	CLinkInBase * _link_min, * _link_max;
	bool _bminlimiter,_bmaxlimiter;
	BOOL m_bLock;

	std::vector<CLinkOutBase *> _links_to_me;
};


