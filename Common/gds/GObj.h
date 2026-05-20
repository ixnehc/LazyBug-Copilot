#pragma once

#include "GVar.h"
#include "../datapacket/DataPacket.h"

#include <string>
#include <unordered_map>
#include <set>

//////////////////////////////////////////////////////////////////////////
//The macro for defining GObj

// 	BEGIN_GOBJ(classtype,version)
// 	BEGIN_GOBJ_PURE(classtype,version)
// 	DERIVE_GOBJ(classtype,baseclasstype)//not tested
// 				GELEM_VAR_INIT(type,name,initv)
// 				GELEM_VAR(type,name)
// 				GELEM_STRING_INIT(name,initstr)
// 				GELEM_STRING(name)
// 				GELEM_VARVECTOR_INIT(type,name,initv)
// 				GELEM_VARVECTOR(type,name)
// 				GELEM_STRINGVECTOR_INIT(name,initv)
// 				GELEM_STRINGVECTOR(name)
// 				GELEM_VARARRAY_INIT(type,name,initv)
// 				GELEM_VARARRAY(type,name)
// 				GELEM_STRINGARRAY_INIT(name,initv)
// 				GELEM_STRINGARRAY(name)
// 				GELEM_OBJ(type,name)
// 				GELEM_OBJVECTOR(type,name)
// 				GELEM_OBJARRAY(type,name)
// 							GELEM_UID(version) version����С��128
// 							GELEM_VERSION(version)
// 							GELEM_EDITVAR(name,vt0,sem0,desc)
// 							GELEM_EDITOBJ(name,desc)
// 	END_GOBJ()

//Sample
// 	struct TestDataBase
// 	{
// 
// 		int v1;
// 		std::string v2;
// 		std::vector<int> v3;
// 
// 		//GObj Define
// 		BEGIN_GOBJ_PURE(TestDataBase,1);
// 		GELEM_VAR_INIT(int,v1,1);
// 		GELEM_STRING_INIT(v2,"hi");
// 		GELEM_VARVECTOR_INIT(int,v3,1);
// 		END_GOBJ();
// 	};
// 	struct TestData
// 	{
// 		int v4;
// 
// 		//GObj Define
// 		BEGIN_GOBJ_PURE(TestData,1);
// 			GELEM_VAR_INIT(int,v4,1);
// 		END_GOBJ();
// 
// 	};
// 	struct TestData2:public TestDataBase
// 	{
// 		int v1;
// 		std::string v2;
// 		std::vector<int> v3;
// 		TestData v4;
// 
// 		//GObj Define
// 		BEGIN_GOBJ_PURE(TestData2,1); DERIVE_GOBJ(TestData2,TestDataBase);
// 			GELEM_VAR_INIT(int,v1,1);
// 			GELEM_STRING_INIT(v2,"hi");
// 			GELEM_VARVECTOR_INIT(int,v3,1);
// 			GELEM_OBJ(TestData,v4);
// 		END_GOBJ();
// 
// 	};



#define ELEM_UID_START (64)
#define GOBJ_UID_START (40000)
#define GOBJ_UID_NULL (0)

class CDataPacket;
struct GObjBase;
class CClass;

struct GElemBase
{
	GElemBase()
	{
		ver=0;
		bEditable=FALSE;
		vt=GVT_None;
		sz=0;
		szArray=0;
		sem=GSem_Unknown;
		uid=0;
	}
	virtual int GetTypeID()=0;//�õ����elem������id
	virtual BOOL CheckCompatible(GElemBase *elemOther)=0;//�����һ��elem�����elem������֮���Ƿ����(�Ƿ���Ի���Copy)

	virtual void Zero(void *objOwner,BOOL bIntuitive)=0;
	virtual void Clear(void *objOwner,BOOL bIntuitive)=0;
	virtual void Save(void *objOwner,CDataPacket &dp,BOOL bNewFmt)=0;
	virtual BOOL Load(void *objOwner,CDataPacket &dp,BOOL bNewFmt)=0;
	virtual void SaveDelta(void *objOwner,void *objOwnerRef,CDataPacket &dp)=0;
	virtual void LoadDelta(void *objOwner,CDataPacket &dp,std::vector<void*>*ptrsDelta)=0;
	virtual void Copy(void *objDest,void *objSrc)=0;

	//Operations for edit
	//NOTE: for all these operation functions,pass NULL for objOwner to check whether this elem
	//support that operation.If the return value is FALSE,this operation is not supported
	//by this elem(for example,if you call NewSub(NULL),and it return FALSE,this
	//elem does not support the operation NewSub(...) )
	virtual BOOL NewSub(void *objOwner)//add a new sub to the tail
			{		return FALSE;	}
	virtual BOOL GetSubCount(void *objOwner,DWORD *c)//how many sub for this elem
			{		return FALSE;	}
	virtual BOOL RemoveSub(void *objOwner,DWORD idx)//remove the specified sub
			{		return FALSE;	}
	virtual BOOL CloneSub(void *objOwner,DWORD idx)//clone a sub and add it to the tail
			{		return FALSE;	}
	virtual BOOL MoveSub(void *objOwner,DWORD idx,BOOL bUp)//exchange the sub's position with its prev/next sibling
			{		return FALSE;	}
	virtual BOOL ClearSub(void *objOwner)//clear all the subs
			{		return FALSE;	}
	virtual BOOL SaveSub(void *objOwner,DWORD idx,CDataPacket &dp) 	
			{		return FALSE;	}
	virtual BOOL LoadSub(void *objOwner,DWORD idx,CDataPacket &dp)	
			{		return FALSE;	}
	virtual BOOL GetVar(void *objOwner,void **var)//Get the var ptr this elem is working on
			{		return FALSE;	}
	virtual BOOL GetObj(void *objOwner,GObjBase **obj)//Get the obj this elem is working on
			{		return FALSE;	}
	virtual BOOL GetSubVar(void *objOwner,DWORD idx,void **var)//
			{		return FALSE;	}
	virtual BOOL GetSubObj(void *objOwner,DWORD idx,GObjBase **obj)//
			{		return FALSE;	}
	virtual const char *GetObjName()//Get the obj's name this elem is working on
		{		return NULL;	}
	virtual BOOL GetObjClass(void *objOwner,CClass**clss)//Get the obj's class this elem is working on
			{		return FALSE;	}

	const char *GetElemName()	{		return elemname.c_str();	}
	DWORD GetVer()	{		return ver;	}

	GVarType GetVarType()	{		return vt;	}
	GSem& GetSem()	{		return sem;	}
	const char *GetEditName()	{		return name.c_str();	}
	const char *GetEditDesc()	{		return desc.c_str();	}

	void *GetPtr(void *owner)	{		return _Ptr(owner);	}

	BOOL CheckCompatible_General(GElemBase *elem)
	{
		if (elem->GetTypeID()!=GetTypeID())
			return FALSE;
		if (elem->szArray!=szArray)
			return FALSE;
		if (!elem->bEditable)
			return FALSE;
		if (elem->bEditable!=bEditable)
			return FALSE;
		if (elem->vt!=vt)
			return FALSE;
		if (elem->sem.code!=sem.code)
			return FALSE;
		if (elem->sem.constraint!=sem.constraint)
			return FALSE;
		if (elem->subtype!=subtype)
			return FALSE;
		return TRUE;
	}

	//ptr
	DWORD off;
	DWORD sz;
	DWORD szArray;

	//identity of the element
	std::string elemname;
	std::string subtype;//
	WORD ver;
	BYTE uid;

	//edit info
	BOOL bEditable;
	std::string name;
	std::string desc;
	GVarType vt;
	GSem sem;//sementic
	std::set<std::string> contraintsLegacy;

	GElemBase *next;

protected:
	void *_Ptr(void *objOwner)
	{
		return (void *)((BYTE*)objOwner+off);
	}

};

struct GObjBase
{
	virtual void Zero(BOOL bIntuitive)=0;
	virtual void Clear(BOOL bIntuitive)=0;
	virtual void Save(CDataPacket &dp,BOOL bNewFmt)=0;
	virtual BOOL Load(CDataPacket &dp,BOOL bNewFmt,BOOL *bRepaired)=0;//return FALSE if incorrect version found,
																										//(will skip the data of incorrect version)
	virtual BOOL SaveDelta(CDataPacket &dp,GObjBase *gobjRef)=0;
	virtual void LoadDelta(CDataPacket &dp,std::vector<void*>*ptrsDelta)=0;
	virtual void SaveSimple(CDataPacket &dp)=0;
	virtual void LoadSimple(CDataPacket &dp)=0;

	virtual void Copy(GObjBase *src)=0;
	virtual GElemBase *GetElems()=0;
	virtual void *GetOwner()=0;
	virtual int GetOwnerOff()=0;//get the offset of this gobj base in its owner,in byte
	virtual GObjBase *GetBase()=0;//Get the gobj of the owner class's base class.
														//if the owner class is not a derived class,return NULL
	virtual const char*GetName()=0;
	virtual DWORD GetVer()=0;
	virtual WORD GetUID()=0;

	virtual void *FindData(const char*name)=0;
	virtual void *FindData(std::string &name)=0;

	GElemBase *FindElem(const char *nmElem)
	{
		GElemBase *elem=GetElems();
		while(elem)
		{
			if (elem->elemname==nmElem)
			{
				return elem;
			}
			elem=elem->next;
		}
		return NULL;
	}


	BOOL Equals(GObjBase *src)
	{
		std::vector<BYTE>buf;
		DP_BeginSave(dp,buf)
			Save(dp,TRUE);
		DP_EndSave();
		std::vector<BYTE>buf2;
		DP_BeginSave(dp,buf2)
			src->Save(dp,TRUE);
		DP_EndSave();
		if (buf.size()!=buf2.size())
			return FALSE;
		if (memcmp(buf.data(),buf2.data(),buf.size())!=0)
			return FALSE;
		return TRUE;
	}

	virtual const char *GetBrief(void *)	{		return "";	}


};


template <class T>
struct GObj:public GObjBase
{
	//bIntuitive indicates whether this function is called from the contructor&destructor or
	//not.
	virtual void Zero(BOOL bIntuitive)
	{
		//let the base do
		if (!bIntuitive)
		{
			GObjBase *base=_base();
			if (base)
				base->Zero(bIntuitive);
		}

		T *owner=_owner();
		GElemBase *p=__elems();
		while(p)
		{
			p->Zero(owner,bIntuitive);
			p=p->next;
		}
	}
	virtual void Clear(BOOL bIntuitive)
	{
		//First let the base do
		if (!bIntuitive)
		{
			GObjBase *base=_base();
			if (base)
				base->Clear(bIntuitive);
		}

		T *owner=_owner();
		GElemBase *p=__elems();
		while(p)
		{
			p->Clear(owner,bIntuitive);
			p=p->next;
		}
	}

