
#pragma once

#include "ResAnchor.h"



class CRichGrid_ResItem: public CXTPPropertyGridItem,public CResAnchorBase
{
public:
	CRichGrid_ResItem(CString strCaption);

	void Bind(std::string *s,int restype);
	void SetOwnerPath(const char *path);
	virtual void OnValueChanged(CString v);

	virtual int GetResType();
	virtual void NotifyChange(const char *path);//this is from the anchor popup
	virtual CRect GetRect();


protected:
	virtual void OnInplaceButtonDown(CXTPPropertyGridInplaceButton* pButton);
	void _UpdateValueValidity();

	std::string *_path_s;
	int _restype;
	std::string _pathOwner;

	std::string _temp;
	
};



