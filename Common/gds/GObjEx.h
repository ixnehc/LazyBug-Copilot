#pragma once

//一些扩展的GObj相关的内容

#include "GObj.h"
#include "../class/class.h"

struct GElem_DynObjPtrBase:public GElemBase
{
	CClass *init;
	std::unordered_map<std::string,CClass *>classes;
	std::unordered_map<std::string,std::string>names;
};

template<typename T>
struct GElem_DynObjPtr:public GElem_DynObjPtrBase
{
	enum DeltaOp
	{
		Delta_Clear,//空指针
		Delta_Overwrite,//另一个不同的class
		Delta_Patch,//同样的class
	};
	virtual int GetTypeID()	{		return 10;	}
	virtual BOOL CheckCompatible(GElemBase *elemOther)
	{
		if (!CheckCompatible_General(elemOther))
			return FALSE;
		GElem_DynObjPtrBase *other=(GElem_DynObjPtrBase *)elemOther;
		if (TRUE)
		{
			std::unordered_map<std::string,CClass *>::iterator it=classes.begin(),it2=other->classes.begin();
			if (classes.size()!=other->classes.size())
				return FALSE;
			while(it!=classes.end())
			{
				if (!((*it).first==(*it2).first))
					return FALSE;
				if (!((*it).second==(*it2).second))
					return FALSE;
				it++;
				it2++;
			}
		}
		if (TRUE)
		{
			std::unordered_map<std::string,std::string>::iterator it=names.begin(),it2=other->names.begin();
			if (names.size()!=other->names.size())
				return FALSE;
			while(it!=names.end())
			{
				if (!((*it).first==(*it2).first))
					return FALSE;
				if (!((*it).second==(*it2).second))
					return FALSE;
				it++;
				it2++;
			}
		}

		return TRUE;
	}

	virtual void Zero(void *objOwner,BOOL bIntuitive)
	{
		*(T**)_Ptr(objOwner)=NULL;
 		if (init)
 			*(T**)_Ptr(objOwner)=(T*)init->New();
	}
	virtual void Clear(void *objOwner,BOOL bIntuitive)
	{
		T**p=(T**)_Ptr(objOwner);
		if (*p)
		{
			Class_Delete(*p);
			*p=NULL;
		}
	}

	virtual void Copy(void *objDest,void *objSrc)
	{
		T**q=(T**)_Ptr(objSrc);
		if (!(*q))
		{
			Clear(objDest,FALSE);
			if (init)
				*(T**)_Ptr(objDest)=(T*)init->New();
			return;
		}

		T**p=(T**)_Ptr(objDest);
		if (*p)
			Class_Delete(*p);
		*p=(T*)(*q)->GetClass()->New();

		(*p)->GetGObj()->Copy((*q)->GetGObj());
	}

	virtual void Save(void *objOwner,CDataPacket &dp,BOOL bNewFmt)
	{
		T**p=(T**)_Ptr(objOwner);
		if (*p)
		{
			std::unordered_map<std::string,CClass *>::iterator it;
			for (it=classes.begin();it!=classes.end();it++)
			{
				if ((*it).second==(*p)->GetClass())
					break;
			}

			if (it!=classes.end())
			{
				dp.Data_WriteStringSH((*it).first.c_str());
				DP_PreSafeSave(dp);
				(*p)->GetGObj()->Save(dp,bNewFmt);
				DP_PostSafeSave();
			}
			else
			{
				assert(FALSE);
				dp.Data_WriteStringSH("");
			}
		}
		else
			dp.Data_WriteStringSH("");
	}

	virtual void SaveDelta(void *objOwner,void *objOwnerRef,CDataPacket &dp)
	{
		T**p=(T**)_Ptr(objOwner);
		T**q=(T**)_Ptr(objOwnerRef);

		if (*p)
		{
			if ((*p)->GetClass()->CheckName((*q)->GetClass()->GetName()))
			{
				//同一个class
				dp.Data_NextWord()=Delta_Patch;

				dp.Data_WriteStringSH((*p)->GetClass()->GetName());
				DP_PreSafeSave(dp);
				(*p)->GetGObj()->SaveDelta(dp,(*q)->GetGObj());
				DP_PostSafeSave();
			}
			else
			{
				dp.Data_NextWord()=Delta_Overwrite;

				dp.Data_WriteStringSH((*p)->GetClass()->GetName());
				DP_PreSafeSave(dp);
				(*p)->GetGObj()->Save(dp,TRUE);
				DP_PostSafeSave();
			}
		}
		else
			dp.Data_NextWord()=Delta_Clear;
	}

