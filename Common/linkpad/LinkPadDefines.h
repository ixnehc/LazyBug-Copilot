#pragma once


#define MAX_PADSTUB_NAME 16

enum PadStubType
{
	PadStub_None,
	PadStub_In,
	PadStub_Out,
	PadStub_CIn,//Control In
	PadStub_COut,//Control Out
};

typedef WORD PadID_Short;
typedef DWORD PadID;
#define PadID_Null 0


class CLinkPad;
class CClass;
struct LinkPadClasses
{
	virtual CLinkPad *New(const char *nmClass)=0;
	virtual CLinkPad *New(WORD uid)=0;
	virtual WORD UIDFromClassName(const char *nmClass)=0;
	virtual void CollectNames(std::vector<std::string>&buf)=0;
	virtual void CollectPadClasses(std::vector<CClass *>&buf)=0;
};