#pragma once


#include "../fastdelegate/FastDelegate.h"

typedef fastdelegate::FastDelegate1<const char *,BOOL> PrgHandler_SetTitle;
typedef fastdelegate::FastDelegate3<const char *,int ,int ,BOOL> PrgHandler_SetProgess;
typedef fastdelegate::FastDelegate1<const char *,BOOL> PrgHandler_SetBegin;
typedef fastdelegate::FastDelegate0<BOOL> PrgHandler_SetEnd;

class CProgress
{
public:
	BOOL SetTitle(const char *title);

	BOOL SetBegin(const char *name="");
	BOOL SetProgress(const char *desc,int cur,int full);
	BOOL SetEnd();

	void ClearHandlers();
	void SetHandler(PrgHandler_SetTitle &handler);
	void SetHandler(PrgHandler_SetProgess &handler);
	void SetBeginHanlder(PrgHandler_SetBegin &handler);
	void SetEndHanlder(PrgHandler_SetEnd &handler);
protected:
	PrgHandler_SetTitle _settitle;
	PrgHandler_SetProgess _setprogess;
	PrgHandler_SetBegin _setbegin;
	PrgHandler_SetEnd _setend;

};