	virtual void Copy(GObjBase *src)
	{
		//First let the base do
		GObjBase *base=_base();
		GObjBase *baseSrc=((GObj<T>*)src)->_base();
		if (base&&baseSrc)
			base->Copy(baseSrc);

		T *owner=_owner();
		T *ownerSrc=((GObj<T>*)src)->_owner();
		GElemBase *p=__elems();
		while(p)
		{
			p->Copy(owner,ownerSrc);
			p=p->next;
		}
	}

	virtual void SaveSimple(CDataPacket &dp)
	{
		T *owner=_owner();
		GElemBase *p=__elems();
		while(p)
		{
			p->Save(owner,dp,TRUE);
			p=p->next;
		}
	}

	virtual void LoadSimple(CDataPacket &dp)
	{
		T *owner=_owner();
		GElemBase *p=__elems();
		while(p)
		{
			p->Load(owner,dp,TRUE);
			p=p->next;
		}
	}

	virtual void Save(CDataPacket &dp,BOOL bNewFmt)
	{
		//First let the base do
		GObjBase *base=_base();
		if (base)
			base->Save(dp,bNewFmt);

		if (__uid()>=GOBJ_UID_START)
			dp.Data_NextWord()=__uid();
		else
			dp.Data_WriteStringSH(__name());
		dp.Data_NextByte()=(BYTE)__ver();
		CDataPacket dpT;
		T *owner=_owner();
		GElemBase *p=__elems();
		DWORD count=0;
		while(p)
		{
			_SaveElem(owner,p,dpT,bNewFmt);
			count++;
			p=p->next;
		}
		dp.Data_NextByte()=(BYTE)count;//count of the elements

		dp.Data_EncodeDword(dpT.GetDataSize());

		p=__elems();
		while(p)
		{
			_SaveElem(owner,p,dp,bNewFmt);
			p=p->next;
		}
	}
	virtual BOOL Load(CDataPacket &dp,BOOL bNewFmt,BOOL *bRepaired)
	{
		BOOL bOk=TRUE;
		if (bRepaired)
			*bRepaired=FALSE;
		//First let the base do
		GObjBase *base=_base();
		if (base)
			bOk=base->Load(dp,bNewFmt,bRepaired);

		std::string name;
		DWORD ver;
		DWORD count;
		DWORD sz;
		WORD uid=0;
		if (!bNewFmt)
		{
			dp.Data_ReadString(name);
			ver=dp.Data_NextDword();
			count=dp.Data_NextDword();
			sz=dp.Data_NextDword();
		}
		else
		{
			WORD *p=(WORD*)dp.GetCurBufferPointer();
			if (*p>=GOBJ_UID_START)
				uid=dp.Data_NextWord();
			else
				dp.Data_ReadStringSH(name);
			ver=dp.Data_NextByte();
			count=dp.Data_NextByte();
			sz=dp.Data_DecodeDword();
		}
		if (uid>=GOBJ_UID_START)
		{
			if ((ver!=__ver())||(__uid()!=uid))
			{
				dp.Data_MarchData(sz);
				if (bRepaired)
					*bRepaired=TRUE;
				return FALSE;
			}
		}
		else
		{
			if ((ver!=__ver())||(__name()!=name))
			{
				dp.Data_MarchData(sz);
				if (bRepaired)
					*bRepaired=TRUE;
				return FALSE;
			}
		}

		T *owner=_owner();

		for (int i=0;i<count;i++)
		{
			std::string elemname;
			DWORD ver;
			DWORD sz;
			BYTE uid=0;
			if (!bNewFmt)
			{
				dp.Data_ReadString(elemname);
				ver=dp.Data_NextDword();
				sz=dp.Data_NextDword();
			}
			else
			{
				if (*(BYTE *)dp.GetCurBufferPointer()>=ELEM_UID_START)
					uid=dp.Data_NextByte();
				else
					dp.Data_ReadStringSH(elemname);
				ver=dp.Data_NextByte();
				sz=dp.Data_DecodeDword();
			}

			GElemBase *p=__elems();
			if (uid==0)
			{
				while(p)
				{
					if ((p->elemname==elemname)&&(p->ver==ver))
					{
						if (FALSE==p->Load(owner,dp,bNewFmt))
						{
							if (bRepaired)
								*bRepaired=TRUE;
						}
						break;
					}
					p=p->next;
				}
			}
			else
			{
				while(p)
				{
					if ((p->uid==uid)&&(p->ver==ver))
					{
						if (FALSE==p->Load(owner,dp,bNewFmt))
						{
							if (bRepaired)
								*bRepaired=TRUE;
						}
						break;
					}
					p=p->next;
				}
			}
			if (!p)
				dp.Data_MarchData(sz);//Cannot find suitable element,skip the data
		}
		return bOk;
	}

	virtual BOOL SaveDelta(CDataPacket &dp,GObjBase *gobjRef)
	{
		GObj<T> &objRef=*(GObj<T> *)gobjRef;

		BOOL bDelta=FALSE;
		//First let the base do
		GObjBase *base=_base();
		if (base)
		{
			if (base->SaveDelta(dp,objRef._base()))
				bDelta=TRUE;
		}

		if (__uid()>=GOBJ_UID_START)
			dp.Data_NextWord()=__uid();
		else
			dp.Data_WriteStringSH(__name());
		dp.Data_NextByte()=(BYTE)__ver();
		T *owner=_owner();
		T *ownerRef=objRef._owner();
		GElemBase *p=__elems();

		DWORD count=0;
		std::vector<BYTE>flagsDelta;
		flagsDelta.reserve(128);
		if (TRUE)
		{
			std::vector<BYTE> buf;
			std::vector<BYTE> buf2;
			while(p)
			{
				DP_BeginSave(dp,buf)
					_SaveElem(owner,p,dp,TRUE);
				DP_EndSave();

				DP_BeginSave(dp,buf2)
					_SaveElem(ownerRef,p,dp,TRUE);
				DP_EndSave();

				if (!(buf==buf2))
				{
					count++;
					flagsDelta.push_back(1);//��delta
				}
				else
					flagsDelta.push_back(0);//û��delta
				p=p->next;
			}
		}
		dp.Data_NextByte()=(BYTE)count;//count of the elements


		CDataPacket dpT;
		if (TRUE)
		{
			p=__elems();
			int idx=0;
			while(p)
			{
				if (flagsDelta[idx])
					_SaveElemDelta(owner,ownerRef,p,dpT);
				idx++;

				p=p->next;
			}
		}
		dp.Data_EncodeDword(dpT.GetDataSize());

		if (TRUE)
		{
			p=__elems();
			int idx=0;
			while(p)
			{
				if (flagsDelta[idx])
					_SaveElemDelta(owner,ownerRef,p,dp);
				idx++;

				p=p->next;
			}
		}

		if (count>0)
			bDelta=TRUE;
		return bDelta;
	}

	virtual void LoadDelta(CDataPacket &dp,std::vector<void*>*ptrsDelta)
	{
		//First let the base do
		GObjBase *base=_base();
		if (base)
			base->LoadDelta(dp,ptrsDelta);

		std::string name;
		DWORD ver;
		DWORD count;
		DWORD sz;
		WORD uid=0;

		if (TRUE)
		{
			WORD *p=(WORD*)dp.GetCurBufferPointer();
			if (*p>=GOBJ_UID_START)
				uid=dp.Data_NextWord();
			else
				dp.Data_ReadStringSH(name);
			ver=dp.Data_NextByte();
			count=dp.Data_NextByte();
			sz=dp.Data_DecodeDword();
		}
		if (uid>=GOBJ_UID_START)
		{
			if ((ver!=__ver())||(__uid()!=uid))
			{
				dp.Data_MarchData(sz);
				return;
			}
		}
		else
		{
			if ((ver!=__ver())||(__name()!=name))
			{
				dp.Data_MarchData(sz);
				return;
			}
		}

		T *owner=_owner();
		for (int i=0;i<count;i++)
			_LoadElemDelta(owner,dp,ptrsDelta);
	}



	virtual GElemBase *GetElems()	{		return __elems();	}
	virtual void *GetOwner()	{		return (void *)_owner();	}
	virtual int GetOwnerOff()	{		return (int)__thisoff();	}
	virtual GObjBase *GetBase()	{		return _base();	}
	virtual const char*GetName()	{		return __name().c_str();	}
	virtual DWORD GetVer()	{		return __ver();	}
	virtual WORD GetUID()	{		return __uid();	}
	virtual void *FindData(const char*name)
	{
		std::unordered_map<std::string,void*>::iterator it=__data().find(std::string(name));
		if (it==__data().end())
			return NULL;
		return (*it).second;
	}
	virtual void *FindData(std::string &name)
	{
		std::unordered_map<std::string,void*>::iterator it=__data().find(name);
		if (it==__data().end())
			return NULL;
		return (*it).second;
	}

	GElemBase *FindElem(const char *nmElem)
	{
		GElemBase *elem=GetElems();
		while(elem)
		{
			if (elem->elemname==nmElem)
			{
				return elem;
			}
			elem=elem->next;
		}
		return NULL;
	}


public://take it as protected
	T *_owner()
	{
		return (T *)((BYTE*)(this)-__thisoff());
	}
	GObjBase *_base()
	{
		if (__baseoff()==0)
			return NULL;
		return (GObjBase *)((BYTE*)(this)-__baseoff());
	}

	void _SaveElem(T *owner,GElemBase *elem,CDataPacket &dp,BOOL bNewFmt)
	{
		if (elem->uid>=ELEM_UID_START)
			dp.Data_NextByte()=elem->uid;
		else
			dp.Data_WriteStringSH(elem->elemname);
		dp.Data_NextByte()=(BYTE)elem->ver;
		CDataPacket dpT;
		elem->Save(owner,dpT,bNewFmt);
		dp.Data_EncodeDword(dpT.GetDataSize());
		elem->Save(owner,dp,bNewFmt);
	}

	void _SaveElemDelta(T *owner,T *ownerRef,GElemBase *elem,CDataPacket &dp)
	{
		if (elem->uid>=ELEM_UID_START)
			dp.Data_NextByte()=elem->uid;
		else
			dp.Data_WriteStringSH(elem->elemname);
		dp.Data_NextByte()=(BYTE)elem->ver;

		CDataPacket dpT;
		elem->SaveDelta(owner,ownerRef,dpT);
		dp.Data_EncodeDword(dpT.GetDataSize());
		elem->SaveDelta(owner,ownerRef,dp);
	}

