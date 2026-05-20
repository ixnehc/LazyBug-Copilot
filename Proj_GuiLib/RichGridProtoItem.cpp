/********************************************************************
	created:	2007/8/29   16:29
	filename: 	e:\IxEngine\Proj_GuiLib\RichGridResItem.cpp
	author:		cxi
	
	purpose:	grid items used in RichGrid--Resource Item
*********************************************************************/

#include "Stdh.h"
#include "RichGrid.h"
#include "RichGridProtoItem.h"

#include "stringparser/stringparser.h"

#include "ProtoSelectDlg.h"

#include "Log/LogFile.h"

#include "WorldSystem/IEntitySystem.h"



///////////////////////////////////////////////////////////////////////////////

CRichGrid_ProtoItem::CRichGrid_ProtoItem(CString strCaption)
	: CXTPPropertyGridItem(strCaption)
{
	m_nFlags = xtpGridItemHasExpandButton|xtpGridItemHasEdit;

	_path_s=NULL;
	_id=NULL;
	_bLuaOnly=FALSE;
	_protolib=NULL;
}

void CRichGrid_ProtoItem::SetProtoLib(IProtoLib *lib)
{
	_protolib=lib;
}



void CRichGrid_ProtoItem::OnInplaceButtonDown(CXTPPropertyGridInplaceButton* pButton)
{
	if ((!_protolib)||((!_path_s)&&(!_id)))
		return;

	CProtoSelectDlg dlg;
	dlg.SetProtoLib(_protolib);
	dlg.ShowSelNone();//显示<None>的那个按钮
	if (_bLuaOnly)
		dlg.SetLuaProtoOnly();
	else
		dlg.SetNoneLuaProtoOnly();

	
	if (_path_s)
		dlg.SetSelPath(_path_s->c_str());
	if (_id)
		dlg.SetSelPath(_protolib->FindPath(*_id));


	if (IDOK==dlg.DoModal())
	{
		OnValueChanged(CString(dlg.GetSelPath()));
	}
}

void CRichGrid_ProtoItem::OnValueChanged(CString v)
{
	CXTPPropertyGridItem::OnValueChanged(v);
	m_pGrid->SetFocus();

	if ((!_path_s)&&(!_id))
		return;

	GetRichGrid(this)->OnBeginItemChange(this);

	if (_path_s)
		*_path_s = toMBCS((LPCTSTR)v);
	if (_id)
	{
		if (v=="")
			(*_id)=0;
		else
			(*_id)=_protolib->FindProto2(toMBCS((LPCTSTR)v));
	}

	GetRichGrid(this)->OnItemChange(this);
	GetRichGrid(this)->OnEndItemChange(this);
}

void CRichGrid_ProtoItem::Bind(unsigned __int64 *id,BOOL bLuaOnly)
{
	_id=id;
	_bLuaOnly=bLuaOnly;
	if (*id)
	{
		IProto *proto=_protolib->ObtainProto(*_id);
		if (proto)
			SetValue(CString(proto->GetFilePath()));
		else
			SetValue(CString(""));
	}
	else
		SetValue(CString(""));
}


void CRichGrid_ProtoItem::Bind(std::string *s,BOOL bLuaOnly)
{
	_path_s=s;
	_bLuaOnly=bLuaOnly;
	SetValue(CString(s->c_str()));
}
