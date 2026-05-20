/********************************************************************
	created:	2008/03/28
	created:	28:3:2008   13:48
	filename: 	e:\IxEngine\Proj_GuiLib\RichGridVec3Item.cpp
	file path:	e:\IxEngine\Proj_GuiLib
	file base:	RichGridVec3Item
	file ext:	cpp
	author:		star
	purpose:	edit vector3 just time.
*********************************************************************/

#include "stdh.h"
#include ".\richgridvec3item.h"
#include "RichGrid.h"

//////////////////////////////////////////////////////////////////////////
//CRichGridFloatPad

void CRichGridFloatPad::OnValueChanged(CString strValue)
{
	if(!_valueBind)
	{
		SetValue(_T("unbind"));
		return ;
	}
	else
	{
		GetRichGrid(this)->OnBeginItemChange(this);

		float value = (float)atof(toMBCS(strValue.GetString()));
		if(_valueBind)
			(*_valueBind) = value;

		char buf[255]={0};
		sprintf(buf,"%f",value);
		SetValue(fromMBCS(buf));

		m_pParent->OnValueChanged(strValue);

		GetRichGrid(this)->OnItemChange(this);
		GetRichGrid(this)->OnEndItemChange(this);
	}
}
void CRichGridFloatPad::Bind(float * value)
{
	if(!value)
		return;
	_valueBind = value;
	char buf[255]={0};
	sprintf(buf,"%f",*value);
	SetValue(fromMBCS(buf));
}


//////////////////////////////////////////////////////////////////////////
//CRichGridVec3Item

CRichGridVec3Item::~CRichGridVec3Item(void)
{
}
CRichGridVec3Item::CRichGridVec3Item(CString strValue)
	:CXTPPropertyGridItem(strValue)
{
	m_itemX = NULL;
	m_itemY = NULL;
	m_itemZ = NULL;
	m_nFlags&=~xtpGridItemHasEdit;
}

void  CRichGridVec3Item::Bind(i_math::vector3df *vec_)
{
	if(!vec_)
		return;
	vec= vec_;
	m_itemX->Bind(&(vec->x));
	m_itemY->Bind(&(vec->y));
	m_itemZ->Bind(&(vec->z));

	char buf[255] = {0};
	sprintf(buf,"x: %f ,y: %f ,z: %f ",vec->x,vec->y,vec->z);
	SetValue(fromMBCS(buf));

}


void CRichGridVec3Item::OnAddChildItem()
{
	m_itemX	  = (CRichGridFloatPad *)AddChildItem(new CRichGridFloatPad(_T("x"),&(vec->x)));
	m_itemY	  = (CRichGridFloatPad *)AddChildItem(new CRichGridFloatPad(_T("y"),&(vec->y)));
	m_itemZ	  = (CRichGridFloatPad *)AddChildItem(new CRichGridFloatPad(_T("z"),&(vec->z)));

	m_itemX->SetDescription(_T("x element."));
	m_itemY->SetDescription(_T("y element."));
	m_itemZ->SetDescription(_T("z element."));
}

void CRichGridVec3Item::OnValueChanged(CString strValue)
{
	if(!vec)
	{
		SetValue(_T("unbind"));
	}
	else
	{
		char buf[255] = {0};
		sprintf(buf,"x: %f ,y: %f ,z: %f ",vec->x,vec->y,vec->z);
		SetValue(fromMBCS(buf));
	}
}


//////////////////////////////////////////////////////////////////////////
//CRichGridVec4Item

CRichGridVec4Item::~CRichGridVec4Item(void)
{
}
CRichGridVec4Item::CRichGridVec4Item(CString strValue)
:CXTPPropertyGridItem(strValue)
{
	m_itemX = NULL;
	m_itemY = NULL;
	m_itemZ = NULL;
	m_itemW = NULL;
	m_nFlags&=~xtpGridItemHasEdit;
}

void  CRichGridVec4Item::Bind(i_math::vector4df *vec_)
{
	if(!vec_)
		return;
	vec= vec_;
	m_itemX->Bind(&(vec->x));
	m_itemY->Bind(&(vec->y));
	m_itemZ->Bind(&(vec->z));
	m_itemW->Bind(&(vec->w));

	char buf[255] = {0};
	sprintf(buf,"x: %f ,y: %f ,z: %f ,w: %f ",vec->x,vec->y,vec->z,vec->w);
	SetValue(fromMBCS(buf));

}


void CRichGridVec4Item::OnAddChildItem()
{
	m_itemX	  = (CRichGridFloatPad *)AddChildItem(new CRichGridFloatPad(_T("x"),&(vec->x)));
	m_itemY	  = (CRichGridFloatPad *)AddChildItem(new CRichGridFloatPad(_T("y"),&(vec->y)));
	m_itemZ	  = (CRichGridFloatPad *)AddChildItem(new CRichGridFloatPad(_T("z"),&(vec->z)));
	m_itemW	  = (CRichGridFloatPad *)AddChildItem(new CRichGridFloatPad(_T("w"),&(vec->w)));

	m_itemX->SetDescription(_T("x element."));
	m_itemY->SetDescription(_T("y element."));
	m_itemZ->SetDescription(_T("z element."));
	m_itemZ->SetDescription(_T("w element."));
}

void CRichGridVec4Item::OnValueChanged(CString strValue)
{
	if(!vec)
	{
		SetValue(_T("unbind"));
	}
	else
	{
		char buf[255] = {0};
		sprintf(buf,"x: %f ,y: %f ,z: %f ,w: %f ",vec->x,vec->y,vec->z,vec->w);
		SetValue(fromMBCS(buf));
	}
}



//////////////////////////////////////////////////////////////////////////
//CRichGridVec2Item

CRichGridVec2Item::~CRichGridVec2Item(void)
{
}
CRichGridVec2Item::CRichGridVec2Item(CString strValue)
:CXTPPropertyGridItem(strValue)
{
	m_itemX = NULL;
	m_itemY = NULL;
	m_nFlags&=~xtpGridItemHasEdit;
}

void  CRichGridVec2Item::Bind(i_math::vector2df *vec_)
{
	if(!vec_)
		return;
	vec= vec_;
	m_itemX->Bind(&(vec->x));
	m_itemY->Bind(&(vec->y));

	char buf[255] = {0};
	sprintf(buf,"x: %f ,y: %f ",vec->x,vec->y);
	SetValue(fromMBCS(buf));

}


void CRichGridVec2Item::OnAddChildItem()
{
	m_itemX	  = (CRichGridFloatPad *)AddChildItem(new CRichGridFloatPad(_T("x"),&(vec->x)));
	m_itemY	  = (CRichGridFloatPad *)AddChildItem(new CRichGridFloatPad(_T("y"),&(vec->y)));

	m_itemX->SetDescription(_T("x element."));
	m_itemY->SetDescription(_T("y element."));
}

void CRichGridVec2Item::OnValueChanged(CString strValue)
{
	if(!vec)
	{
		SetValue(_T("unbind"));
	}
	else
	{
		char buf[255] = {0};
		sprintf(buf,"x: %f ,y: %f ",vec->x,vec->y);
		SetValue(fromMBCS(buf));
	}
}



