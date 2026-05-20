
#pragma once

class CRichGrid_SscUIDItem: public CXTPPropertyGridItem
{
public:
	CRichGrid_SscUIDItem(CString strCaption);

	void Bind(DWORD*id);
	virtual void OnValueChanged(CString v);

protected:
	virtual void OnInplaceButtonDown(CXTPPropertyGridInplaceButton* pButton);

	DWORD *_id;
	
};



