
#pragma once

#include "class/class.h"
#include "gds/GObj.h"
#include "gds/GStub.h"

struct PropRefTarget
{
	INTERFACE_REFCOUNT;
	virtual CClass *GetClass()=0;
	virtual GObjBase *GetGObj()=0;
};

struct PropRef:public GProperty
{
	DEFINE_CLASS(PropRef);
	PropRef()
	{
		clss=NULL;
		stuff=NULL;
	}

	virtual BOOL IsRef(){return TRUE;}//是不是PropRef

	virtual void DeleteThis()
	{
		SAFE_RELEASE(stuff);
		Class_Delete(this);
	}
	//-------------------------------------------------------------------------------------------------------
	// Trick:这个Property的Clone比较古怪,是因为以下原因:
	// 1. 这个Property内部包含了一些带引用计数的指针
	// 2. Property会被保留在Stub的静态成员变量里作为缺省值,我们不能在这个缺省值里保留指针,
	//	  因为静态成员变量会很晚才被析构(晚于mempool的析构函数),这样会造成mempool报内存泄漏的错)
	// 3. 所以我们在为Property设缺省值时直接设到一个数值变量里(def)
	// 4. 当这个Property被clone时,我们需要new一个clss 的指针
	// 5. 我们可以这么干是基于,每个保存在stb缺省值中的Property是不会被直接使用的,它肯定会被Clone()一下再使用
	//-------------------------------------------------------------------------------------------------------
	virtual GProperty* Clone()
	{
		PropRef* ret = Class_New2(PropRef);
		ret->clss=clss;
		if (!stuff)
			ret->stuff=(PropRefTarget*)clss->New();
		else
			ret->stuff=stuff;
		SAFE_ADDREF(ret->stuff);
		return ret;
	}

	virtual BOOL Equals(GProperty *prop)
	{
		if ((GetGObj()!=0)!=(prop->GetGObj()!=0))
			return FALSE;
		if (!GetGObj())
			return TRUE;
		return GetGObj()->Equals(prop->GetGObj());
	}

	GProperty *CloneDeep()
	{
		PropRef* ret = Class_New2(PropRef);
		ret->clss=clss;
		ret->stuff=(PropRefTarget*)clss->New();
		if (stuff)
			ret->stuff->GetGObj()->Copy(stuff->GetGObj());
		SAFE_ADDREF(ret->stuff);
		return ret;
	}

	virtual void Save( CDataPacket &dp )
	{
		if (!stuff)
			dp.Data_NextWord() = 0;
		else
		{
			dp.Data_NextWord() = 1;
			SaveGObj(dp,stuff->GetGObj());
		}
	}

	virtual BOOL Load( CDataPacket &dp, BOOL *bRepaired )
	{
		assert(clss);
		if (!clss)
			return FALSE;
		WORD flag=dp.Data_NextWord();
		if (flag==0)
		{
			stuff= (PropRefTarget *)clss->New();
			SAFE_ADDREF( stuff);
		}
		else
		{
			if (flag==1)
			{
				if ( NULL == stuff)
				{
					stuff= (PropRefTarget *)clss->New();
					SAFE_ADDREF( stuff);
				}
				if (FALSE==LoadGObj(dp,stuff->GetGObj(),bRepaired))
					return FALSE;
			}
			else
			{
				return FALSE;//格式不对,不是这个对象的数据
			}
		}
		return TRUE;
	}

	virtual GObjBase *GetGObj()
	{
		if (stuff)
			return stuff->GetGObj();
		return NULL;
	}

	CClass *clss;
	PropRefTarget*stuff;
};

#define GPropRefDefine(name,refclss)										\
	GPropDefine(name,PropRef)													\
	GPropDefault(PropRef)->clss=Class_Ptr2(refclss);

