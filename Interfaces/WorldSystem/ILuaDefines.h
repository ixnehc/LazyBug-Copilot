#pragma once


//////////////////////////////////////////////////////////////////////////
//一些用于lua 调用的c函数的类型检测相关的宏

//Extra position,32 is a LUAI_EXTRASPACE value
class ILuaMachine;
class IAssetSystem;
class IDebugger;
#define LuaMachineFromL(L) (*((ILuaMachine **)(((BYTE*)L)-32	+0*sizeof(void*))))
#define AssetSystemFromL(L) (*((IAssetSystem**)(((BYTE*)L)-32	+1*sizeof(void*))))
#define DebuggerFromL(L) (*((IDebugger**)(((BYTE*)L)			-32	+2*sizeof(void *)))) 
#define EntitySystemFromL(L) (*((IEntitySystem**)(((BYTE*)L)-32	+3*sizeof(void *))))



#define LArg_Begin() BOOL __bOk=TRUE;BOOL __bSilent=FALSE;BOOL __bAllOk=TRUE;extern void LuaDebugOutput(lua_State *L,DebugOutput::Type type,const char *content,...);

#define LArg_EnableWarning(bEnable) __bSilent=!bEnable;

#define LArg_Ok() (__bOk)
#define LArg_AllOk() (__bAllOk)

#define LArg_UD_NoCheck(arg,classname,idx) classname *arg=*(classname**)lua_touserdata(L,idx);