	void _LoadElemDelta(T *owner,CDataPacket &dp,std::vector<void*>*ptrsDelta)
	{
		std::string elemname;
		DWORD ver;
		DWORD sz;
		BYTE uid=0;
		if (TRUE)
		{
			if (*(BYTE *)dp.GetCurBufferPointer()>=ELEM_UID_START)
				uid=dp.Data_NextByte();
			else
				dp.Data_ReadStringSH(elemname);
			ver=dp.Data_NextByte();
			sz=dp.Data_DecodeDword();
		}

		GElemBase *p=__elems();
		if (uid==0)
		{
			while(p)
			{
				if ((p->elemname==elemname)&&(p->ver==ver))
				{
					p->LoadDelta(owner,dp,ptrsDelta);
					break;
				}
				p=p->next;
			}
		}
		else
		{
			while(p)
			{
				if ((p->uid==uid)&&(p->ver==ver))
				{
					p->LoadDelta(owner,dp,ptrsDelta);
					break;
				}
				p=p->next;
			}
		}
		if (!p)
		{
			dp.Data_MarchData(sz);//Cannot find suitable element,skip the data
		}
	}

	const char *GetBrief(void *param)
	{
		GetBriefFunc func=__brieffunc();
		if (func)
			return (_owner()->*func)(param);
		return "";
	}


	typedef const char*(T::*GetBriefFunc)(void *);


	static int &__thisoff()	{		static int s=0;		return s;	}
	static int &__baseoff()	{		static int s=0;		return s;	}
	static GElemBase *&__elems	(){		static GElemBase *s=NULL;		return s;	}
	static std::string& __name(){		static std::string s;		return s;	}
	static DWORD &__ver()	{		static DWORD s=0;		return s;	}
	static WORD &__uid()	{		static WORD s=0;		return s;	}
	static std::unordered_map<std::string,void*>&__data()	{		static std::unordered_map<std::string,void*> data;		return data;	}
	static GetBriefFunc &__brieffunc()	{		static GetBriefFunc func=NULL; return func;	}
};


#define MOVESUB_CHECK_IDX(idx2,bUp,total)\
	if (idx>=(total))\
		return FALSE;\
	DWORD idx2;\
	if (bUp)\
		idx2=idx-1;\
	else\
		idx2=idx+1;\
	if (idx2>=(total))\
		return FALSE;

template <class T>
void _CopyT(T *target,T *src)
{
	CDataPacket dp;
	std::vector<BYTE>buf;
	src->__gobj.Save(dp);
	buf.resize(dp.GetDataSize());
	dp.SetDataBufferPointer(buf.data());
	src->__gobj.Save(dp);
	dp.Reset();
	target->__gobj.Load(dp);
}




template <class T>
struct GElem_VarSingle:public GElemBase
{
	virtual int GetTypeID()	{		return 0;	}
	
	virtual BOOL CheckCompatible(GElemBase *elemOther)
	{
		if (!CheckCompatible_General(elemOther))
			return FALSE;
		return TRUE;
	}
	virtual void Zero(void *objOwner,BOOL bIntuitive)
	{
		*(T*)_Ptr(objOwner)=init;
	}
	virtual void Clear(void *objOwner,BOOL bIntuitive)	{	}
	virtual void Save(void *objOwner,CDataPacket &dp,BOOL bNewFmt)
	{
		dp.Data_WriteData(_Ptr(objOwner),sizeof(T));
	}
	virtual void SaveDelta(void *objOwner,void *objOwnerRef,CDataPacket &dp)
	{
		BYTE *data,*dataRef;
		data=(BYTE*)_Ptr(objOwner);
		dataRef=(BYTE*)_Ptr(objOwnerRef);
		dp.Data_WriteData(_Ptr(objOwner),sizeof(T));
	}

	virtual BOOL Load(void *objOwner,CDataPacket &dp,BOOL bNewFmt)
	{
		dp.Data_ReadData(_Ptr(objOwner),sizeof(T));
		return TRUE;
	}

	virtual void LoadDelta(void *objOwner,CDataPacket &dp,std::vector<void*>*ptrsDelta)
	{
		dp.Data_ReadData(_Ptr(objOwner),sizeof(T));
		if (ptrsDelta)
			ptrsDelta->push_back(_Ptr(objOwner));
	}

	virtual void Copy(void *objDest,void *objSrc)
	{
		*(T*)_Ptr(objDest)=*(T*)_Ptr(objSrc);
	}


	virtual BOOL GetVar(void *objOwner,void **var)
	{		
		if (var)
			*var=NULL;
		if (objOwner&&var)
			*var=_Ptr(objOwner);
		return TRUE;
	}

	T init;
};

struct GElem_String:public GElemBase
{
	virtual int GetTypeID()	{		return 1;	}
	virtual BOOL CheckCompatible(GElemBase *elemOther)
	{
		if (!CheckCompatible_General(elemOther))
			return FALSE;
		return TRUE;
	}
	virtual void Zero(void *objOwner,BOOL bIntuitive)
	{
		*(std::string*)_Ptr(objOwner)=init;
	}
	virtual void Clear(void *objOwner,BOOL bIntuitive)	{	}
	virtual void Save(void *objOwner,CDataPacket &dp,BOOL bNewFmt)
	{
		dp.Data_WriteStringSH(*(std::string*)_Ptr(objOwner));
	}
	virtual void SaveDelta(void *objOwner,void *objOwnerRef,CDataPacket &dp)
	{
		dp.Data_WriteStringSH(*(std::string*)_Ptr(objOwner));
	}

	virtual BOOL Load(void *objOwner,CDataPacket &dp,BOOL bNewFmt)
	{
		if (!bNewFmt)
			dp.Data_ReadString(*(std::string*)_Ptr(objOwner));
		else
			dp.Data_ReadStringSH(*(std::string*)_Ptr(objOwner));
		return TRUE;
	}

	virtual void LoadDelta(void *objOwner,CDataPacket &dp,std::vector<void*>*ptrsDelta)
	{
		dp.Data_ReadStringSH(*(std::string*)_Ptr(objOwner));
		if (ptrsDelta)
			ptrsDelta->push_back(_Ptr(objOwner));
	}

	virtual void Copy(void *objDest,void *objSrc)
	{
		*(std::string*)_Ptr(objDest)=*(std::string*)_Ptr(objSrc);
	}


	virtual BOOL GetVar(void *objOwner,void **var)
	{		
		if (var)
			*var=NULL;
		if (objOwner&&var)
			*var=_Ptr(objOwner);

		return TRUE;
	}


	std::string init;
};

template <class T>
struct GElem_ObjSingle:public GElemBase
{
	virtual int GetTypeID()	{		return 2;	}
	virtual BOOL CheckCompatible(GElemBase *elemOther)
	{
		if (!CheckCompatible_General(elemOther))
			return FALSE;
		return TRUE;
	}
	virtual void Zero(void *objOwner,BOOL bIntuitive)
	{
		if(!bIntuitive)
			((T*)_Ptr(objOwner))->__gobj.Zero(bIntuitive);
	}
	virtual void Clear(void *objOwner,BOOL bIntuitive)
	{
		if (!bIntuitive)
			((T*)_Ptr(objOwner))->__gobj.Clear(bIntuitive);
	}

	virtual void Copy(void *objDest,void *objSrc)
	{
		((T*)_Ptr(objDest))->__gobj.Copy(&((T*)_Ptr(objSrc))->__gobj);
	}

	virtual void Save(void *objOwner,CDataPacket &dp,BOOL bNewFmt)
	{
		((T*)_Ptr(objOwner))->__gobj.Save(dp,bNewFmt);
	}
	virtual void SaveDelta(void *objOwner,void *objOwnerRef,CDataPacket &dp)
	{
		((T*)_Ptr(objOwner))->__gobj.SaveDelta(dp,&((T*)_Ptr(objOwnerRef))->__gobj);
	}

	virtual BOOL Load(void *objOwner,CDataPacket &dp,BOOL bNewFmt)
	{
		return ((T*)_Ptr(objOwner))->__gobj.Load(dp,bNewFmt,NULL);
	}

	virtual void LoadDelta(void *objOwner,CDataPacket &dp,std::vector<void*>*ptrsDelta)
	{
		((T*)_Ptr(objOwner))->__gobj.LoadDelta(dp,ptrsDelta);
	}

	virtual BOOL GetObj(void *objOwner,GObjBase **obj)
	{		
		if (obj)
			*obj=NULL;
		if (objOwner&&obj)
			*obj=&(((T*)_Ptr(objOwner))->__gobj);
		return TRUE;	
	}
	virtual const char *GetObjName()//Get the obj's name this elem is working on
	{
		static T t;
		return t.__gobj.GetName();
	}

};


template <class T>
struct GElem_ObjVar:public GElemBase
{
	virtual int GetTypeID()	{		return 3;	}
	virtual BOOL CheckCompatible(GElemBase *elemOther)
	{
		if (!CheckCompatible_General(elemOther))
			return FALSE;
		return TRUE;
	}

	virtual void Zero(void *objOwner,BOOL bIntuitive)
	{
		if(!bIntuitive)
			((T*)_Ptr(objOwner))->Zero(bIntuitive);
	}
	virtual void Clear(void *objOwner,BOOL bIntuitive)
	{
		if (!bIntuitive)
			((T*)_Ptr(objOwner))->Clear();
	}

	virtual void Copy(void *objDest,void *objSrc)
	{
		((T*)_Ptr(objDest))->Copy(((T*)_Ptr(objSrc)));
	}

	virtual void Save(void *objOwner,CDataPacket &dp,BOOL bNewFmt)
	{
		((T*)_Ptr(objOwner))->Save(dp);
	}

	virtual void SaveDelta(void *objOwner,void *objOwnerRef,CDataPacket &dp)
	{
		((T*)_Ptr(objOwner))->SaveDelta(dp,((T*)_Ptr(objOwnerRef)));
// 		((T*)_Ptr(objOwner))->Save(dp);
	}

	virtual BOOL Load(void *objOwner,CDataPacket &dp,BOOL bNewFmt)
	{
		return ((T*)_Ptr(objOwner))->Load(dp);
	}
	virtual void LoadDelta(void *objOwner,CDataPacket &dp,std::vector<void*>*ptrsDelta)
	{
		((T*)_Ptr(objOwner))->LoadDelta(dp,ptrsDelta);
// 		((T*)_Ptr(objOwner))->Load(dp);
// 		if (ptrsDelta)
// 			ptrsDelta->push_back(_Ptr(objOwner));
	}

	virtual BOOL GetObj(void *objOwner,GObjBase **obj)
	{		
		if (obj)
			*obj=NULL;
		if (objOwner&&obj)
			*obj=&(((T*)_Ptr(objOwner))->__gobj);
		return TRUE;	
	}
	virtual const char *GetObjName()//Get the obj's name this elem is working on
	{
		static T t;
		return t.__gobj.GetName();
	}

};



