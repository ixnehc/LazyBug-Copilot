#pragma once

#include <vector>

struct GStubBase;
class CClass;
struct GStackEntry
{
	enum Type
	{
		General,
		Stub,
	};
	Type type;
	const char *name;

	void *owner;
	GStubBase *stb;
	CClass *clss;
};

struct GStack
{
	void Clear()
	{
		entries.clear();
	}
	std::vector<GStackEntry> entries;

	static GStack *&stack()
	{
		static GStack *v=NULL;
		return v;
	}
};

#define GStackPush_General(__name,__owner,__clss)											\
{																																\
	GStack *stk=GStack::stack();																				\
	if (stk)																													\
	{																															\
		stk->entries.resize(stk->entries.size()+1);													\
		GStackEntry *entry=&stk->entries[stk->entries.size()-1];							\
		entry->type=GStackEntry::General;																\
		entry->name=(__name);																				\
		entry->stb=NULL;																							\
		entry->owner=(void*)(__owner);																	\
		entry->clss=(__clss);																						\
	}																															\
}


#define GStackPush_Stub(__name,__stb,__owner)												\
{																																\
	GStack *stk=GStack::stack();																				\
	if (stk)																													\
	{																															\
		stk->entries.resize(stk->entries.size()+1);													\
		GStackEntry *entry=&stk->entries[stk->entries.size()-1];							\
		entry->type=GStackEntry::Stub;																	\
		entry->name=__name;																					\
		entry->stb=(GStubBase*)(__stb);																	\
		entry->owner=(void*)(__owner);																	\
		entry->clss=NULL;																							\
	}																															\
}

#define GStackPop()																								\
{																																\
	GStack *stk=GStack::stack();																				\
	if (stk)																													\
		stk->entries.pop_back();																				\
}
