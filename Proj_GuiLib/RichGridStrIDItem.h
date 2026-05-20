
#pragma once

#include "ResAnchor.h"

#include "strlib/strlib.h"

class IProtoLib;

class CRichGrid_StrIDItem: public CXTPPropertyGridItem
{
public:
	CRichGrid_StrIDItem(CString strCaption);

	void Bind(StringID*id,const char *grp,DWORD iCategory);
	virtual void OnValueChanged(CString v);

protected:
	virtual void OnInplaceButtonDown(CXTPPropertyGridInplaceButton* pButton);

	StringID *_id;
	std::string _grp;
	DWORD _iCategory;

	
};