template <class T>
struct GElem_VarVector:public GElemBase
{
	enum DeltaOp
	{
		Delta_Overwrite,
		Delta_Append,
	};
	virtual int GetTypeID()	{		return 4;	}
	virtual BOOL CheckCompatible(GElemBase *elemOther)
	{
		if (!CheckCompatible_General(elemOther))
			return FALSE;
		return TRUE;
	}
	virtual void Zero(void *objOwner,BOOL bIntuitive)
	{
	}
	virtual void Clear(void *objOwner,BOOL bIntuitive)
	{
		((std::vector<T>*)_Ptr(objOwner))->clear();
	}

	virtual void Copy(void *objDest,void *objSrc)
	{
		*((std::vector<T>*)_Ptr(objDest))=*((std::vector<T>*)_Ptr(objSrc));
	}

	virtual void Save(void *objOwner,CDataPacket &dp,BOOL bNewFmt)
	{
		std::vector<T>*p=((std::vector<T>*)_Ptr(objOwner));
		dp.Data_EncodeDword(p->size());
		if (p->size()>0)
			dp.Data_WriteData(&(*p)[0],p->size()*sizeof(T));
	}

	virtual void SaveDelta(void *objOwner,void *objOwnerRef,CDataPacket &dp)
	{
		std::vector<T>*p=((std::vector<T>*)_Ptr(objOwner));
		std::vector<T>*pRef=((std::vector<T>*)_Ptr(objOwnerRef));

		DeltaOp op=Delta_Overwrite;
		if (p->size()>pRef->size())
		{
			if (memcmp(&(*pRef)[0],&(*p)[0],pRef->size()*sizeof(T))==0)
			{//ǰ��һ��
				op=Delta_Append;

				dp.Data_NextDword()=op;
				dp.Data_EncodeDword(p->size()-pRef->size());
				dp.Data_WriteData(&(*p)[pRef->size()],(p->size()-pRef->size())*sizeof(T));
				return;
			}
		}

		dp.Data_NextDword()=op;
		dp.Data_EncodeDword(p->size());
		dp.Data_WriteData(&(*p)[0],p->size()*sizeof(T));
	}

	virtual BOOL Load(void *objOwner,CDataPacket &dp,BOOL bNewFmt)
	{
		std::vector<T>*p=((std::vector<T>*)_Ptr(objOwner));
		if (!bNewFmt)
			p->resize(dp.Data_NextDword());
		else
			p->resize(dp.Data_DecodeDword());
		if (p->size()>0)
			dp.Data_ReadData(&(*p)[0],p->size()*sizeof(T));
		return TRUE;
	}

	virtual void LoadDelta(void *objOwner,CDataPacket &dp,std::vector<void*>*ptrsDelta)
	{
		std::vector<T>*p=((std::vector<T>*)_Ptr(objOwner));

		DeltaOp op=(DeltaOp)dp.Data_NextDword();

		if (op==Delta_Overwrite)
		{
			p->resize(dp.Data_DecodeDword());
			dp.Data_ReadData(&(*p)[0],p->size()*sizeof(T));
			if (ptrsDelta)
				ptrsDelta->push_back(_Ptr(objOwner));
		}
		else
		{
			//Append
			DWORD szAppend=dp.Data_DecodeDword();
			DWORD sz=p->size();
			p->resize(sz+szAppend);

			dp.Data_ReadData(&(*p)[sz],szAppend*sizeof(T));

			if (ptrsDelta)
			{
				for (int i=0;i<szAppend;i++)
					ptrsDelta->push_back(&(*p)[sz+i]);
			}
		}

	}


	virtual BOOL NewSub(void *objOwner)
	{	
		if (objOwner)
		{
			std::vector<T>*p=((std::vector<T>*)_Ptr(objOwner));
			p->resize(p->size()+1);
			(*p)[p->size()-1]=init;
		}
		return TRUE;	
	}
	virtual BOOL GetSubCount(void *objOwner,DWORD *c)
	{	
		if (c)
			*c=0;
		if (objOwner)
		{
			std::vector<T>*p=((std::vector<T>*)_Ptr(objOwner));
			*c=p->size();
		}
		return TRUE;	
	}
	virtual BOOL RemoveSub(void *objOwner,DWORD idx)
	{		
		if (objOwner)
		{
			std::vector<T>*p=((std::vector<T>*)_Ptr(objOwner));
			if (idx>=p->size())
				return FALSE;
			p->erase(p->begin()+idx);
		}
		return TRUE;	
	}
	virtual BOOL CloneSub(void *objOwner,DWORD idx)
	{		
		if (objOwner)
		{
			std::vector<T>*p=((std::vector<T>*)_Ptr(objOwner));
			if (idx>=p->size())
				return FALSE;
			p->resize(p->size()+1);
			(*p)[p->size()-1]=(*p)[idx];
		}
		return TRUE;	
	}
	virtual BOOL MoveSub(void *objOwner,DWORD idx,BOOL bUp)
	{		
		if (objOwner)
		{
			std::vector<T>*p=((std::vector<T>*)_Ptr(objOwner));

			MOVESUB_CHECK_IDX(idx2,bUp,p->size());

			T t=(*p)[idx];
			(*p)[idx]=(*p)[idx2];
			(*p)[idx2]=t;
		}
		return TRUE;	
	}
	virtual BOOL ClearSub(void *objOwner)
	{		
		if (objOwner)
		{
			std::vector<T>*p=((std::vector<T>*)_Ptr(objOwner));
			p->clear();
		}
		return TRUE;
	}
	virtual BOOL GetSubVar(void *objOwner,DWORD idx,void **var)
	{		
		if (var)
			*var=NULL;
		if (objOwner)
		{
			std::vector<T>*p=((std::vector<T>*)_Ptr(objOwner));
			if (idx>=p->size())
				return FALSE;
			if (var)
				*var=&(*p)[idx];
		}
		return TRUE;	
	}
	virtual BOOL SaveSub(void *objOwner,DWORD idx,CDataPacket &dp)
	{
		if (objOwner)
		{
			std::vector<T>*p=((std::vector<T>*)_Ptr(objOwner));
			if (idx<p->size())
			{
				dp.Data_WriteData(&(*p)[idx],sizeof(T));
				return TRUE;
			}
		}
		return FALSE;	
	}
	virtual BOOL LoadSub(void *objOwner,DWORD idx,CDataPacket &dp)	
	{
		if (objOwner)
		{
			std::vector<T>*p=((std::vector<T>*)_Ptr(objOwner));
			if (idx<p->size())
			{
				dp.Data_ReadData(&(*p)[idx],sizeof(T));
				return TRUE;
			}
		}
		return FALSE;	
	}

	T init;
};


struct GElem_StringVector:public GElemBase
{
	enum DeltaOp
	{
		Delta_Overwrite,
		Delta_Append,
	};

	virtual int GetTypeID()	{		return 5;	}
	virtual BOOL CheckCompatible(GElemBase *elemOther)
	{
		if (!CheckCompatible_General(elemOther))
			return FALSE;
		return TRUE;
	}
	virtual void Zero(void *objOwner,BOOL bIntuitive)
	{
	}
	virtual void Clear(void *objOwner,BOOL bIntuitive)
	{
		((std::vector<std::string>*)_Ptr(objOwner))->clear();
	}

	virtual void Copy(void *objDest,void *objSrc)
	{
		*((std::vector<std::string>*)_Ptr(objDest))=*((std::vector<std::string>*)_Ptr(objSrc));
	}

	virtual void Save(void *objOwner,CDataPacket &dp,BOOL bNewFmt)
	{
		std::vector<std::string>*p=((std::vector<std::string>*)_Ptr(objOwner));
		dp.Data_EncodeDword(p->size());
		for (int i=0;i<p->size();i++)
			dp.Data_WriteStringSH((*p)[i].c_str());
	}

	virtual void SaveDelta(void *objOwner,void *objOwnerRef,CDataPacket &dp)
	{
		std::vector<std::string>*p=((std::vector<std::string>*)_Ptr(objOwner));
		std::vector<std::string>*pRef=((std::vector<std::string>*)_Ptr(objOwnerRef));

		if (p->size()>=pRef->size())
		{
			int i;
			for (i=0;i<pRef->size();i++)
			{
				if ((*pRef)[i]!=(*p)[i])
					break;
			}
			if (i>pRef->size())
			{//ǰ��һ��
				dp.Data_NextDword()=Delta_Append;
				dp.Data_EncodeDword(p->size()-pRef->size());
				for (;i<p->size();i++)
				{
					dp.Data_WriteStringSH((*p)[i].c_str());
				}
				return;
			}
		}

		dp.Data_NextDword()=Delta_Overwrite;
		dp.Data_EncodeDword(p->size());
		for (int i=0;i<p->size();i++)
			dp.Data_WriteStringSH((*p)[i].c_str());
	}

	virtual BOOL Load(void *objOwner,CDataPacket &dp,BOOL bNewFmt)
	{
		std::vector<std::string>*p=((std::vector<std::string>*)_Ptr(objOwner));
		if (!bNewFmt)
		{
			p->resize(dp.Data_NextDword());
			for (int i=0;i<p->size();i++)
				dp.Data_ReadString((*p)[i]);
		}
		else
		{
			p->resize(dp.Data_DecodeDword());
			for (int i=0;i<p->size();i++)
				dp.Data_ReadStringSH((*p)[i]);
		}
		return TRUE;
	}

	virtual void LoadDelta(void *objOwner,CDataPacket &dp,std::vector<void*>*ptrsDelta)
	{
		std::vector<std::string>*p=((std::vector<std::string>*)_Ptr(objOwner));

		DeltaOp op=(DeltaOp)dp.Data_NextDword();

		if (op==Delta_Overwrite)
		{
			p->resize(dp.Data_DecodeDword());
			for (int i=0;i<p->size();i++)
				dp.Data_ReadStringSH((*p)[i]);
			if (ptrsDelta)
				ptrsDelta->push_back(_Ptr(objOwner));
		}
		else
		{
			//Append
			DWORD szAppend=dp.Data_DecodeDword();
			DWORD sz=p->size();
			p->resize(sz+szAppend);

			for (int i=0;i<szAppend;i++)
				dp.Data_ReadStringSH((*p)[i+sz]);

			if (ptrsDelta)
			{
				for (int i=0;i<szAppend;i++)
					ptrsDelta->push_back(&(*p)[i+sz]);
			}
		}

	}


