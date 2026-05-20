#pragma once

#include "class/class.h"

#include "anim/animdefines.h"

#include "gds/GObj.h"
#include "gds/GObjEx.h"


#include "records/records.h"

#include "LevelAI.h"



class CClass;
struct GObjBase;
class CLevelAI;


#define GELEM_DYNOBJPTR_AIPARAM(type,name0,initclss,editname,editdesc)											\
	{																												\
		GElem_DynObjPtr<type>*p=new GElem_DynObjPtr<type>;										\
		p->off=(DWORD)((BYTE*)&ptr->name0-(BYTE*)ptr);						\
		p->elemname=#name0;																		\
		extern CClass*GetClass_##initclss();														\
		p->init=GetClass_##initclss();																\
		p->sem=GSem(GSem_Unknown,"DynObjPtr");									\
		p->name=editname;																			\
		p->desc=editdesc;																				\
		p->bEditable=TRUE;																			\
		_ELEM_LINK;																						\
	}

#define GELEM_DYNOBJPTR_CLASS_AIPARAM(name,clss)																					\
	extern CClass*GetClass_##clss();														\
	((GElem_DynObjPtrBase*)curelem)->classes[std::string(#clss)]=GetClass_##clss();						\
	((GElem_DynObjPtrBase*)curelem)->names[std::string(#clss)]=std::string(name);



struct LevelAIParam;
struct LevelRecordAI:public CRecord
{
	DEFINE_CLASS(LevelRecordAI);

	std::string Name;

	LevelAIParam *Param;

	template <typename T>
	T *GetParam()
	{
		if (Param->GetClass()->IsSameWith(Class_Ptr2(T)))
			return (T*)Param;
		return NULL;
	}

	BEGIN_GOBJ_PURE(LevelRecordAI,1);

		GELEM_STRING_INIT(Name,"");
			GELEM_EDITVAR("名称",GVT_String,GSem_Name,"AI的名称");
		GELEM_DYNOBJPTR_AIPARAM(LevelAIParam,Param,AIParam_RockScorpion, "AI", "选择不同的AI" );
			GELEM_DYNOBJPTR_CLASS_AIPARAM( "01.岩蝎", AIParam_RockScorpion);
			GELEM_DYNOBJPTR_CLASS_AIPARAM( "02.地精", AIParam_Goblin);
			GELEM_DYNOBJPTR_CLASS_AIPARAM( "03.巨蝠兽", AIParam_FlyingRat);
			//XXXXX:More LevelAI

	END_GOBJ();


};
