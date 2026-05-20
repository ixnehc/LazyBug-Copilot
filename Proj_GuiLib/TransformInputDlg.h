#pragma once

#include "PinControls.h"

#include "fastdelegate/FastDelegate.h"

/************************************************************************/
/* 修改Local矩阵                                                        */
/************************************************************************/

typedef fastdelegate::FastDelegate4<BOOL,DWORD,const i_math::vector3df&,i_math::matrix43f&> FunOnMatrixChange;
class CTransformInputDlg:public CXTPDialog
{
public:
	CTransformInputDlg(CWnd * pParent = NULL);
	virtual ~CTransformInputDlg();
protected:

	virtual BOOL OnInitDialog();
	afx_msg void DoDataExchange(CDataExchange* pDX);	
	
	afx_msg void BeginChange(NMHDR * pNotifyStruct,LRESULT * pResult);
	afx_msg void OnChange(NMHDR * pNotifyStruct,LRESULT * pResult);
	afx_msg void EndChange(NMHDR * pNotifyStruct,LRESULT * pResult);

	DECLARE_MESSAGE_MAP()	
	
public:
	enum 
	{ 
		TYPE_Move,
		TYPE_Rotate,
		TYPE_Scale
	};
	
	enum
	{
		Change_Begin,
		Change_On,
		Change_End
	};

	void Create(CWnd * pParent);
	template <class X>	
		void Bind(DWORD typeTrans,const i_math::matrix43f &matLocal,const i_math::matrix43f &matParent,X* pThis,
					void(X:: *funListener)(BOOL,DWORD,const i_math::vector3df&,i_math::matrix43f&))
	{
	
		i_math::matrix43f matWorld = matLocal*matParent;
		_funMatrixChange.bind(pThis,funListener);

		_typeEdit = typeTrans;			
		_matParent = matParent;
		_matWorld = matWorld;
	}
	
	virtual void OnOK(){}	
protected:
	void _HandleBind();
	void _SetDefaultRange();
	void _OnMatrixMoveChange(DWORD idCmd,i_math::vector3df &vecChange);
	BOOL _OnMatrixRoateChange(DWORD idCmd,i_math::vector3df &vecChange);
	void _OnMatrixScaleChange(DWORD idCmd,i_math::vector3df &vecChange);
	i_math::vector3df _GetChange(DWORD idCmd,const i_math::vector3df &oldVec);
	
	float _GetSafeChange(float v,float oldValue);
	float _AddChange(float v,float oldChange);
	float _ToDegree(float v);	
	void _DoChange(DWORD_PTR ctrID,CPinboard * pinboard);

	afx_msg void OnReset();

private:
	// offset value
	CPinboardEdit _editOffX;
	CPinboardEdit _editOffY;
	CPinboardEdit _editOffZ;
	CPinboardEdit _editOffXYZ;
	
	CPinSpinner _spinOffX;
	CPinSpinner _spinOffY;
	CPinSpinner _spinOffZ;
	CPinSpinner _spinOffXYZ;
	
	// absolute value
	CPinboardEdit _editAbX;
	CPinboardEdit _editAbY;
	CPinboardEdit _editAbZ;
	CPinboardEdit _editAbXYZ;

	CPinSpinner _spinAbX;
	CPinSpinner _spinAbY;
	CPinSpinner _spinAbZ;
	CPinSpinner _spinAbXYZ;
	
	i_math::matrix43f _matWorld;
	i_math::matrix43f _matParent;
	FunOnMatrixChange _funMatrixChange;
	DWORD _typeEdit;

	i_math::vector3df _vecInit;
	DWORD _changeType;
	BOOL _bLock;
};

extern CTransformInputDlg * GetTransformDlg();


