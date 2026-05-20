

#ifndef _ParticleFeatureDialog_H_
#define _ParticleFeatureDialog_H_




#include "GuiLib.h"
#include ".\resource.h"
#include "particleFeatureEdit.h"
#include "brokenLineEdit.h"
#include "afxwin.h"


class GuiLib_Api CParticleFeatureDialog : public CDialog
{
public:
	CParticleFeatureDialog( CWnd* pParent = NULL );
	// dialog template
	enum { IDD = IDD_PARTICLE_FEATURE_DLG };
private:
	virtual void DoDataExchange( CDataExchange* pDX );
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL PreTranslateMessage( MSG* pMsg );
public:
	DECLARE_MESSAGE_MAP()
	afx_msg virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonResetBl();
	afx_msg void OnCbnSelchangeComboParticleFeature();
	afx_msg void OnBnClickedCheckConst();
public:
	void BindProperty( BrokenLineRef *p, const float fMinX, const float fMaxX, const float fMinY, const float fMaxY );
	DataType LoadData();
	void SaveData();
private:
	void _CreateEditor();
private:
	CParticleFeatureEditor _Editor;
	CComboBox	_ComboBox;
	DataType	_eDataType;
	CButton		_btnCheck;
	bool		_bCheck;
};


#endif
