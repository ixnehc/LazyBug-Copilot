#include "stdh.h"
#include "RichGridSizeItem.h"
#include "RichGrid.h"


//////////////////////////////////////////////////////////////////////////
//CRichGridIntPad
void CRichGridIntPad::OnValueChanged(CString strValue)
{
	if((!_valueBind)&&(!_valueBindB))
	{
		SetValue(_T("unbind"));
		return ;
	}
	else
	{
		GetRichGrid(this)->OnBeginItemChange(this);

		int value = (int)atoi(toMBCS(strValue.GetString()));
		if(_valueBind)
			(*_valueBind) = value;
		if (_valueBindB)
			(*_valueBindB) = (BYTE)i_math::clamp_i(value,0,255);

		char buf[255]={0};
		sprintf(buf,"%d",value);
		SetValue(fromMBCS(buf));

		m_pParent->OnValueChanged(strValue);

		GetRichGrid(this)->OnItemChange(this);
		GetRichGrid(this)->OnEndItemChange(this);
	}
}
void CRichGridIntPad::Bind(int * value)
{
	if(!value)
		return;
	_valueBind = value;
	char buf[255]={0};
	sprintf(buf,"%d",*value);
	SetValue(fromMBCS(buf));
}

void CRichGridIntPad::Bind(BYTE* value)
{
	if(!value)
		return;
	_valueBindB = value;
	char buf[255]={0};
	sprintf(buf,"%d",(int)*value);
	SetValue(fromMBCS(buf));
}


//////////////////////////////////////////////////////////////////////////
//CRichGridSizeItem

CRichGridSizeItem::~CRichGridSizeItem(void)
{
}
CRichGridSizeItem::CRichGridSizeItem(CString strValue)
:CXTPPropertyGridItem(strValue)
{
	m_itemW = NULL;
	m_itemH = NULL;
	m_nFlags&=~xtpGridItemHasEdit;

	m_vec=NULL;
	m_vecB=NULL;
}

void  CRichGridSizeItem::Bind(i_math::vector2di *vec_)
{
	if(!vec_)
		return;

	m_vec= vec_;
	if (!m_itemW)
	{
		m_itemW = (CRichGridIntPad*)AddChildItem(new CRichGridIntPad(fromMBCS(GetHorDesc()), &(m_vec->x)));
		m_itemW->SetDescription(fromMBCS(GetHorDesc()));
		m_itemW->Bind(&(m_vec->x));
	}

	if (!m_itemH)
	{
		m_itemH	  = (CRichGridIntPad *)AddChildItem(new CRichGridIntPad(fromMBCS(GetVerDesc()),&(m_vec->y)));
		m_itemH->SetDescription(fromMBCS(GetVerDesc()));
		m_itemH->Bind(&(m_vec->y));
	}

	char buf[255] = {0};
	sprintf(buf,"%s: %d ,%s: %d",GetHorDesc(),m_vec->x,GetVerDesc(),m_vec->y);
	SetValue(fromMBCS(buf));
}

void CRichGridSizeItem::Bind(i_math::vector2db *vec_)
{
	if(!vec_)
		return;
	m_vecB= vec_;
	if (!m_itemW)
	{
		m_itemW= (CRichGridIntPad *)AddChildItem(new CRichGridIntPad(fromMBCS(GetHorDesc()),&(m_vecB->x)));
		m_itemW->SetDescription(fromMBCS(GetHorDesc()));
		m_itemW->Bind(&(m_vecB->x));
	}
	if (!m_itemH)
	{
		m_itemH= (CRichGridIntPad *)AddChildItem(new CRichGridIntPad(fromMBCS(GetHorDesc()),&(m_vecB->y)));
		m_itemH->SetDescription(fromMBCS(GetVerDesc()));
		m_itemH->Bind(&(m_vecB->y));
	}

	char buf[255] = {0};
	sprintf(buf,"%s: %d ,%s: %d",GetHorDesc(),(int)m_vecB->x,GetVerDesc(),(int)m_vecB->y);
	SetValue(fromMBCS(buf));
}


void CRichGridSizeItem::OnAddChildItem()
{
}

void CRichGridSizeItem::OnValueChanged(CString strValue)
{
	if((!m_vec)&&(!m_vecB))
	{
		SetValue(_T("unbind"));
	}
	else
	{
		char buf[255] = {0};
		if (m_vec)
			sprintf(buf,"%s: %d ,%s: %d",GetHorDesc(),m_vec->x,GetVerDesc(),m_vec->y);
		else
			sprintf(buf,"%s: %d ,%s: %d",GetHorDesc(),(int)m_vecB->x,GetVerDesc(),(int)m_vecB->y);
		SetValue(fromMBCS(buf));
	}
}


