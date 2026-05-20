
#pragma once

#include <assert.h>

#include "RichGridButtonItem.h"

struct GElemBase;
class CGObjVectorItem:public CRichGrid_ButtonItem
{
public:
	CGObjVectorItem(CString strCaption):CRichGrid_ButtonItem(strCaption)
	{
		_elem=NULL;
		_owner=NULL;
		_maskSub=0;
	}

	BOOL Bind(void *owner,GElemBase *elem,DWORD maskSub)
	{
		_owner=owner;
		_elem=elem;
		_maskSub=maskSub;
		return TRUE;
	}

	virtual void OnButtonClick(DWORD idButton);

protected:

	DECLARE_DYNAMIC(CGObjVectorItem)

	GElemBase*_elem;
	void *_owner;
	DWORD _maskSub;

	friend class CGObjGrid;
};

class CGObjSubItem:public CRichGrid_ButtonItem
{
public:
	CGObjSubItem(CString strCaption):CRichGrid_ButtonItem(strCaption)
	{
		_elem=NULL;
		_iSub=0;
		_owner=NULL;
	}

	BOOL Bind(void *owner,GElemBase *elem,DWORD iSub)
	{
		_elem=elem;
		_owner=owner;
		_iSub=iSub;
		return TRUE;
	}

	virtual void OnButtonClick(DWORD idButton);

protected:
	GElemBase*_elem;
	DWORD _iSub;
	void *_owner;
};

