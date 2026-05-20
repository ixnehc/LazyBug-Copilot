
#pragma once


class CThread
{
public:
	CThread();
	~CThread();
	void Start();

	virtual int OnDo();


protected:
	HANDLE _handle;

};

