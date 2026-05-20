// ***************************************************************
//  stub   version:  1.0   ? date: 12/16/2007
//  -------------------------------------------------------------
//  author:		ixnehc
//  -------------------------------------------------------------
//  Copyright (C) 2007 - All Rights Reserved
// ***************************************************************
//  Purpose: stub implement
// ***************************************************************
#include "stdh.h"
#include "stub.h"


CStubCore::CStubCore()
{
	memset(_name,0,sizeof(_name));
	_link=NULL;
	_next=NULL;

	_owner=NULL;

	_dlgtLink=NULL;
	_dlgtBreak=NULL;
}


static void LinkStub(CStubCore *i,CStubCore *o)
{
	i->_link=o;
	CStubCore **p=&o->_link;

	while(*p)
	{
		if (*p==i)
			return;//already linked
		p=&(*p)->_next;
	}
	*p=i;
	if (i->_dlgtLink)
		i->_dlgtLink(i);
	i->_OnModify();
}

BOOL CStubCore::Link(CStubCore *other)
{
	if (!other)
		return FALSE;
	if (_bIn==other->_bIn)
		return FALSE;

	if (GetType()!=other->GetType())
		return FALSE;//not the same type

	if ((_link==other)&&(other->_link==this))
		return TRUE;//already linked

	//stubin should link to a single stubout,so break if already linked.
	if (_bIn)
		Break();
	if (other->_bIn)
		other->Break();

	if (_bIn)
		LinkStub(this,other);
	else
		LinkStub(other,this);

	return TRUE;
}

void CStubCore::Break()
{
	if (_bIn)
	{
		if (_link)
		{
			CStubCore **p=&_link->_link;
			while(*p)
			{
				if (*p==this)
				{
					if (_dlgtBreak)
						_dlgtBreak(this);
					*p=_next;
					break;
				}
				p=&(*p)->_next;
			}
		}
	}
	else
	{
		CStubCore *p=_link;
		//notify first
		while(p)
		{
			if (p->_dlgtBreak)
				p->_dlgtBreak(p);
			p=p->_next;
		}
		//now break
		p=_link;
		while(p)
		{
			p->_link=NULL;
			CStubCore *t=p->_next;
			p->_next=NULL;
			p=t;
		}
	}
	_link=NULL;
	_next=NULL;

}
