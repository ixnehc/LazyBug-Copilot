
#pragma once

#include "GuiLib.h"


class IRenderSystem;
class GuiLib_Api CRichGrid_FileItem: public CXTPPropertyGridItem
{
public:
	CRichGrid_FileItem(CString strCaption);

	void Bind(std::string *s);
	virtual void OnValueChanged(CString v);

	void SetRootPath(const char *path);
	void SetFilter(const char *suffix,const char *filter);

protected:
	virtual void OnInplaceButtonDown(CXTPPropertyGridInplaceButton* pButton);

	std::string *_path_s;
	std::string _rootpath;
	std::string _suffix;
	std::string _filter;

	
};





