
#pragma once


class CRichGrid_FolderItem: public CXTPPropertyGridItem
{
public:
	CRichGrid_FolderItem(CString strCaption);

	void SetRootPath(const char *pathRoot)	{		_pathRoot=pathRoot;	}
	void Bind(std::string *s);
	virtual void OnValueChanged(CString v);

protected:
	virtual void OnInplaceButtonDown(CXTPPropertyGridInplaceButton* pButton);
	std::string _pathRoot;

	std::string *_path_s;
	
};