	virtual BOOL NewSub(void *objOwner)
	{	
		if (objOwner)
		{
			std::vector<std::string>*p=((std::vector<std::string>*)_Ptr(objOwner));
			p->resize(p->size()+1);
			(*p)[p->size()-1]=init;
		}
		return TRUE;	
	}
	virtual BOOL GetSubCount(void *objOwner,DWORD *c)
	{	
		if (c)
			*c=0;
		if (objOwner)
		{
			std::vector<std::string>*p=((std::vector<std::string>*)_Ptr(objOwner));
			*c=p->size();
		}
		return TRUE;	
	}
	virtual BOOL RemoveSub(void *objOwner,DWORD idx)
	{		
		if (objOwner)
		{
			std::vector<std::string>*p=((std::vector<std::string>*)_Ptr(objOwner));
			if (idx>=p->size())
				return FALSE;
			p->erase(p->begin()+idx);
		}
		return TRUE;	
	}
	virtual BOOL CloneSub(void *objOwner,DWORD idx)
	{		
		if (objOwner)
		{
			std::vector<std::string>*p=((std::vector<std::string>*)_Ptr(objOwner));
			if (idx>=p->size())
				return FALSE;
			p->resize(p->size()+1);
			(*p)[p->size()-1]=(*p)[idx];
		}
		return TRUE;	
	}
	virtual BOOL MoveSub(void *objOwner,DWORD idx,BOOL bUp)
	{		
		if (objOwner)
		{
			std::vector<std::string>*p=((std::vector<std::string>*)_Ptr(objOwner));

			MOVESUB_CHECK_IDX(idx2,bUp,p->size());

			std::string t=(*p)[idx];
			(*p)[idx]=(*p)[idx2];
			(*p)[idx2]=t;
		}
		return TRUE;	
	}
	virtual BOOL ClearSub(void *objOwner)
	{		
		if (objOwner)
		{
			std::vector<std::string>*p=((std::vector<std::string>*)_Ptr(objOwner));
			p->clear();
		}
		return TRUE;
	}
	virtual BOOL GetSubVar(void *objOwner,DWORD idx,void **var)
	{		
		if (var)
			*var=NULL;
		if (objOwner)
		{
			std::vector<std::string>*p=((std::vector<std::string>*)_Ptr(objOwner));
			if (idx>=p->size())
				return FALSE;
			if (var)
				*var=&(*p)[idx];
		}
		return TRUE;	
	}

	virtual BOOL SaveSub(void *objOwner,DWORD idx,CDataPacket &dp)
	{
		if (objOwner)
		{
			std::vector<std::string>*p=((std::vector<std::string>*)_Ptr(objOwner));
			if (idx<p->size())
			{
				dp.Data_WriteString((*p)[idx].c_str());
				return TRUE;
			}
		}
		return FALSE;	
	}
	virtual BOOL LoadSub(void *objOwner,DWORD idx,CDataPacket &dp)	
	{
		if (objOwner)
		{
			std::vector<std::string>*p=((std::vector<std::string>*)_Ptr(objOwner));
			if (idx<p->size())
			{
				dp.Data_ReadString((*p)[idx]);
				return TRUE;
			}
		}
		return FALSE;	
	}


	std::string init;
};


template <class T>
struct GElem_ObjVector:public GElemBase
{
	enum DeltaOp
	{
		Delta_Overwrite,
		Delta_Append,
	};

	virtual int GetTypeID()	{		return 6;	}

	virtual BOOL CheckCompatible(GElemBase *elemOther)
	{
		if (!CheckCompatible_General(elemOther))
			return FALSE;
		return TRUE;
	}

	virtual void Zero(void *objOwner,BOOL bIntuitive)
	{
	}
	virtual void Clear(void *objOwner,BOOL bIntuitive)
	{
		std::vector<T>*p=(std::vector<T>*)_Ptr(objOwner);
//		for (int i=0;i<p->size();i++)		//the clear task will be done in T::GClear() in T::~T()
//			(*p)[i].__gobj.Clear();
		p->clear();
	}

	virtual void Copy(void *objDest,void *objSrc)
	{
		std::vector<T>*p=(std::vector<T>*)_Ptr(objDest);
		std::vector<T>*q=(std::vector<T>*)_Ptr(objSrc);

		p->resize(q->size());
		for (int i=0;i<q->size();i++)
			(*p)[i].__gobj.Copy(&(*q)[i].__gobj);
	}


	virtual void Save(void *objOwner,CDataPacket &dp,BOOL bNewFmt)
	{
		std::vector<T>*p=((std::vector<T>*)_Ptr(objOwner));
		dp.Data_EncodeDword(p->size());
		for (int i=0;i<p->size();i++)
			(*p)[i].__gobj.Save(dp,bNewFmt);
	}

	virtual void SaveDelta(void *objOwner,void *objOwnerRef,CDataPacket &dp)
	{
		std::vector<T>*p=((std::vector<T>*)_Ptr(objOwner));
		std::vector<T>*pRef=((std::vector<T>*)_Ptr(objOwnerRef));

		if (p->size()>=pRef->size())
		{
			int i;
			for (i=0;i<pRef->size();i++)
			{
				if (!(*pRef)[i].__gobj.Equals(&(*p)[i].__gobj))
					break;
			}
			if (i>=pRef->size())
			{//ǰ��һ��
				dp.Data_NextDword()=Delta_Append;
				dp.Data_EncodeDword(p->size()-pRef->size());
				for (;i<p->size();i++)
					(*p)[i].__gobj.Save(dp,TRUE);
				return;
			}
		}

		dp.Data_NextDword()=Delta_Overwrite;
		dp.Data_EncodeDword(p->size());
		for (int i=0;i<p->size();i++)
			(*p)[i].__gobj.Save(dp,TRUE);
	}

	virtual BOOL Load(void *objOwner,CDataPacket &dp,BOOL bNewFmt)
	{
		Clear(objOwner,FALSE);
		std::vector<T>*p=((std::vector<T>*)_Ptr(objOwner));
		if (!bNewFmt)
			p->resize(dp.Data_NextDword());
		else
			p->resize(dp.Data_DecodeDword());
		BOOL bOk=TRUE;
		for (int i=0;i<p->size();i++)
		{
			if (FALSE==(*p)[i].__gobj.Load(dp,bNewFmt,NULL))
				bOk=FALSE;
		}
		return bOk;
	}

	virtual void LoadDelta(void *objOwner,CDataPacket &dp,std::vector<void*>*ptrsDelta)
	{
		std::vector<T>*p=((std::vector<T>*)_Ptr(objOwner));

		DeltaOp op=(DeltaOp)dp.Data_NextDword();

		if (op==Delta_Overwrite)
		{
			p->resize(dp.Data_DecodeDword());
			for (int i=0;i<p->size();i++)
				(*p)[i].__gobj.Load(dp,TRUE,NULL);
			if (ptrsDelta)
				ptrsDelta->push_back(_Ptr(objOwner));
		}
		else
		{
			//Append

			std::vector<T> buf;
			DWORD szAppend=dp.Data_DecodeDword();
			DWORD sz=p->size();
			buf.resize(sz+szAppend);

			for (int i=0;i<p->size();i++)
				buf[i].__gobj.Copy(&(*p)[i].__gobj);

			for (int i=0;i<szAppend;i++)
				buf[i+sz].__gobj.Load(dp,TRUE,NULL);

			if (ptrsDelta)
			{
				for (int i=0;i<szAppend;i++)
					ptrsDelta->push_back(buf[i+sz].__gobj.GetOwner());
			}

			buf.swap(*p);
		}

	}


	virtual BOOL NewSub(void *objOwner)//add a new sub to the tail
	{		
		if (objOwner)
		{
			std::vector<T>*p=((std::vector<T>*)_Ptr(objOwner));
			T t;
			p->push_back(t);
		}
		return TRUE;	
	}
	virtual BOOL GetSubCount(void *objOwner,DWORD *c)
	{		
		if (objOwner)
		{
			std::vector<T>*p=((std::vector<T>*)_Ptr(objOwner));
			if (c)
				*c=p->size();
		}
		return TRUE;	
	}
	virtual BOOL RemoveSub(void *objOwner,DWORD idx)
	{		
		if (objOwner)
		{
			std::vector<T>*p=((std::vector<T>*)_Ptr(objOwner));
			if (idx>=p->size())
				return FALSE;
			p->erase(p->begin()+idx);
		}
		return TRUE;	
	}
	virtual BOOL CloneSub(void *objOwner,DWORD idx)
	{		
		if (objOwner)
		{
			std::vector<T>*p=((std::vector<T>*)_Ptr(objOwner));
			if (idx>=p->size())
				return FALSE;
			p->push_back((*p)[idx]);
		}
		return TRUE;	
	}
	virtual BOOL MoveSub(void *objOwner,DWORD idx,BOOL bUp)
	{
		if (objOwner)
		{
			std::vector<T>*p=((std::vector<T>*)_Ptr(objOwner));

			MOVESUB_CHECK_IDX(idx2,bUp,p->size());

			T t;
			t.__gobj.Copy(&(*p)[idx].__gobj);
			(*p)[idx].__gobj.Copy(&(*p)[idx2].__gobj);
			(*p)[idx2].__gobj.Copy(&t.__gobj);
		}
		return TRUE;
	}
	virtual BOOL ClearSub(void *objOwner)
	{		
		if (objOwner)
			Clear(objOwner,FALSE);
		return TRUE;	
	}
	virtual BOOL GetSubObj(void *objOwner,DWORD idx,GObjBase **obj)
	{	
		if (obj)
			*obj=NULL;
		if (objOwner)
		{
			std::vector<T>*p=((std::vector<T>*)_Ptr(objOwner));
			if (idx>=p->size())
				return FALSE;
			*obj=&(*p)[idx].__gobj;
		}
		return TRUE;	
	}
	virtual BOOL SaveSub(void *objOwner,DWORD idx,CDataPacket &dp)
	{
		if (objOwner)
		{
			std::vector<T>*p=((std::vector<T>*)_Ptr(objOwner));
			if (idx<p->size())
			{
				(*p)[idx].__gobj.Save(dp,TRUE);
				return TRUE;
			}
		}
		return FALSE;	
	}
	virtual BOOL LoadSub(void *objOwner,DWORD idx,CDataPacket &dp)	
	{
		if (objOwner)
		{
			std::vector<T>*p=((std::vector<T>*)_Ptr(objOwner));
			if (idx<p->size())
			{
				if ((*p)[idx].__gobj.Load(dp,TRUE,NULL))
					return TRUE;
			}
		}
		return FALSE;	
	}

	virtual const char *GetObjName()//Get the obj's name this elem is working on
	{
		static T t;
		return t.__gobj.GetName();
	}
};


template <class T>
struct GElem_VarArray:public GElemBase
{
	virtual int GetTypeID()	{		return 7;	}
	virtual BOOL CheckCompatible(GElemBase *elemOther)
	{
		if (!CheckCompatible_General(elemOther))
			return FALSE;
		return TRUE;
	}

	virtual void Zero(void *objOwner,BOOL bIntuitive)
	{
		T* p=(T*)_Ptr(objOwner);
		for (int i=0;i<szArray;i++)
			p[i]=init;
	}
	virtual void Clear(void *objOwner,BOOL bIntuitive)
	{
	}
	virtual void Copy(void *objDest,void *objSrc)
	{
		T* p=(T*)_Ptr(objDest);
		T* q=(T*)_Ptr(objSrc);

		for (int i=0;i<szArray;i++)
			p[i]=q[i];
	}

