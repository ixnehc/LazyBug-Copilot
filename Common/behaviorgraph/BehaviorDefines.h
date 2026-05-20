#pragma once

typedef unsigned int BgnID;
#define BgnID_Invalid (0)


enum BehaviorMemFlag
{
	BehaviorMemFlag_None=0,
	BehaviorMemFlag_Persist=1,
	BehaviorMemFlag_Sync=2,

	BehaviorMemFlag_ForceDword=0xffffffff,
};

enum BehaviorMemType
{
	BehaviorMemType_None,
	BehaviorMemType_Bit,
	BehaviorMemType_Integer,
	BehaviorMemType_Float,
	BehaviorMemType_StringID,
	BehaviorMemType_SkillRecord,
	BehaviorMemType_BuffRecord,
	BehaviorMemType_ItemRecord,
	BehaviorMemType_UnitRecord,
	BehaviorMemType_Pos,
	BehaviorMemType_ObjID,
	BehaviorMemType_GUID,
	BehaviorMemType_Obj,
	BehaviorMemType_ResourceRecord,

	//XXXXX:more BehaviorMemType

	BehaviorMemType_ForceDword=0xffffffff,
};

inline const char *GetBehaviorMemTypeDesc(BehaviorMemType tp)
{
	switch (tp)
	{
		case BehaviorMemType_Bit:
			return "布尔型";
		case BehaviorMemType_Integer:
			return "整数";
		case BehaviorMemType_Float:
			return "浮点数";
		case BehaviorMemType_StringID:
			return "字符串ID";
		case BehaviorMemType_SkillRecord:
			return "技能表格项";
		case BehaviorMemType_BuffRecord:
			return "Buff表格项";
		case BehaviorMemType_ItemRecord:
			return "道具表格项";
		case BehaviorMemType_UnitRecord:
			return "单位表格项";
		case BehaviorMemType_ResourceRecord:
			return "Resource表格项";
		case BehaviorMemType_Pos:
			return "位置";
		case BehaviorMemType_ObjID:
			return "ObjID";
		case BehaviorMemType_GUID:
			return "GUID";
		default:
			return "n/a";
		//XXXXX:more BehaviorMemType
	}
}

inline BOOL BehaviorMemType_IsNumber(BehaviorMemType tp)
{
	return tp==BehaviorMemType_Bit||tp==BehaviorMemType_Integer||tp==BehaviorMemType_Float;
	//XXXXX:more BehaviorMemType
}

#define GELEM_BVR()																		\
	{																												\
		GElem_VarSingle<StringID>*p=new GElem_VarSingle<StringID>;			\
		p->off=curelem->off+curelem->sz;						\
		p->sz=sizeof(StringID);																	\
		p->elemname="__bvr_";																		\
		p->elemname+=curelem->elemname;										\
		p->subtype="StringID";																			\
		p->init=StringID_BhvValInvalidRef;													\
		p->ver=curelem->ver;																		\
		p->next=NULL;															\
		*lastelem=p;																							\
		lastelem=&p->next;																				\
	}

#define _BVR(__nm__) __bvr_##__nm__

#define DEFINE_BVR(__type__,__nm__)												\
	__type__ __nm__;												\
	__pragma (pack(push))												\
	__pragma (pack(1))												\
	StringID __bvr_##__nm__;												\
	__pragma (pack(pop))											\
	;



#define BVR_ARG(__nm__)								\
__nm__,__bvr_##__nm__



struct GObjBase;
class CClass;
class CBehaviorMemObj
{
public:
	virtual CClass *GetClass()=0;
	virtual GObjBase *GetGObj()=0;
};
