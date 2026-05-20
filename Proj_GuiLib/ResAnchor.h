#pragma once

#include "GuiLib.h"

#include <map>
#include <string>

class GuiLib_Api CResAnchorBase
{
public:

	virtual int GetResType()	=0;
	virtual void NotifyChange(const char *path){}//this is from the anchor popup
	virtual CRect GetRect()=0;
protected:
};


class GuiLib_Api CResAnchor:public CXTButton,public CResAnchorBase
{
public:
	CResAnchor(int type,const char *label,BOOL bUndo=TRUE);
	CImageList *GetImageList();


	void SetDefaultStyle();

	virtual void NotifyChange(const char *path);//this is from the anchor popup

	void SetPath(const char *path);
	const char *GetPath();
	void SetRelativePath(const char *path);
	const char *GetRelativePath();

	void SetLabel(const char * labelName){ _label=labelName;}
	void SetResType(int type)	{		_type=type;	}
	virtual int GetResType()	{		return _type;	}

	const char *GetLabel()	{		return _label.c_str();	}
	BOOL NeedUndo()	{		return _bUndo;	}

	virtual CRect GetRect();


	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClicked();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

protected:

	static CImageList _imagelist;

	std::string _path;
	std::string _pathRelative;
	int _type;//ResType
	std::string _label;//label used to indentify a ResAnchor within a CResEditPanel
	BOOL _bUndo;//whether this anchor need undo/redo support
	

};
