/********************************************************************
	created:	24:1:2010   22:35
	file path:	d:\IxEngine\Interfaces\WorldSystem
	author:		cxi
	
	purpose:	LuaMachine interfaces
*********************************************************************/

#pragma once

#include "gds/GDefines.h"

enum GVarTypeEx
{
	GVTEx_Start=GVT_Max,

	GVTEx_AEL,
	GVTEx_ProtoNode,
	GVTEx_CppTbl,
	GVTEx_Sheet,
	GVTEx_SheetRow,

	GVTEx_AvtrStates,
	GVTEx_AnimNode,

	GVTEx_Input,

	GVTEx_PhysEvent,
	GVTEx_AnimEvent,

	GVTEx_CameraLink_obsolete,

	GVTEx_Mano,
	GVTEx_AvatarLink_obsolete,

	GVTEx_GameStart=GVT_Max+64,


	GVTEx_Max=GVTEx_GameStart+128,
};



class ILuaMachine;
struct lua_State;
struct GProperty;
//Lud 代表Lua User Data
class CLud
{
public:
	virtual BOOL Index(lua_State *L,int idxUD,int idxKey){return FALSE;}
	virtual BOOL NewIndex(lua_State *L)	{		return FALSE;	}//栈上1为user data,2为key,3为value
	virtual BOOL GetKey(GProperty *prop,lua_State*L,DWORD idx)	{		return FALSE;	}
						//得到第idx个key,得到的key压在堆栈的顶上,如果idx超出了范围,返回FALSE,并且不要压任何东西
						//注意,key必须是一个字符串

	virtual BOOL Add(lua_State *L)	{		return FALSE;	}
	virtual BOOL Sub(lua_State *L)	{		return FALSE;	}
	virtual BOOL Mul(lua_State *L)	{		return FALSE;	}
	virtual BOOL Div(lua_State *L)	{		return FALSE;	}
	virtual BOOL Unm(lua_State *L)	{		return FALSE;	}

	//注意: Index(..)/NewIndex(..)/NextKey(..)如果返回失败,不要往堆栈上放任何东西

	virtual GVarType GetGVT()=0;
	virtual const char *GetName()=0;//这个名字为user data在lua脚本里看到的对象名称
	virtual const char *GetHelp()	{		return "";	}

	virtual BOOL ToString(GProperty *prop,std::string &str)	{		return FALSE;	}

	static void Register(CLud *lud)
	{
		Luds()[lud->GetGVT()]=lud;
	}

	static CLud ** Luds()
	{
		static CLud * t[256];//big enough
		static int bInit=FALSE;
		if (!bInit)
		{
			memset(t,0,sizeof(t));
			bInit=TRUE;
		}
		return t;
	}



};


#define DECLARE_LUD(gvt,classname)															\
virtual GVarType GetGVT(){	return (GVarType)gvt;}										\
static classname*_instance;


#define IMPLEMENT_LUD(classname) 															\
classname*Register##classname()																	\
{																															\
	static classname t;																							\
	CLud::Register((CLud*)&t);																			\
	return &t;																										\
}																															\
classname *classname::_instance=Register##classname();

#define BEGIN_LUDFUNC(self,key)																	\
lua_CFunction __funcFound=NULL;																	\
void *__self=(void*)self;																						\
{																															\
	static std::unordered_map<std::string,lua_CFunction>__table;							\
	static BOOL __bInit=FALSE;																			\
	const char *__key=key;																					\
	if (!__bInit)																										\
	{																														\
		std::unordered_map<std::string,lua_CFunction>*__p=&__table;						\
		__bInit=TRUE;

#define LUD_FUNC(name,func)	(*__p)[std::string(name)]=func;


#define END_LUDFUNC()																					\
	}																														\
	std::unordered_map<std::string,lua_CFunction>::iterator it=								\
													__table.find(std::string(__key));						\
	if (it==__table.end())																						\
		__funcFound=NULL;																					\
	else																													\
		__funcFound=(*it).second;																			\
}																															\
if (__funcFound)																									\
{																															\
	lua_pushlightuserdata(L,__self);																		\
	lua_pushcclosure(L,__funcFound,1);																\
	return TRUE;																									\
}


#define BEGIN_LUD_HELP()		virtual const char *GetHelp()	{		return 
#define END_LUD_HELP()	;}

#define BEGIN_LUD_KEY()																					\
virtual BOOL GetKey(GProperty *prop,lua_State*L,DWORD idx)					\
{																															\
	static const char *keys[]=																				\
	{

#define END_LUD_KEY()																					\
	};																														\
	if (idx>=ARRAY_SIZE(keys))																			\
		return FALSE;																								\
	lua_pushstring(L,keys[idx]);																			\
	return TRUE;																									\
}




#define LOAD_LUDS(lm)											\
CLud **luds=CLud::Luds();											\
for (int i=0;i<GVTEx_Max;i++)									\
{																					\
	if (luds[i])																	\
		(lm)->RegisterLud(luds[i]);								\
}





struct StbParams;
struct GProperty;
class CEnums;
class ILuaMachine
{
public:
	virtual void GarbageCollect()=0;
	virtual DWORD GetMemoryCount()=0;

	virtual BOOL BeginAddLib(const char *lib)=0;
	virtual BOOL AddLibFunc(const char *funcname,void *func)=0;
	virtual BOOL AddLibFuncHelp(const char *funcname,const char *help)=0;
	virtual void EndAddLib()=0;
	virtual void ClearLibFunc(const char *lib)=0;
	virtual BOOL AddTypeHelp(const char *typenm,const char *help)=0;
	virtual BOOL AddEnumsHelp(CEnums *enms)=0;

	virtual int PushParam(GProperty *prop)=0;
	virtual void ParseParam(StbParams *params,DWORD iStart=1)=0;

	virtual void RegisterLud(CLud *lud)=0;
	virtual CLud *Find(int gvt)=0;

	virtual DWORD GetLibCount()=0;
	virtual const char *GetLibName(DWORD iLib)=0;
	virtual const char *GetLibFuncs(DWORD iLib)=0;//以","分隔
	virtual const char *GetLibFuncHelp(const char *funcname,int iLib)=0;//如果iLib为-1,在所有的lib里找

	virtual const char *GetTypes()=0;
	virtual const char *GetTypeHelp(const char *typenm)=0;


};

