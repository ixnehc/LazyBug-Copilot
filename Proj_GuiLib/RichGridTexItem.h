
#pragma once

#include "GuiLib.h"


class IRenderSystem;
class GuiLib_Api CRichGrid_TexItem: public CXTPPropertyGridItem
{
public:
	CRichGrid_TexItem(CString strCaption);

	void Bind(std::string *s,BOOL bSelPart);
	virtual void OnValueChanged(CString v);
	void SetOwnerPath(const char *path);


protected:
	virtual void OnInplaceButtonDown(CXTPPropertyGridInplaceButton* pButton);

	void _UpdateValueValidity();

	BOOL _bSelPart;
	std::string *_path_s;
	std::string _pathOwner;

	
};





