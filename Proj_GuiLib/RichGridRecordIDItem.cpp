/********************************************************************
	created:	2012/03/22
	file base:	RichGridRecordIDItem
	author:		cxi
	
	purpose:	
*********************************************************************/
#include "Stdh.h"
#include "RichGrid.h"
#include "RichGridRecordIDItem.h"

#include "stringparser/stringparser.h"


#include "RecordDlg.h"

#include "Log/LogFile.h"

#include "stringparser/stringparser.h"

#include "RenderSystem/IRenderSystem.h"
#include "RenderSystem/IRecords.h"



///////////////////////////////////////////////////////////////////////////////

CRichGrid_RecordIDItem::CRichGrid_RecordIDItem(CString strCaption)
	: CXTPPropertyGridItem(strCaption)
{
	m_nFlags = xtpGridItemHasExpandButton|xtpGridItemHasEdit;

	_id=RecordID_Invalid;
}

void CRichGrid_RecordIDItem::OnInplaceButtonDown(CXTPPropertyGridInplaceButton* pButton)
{
	if (!_id)
		return;

	RecordID id=*_id;
	std::string name;
	extern BOOL RecordDlg_Browse(RecordID&id,std::string &name,const char *nameRecords);
	if (RecordDlg_Browse(id,name,_nameRecords.c_str()))
	{
		const char *str=MakeShortStr(name.c_str(),32);
		CXTPPropertyGridItem::OnValueChanged(CString(str));
		GetRichGrid(this)->OnBeginItemChange(this);
		(*_id)=id;
		GetRichGrid(this)->OnItemChange(this);
		GetRichGrid(this)->OnEndItemChange(this);
	}

}

void CRichGrid_RecordIDItem::OnValueChanged(CString v)
{
//	CXTPPropertyGridItem::OnValueChanged(v);
}

const char *GetRecordName(const char *nameRecords,RecordID id)
{
	static std::string ret;
	ret="";

	std::string s;
	s=nameRecords;
	MakeFileSuffix(s,"rcs");
	IRecords *records=(IRecords *)g_ssGuiLib.pRS->GetRecordsMgr()->ObtainRes(s.c_str());

	if (records)
	{
		CRecords *p=records->GetRecords();
		if (p)
			ret=p->GetName(id);
	}

	SAFE_RELEASE(records);

	if ((id!=RecordID_Invalid)&&(ret.empty()))
		ret="[missing]";

	return ret.c_str();
}

void CRichGrid_RecordIDItem::Bind(RecordID*id,const char *nameRecords)
{
	_id=id;
	_nameRecords=nameRecords;
	_nameRecords+=".rcs";

	BOOL bMissing=FALSE;
	const char *name=GetRecordName(_nameRecords.c_str(),*id);
	if (((*id)!=RecordID_Invalid)&&(name[0]==0))
		bMissing=TRUE;
	if (!bMissing)
	{
		const char *str=MakeShortStr(GetRecordName(_nameRecords.c_str(),*id),32);
		SetValue(CString(str));
	}
	else
		SetValue(CString("[missing]"));

	CXTPPropertyGridItemMetrics*metrics=GetValueMetrics();
	if (bMissing)
		metrics->m_clrFore=0x0000ff;
	else
		metrics->m_clrFore=0;
}
