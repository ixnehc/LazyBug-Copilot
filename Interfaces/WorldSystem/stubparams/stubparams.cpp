// ***************************************************************
//  stubparams   version:  1.0   ? date: 06/21/2008
//  -------------------------------------------------------------
//  author:		ixnehc
//  -------------------------------------------------------------
//  Copyright (C) 2008 - All Rights Reserved
// ***************************************************************
//  Purpose:a property used to transfer param between stubs 
// ***************************************************************

#include "stdh.h"

#include <assert.h>

#include "class/class.h"
#include "gds/GObj.h"
#include "stubparams.h"
#include "gds/GProp.h"

//////////////////////////////////////////////////////////////////////////
//StbParam


//////////////////////////////////////////////////////////////////////////
//StbParams
IMPLEMENT_CLASS(StbParams);

void StbParams::Clear()
{
	if (bOwnObj)
	{
		for (int i=0;i<count;i++)
		{
			if ((entries[i].obj)&&(entries[i].type==StbParam::Obj))
			{
				entries[i].obj->DeleteThis();
				entries[i].obj=NULL;
			}
		}
	}
	count=0;
}

GProperty*StbParamToObj(StbParam *entry,GProperty *dest)
{
	CClass *clss=dest->GetClass();
	GVarType gvt=dest->GetGVT();
	switch(entry->type)
	{
		case StbParam::Obj:
		{
			if (entry->obj)
			{
				if (entry->obj->GetClass()->IsSameWith(clss))
					return entry->obj;
			}
		}
		case StbParam::Number:
		{
			switch(gvt)
			{
				case GVT_S:
				{
					((Prop_S*)dest)->v=(int)entry->v;
					return dest;
				}
				case GVT_U:
				{
					((Prop_U*)dest)->v=(DWORD)entry->v;
					return dest;
				}
				case GVT_F:
				{
					((Prop_F*)dest)->v=(float)entry->v;
					return dest;
				}
				break;
			}
		}
		case StbParam::String:
		{
			if (gvt==GVT_String)
			{
				((Prop_String*)dest)->v=entry->str;
				return dest;
			}
		}
	}

	return NULL;
}


GProperty *StbParams::To(GProperty *dest)
{
	assert(!dest->IsSuperb());

	if (!IsEmpty())
	{
		GProperty *ret=StbParamToObj(&entries[0],dest);
		if (ret)
			return ret;
	}

	dest->GetGObj()->Zero(FALSE);
	return dest;
}

BOOL StbParams::From(GProperty *src)
{
	Clear();
	assert(!src->IsSuperb());

	GVarType gvt=src->GetGVT();
	switch(gvt)
	{
		case GVT_Void:
			return TRUE;
		case GVT_S:
		{
			entries[0].v=((Prop_S*)src)->v;
			entries[0].type=StbParam::Number;
			break;
		}
		case GVT_U:
		{
			entries[0].v=((Prop_U*)src)->v;
			entries[0].type=StbParam::Number;
			break;
		}
		case GVT_F:
		{
			entries[0].v=((Prop_F*)src)->v;
			entries[0].type=StbParam::Number;
			break;
		}
		case GVT_String:
		{
			entries[0].str=((Prop_String*)src)->v;
			entries[0].type=StbParam::String;
			break;
		}
		default:
		{
			entries[0].type=StbParam::Obj;
			entries[0].obj=src;
			break;
		}

	}
	count=1;

	return TRUE;
}



void StbParams::Copy(StbParams *src)
{
	Clear();
	SetOwnObj(TRUE);
	for (int i=0;i<src->count;i++)
	{
		StbParam *param=&src->entries[i];
		switch(param->type)
		{
			case StbParam::String:
			{
				Add(param->str.c_str());
				break;
			}
			case StbParam::Number:
			{
				Add(param->v);
				break;
			}
			case StbParam::Obj:
			{
				if (param->obj)
					AddObj(param->obj);
				break;
			}
		}
	}
}

void StbParams::Fetch(StbParams *src)
{
	Clear();
	for (int i=0;i<src->count;i++)
	{
		StbParam *paramSrc=&src->entries[i];
		StbParam *param=&entries[i];
		Swap(param->type,paramSrc->type);
		Swap(param->obj,paramSrc->obj);
		Swap(param->v,paramSrc->v);
		param->str.swap(paramSrc->str);
	}
	Swap(bOwnObj,src->bOwnObj);
	Swap(count,src->count);
}

