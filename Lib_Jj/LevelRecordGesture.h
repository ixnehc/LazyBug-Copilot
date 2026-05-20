#pragma once

#include "class/class.h"

#include "anim/animdefines.h"

#include "gds/GObj.h"
#include "gds/GObjEx.h"


#include "records/records.h"

#include "LevelGesture.h"



class CClass;
struct GObjBase;
class CLevelGesture;


#define GELEM_DYNOBJPTR_GESTUREPARAM(type,name0,initclss,editname,editdesc)											\
	{																												\
		GElem_DynObjPtr<type>*p=new GElem_DynObjPtr<type>;										\
		p->off=(DWORD)((BYTE*)&ptr->name0-(BYTE*)ptr);						\
		p->sz=sizeof(ptr->name0);																	\
		p->elemname=#name0;																		\
		extern CClass*GetClass_##initclss();														\
		p->init=GetClass_##initclss();																\
		p->sem=GSem(GSem_Unknown,"DynObjPtr");									\
		p->name=editname;																			\
		p->desc=editdesc;																				\
		p->bEditable=TRUE;																			\
		_ELEM_LINK;																						\
	}

#define GELEM_DYNOBJPTR_CLASS_GESTUREPARAM(name,clss)																					\
	extern CClass*GetClass_##clss();														\
	((GElem_DynObjPtrBase*)curelem)->classes[std::string(#clss)]=GetClass_##clss();						\
	((GElem_DynObjPtrBase*)curelem)->names[std::string(#clss)]=std::string(name);



struct LevelGestureParam;
struct LevelRecordGesture:public CRecord
{
	DEFINE_CLASS(LevelRecordGesture);

	std::string Name;

	unsigned __int64 Effect;

	std::vector<LevelGestureEvent> Events;

	LevelGestureParam *Param;


	template <typename T>
	T *GetParam()
	{
		if (Param->GetClass()->IsSameWith(Class_Ptr2(T)))
			return (T*)Param;
		return NULL;
	}

	BEGIN_GOBJ_PURE(LevelRecordGesture,1);

		GELEM_STRING_INIT(Name,"");
			GELEM_EDITVAR("名称",GVT_String,GSem_Name,"Gesture的名称");
			GELEM_VAR_INIT(unsigned __int64,Effect,0);
				GELEM_EDITVAR("表现效果",GVT_Bx8,GSem_ProtoPath,"配合Gesture路径的动作表现效果");
			GELEM_OBJVECTOR(LevelGestureEvent,Events);
				GELEM_EDITOBJ("事件","事件");

		GELEM_DYNOBJPTR_GESTUREPARAM(LevelGestureParam,Param,GestureParam_FlyUp, "Gesture类型", "选择不同的Gesture" );
			GELEM_DYNOBJPTR_CLASS_GESTUREPARAM( "01.垂直起飞", GestureParam_FlyUp);
			GELEM_DYNOBJPTR_CLASS_GESTUREPARAM( "02.降落", GestureParam_FlyDown);
			GELEM_DYNOBJPTR_CLASS_GESTUREPARAM( "03.飞行冲刺", LevelGestureParam_FlyThrust);
			GELEM_DYNOBJPTR_CLASS_GESTUREPARAM( "04.跳跃闪避", LevelGestureParam_Jump);
			//XXXXX:More LevelGesture

	END_GOBJ();


};
