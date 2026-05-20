/********************************************************************
	created:	2007/10/10   11:01
	filename: 	e:\IxEngine\Proj_GuiLib\GObjGrid.cpp
	author:		cxi
	
	purpose:	a property grid to edit a GObj
*********************************************************************/
#include "stdh.h"

#include <vector>

#include "GPropGrid.h"

#include "gds/GObj.h"
#include "gds/GProp.h"


#include "Log/LogFile.h"
#include "stringparser/stringparser.h"

#include <assert.h>


//////////////////////////////////////////////////////////////////////////
//CGPropGrid
GObjBase *FindOnlySubObj(GObjBase *obj)
{
	GElemBase *elem=obj->GetElems();
	if (TRUE)
	{
		GElemBase *elemFirst=NULL;
		while(elem)
		{
			if (elem->bEditable)
			{
				if (elemFirst)
					return NULL;
				elemFirst=elem;
			}
			elem=elem->next;
		}
		if (!elemFirst)
			return NULL;
		elem=elemFirst;
	}

	void *owner=obj->GetOwner();
	GObjBase *objSub;
	if (FALSE==elem->GetObj(owner,&objSub))
		return NULL;
	GSem &sem=elem->GetSem();
	if ((sem.code==GSem_Unknown)&&(sem.constraint=="DynObjPtr"))
		return NULL;//忽略可以动态改变的sub obj
	if (std::string("ValueSet")==objSub->GetName())
		return NULL;
	if (std::string("ColorSet")==objSub->GetName())
		return NULL;
	return objSub;
}

CXTPPropertyGridItem *CGPropGrid::InsertProp(GProperty *prop,const char *name,GSem &sem,const char *desc)
{
	std::string clssname=	prop->GetClass()->GetName();

	while(1)
	{
		if (clssname=="Prop_Void")
			break;
		if (clssname=="Prop_String")
		{
			Prop_String *p=(Prop_String *)prop;
			return InsertVar(&p->v,name,desc,GVT_String,sem);
		}
		
		if (clssname=="Prop_S")
		{
			Prop_S *p=(Prop_S *)prop;
			return InsertVar(&p->v,name,desc,GVT_S,sem);
		}
		if (clssname=="Prop_U")
		{
			Prop_U *p=(Prop_U *)prop;
			return InsertVar(&p->v,name,desc,GVT_U,sem);
		}
		if (clssname=="Prop_F")
		{
			Prop_F *p=(Prop_F*)prop;
			return InsertVar(&p->v,name,desc,GVT_F,sem);
		}
		if (clssname=="Prop_Fx2")
		{
			Prop_Fx2 *p=(Prop_Fx2*)prop;
			return InsertVar(&p->v,name,desc,GVT_Fx2,sem);
		}
		if (clssname=="Prop_Fx3")
		{
			Prop_Fx3 *p=(Prop_Fx3*)prop;
			return InsertVar(&p->v,name,desc,GVT_Fx3,sem);
		}
		if (clssname=="Prop_Fx4")
		{
			Prop_Fx4 *p=(Prop_Fx4*)prop;
			return InsertVar(&p->v,name,desc,GVT_Fx4,sem);
		}
		if (clssname=="Prop_Fx6")
		{
			Prop_Fx6 *p=(Prop_Fx6*)prop;
			return InsertVar(&p->v,name,desc,GVT_Fx6,sem);
		}
		if (clssname=="Prop_Fx12")
		{
			Prop_Fx12 *p=(Prop_Fx12*)prop;
			return InsertVar(&p->v,name,desc,GVT_Fx12,sem);
		}
		if (clssname=="Prop_Fx16")
		{
			Prop_Fx16 *p=(Prop_Fx16*)prop;
			return InsertVar(&p->v,name,desc,GVT_Fx16,sem);
		}
		if (clssname=="Prop_Sx4")
		{
			Prop_Sx4 *p=(Prop_Sx4*)prop;
			return InsertVar(&p->v,name,desc,GVT_Sx4,sem);
		}
		if (clssname=="Prop_Sx2")
		{
			Prop_Sx2*p=(Prop_Sx2*)prop;
			return InsertVar(&p->v,name,desc,GVT_Sx2,sem);
		}
		if (clssname=="Prop_Bx4")
		{
			Prop_Bx4*p=(Prop_Bx4*)prop;
			return InsertVar(&p->v,name,desc,GVT_Bx4,sem);
		}
		GObjBase *obj=prop->GetGObj();


		CXTPPropertyGridItem* item = InsertCategory(name, desc, new CXTPPropertyGridItem(fromMBCS(name)));
		PushInsert();

		if (TRUE)
		{
			GObjBase *objSub=FindOnlySubObj(obj);
			if (objSub)
				obj=objSub;
		}

		_Bind(obj,sem);

		PopInsert();

		return item;

	}

	return NULL;
}