	virtual void Save(void *objOwner,CDataPacket &dp,BOOL bNewFmt)
	{
		T* p=(T*)_Ptr(objOwner);
		dp.Data_WriteData(p,sizeof(T)*szArray);
	}

	virtual void SaveDelta(void *objOwner,void *objOwnerRef,CDataPacket &dp)
	{
		T* p=(T*)_Ptr(objOwner);
		T* q=(T*)_Ptr(objOwnerRef);

		for (int i=0;i<szArray;i++)
		{
			if (memcmp(&p[i],&q[i],sizeof(T))!=0)
			{
				dp.Data_NextWord()=i;
				dp.Data_WriteData(&p[i],sizeof(T));
			}
		}
		dp.Data_NextWord()=0xffff;//��ֹ��
	}

	virtual BOOL Load(void *objOwner,CDataPacket &dp,BOOL bNewFmt)
	{
		T* p=(T*)_Ptr(objOwner);
		dp.Data_ReadData(p,sizeof(T)*szArray);
		return TRUE;
	}

	virtual void LoadDelta(void *objOwner,CDataPacket &dp,std::vector<void*>*ptrsDelta)
	{
		T* p=(T*)_Ptr(objOwner);

		while(1)
		{
			WORD idx=dp.Data_NextWord();
			if (idx==0xffff)
				break;

			dp.Data_ReadData(&p[idx],sizeof(T));
			if (ptrsDelta)
				ptrsDelta->push_back(&p[idx]);
		}
	}


	virtual BOOL GetSubCount(void *objOwner,DWORD *c)
	{
		if (c)
			*c=szArray;
		return TRUE;
	}
	virtual BOOL MoveSub(void *objOwner,DWORD idx,BOOL bUp)//exchange the sub's position with its prev/next sibling
	{
		if (objOwner)
		{
			T* p=(T*)_Ptr(objOwner);

			MOVESUB_CHECK_IDX(idx2,bUp,szArray);

			T t=p[idx];
			p[idx]=p[idx2];
			p[idx2]=t;
		}
		return TRUE;
	}
	virtual BOOL GetSubVar(void *objOwner,DWORD idx,void **var)//
	{		
		if (var)
			*var=NULL;
		if (objOwner)
		{
			T* p=(T*)_Ptr(objOwner);
			if (idx>=szArray)
				return FALSE;
			if (var)
				*var=&p[idx];
		}
		return TRUE;	
	}
	virtual BOOL SaveSub(void *objOwner,DWORD idx,CDataPacket &dp)
	{
		if (objOwner)
		{
			T* p=(T*)_Ptr(objOwner);
			if (idx<szArray)
			{
				dp.Data_WriteData(&p[idx],sizeof(T));
				return TRUE;
			}
		}
		return FALSE;	
	}
	virtual BOOL LoadSub(void *objOwner,DWORD idx,CDataPacket &dp)	
	{
		if (objOwner)
		{
			T* p=(T*)_Ptr(objOwner);
			if (idx<szArray)
			{
				dp.Data_ReadData(&p[idx],sizeof(T));
				return TRUE;
			}
		}
		return FALSE;	
	}

	T init;
};


//����������������С�����仯ʱ,�ܹ�����֮ǰ�����ݰ汾
template <class T>
struct GElem_VarArray2:public GElemBase
{
	virtual int GetTypeID()	{		return 11;	}
	virtual BOOL CheckCompatible(GElemBase *elemOther)
	{
		if (!CheckCompatible_General(elemOther))
			return FALSE;
		return TRUE;
	}

	virtual void Zero(void *objOwner,BOOL bIntuitive)
	{
		T* p=(T*)_Ptr(objOwner);
		for (int i=0;i<szArray;i++)
			p[i]=init;
	}
	virtual void Clear(void *objOwner,BOOL bIntuitive)
	{
	}
	virtual void Copy(void *objDest,void *objSrc)
	{
		T* p=(T*)_Ptr(objDest);
		T* q=(T*)_Ptr(objSrc);

		for (int i=0;i<szArray;i++)
			p[i]=q[i];
	}

	virtual void Save(void *objOwner,CDataPacket &dp,BOOL bNewFmt)
	{
		T* p=(T*)_Ptr(objOwner);
		WORD szT=szArray;
		dp.Data_WriteSimple(szT);
		dp.Data_WriteData(p,sizeof(T)*szArray);
	}
	virtual void SaveDelta(void *objOwner,void *objOwnerRef,CDataPacket &dp)
	{
		T* p=(T*)_Ptr(objOwner);
		T* q=(T*)_Ptr(objOwnerRef);

		for (int i=0;i<szArray;i++)
		{
			if (memcmp(&p[i],&q[i],sizeof(T))!=0)
			{
				dp.Data_NextWord()=i;
				dp.Data_WriteData(&p[i],sizeof(T));
			}
		}
		dp.Data_NextWord()=0xffff;//��ֹ��
	}

	virtual BOOL Load(void *objOwner,CDataPacket &dp,BOOL bNewFmt)
	{
		T* p=(T*)_Ptr(objOwner);
		WORD szT;
		dp.Data_ReadSimple(szT);
		if (szT<=szArray)
			dp.Data_ReadData(p,sizeof(T)*szT);
		else
		{
			dp.Data_ReadData(p,sizeof(T)*szArray);
			dp.Data_MarchData(sizeof(T)*(szT-szArray));
		}
		return TRUE;
	}

	virtual void LoadDelta(void *objOwner,CDataPacket &dp,std::vector<void*>*ptrsDelta)
	{
		T* p=(T*)_Ptr(objOwner);

		while(1)
		{
			WORD idx=dp.Data_NextWord();
			if (idx==0xffff)
				break;

			dp.Data_ReadData(&p[idx],sizeof(T));
			if (ptrsDelta)
				ptrsDelta->push_back(&p[idx]);
		}
	}

	virtual BOOL GetSubCount(void *objOwner,DWORD *c)
	{
		if (c)
			*c=szArray;
		return TRUE;
	}
	virtual BOOL MoveSub(void *objOwner,DWORD idx,BOOL bUp)//exchange the sub's position with its prev/next sibling
	{
		if (objOwner)
		{
			T* p=(T*)_Ptr(objOwner);

			MOVESUB_CHECK_IDX(idx2,bUp,szArray);

			T t=p[idx];
			p[idx]=p[idx2];
			p[idx2]=t;
		}
		return TRUE;
	}
	virtual BOOL GetSubVar(void *objOwner,DWORD idx,void **var)//
	{		
		if (var)
			*var=NULL;
		if (objOwner)
		{
			T* p=(T*)_Ptr(objOwner);
			if (idx>=szArray)
				return FALSE;
			if (var)
				*var=&p[idx];
		}
		return TRUE;	
	}
	virtual BOOL SaveSub(void *objOwner,DWORD idx,CDataPacket &dp)
	{
		if (objOwner)
		{
			T* p=(T*)_Ptr(objOwner);
			if (idx<szArray)
			{
				dp.Data_WriteData(&p[idx],sizeof(T));
				return TRUE;
			}
		}
		return FALSE;	
	}
	virtual BOOL LoadSub(void *objOwner,DWORD idx,CDataPacket &dp)	
	{
		if (objOwner)
		{
			T* p=(T*)_Ptr(objOwner);
			if (idx<szArray)
			{
				dp.Data_ReadData(&p[idx],sizeof(T));
				return TRUE;
			}
		}
		return FALSE;	
	}

	T init;
};


struct GElem_StringArray:public GElemBase
{
	virtual int GetTypeID()	{		return 8;	}
	virtual BOOL CheckCompatible(GElemBase *elemOther)
	{
		if (!CheckCompatible_General(elemOther))
			return FALSE;
		return TRUE;
	}
	virtual void Zero(void *objOwner,BOOL bIntuitive)
	{
		std::string* p=(std::string*)_Ptr(objOwner);
		for (int i=0;i<szArray;i++)
			p[i]=init;
	}
	virtual void Clear(void *objOwner,BOOL bIntuitive)
	{
	}
	virtual void Copy(void *objDest,void *objSrc)
	{
		std::string* p=(std::string*)_Ptr(objDest);
		std::string* q=(std::string*)_Ptr(objSrc);

		for (int i=0;i<szArray;i++)
			p[i]=q[i];
	}

	virtual void Save(void *objOwner,CDataPacket &dp,BOOL bNewFmt)
	{
		std::string* p=(std::string*)_Ptr(objOwner);
		for (int i=0;i<szArray;i++)
			dp.Data_WriteStringSH(p[i].c_str());
	}
	virtual void SaveDelta(void *objOwner,void *objOwnerRef,CDataPacket &dp)
	{
		std::string* p=(std::string*)_Ptr(objOwner);
		std::string* q=(std::string*)_Ptr(objOwnerRef);

		for (int i=0;i<szArray;i++)
		{
			if (p[i]!=q[i])
			{
				dp.Data_NextWord()=i;
				dp.Data_WriteStringSH(p[i].c_str());
			}
		}
		dp.Data_NextWord()=0xffff;//��ֹ��
	}

	virtual BOOL Load(void *objOwner,CDataPacket &dp,BOOL bNewFmt)
	{
		std::string* p=(std::string*)_Ptr(objOwner);
		if (!bNewFmt)
		{
			for (int i=0;i<szArray;i++)
				dp.Data_ReadString(p[i]);
		}
		else
		{
			for (int i=0;i<szArray;i++)
				dp.Data_ReadStringSH(p[i]);
		}
		return TRUE;
	}

	virtual void LoadDelta(void *objOwner,CDataPacket &dp,std::vector<void*>*ptrsDelta)
	{
		std::string* p=(std::string*)_Ptr(objOwner);

		while(1)
		{
			WORD idx=dp.Data_NextWord();
			if (idx==0xffff)
				break;

			dp.Data_ReadStringSH(p[idx]);
			if (ptrsDelta)
				ptrsDelta->push_back(&p[idx]);
		}
	}