	virtual BOOL Load(void *objOwner,CDataPacket &dp,BOOL bNewFmt)
	{
		Clear(objOwner,FALSE);
		T**p=(T**)_Ptr(objOwner);

		std::string name;
		if (!bNewFmt)
			dp.Data_ReadString(name);
		else
			dp.Data_ReadStringSH(name);
		if (!name.empty())
		{
			DP_PreSafeLoad(dp);
			std::unordered_map<std::string,CClass *>::iterator it=classes.find(name);
			if (it!=classes.end())
			{
				*p=(T*)((*it).second)->New();
				(*p)->GetGObj()->Load(dp,bNewFmt,NULL);
			}
			else
			{//找不到,使用缺省的类
				*p=(T*)(init)->New();
			}
			DP_PostSafeLoad();
		}
		return TRUE;
	}

	virtual void LoadDelta(void *objOwner,CDataPacket &dp,std::vector<void*>*ptrsDelta)
	{
		T**p=(T**)_Ptr(objOwner);

		DeltaOp op=(DeltaOp)dp.Data_NextWord();

		std::string name;
		switch(op)
		{
			case Delta_Overwrite:
			{
				dp.Data_ReadStringSH(name);

				Safe_Class_Delete(*p);

				DP_PreSafeLoad(dp);
				std::unordered_map<std::string,CClass *>::iterator it=classes.find(name);
				if (it!=classes.end())
				{
					*p=(T*)((*it).second)->New();
					(*p)->GetGObj()->Load(dp,TRUE,NULL);
				}
				else
				{//找不到,使用缺省的类
					*p=(T*)(init)->New();
				}
				DP_PostSafeLoad();

				if (ptrsDelta)
				{
					ptrsDelta->push_back(_Ptr(objOwner));
				}
				break;
			}
			case Delta_Patch:
			{
				dp.Data_ReadStringSH(name);

				DP_PreSafeLoad(dp);
				if (*p)
				{
					if ((*p)->GetClass()->CheckName(name.c_str()))
						(*p)->GetGObj()->LoadDelta(dp,ptrsDelta);
				}

				DP_PostSafeLoad();

				break;
			}
			case Delta_Clear:
			{
				Safe_Class_Delete(*p);
				break;
			}
		}
	}


	virtual BOOL GetObjClass(void *objOwner,CClass**clss)
	{
		if (clss)
			*clss=NULL;
		if (objOwner&&clss)
		{
			T**p=(T**)_Ptr(objOwner);
			if (*p)
				*clss=(*p)->GetClass();
		}
		return TRUE;
	}


	virtual BOOL GetObj(void *objOwner,GObjBase **obj)
	{		
		if (obj)
			*obj=NULL;
		if (objOwner&&obj)
		{
			T**p=(T**)_Ptr(objOwner);
			if (*p)
				*obj=(*p)->GetGObj();
		}
		return TRUE;	
	}
	virtual BOOL GetVar(void *objOwner,void **var)
	{		
		if (var)
			*var=NULL;
		if (objOwner&&var)
			*var=_Ptr(objOwner);
		return TRUE;
	}

};


#define GELEM_DYNOBJPTR(type,name0,initclss,editname,editdesc)											\
	{																												\
		GElem_DynObjPtr<type>*p=new GElem_DynObjPtr<type>;										\
		p->off=(DWORD)((BYTE*)&ptr->name0-(BYTE*)ptr);						\
		p->sz=sizeof(ptr->name0);																	\
		p->elemname=#name0;																		\
		p->init=Class_Ptr2(initclss);																\
		p->sem=GSem(GSem_Unknown,"DynObjPtr");									\
		p->name=editname;																			\
		p->desc=editdesc;																				\
		p->bEditable=TRUE;																			\
		_ELEM_LINK;																						\
	}

#define GELEM_DYNOBJPTR_CLASS(name,clss)																					\
	((GElem_DynObjPtrBase*)curelem)->classes[std::string(#clss)]=Class_Ptr2(clss);						\
	((GElem_DynObjPtrBase*)curelem)->names[std::string(#clss)]=std::string(name);


