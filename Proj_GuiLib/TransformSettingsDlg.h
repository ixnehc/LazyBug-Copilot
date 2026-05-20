
#pragma once

class TransformSettings
{
public:	
	TransformSettings(){
			bSnapMove = FALSE;
			bSnapScale = TRUE;
			bSnapRotate = TRUE;
			moveSnap = 0.1f;
			scaleSnap = 10;
			rotateSnap = 1.0f;
	}
	float SnapMove(float v);
	float SnapScale(float v);
	float SnapRotate(float v);

protected:
	float _SnapValue(float v,float grid);

public:
	BOOL  bSnapMove;
	BOOL  bSnapRotate;
	BOOL  bSnapScale;
	float moveSnap;
	float scaleSnap;
	float rotateSnap;
};

class CTransformSettingsDlg :public CXTPDialog
{
public:
	CTransformSettingsDlg();
	virtual ~CTransformSettingsDlg(){}

protected:
	DECLARE_MESSAGE_MAP()
	void DoDataExchange(CDataExchange* pDX);
	afx_msg void OnSettingChange();
	void OnOK(){}
	float _SnapValue(float v,float grid);
private:
	TransformSettings _settings;	
};

extern TransformSettings & GetTransformSettings();




