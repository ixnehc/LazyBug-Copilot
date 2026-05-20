#pragma once

enum ChangeState
{
	PreChange,
	OnChange,
	EndChange,
};

class CPin;
class CPinboard;
class CPinboard
{
public:
	CPinboard(void);
	~CPinboard(void);

	virtual float GetFVal() = 0;
	virtual int GetIVal();

	//用户函数
	virtual BOOL SetValue(int value,BOOL bNotify = FALSE);   //设置值后是否需要 发送 WM_NOTIFY消息
	virtual BOOL SetValue(float value,BOOL bNotify = FALSE);
	virtual void Enable(BOOL bOnoff);

	//约束相关
	virtual void GetLimits(float & minValue,float &maxValue);
	virtual BOOL SetLimits(float minValue,float maxValue);
	virtual BOOL SetLimitMin(CPinboard * link,bool bLimiter);
	virtual BOOL SetLimitMax(CPinboard * link,bool bLimiter);
	virtual void SetType(BOOL bFloat){_bFloat = bFloat;}
	virtual BOOL IsFloat(){return _bFloat;}
	
protected:

	// 子类界面 表现相关
	virtual void OnSetValue(int value,ChangeState state) = 0;
	virtual void OnSetValue(float value,ChangeState state) = 0; 
	virtual void OnEnable(BOOL bOnoff) = 0;
	virtual void OnSetLimits(float minValue,float maxValue){}

	//子类 编辑过程 通知
	void NotifyBeginChange();
	void NotifyOnChange();
	void NotifyEndChange();
	virtual void OnNotifyChange(ChangeState state) {}
	void _NotifyChange(ChangeState state);

	//CPin 
	void _NotifyLink(CPin * pin);
	BOOL _CheckAndSetValue(ChangeState state,float value,BOOL bSendMessage);

protected:
	void Lock();
	void UnLock();
	BOOL IsLocked(){return _bLock;}
	void Flush(bool bNotify,bool bCommitChange);

	BOOL _CheckValid(float value);						//检验数据的有效性
	BOOL _PushLimiter(ChangeState state,float value,BOOL bSendMessage);	//推进其他的约束使其满足本对象
	BOOL _AdjustOnLimitChange();
	
	void LockMessage(){_bMessageLock = TRUE;}
	BOOL IsMessageLocked(){return _bMessageLock;}
	void UnLockMessage(){_bMessageLock = FALSE;}

	friend class CPin;

	float _range_min,_range_max;	
	CPinboard * _link_min, * _link_max;
	bool _bminlimiter,_bmaxlimiter;
	
	BOOL _bFloat;
	BOOL _bLock;
	BOOL _bMessageLock; //消息锁 避免在消息处理过程中 引起信息的重入  陷入循环的僵局

	std::vector<CPin *> _links_to_me;
};

// 非数据中心不向系统外发布数据改变
class CPin
{
public:
	CPin();
	virtual ~CPin();

	virtual float GetFVal() = 0;
	virtual int GetIVal();
	//外部使用
	virtual void LinkTo(CPinboard * pinboard);
protected:

	//子类响应界面 表现
	virtual void OnSetValue(int value,ChangeState state);
	virtual void OnSetValue(float value,ChangeState state) = 0;
	virtual void OnEnable(BOOL bOnoff) = 0;	
	virtual void OnSetLimits(float minValue,float maxValue){}

	//子类 编辑过程 通知
	void NotifyBeginChange();
	void NotifyOnChange();
	void NotifyEndChange();
	
	void _NotifyChange(ChangeState state);
protected:
	//Lock
	void Lock(){_bLock = TRUE;}
	void UnLock(){_bLock = FALSE;}
	BOOL IsLocked(){return _bLock;}

	friend CPinboard;
protected:
	BOOL _bLock;
	BOOL _bFloat;
	std::vector<CPinboard *> _pinborads;
};


/************************************************************************/
/* 职责分配： CPin 负责将更改提交 
/* CPinboradBase 复杂控制数据的约束 和 数据的同步
/************************************************************************/







