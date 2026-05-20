
#pragma once


class CRichGrid_NameItem: public CXTPPropertyGridItem
{
public:
	CRichGrid_NameItem(CString strCaption);

	void Bind(std::string *s);
	virtual void OnValueChanged(CString v);
protected:

	std::string *_s;
};



