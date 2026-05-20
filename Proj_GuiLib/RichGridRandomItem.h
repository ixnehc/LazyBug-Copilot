
#pragma once

#include "ResAnchor.h"

#include "strlib/strlib.h"

class IProtoLib;

class CRichGrid_RandomItem: public CXTPPropertyGridItem
{
public:
	CRichGrid_RandomItem(CString strCaption);

	void Bind(DWORD*v);
	virtual void OnValueChanged(CString v);

protected:
	virtual void OnInplaceButtonDown(CXTPPropertyGridInplaceButton* pButton);

	DWORD *_bind;

	
};