	virtual BOOL GetSubCount(void *objOwner,DWORD *c)
	{
		if (c)
			*c=szArray;
		return TRUE;
	}
	virtual BOOL MoveSub(void *objOwner,DWORD idx,BOOL bUp)//exchange the sub's position with its prev/next sibling
	{
		if (objOwner)
		{
			std::string* p=(std::string*)_Ptr(objOwner);

			MOVESUB_CHECK_IDX(idx2,bUp,szArray);

			std::string t=p[idx];
			p[idx]=p[idx2];
			p[idx2]=t;
		}
		return TRUE;
	}
	virtual BOOL GetSubVar(void *objOwner,DWORD idx,void **var)//
	{		
		if (var)
			*var=NULL;
		if (objOwner)
		{
			std::string* p=(std::string*)_Ptr(objOwner);
			if (idx>=szArray)
				return FALSE;
			if (var)
				*var=&p[idx];
		}
		return TRUE;	
	}
	virtual BOOL SaveSub(void *objOwner,DWORD idx,CDataPacket &dp)
	{
		if (objOwner)
		{
			std::string* p=(std::string*)_Ptr(objOwner);
			if (idx<szArray)
			{
				dp.Data_WriteStringSH(p[idx].c_str());
				return TRUE;
			}
		}
		return FALSE;	
	}
	virtual BOOL LoadSub(void *objOwner,DWORD idx,CDataPacket &dp)	
	{
		if (objOwner)
		{
			std::string* p=(std::string*)_Ptr(objOwner);
			if (idx<szArray)
			{
				dp.Data_ReadStringSH(p[idx]);
				return TRUE;
			}
		}
		return FALSE;	
	}

	std::string init;
};


template <class T>
struct GElem_ObjArray:public GElemBase
{
	virtual int GetTypeID()	{		return 9;	}
	virtual BOOL CheckCompatible(GElemBase *elemOther)
	{
		if (!CheckCompatible_General(elemOther))
			return FALSE;
		return TRUE;
	}
	virtual void Zero(void *objOwner,BOOL bIntuitive)
	{
		if (!bIntuitive)
		{
			T* p=(T*)_Ptr(objOwner);
			for (int i=0;i<szArray;i++)
				p[i].__gobj.Zero(bIntuitive);
		}
	}
	virtual void Clear(void *objOwner,BOOL bIntuitive)
	{
		if (!bIntuitive)
		{
			T* p=(T*)_Ptr(objOwner);
			for (int i=0;i<szArray;i++)
				p[i].__gobj.Clear(bIntuitive);
		}
	}

	virtual void Copy(void *objDest,void *objSrc)
	{
		T* p=(T*)_Ptr(objDest);
		T* q=(T*)_Ptr(objSrc);

		for (int i=0;i<szArray;i++)
			p[i].__gobj.Copy(&q[i].__gobj);
	}


	virtual void Save(void *objOwner,CDataPacket &dp,BOOL bNewFmt)
	{
		T* p=(T*)_Ptr(objOwner);
		for (int i=0;i<szArray;i++)
			p[i].__gobj.Save(dp,bNewFmt);
	}

	virtual void SaveDelta(void *objOwner,void *objOwnerRef,CDataPacket &dp)
	{
		T* p=(T*)_Ptr(objOwner);
		T* q=(T*)_Ptr(objOwnerRef);

		for (int i=0;i<szArray;i++)
		{
			if (!p[i].__gobj.Equals(&q[i].__gobj))
			{
				dp.Data_NextWord()=i;
				p[i].__gobj.SaveDelta(dp,&q[i].__gobj);
			}
		}
		dp.Data_NextWord()=0xffff;//��ֹ��
	}

	virtual BOOL Load(void *objOwner,CDataPacket &dp,BOOL bNewFmt)
	{
		Clear(objOwner,FALSE);
		T* p=(T*)_Ptr(objOwner);
		BOOL bOk=TRUE;
		for (int i=0;i<szArray;i++)
		{
			if (FALSE==p[i].__gobj.Load(dp,bNewFmt,NULL))
				bOk=FALSE;
		}
		return bOk;
	}

	virtual void LoadDelta(void *objOwner,CDataPacket &dp,std::vector<void*>*ptrsDelta)
	{
		T* p=(T*)_Ptr(objOwner);

		while(1)
		{
			WORD idx=dp.Data_NextWord();
			if (idx==0xffff)
				break;

			p[idx].__gobj.LoadDelta(dp,ptrsDelta);
		}
	}


	virtual BOOL GetSubCount(void *objOwner,DWORD *c)
	{
		if (c)
			*c=szArray;
		return TRUE;
	}
	virtual BOOL MoveSub(void *objOwner,DWORD idx,BOOL bUp)//exchange the sub's position with its prev/next sibling
	{
		if (objOwner)
		{
			T* p=(T*)_Ptr(objOwner);

			MOVESUB_CHECK_IDX(idx2,bUp,szArray);

			T t;
			t.__gobj.Copy(&p[idx].__gobj);
			p[idx].__gobj.Copy(&p[idx2].__gobj);
			p[idx2].__gobj.Copy(&t.__gobj);
		}
		return TRUE;
	}
	virtual BOOL GetSubObj(void *objOwner,DWORD idx,GObjBase **obj)
	{	
		if (obj)
			*obj=NULL;
		if (objOwner)
		{
			T* p=(T*)_Ptr(objOwner);
			if (idx>=szArray)
				return FALSE;
			if (obj)
				*obj=&(p[idx].__gobj);
		}
		return TRUE;	
	}
	virtual BOOL SaveSub(void *objOwner,DWORD idx,CDataPacket &dp)
	{
		if (objOwner)
		{
			T* p=(T*)_Ptr(objOwner);
			if (idx<szArray)
			{
				p[idx].__gobj.Save(dp,TRUE);
				return TRUE;
			}
		}
		return FALSE;	
	}
	virtual BOOL LoadSub(void *objOwner,DWORD idx,CDataPacket &dp)	
	{
		if (objOwner)
		{
			T* p=(T*)_Ptr(objOwner);
			if (idx<szArray)
			{
				if (p[idx].__gobj.Load(dp,TRUE,NULL))
					return TRUE;
			}
		}
		return FALSE;	
	}

	virtual const char *GetObjName()//Get the obj's name this elem is working on
	{
		static T t;
		return t.__gobj.GetName();
	}
};

inline void SaveGObj(CDataPacket &dp,GObjBase *gobj)
{
	dp.Data_NextWord()=0xffff;
	gobj->Save(dp,TRUE);
}


inline BOOL LoadGObj(CDataPacket &dp,GObjBase *gobj,BOOL *bRepaired)
{
	WORD *p=(WORD*)dp.GetCurBufferPointer();
	BOOL bNewFmt=FALSE;
	if (*p==0xffff)
	{
		bNewFmt=TRUE;
		dp.Data_MarchData(sizeof(WORD));
	}
	return gobj->Load(dp,bNewFmt,bRepaired);
}


//IMPORTANT: the class that embedding BEGIN_GOBJ ***MUST*** call GConstructor() in its
//constructor,and GDestructor() in its destructor
#define BEGIN_GOBJ(classtype,version)													\
public:																											\
GObj<classtype> __gobj;																			\
virtual GObjBase *GetGObj(){	return &__gobj;}										\
void GConstructor()																					\
{																													\
	static classtype::_dummyclass dummy;												\
	__gobj.Zero(TRUE);																					\
}																													\
void GDestructor()																						\
{																													\
	__gobj.Clear(TRUE);																				\
}																													\
void GClear()																								\
{																													\
	__gobj.Clear(FALSE);																				\
	__gobj.Zero(FALSE);																				\
}																													\
void GSave(CDataPacket &dp)																	\
{																													\
	SaveGObj(dp,&__gobj);																			\
}																													\
BOOL GLoad(CDataPacket &dp)																\
{																													\
	return LoadGObj(dp,&__gobj,NULL);													\
}																													\
BOOL GLoad(CDataPacket &dp,BOOL *bRepaired)								\
{																													\
	return LoadGObj(dp,&__gobj,bRepaired);											\
}																													\
classtype &operator=(const classtype &src)											\
{																													\
	__gobj.Copy((GObjBase*)&src.__gobj);													\
	return *this;																							\
}																													\
BOOL operator==(const classtype &src)													\
{																													\
	return __gobj.Equals((GObjBase*)&src.__gobj);									\
}																													\
struct _dummyclass																					\
{																													\
	~_dummyclass()																						\
	{																												\
		GElemBase *p=GObj<classtype>::__elems();									\
		while(p)																								\
		{																											\
			GElemBase *p2=p->next;																\
			delete p;																							\
			p=p2;																								\
		}																											\
		GObj<classtype>::__elems()=NULL;													\
	}																												\
	BOOL VerifyElemUID(BYTE uid)															\
	{																												\
		GElemBase *p=GObj<classtype>::__elems();									\
		while(p)																								\
		{																											\
			if (p->uid==uid)																				\
				return FALSE;																				\
			p=p->next;																						\
		}																											\
		return TRUE;																						\
	}																												\
	_dummyclass()																						\
	{																												\
		typedef classtype ClassType;																\
		classtype *ptr=NULL;																			\
		GObj<classtype>::__thisoff()=															\
						(int)((BYTE*)&ptr->__gobj-(BYTE*)ptr);								\
		GObj<classtype>::__name()=#classtype;											\
		GObj<classtype>::__ver()=version;													\
		std::unordered_map<std::string,void*>&datamp=									\
											GObj<classtype>::__data();							\
		GElemBase **lastelem=&GObj<classtype>::__elems();					\
		GElemBase *curelem=*lastelem;

#define GOBJ_GETBRIEF_FUNC(funcname)								\
	GObj<ClassType>::__brieffunc()=&ClassType::funcname;



#define END_GOBJ()																					\
	}																												\
	friend struct _dummyclass;																	\
};																													

#define BEGIN_GOBJ_PURE(classtype,version)										\
public:																											\
classtype()																									\
{																													\
	GConstructor();																						\
}																													\
classtype(const classtype&src)																	\
{																													\
	GConstructor();																						\
	*this=src;																								\
}																													\
virtual ~classtype()																					\
{																													\
	GDestructor();																							\
}																													\
BEGIN_GOBJ(classtype,version)

#define BEGIN_GOBJ_PURE_NESTED(classtype,fullclasstype,version)										\
public:																											\
classtype()																									\
{																													\
	GConstructor();																						\
}																													\
classtype(const fullclasstype&src)																	\
{																													\
	GConstructor();																						\
	*this=src;																								\
}																													\
virtual ~classtype()																					\
{																													\
	GDestructor();																							\
}																													\
BEGIN_GOBJ(fullclasstype,version)

#define BEGIN_GOBJ_PURE_NESTED(classtype,fullclasstype,version)										\
public:																											\
classtype()																									\
{																													\
	GConstructor();																						\
}																													\
classtype(const fullclasstype&src)																	\
{																													\
	GConstructor();																						\
	*this=src;																								\
}																													\
virtual ~classtype()																					\
{																													\
	GDestructor();																							\
}																													\
BEGIN_GOBJ(fullclasstype,version)


#define _ELEM_LINK																					\
	p->next=NULL;																						\
	*lastelem=p;																							\
	lastelem=&p->next;																				\
	curelem=p;

#define DERIVE_GOBJ(classtype,classtypebase)										\
	GObj<classtype>::__baseoff()=(int)((BYTE*)&ptr->__gobj					\
		-(BYTE*)&ptr->classtypebase::__gobj);											

