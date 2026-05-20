
#pragma once

#include "ResAnchor.h"

#include "records/records.h"


class CRichGrid_RecordIDItem: public CXTPPropertyGridItem
{
public:
	CRichGrid_RecordIDItem(CString strCaption);

	void Bind(RecordID*id,const char *nameRecords);
	virtual void OnValueChanged(CString v);

protected:
	virtual void OnInplaceButtonDown(CXTPPropertyGridInplaceButton* pButton);

	RecordID *_id;
	std::string _nameRecords;

	
};



