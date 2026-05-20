#pragma once

#include "GuiLib.h"
#include "WorldSystem/stubparams/param_sys.h"

#include "ref/ref.h"


class CRichGrid_MatSetItem : public CXTPPropertyGridItem
{
public:
	CRichGrid_MatSetItem( CString strCaption );
	virtual ~CRichGrid_MatSetItem()
	{
	}
	BOOL IsLS()
	{
		return _bLS;
	}
	
	void Bind(std::vector<i_math::matrix43f> *mats);
	void Bind(std::vector<i_math::vector3df> *vecs);
	void Bind(std::vector<i_math::spheref> *sphs);
	void SetLS(BOOL bLS)	{		_bLS=bLS;	}
	std::vector<i_math::matrix43f> *GetBindMats()	{		return _mats;	};
	std::vector<i_math::vector3df> *GetBindVecs()	{		return _vecs;	};
	std::vector<i_math::spheref> *GetBindSphs()	{		return _sphs;	};
	const char *GetMode()	{		return _mode.c_str();	}
	void SetMode(const char *mode)	{		_mode=mode;	}

	void UpdateValue();
protected:
	virtual BOOL OnLButtonDown(UINT nFlags, CPoint point);
	virtual void OnLButtonDblClk(UINT nFlags, CPoint point);

	virtual void OnValueChanged(CString strValue);

	virtual BOOL OnDrawItemValue(CDC& dc, CRect rcValue);

	std::vector<i_math::matrix43f> *_mats;
	std::vector<i_math::vector3df> *_vecs;
	std::vector<i_math::spheref> *_sphs;
	BOOL _bLS;
	std::string _mode;

	DECLARE_DYNAMIC(CRichGrid_MatSetItem)

};



