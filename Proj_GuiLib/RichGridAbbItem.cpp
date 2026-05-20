#include "stdh.h"
#include ".\richgridabbitem.h"
#include "RichGrid.h"


CRichGridAbbItem::~CRichGridAbbItem(void)
{

}

class CRichGridAbbItem::CRichGridAbbItemFloat :public CRichGrid_FloatItem
{
public:
	CRichGridAbbItem::CRichGridAbbItemFloat(CString strCaption):CRichGrid_FloatItem(strCaption)
	{
	}
protected:
	 void OnValueChanged(SlideSpinValue v)
	{
		//CRichGrid_FloatItem::OnValueChanged(v);
		if(_f)
			*_f = (float)v; 
		CRichGridAbbItem * parent = (CRichGridAbbItem *) GetParentItem();
		parent->UpdateSize();
	}
};

void CRichGridAbbItem::Bind(i_math::aabbox3df * aabb)
{	
	_aabb = aabb;
	_padMin->Bind(&(_aabb->MinEdge));
	_padMax->Bind(&(_aabb->MaxEdge));

	_w = _aabb->MaxEdge.x - _aabb->MinEdge.x;
	_h = _aabb->MaxEdge.y - _aabb->MinEdge.y;
	_t = _aabb->MaxEdge.z - _aabb->MinEdge.z;
	_widthItem->Bind(&_w,0,(float)0x8fffffff);
	_widthItem->SetSlideSpeed(0.01f);
	_heightItem->Bind(&_h,0,(float)0x8fffffff);
	_heightItem->SetSlideSpeed(0.01f);
	_thickItem->Bind(&_t,0,(float)0x8fffffff);
	_thickItem->SetSlideSpeed(0.01f);
}

void CRichGridAbbItem::OnValueChanged(CString strValue)
{
	if(!_aabb)
		SetValue(_T("unBind"));
	else
	{
		char buf[255];
		sprintf(buf,"MinEdge: %f,%f,%f  MaxEdge: %f,%f,%f",_aabb->MinEdge.x,_aabb->MinEdge.y,_aabb->MinEdge.z,_aabb->MaxEdge.x,_aabb->MaxEdge.y,_aabb->MaxEdge.z);
		
		GetRichGrid(this)->OnBeginItemChange(this);;
		GetRichGrid(this)->OnItemChange(this);
		GetRichGrid(this)->OnEndItemChange(this);
	}
}
void CRichGridAbbItem::UpdateSize()
{	
	_aabb->MaxEdge.x = _aabb->MinEdge.x + _w;
	_aabb->MaxEdge.y = _aabb->MinEdge.y + _h;
	_aabb->MaxEdge.z = _aabb->MinEdge.z + _t;
	_aabb->repair();
	_padMax->OnValueChanged(_T(""));
}
void CRichGridAbbItem::OnAddChildItem()
{	
	_padMin = (CRichGridVec3Item *) AddChildItem(new CRichGridVec3Item(_T("MinEdge")));
	
	_padMax = (CRichGridVec3Item *)AddChildItem(new CRichGridVec3Item(_T("MaxEdge")));
	
	_widthItem =(CRichGridAbbItemFloat *) AddChildItem(new CRichGridAbbItemFloat(_T("Width")));
	
	_heightItem =(CRichGridAbbItemFloat *) AddChildItem(new CRichGridAbbItemFloat(_T("Height")));

	_thickItem =(CRichGridAbbItemFloat *) AddChildItem(new CRichGridAbbItemFloat(_T("Thickness")));

}


