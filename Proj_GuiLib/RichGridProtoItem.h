
#pragma once

#include "ResAnchor.h"

class IProtoLib;

class CRichGrid_ProtoItem: public CXTPPropertyGridItem
{
public:
	CRichGrid_ProtoItem(CString strCaption);

	void SetProtoLib(IProtoLib *lib);
	void Bind(std::string *s,BOOL bLuaOnly);
	void Bind(unsigned __int64 *id,BOOL bLuaOnly);
	virtual void OnValueChanged(CString v);

protected:
	virtual void OnInplaceButtonDown(CXTPPropertyGridInplaceButton* pButton);

	std::string *_path_s;
	unsigned __int64 *_id;
	BOOL _bLuaOnly;

	IProtoLib *_protolib;

	
};