#define LArg_UD(arg,classname,idx)																																\
classname*arg=NULL;																																						\
{																																															\
	__bOk=FALSE;																																									\
	if (lua_isuserdata(L,idx))																																					\
	{																																														\
		GProperty**pp = (GProperty**)lua_touserdata(L, idx);																							\
		if ((*pp)->CheckClassName(#classname))																												\
		{																																													\
			arg=(classname*)(*pp);																																			\
			__bOk=TRUE;																																							\
		}																																													\
	}																																														\
	if (!__bOk)																																										\
	{																																														\
		__bAllOk=FALSE;																																						\
		if (!__bSilent)																																								\
		{																																													\
			classname t;																																							\
			ILuaMachine *lm=LuaMachineFromL(L);																												\
			CLud *lud=lm->Find(t.GetGVT());																															\
			const char *ss=t.GetClass()->GetName();																											\
			if (lud)																																										\
				ss=lud->GetName();																																			\
			LuaDebugOutput(L,DebugOutput::Warning,"参数错误,第%d个参数必须为%s对象!",idx,ss);								\
		}																																													\
	}																																														\
}

#define LArg_IsGVT(gvt,idx)	lua_isuserdata(L,idx)? ((*(GProperty**)lua_touserdata(L,idx))->GetGVT()==gvt?TRUE: FALSE ) : FALSE
#define LArg_GVT(arg,idx)																																					\
GVarType arg=GVT_None;																																				\
{																																															\
	if (lua_isuserdata(L,idx))																																					\
	{																																														\
		GProperty**pp = (GProperty**)lua_touserdata(L, idx);																							\
		arg=(*pp)->GetGVT();																																				\
	}																																														\
}


#define LArg_number(type,arg,idx)																																	\
type arg=0;																																											\
{																																															\
	__bOk=TRUE;																																									\
	if (!lua_isnumber(L,idx))																																					\
	{																																														\
		if (!__bSilent)																																								\
			LuaDebugOutput(L,DebugOutput::Warning,"参数错误,第%d个参数必须为数值!",idx);										\
		__bOk=FALSE;																																								\
		__bAllOk=FALSE;																																						\
	}																																														\
	arg=(type)lua_tonumber(L,idx);																																	\
}																																															

#define LArg_number_Default(type,arg,idx,def)																												\
type arg=def;																																										\
{																																															\
	__bOk=TRUE;																																									\
	if ((!lua_isnumber(L,idx))&&(lua_type(L,idx)!=LUA_TNONE)&&(lua_type(L,idx)!=LUA_TNIL))									\
	{																																														\
		if (!__bSilent)																																								\
			LuaDebugOutput(L,DebugOutput::Warning,"参数错误,第%d个参数必须为数值或nil!",idx);								\
		__bOk=FALSE;																																								\
		__bAllOk=FALSE;																																						\
	}																																														\
	if (lua_isnumber(L,idx))																																					\
		arg=(type)lua_tonumber(L,idx);																																\
}


#define LArg_float(arg,idx) LArg_number(float,arg,idx)
#define LArg_float_Default(arg,idx,def) LArg_number_Default(float,arg,idx,def)

#define LArg_int(arg,idx) LArg_number(int,arg,idx)
#define LArg_int_Default(arg,idx,def) LArg_number_Default(int,arg,idx,def)

#define LArg_Dword(arg,idx) LArg_number(DWORD,arg,idx)
#define LArg_Dword_Default(arg,idx,def) LArg_number_Default(DWORD,arg,idx,def)


#define LArg_cstr(arg,idx)																																					\
const char *arg="";																																							\
{																																															\
	__bOk=TRUE;																																									\
	if (lua_type(L,idx)!=LUA_TSTRING)																																\
	{																																														\
		if (!__bSilent)																																								\
			LuaDebugOutput(L,DebugOutput::Warning,"参数错误,第%d个参数必须为字符串!",idx);									\
		__bOk=FALSE;																																								\
		__bAllOk=FALSE;																																						\
	}																																														\
	else																																													\
		arg=lua_tostring(L,idx);																																															\
}


#define LArg_Mat(arg,idx)																																					\
i_math::matrix43f *arg;																																						\
{																																															\
	LArg_UD(t,Prop_Fx12,idx);																																				\
	if (LArg_Ok())																																									\
		arg=&t->v;																																									\
	else																																													\
		arg=i_math::matrix43f::identity();																																\
}

#define LArg_vector3df(arg,idx)																																		\
i_math::vector3df arg;																																						\
{																																															\
	LArg_UD(t,Prop_Fx3,idx);																																				\
	if (LArg_Ok())																																									\
		arg=t->v;																																									\
}



#define LArg_enum(arg,ename,idx)																																	\
ename arg=(ename)-1;																																						\
{																																															\
	__bOk=TRUE;																																									\
	if (lua_type(L,idx)!=LUA_TSTRING)																																\
	{																																														\
		if (!__bSilent)																																								\
			LuaDebugOutput(L,DebugOutput::Warning,"参数错误,第%d个参数必须为字符串!",idx);									\
		__bOk=FALSE;																																								\
		__bAllOk=FALSE;																																						\
	}																																														\
	else																																													\
	{																																														\
		const char *str=lua_tostring(L,idx);																															\
		arg=Enums_FindValue(ename,str);																															\
		if (arg==-1)																																									\
		{																																													\
			if (!__bSilent)																																							\
				LuaDebugOutput(L,DebugOutput::Warning,"无效的%s枚举值(%s)",#ename,str);											\
			__bOk=FALSE;																																							\
			__bAllOk=FALSE;																																					\
		}																																													\
	}																																														\
}




#define LUv_UD_NoCheck(arg,classname,idx) classname *arg=(classname*)lua_touserdata(L,lua_upvalueindex(idx));

#define LUv_cstr_NoCheck(arg,idx) const char *arg=lua_tostring(L,lua_upvalueindex(idx));


#define LRet_nil()																				\
{																											\
	lua_pushnil(L);																				\
	return 1;																							\
}

#define LRet_bool(b)																			\
{																											\
	lua_pushboolean(L,b);																	\
	return 1;																							\
}


#define LRet_number(v)																	\
{																											\
	lua_pushnumber(L,(lua_Number)(v));											\
	return 1;																							\
}

#define LRet_str(str)																			\
{																											\
	lua_pushstring(L,(str));																	\
	return 1;																							\
}


#define LRet_vector3df(__v)															\
{																										\
	Prop_Fx3 t;																					\
	t.v=__v;																						\
	ILuaMachine *lm=LuaMachineFromL(L);									\
	return lm->PushParam(&t);														\
}

#define LRet_pos2di(__v)																\
{																										\
	Prop_Sx2 t;																					\
	t.v.set(__v.x,__v.y);																		\
	ILuaMachine *lm=LuaMachineFromL(L);									\
	return lm->PushParam(&t);														\
}

#define LRet_AnimNode(___an)														\
{																										\
	IAnimNode *__an=___an;															\
	if (!__an)																						\
		LRet_nil();																				\
	Prop_AnimNode t;																		\
	t.an=__an;																					\
	ILuaMachine *lm=LuaMachineFromL(L);									\
	return lm->PushParam(&t);														\
}