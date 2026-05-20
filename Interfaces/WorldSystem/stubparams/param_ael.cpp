// ***************************************************************
//  param_sys   version:  1.0   ? date: 06/21/2008
//  -------------------------------------------------------------
//  author:		ixnehc
//  -------------------------------------------------------------
//  Copyright (C) 2008 - All Rights Reserved
// ***************************************************************
//  Purpose: system data used for param
// ***************************************************************
#include "stdh.h"

#include "param_ael.h"

#include "log/LogDump.h"



//////////////////////////////////////////////////////////////////////////
//NodeAEL


void NodeAEL::Clear()
{
	switch(type)
	{
	case 0:
		SAFE_RELEASE(ast);
		break;
	case 1:
		SAFE_RELEASE(entity);
		break;
	case 2:
		SAFE_RELEASE(lo);
		break;
	}

	Zero();
}

const char *NodeAEL::GetDebugName()
{
	if (IsEmpty())
		return "";
	switch(type)
	{
		case 0:
			return ast->GetClass()->GetName();
		case 1:
			return entity->GetProto()->GetFilePath();
		case 2:
			return "[LuaScript]";
	}
	return "";
}



BOOL NodeAEL::SetProp(const char *name,GProperty *prop)
{
	void *owner;
	GStubBase *stb=FindStub(name,owner);

	if (!stb)
	{
		const char *nm=GetDebugName();

		LOG_DUMP_2P("Ael",Log_Error,"无法找到名为\"%s\"的Stub(%s)",name,nm);

		return FALSE;
	}

	return stb->SetProp(owner,prop);
}

GProperty *NodeAEL::GetProp(const char *name)
{
	void *owner;
	GStubBase *stb=FindStub(name,owner);
	if (!stb)
		return NULL;
	return stb->GetProp(owner);
}
GProperty *NodeAEL::Call(const char *name,GProperty *prop)
{
	void *owner;
	GStubBase *stb=FindStub(name,owner);
	if (!stb)
		return NULL;
	return stb->Call(owner,prop);
}

void NodeAEL::CopyFrom(NodeAEL &src)
{
	switch(type)
	{
		case 0:
			SAFE_RELEASE(ast);
			break;
		case 1:
			SAFE_RELEASE(entity);
			break;
		case 2:
			SAFE_RELEASE(lo);
			break;
	}
	type=src.type;
	switch(type)
	{
		case 0:
			SAFE_REPLACE(ast,src.ast);
			break;
		case 1:
			SAFE_REPLACE(entity,src.entity);
			break;
		case 2:
			SAFE_REPLACE(lo,src.lo);
			break;
	}
}

BOOL NodeAEL::Equal(NodeAEL &other)
{
	if (type!=other.type)
		return FALSE;
	if (ptr!=other.ptr)
		return FALSE;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//PropAEL


GProperty *PropAEL::Clone()
{
	PropAEL *p=Class_New2(PropAEL);
	p->v.CopyFrom(v);

	return p;
}

BOOL PropAEL::Equals(GProperty *other)
{
	if (!other->CheckClassName("PropAEL"))
		return FALSE;
	return v.Equal(((PropAEL*)other)->v);
}


void PropAEL::DeleteThis()
{
	v.Clear();
	Class_Delete(this);
}
