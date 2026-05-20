
#pragma once
#include "GuiLib.h"

#include "SlideSpin.h"


class CGridItemSpinInplaceButton : public CSlideSpin
{
	friend class CGridItemSpin;
	CGridItemSpin* m_pItem;
public:
	virtual void OnBeginValueChange();
	virtual void OnEndValueChange();

	virtual void OnValueChange(SlideSpinValue vNew);
};

class CGridItemSpin : public CXTPPropertyGridItemNumber
{
public:
	CGridItemSpin(CString strCaption);
	virtual ~CGridItemSpin()	{	}

	void EnableFloatMode(BOOL bEnable)	{		m_bFloatMode=bEnable;	}
	void SetRange(SlideSpinValue start,SlideSpinValue end);
	void SetValue(SlideSpinValue v);

	//default is 1.0f
	void SetSlideSpeed(SlideSpinValue speed)	{		m_wndSpin.SetSlideSpeed(speed);	}

	virtual void OnBeginValueChange()	{	}
	virtual void OnValueChanged(SlideSpinValue v){}
	virtual void OnEndValueChange(){	}

protected:
	void ValueToString(SlideSpinValue v,CString &strValue);
	virtual void OnValueChanged(CString strValue);//this will be only called when the user changed something in the editor
	virtual void OnDeselect();
	virtual void OnSelect();
	virtual CRect GetValueRect();

	CGridItemSpinInplaceButton *GetSpin();

protected:
	CGridItemSpinInplaceButton m_wndSpin;
	BOOL m_bFloatMode;
	friend class CGridItemSpinInplaceButton;
};