//type name; (sample: int v; vector3df v)
#define GELEM_VAR_INIT(type,name,initv)												\
	{																												\
		GElem_VarSingle<type>*p=new GElem_VarSingle<type>;			\
		p->off=(DWORD)((BYTE*)&ptr->name-(BYTE*)ptr);						\
		p->sz=sizeof(ptr->name);																	\
		p->init=initv;																						\
		p->elemname=#name;																		\
		p->subtype=#type;																			\
		_ELEM_LINK;																						\
	}

//type name; (sample: int v;)
#define GELEM_VAR(type,name)																\
	{																												\
		GElem_VarSingle<type>*p=new GElem_VarSingle<type>;			\
		p->off=(DWORD)((BYTE*)&ptr->name-(BYTE*)ptr);						\
		p->sz=sizeof(ptr->name);																	\
		p->elemname=#name;																		\
		p->subtype=#type;																			\
		_ELEM_LINK;																						\
	}

//type name; (sample: int v;)
#define GELEM_VAR_ALIAS(type,name,alias)											\
	{																												\
		GElem_VarSingle<type>*p=new GElem_VarSingle<type>;			\
		p->off=(DWORD)((BYTE*)&ptr->name-(BYTE*)ptr);						\
		p->sz=sizeof(ptr->name);																	\
		p->elemname=alias;																			\
		p->subtype=#type;																			\
		_ELEM_LINK;																						\
	}

//std::string name;(sample: std::string v;)
#define GELEM_STRING_INIT(name,initstr)												\
	{																												\
		GElem_String*p=new GElem_String;												\
		p->off=(DWORD)((BYTE*)&ptr->name-(BYTE*)ptr);						\
		p->sz=sizeof(ptr->name);																	\
		p->init=initstr;																					\
		p->elemname=#name;																		\
		p->subtype="string";																			\
		_ELEM_LINK;																						\
	}

//std::string name;(sample: std::string v;)
#define GELEM_STRING(name) GELEM_STRING_INIT(name,"")

//std::vector<type> name; (sample: std::vector<int> v)
#define GELEM_VARVECTOR_INIT(type,name,initv)								\
	{																												\
		GElem_VarVector<type>*p=new GElem_VarVector<type>;			\
		p->off=(DWORD)((BYTE*)&ptr->name-(BYTE*)ptr);						\
		p->sz=sizeof(ptr->name);																	\
		p->init=initv;																						\
		p->elemname=#name;																		\
		p->subtype=#type;																			\
		_ELEM_LINK;																						\
	}

//std::vector<type> name; (sample: std::vector<int> v)
#define GELEM_VARVECTOR(type,name)												\
	{																												\
		GElem_VarVector<type>*p=new GElem_VarVector<type>;			\
		p->off=(DWORD)((BYTE*)&ptr->name-(BYTE*)ptr);						\
		p->sz=sizeof(ptr->name);																	\
		p->elemname=#name;																		\
		p->subtype=#type;																			\
		_ELEM_LINK;																						\
	}

//std::vector<std::sting> name; (sample: std::vector<std::sting> v)
#define GELEM_STRINGVECTOR_INIT(name,initv)									\
	{																												\
		GElem_StringVector*p=new GElem_StringVector;							\
		p->off=(DWORD)((BYTE*)&ptr->name-(BYTE*)ptr);						\
		p->sz=sizeof(ptr->name);																	\
		p->init=initv;																						\
		p->elemname=#name;																		\
		p->subtype="string";																			\
		_ELEM_LINK;																						\
	}

//std::vector<std::string> name; (sample: std::vector<std::string> v)
#define GELEM_STRINGVECTOR(name)													\
		GELEM_STRINGVECTOR_INIT(name,"")


//type name[const]; (sample: int v[100])
#define GELEM_VARARRAY_INIT(type,name,initv)									\
	{																												\
		GElem_VarArray<type>*p=new GElem_VarArray<type>;				\
		p->off=(DWORD)((BYTE*)ptr->name-(BYTE*)ptr);							\
		p->sz=sizeof(ptr->name);																	\
		p->szArray=sizeof(ptr->name)/sizeof(ptr->name[0]);							\
		p->init=initv;																						\
		p->elemname=#name;																		\
		p->subtype=#type;																			\
		_ELEM_LINK;																						\
	}


//type name[const]; (sample: int v[100])
#define GELEM_VARARRAY(type,name)													\
	{																												\
		GElem_VarArray<type>*p=new GElem_VarArray<type>;				\
		p->off=(DWORD)((BYTE*)ptr->name-(BYTE*)ptr);							\
		p->sz=sizeof(ptr->name);																	\
		p->szArray=sizeof(ptr->name)/sizeof(ptr->name[0]);							\
		p->elemname=#name;																		\
		p->subtype=#type;																			\
		_ELEM_LINK;																						\
	}

//type name[const]; (sample: int v[100])
#define GELEM_VARARRAY2_INIT(type,name,initv)								\
	{																												\
		GElem_VarArray2<type>*p=new GElem_VarArray2<type>;			\
		p->off=(DWORD)((BYTE*)ptr->name-(BYTE*)ptr);							\
		p->sz=sizeof(ptr->name);																	\
		p->szArray=sizeof(ptr->name)/sizeof(ptr->name[0]);							\
		p->init=initv;																						\
		p->elemname=#name;																		\
		p->subtype=#type;																			\
		_ELEM_LINK;																						\
	}

//type name[const]; (sample: int v[100])
#define GELEM_VARARRAY2(type,name)												\
	{																												\
		GElem_VarArray2<type>*p=new GElem_VarArray2<type>;			\
		p->off=(DWORD)((BYTE*)ptr->name-(BYTE*)ptr);							\
		p->sz=sizeof(ptr->name);																	\
		p->szArray=sizeof(ptr->name)/sizeof(ptr->name[0]);							\
		p->elemname=#name;																		\
		p->subtype=#type;																			\
		_ELEM_LINK;																						\
	}



//std::string name[const]; (sample: std::string v[100])
#define GELEM_STRINGARRAY_INIT(name,initv)									\
	{																												\
		GElem_StringArray*p=new GElem_StringArray;								\
		p->off=(DWORD)((BYTE*)ptr->name-(BYTE*)ptr);							\
		p->sz=sizeof(ptr->name);																	\
		p->szArray=sizeof(ptr->name)/sizeof(ptr->name[0]);							\
		p->init=initv;																						\
		p->elemname=#name;																		\
		p->subtype="string";																			\
		_ELEM_LINK;																						\
	}

#define GELEM_STRINGARRAY(name)		GELEM_STRINGARRAY_INIT(name,"")


//type name; (sample: CMyObj v;)
#define GELEM_OBJ(type,name)																\
	{																												\
		GElem_ObjSingle<type>*p=new GElem_ObjSingle<type>;			\
		p->off=(DWORD)((BYTE*)&ptr->name-(BYTE*)ptr);						\
		p->sz=sizeof(ptr->name);																	\
		p->elemname=#name;																		\
		p->subtype=#type;																			\
		_ELEM_LINK;																						\
	}

//type name; (sample: CMyObj v;)
//CMyObj��Ҫ����: Zero(),Clear(),Save(),Load(),Copy()
#define GELEM_OBJVAR(type,name)														\
	{																												\
		GElem_ObjVar<type>*p=new GElem_ObjVar<type>;					\
		p->off=(DWORD)((BYTE*)&ptr->name-(BYTE*)ptr);						\
		p->sz=sizeof(ptr->name);																	\
		p->elemname=#name;																		\
		p->subtype=#type;																			\
		_ELEM_LINK;																						\
	}

//std::vector<type> name; (sample: std::vector<CMyObj> v;)
#define GELEM_OBJVECTOR(type,name)													\
	{																												\
		GElem_ObjVector<type>*p=new GElem_ObjVector<type>;			\
		p->off=(DWORD)((BYTE*)&ptr->name-(BYTE*)ptr);						\
		p->sz=sizeof(ptr->name);																	\
		p->elemname=#name;																		\
		p->subtype=#type;																			\
		_ELEM_LINK;																						\
	}

//type name[const]; (sample: CMyObj v[100];)
#define GELEM_OBJARRAY(type,name)													\
	{																												\
		GElem_ObjArray<type>*p=new GElem_ObjArray<type>;				\
		p->off=(DWORD)((BYTE*)ptr->name-(BYTE*)ptr);							\
		p->sz=sizeof(ptr->name);																	\
		p->szArray=sizeof(ptr->name)/sizeof(ptr->name[0]);							\
		p->elemname=#name;																		\
		p->subtype=#type;																			\
		_ELEM_LINK;																						\
	}



#define GELEM_VERSION(v)																		\
	curelem->ver=v;

#define GELEM_UID(v)																				\
	if (VerifyElemUID(ELEM_UID_START+(v)))											\
		curelem->uid=ELEM_UID_START+(v);

#define GELEM_LEGACY_CONSTRAINT(contraint)									\
	curelem->contraintsLegacy.insert(contraint)

//name/desc is const char * string,
//vt0 is a GVarType,sem0 is a GSem
#define GELEM_EDITVAR(nm0,vt0,sem0,desc0)										\
	curelem->name=nm0;																			\
	curelem->desc=desc0;																			\
	curelem->bEditable=TRUE;																	\
	curelem->vt=vt0;																					\
	curelem->sem=sem0;

#define GELEM_EDITOBJ(name0,desc0)													\
	curelem->bEditable=TRUE;																	\
	curelem->name=name0;																		\
	curelem->desc=desc0;																			\
	curelem->vt=GVT_None;																		\
	curelem->sem=GSem_Unknown;

#define GELEM_EDITOBJ_EX(name0,desc0,sem0)									\
	curelem->bEditable=TRUE;																	\
	curelem->name=name0;																		\
	curelem->desc=desc0;																			\
	curelem->vt=GVT_None;																		\
	curelem->sem=sem0;

#define GDATA_DEFINE(name,data)														\
	datamp[std::string(name)]=(data);


//���ǿ�����Save һ��GObj��ʱ��,ֻ������Щ��ȱʡֵ��ͬ��elem,
//��������Ϊ,
//ÿ��elemҪʵ��һ��Equal(..)����
//ÿ��GObjBaseҪʵ��һ��void *GetDef()����,�����õ�һ��ȱʡֵ
//��SaveGObj()��,�������ȵ���GetDef()�õ����ȱʡֵ,Ȼ��������GObjBase��Save(..)����,Save(..)������Saveÿ��elem֮ǰ
//����elem��Equal(..)�ж����elem�ĵ�ǰֵ��ȱʡֵ�Ƿ�һ��,���һ���Ļ�,����Բ��������elem
//��LoadGObj()��,�������ȵ���GetDef()�õ�һ��ȱʡֵ,Ȼ����Copy������Ҫ�����GObjBase,Ȼ���ٵ������GObjBase��Load(..)����
