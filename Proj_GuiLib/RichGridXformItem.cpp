/********************************************************************
	created:	2006/10/31   16:27
	filename: 	e:\IxEngine\Proj_GuiLib\RichGridTexItem.cpp
	author:		cxi
	
	purpose:	grid items used in RichGrid--Xform Item
*********************************************************************/

#include "Stdh.h"
#include "RichGrid.h"
#include "RichGridXformItem.h"


#include "stringparser/stringparser.h"

#include "Log/LogFile.h"



///////////////////////////////////////////////////////////////////////////////

CRichGrid_XformItem::CRichGrid_XformItem(CString strCaption)
	: CXTPPropertyGridItem(strCaption)
{
	m_nFlags = xtpGridItemHasExpandButton|xtpGridItemHasEdit;

	_edittype=RGXI_Edit_None;
	_mat=NULL;
	_bMod=FALSE;
}

void CRichGrid_XformItem::_OnMEDlgNotify(MatEditDlgNotify notify)
{
	_bMod=TRUE;
	GetRichGrid(this)->RedrawDueToItemChange();
}


void CRichGrid_XformItem::OnInplaceButtonDown(CXTPPropertyGridInplaceButton* pButton)
{
	CMatEditDlg dlg;
	_bMod=FALSE;
	dlg.Bind(_mat);
	MatEditDlgHandler dlgt;
	dlgt.bind(this,&CRichGrid_XformItem::_OnMEDlgNotify);
	dlg.SetHandler(dlgt);
	dlg.DoModal();
	if (_bMod)
	{
		if (_mat)
		{
			i_math::matrix44f mat44;
			mat44from43(mat44,*_mat);
			std::string s;
			_toStr(mat44,s);
			CXTPPropertyGridItem::OnValueChanged(CString(s.c_str()));
		}

		GetRichGrid(this)->OnBeginItemChange(this);
		GetRichGrid(this)->OnItemChange(this);
		GetRichGrid(this)->OnEndItemChange(this);

	}
}




void CRichGrid_XformItem::Bind(i_math::matrix43f *mat,RGXI_EditType type)
{
	_mat=mat;
	_edittype=type;

	if (_mat)
	{
		i_math::matrix44f mat44;
		mat44from43(mat44,*_mat);

		std::string s;
		_toStr(mat44,s);
		SetValue(CString(s.c_str()));
	}
}


void CRichGrid_XformItem::OnValueChanged(CString v)
{
	i_math::matrix44f mat44;
	_fromStr(mat44, toMBCS((LPCTSTR)v));
	std::string s;
	_toStr(mat44,s);
	v=s.c_str();

	CXTPPropertyGridItem::OnValueChanged(v);

	m_pGrid->SetFocus();

	if (!_mat)
		return;

	GetRichGrid(this)->OnBeginItemChange(this);

	mat43from44(*_mat,mat44);

	GetRichGrid(this)->OnItemChange(this);
	GetRichGrid(this)->OnEndItemChange(this);
}

void CRichGrid_XformItem::_toStr(i_math::matrix44f &mat44,std::string &s)
{
	float *f=(float *)&mat44;
	FormatString(s,"%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f",
									f[0],f[1],f[2],f[3],
									f[4],f[5],f[6],f[7],
									f[8],f[9],f[10],f[11],
									f[12],f[13],f[14],f[15]		);
}

void CRichGrid_XformItem::_fromStr(i_math::matrix44f &mat44,const char *s)
{
	std::vector<std::string>vecTemp;
	SplitStringBy(",",std::string(s),&vecTemp);
	mat44.makeIdentity();
	float *f=(float *)&mat44;

	for (int i=0;i<vecTemp.size();i++)
		f[i]=(float)DoubleFromString(vecTemp[i].c_str());
}


