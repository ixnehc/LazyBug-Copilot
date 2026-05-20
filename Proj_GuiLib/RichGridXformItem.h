
#pragma once

#include "MatEditDlg.h"


enum RGXI_EditType
{
	RGXI_Edit_None,
	RGXI_Edit_2D,
	RGXI_Edit_3D,
};


class CRichGrid_XformItem: public CXTPPropertyGridItem
{
public:
	CRichGrid_XformItem(CString strCaption);

	void Bind(i_math::matrix43f *mat,RGXI_EditType type=RGXI_Edit_3D);
	virtual void OnValueChanged(CString v);

protected:
	virtual void OnInplaceButtonDown(CXTPPropertyGridInplaceButton* pButton);

	void _OnMEDlgNotify(MatEditDlgNotify notify);

	void _toStr(i_math::matrix44f &mat44,std::string &s);
	void _fromStr(i_math::matrix44f &mat44,const char *s);

	i_math::matrix43f *_mat;
	RGXI_EditType _edittype;

	BOOL _bMod;

	
};



