

#pragma once


typedef BYTE NetMsgType;

class CClass;
struct GObjBase;
struct NetMsg
{
	virtual NetMsgType GetType()=0;
	virtual CClass *GetClass()=0;
	virtual GObjBase *GetGObj()	
	{		
		return NULL;	
	}
	virtual BYTE *GetData(DWORD &sz)
	{
		sz=0;
		return NULL;
	}

	virtual void Save(CDataPacket *dp)	{	}
	virtual BOOL Load(CDataPacket *dp,DWORD szData)	{	return FALSE;}

	NetMsg *Clone();
};

struct NetMsgSC:public NetMsg
{
	NetMsgSC()
	{
		tServer=0;
	}
	void SetT(DWORD t)
	{
		tServer=t;
	}
	DWORD tServer;
};

#define DEFINE_SIMPLE_MSG(clss)						\
DEFINE_CLASS(clss);												\
virtual NetMsgType GetType()								\
{																				\
	return NetMsg_##clss;										\
}																				\
virtual BYTE *GetData(DWORD &sz)					\
{																				\
	sz=sizeof(*this)-8;												\
	return ((BYTE*)this)+8;										\
}

#define BEGIN_GOBJ_MSG_PURE(clss)					\
DEFINE_CLASS(clss);												\
virtual NetMsgType GetType()								\
{																				\
return NetMsg_##clss;											\
}																				\
BEGIN_GOBJ_PURE(clss,1)

#define END_GOBJ_MSG() END_GOBJ()

#define BEGIN_GOBJ_MSG(clss)							\
DEFINE_CLASS(clss);												\
virtual NetMsgType GetType()								\
{																				\
return NetMsg_##clss;											\
}																				\
BEGIN_GOBJ(clss,1)



template <typename T>
T *ConvertNetMsg(NetMsg *msg)
{
	if (msg->GetClass()->CheckName(Class_Ptr2(T)->GetName()))
		return (T*)msg;
	return NULL;
}