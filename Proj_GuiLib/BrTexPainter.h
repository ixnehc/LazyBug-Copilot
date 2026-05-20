#pragma  once

#include "BrushUtil.h"

#include "TBLTexSetDlg.h"

#include "TBLImageLib.h"

#include "PinControls.h"

#include "GuiAgent_TerrainPaint.h"

#include "SscBtn.h"

class CBrTexPainter:public CBrushUtil
{
public:
	CBrTexPainter(void);
	virtual BOOL DlgProc(UINT message,WPARAM wParam,LPARAM lParam,CGeActor * actor,int mode);
	virtual BOOL BeginParam(CWnd * pParent,int mode,CGeActor * actor,int level,const char * nameView,int priority = AGENTPRIORITY_STANDARD);
	virtual void EndParam(int mode);
	virtual BOOL InitDlg(CWnd * pParent);
	virtual void OnInitDlg(CGeActor * actor);
	virtual BOOL OnCommand(DWORD ctrlID ,DWORD code,LPARAM lParam,CGeActor * actor);
	virtual void RegisterAgent();
	virtual void RegisterMode();
	virtual void OnUpdateUI(CGeActor * actor);

	friend class CGuiAgent_TerrainPaint;
protected:

	void _UpdateGuiData(CGeActor * actor);
	void _RefreshTrrnLib(ITrrnBrushLib *pTrrnBrLib);
	void _RefreshSscState(CGeActor * actor);
	void _CheckLibPathChange();
	BOOL _OnReLoadBrushLib(void);

	class _TexCtrl:public CTexCtrl
	{
	public:
		_TexCtrl(CTBLImageLib * lib){_lib = lib;}
	protected:
		virtual CTBLImageLib *_GetImageLib(){return _lib;}
		CTBLImageLib * _lib;
		friend class CBrTexPainter;
	};
	struct _Param
	{
		_Param()
		{
			radius = 1.0f; 
			speed = 10.0f; hardness = 6.0f;
			GConstructor();
		}
		~_Param(){GDestructor();}
		float radius;
		float speed;
		float hardness;

		BEGIN_GOBJ(_Param,0)
		GELEM_VAR_INIT(float,radius,1.0f)
		GELEM_VAR_INIT(float,speed,10.0f)
		GELEM_VAR_INIT(float,hardness,6.0f)
		END_GOBJ();
	};

	BEGIN_DECLARE_TOOL_CLASS(CBrTexPainter,TOOL_TERRAINBRUSH)
		GELEM_OBJARRAY(_Param,_params)
	END_DECLARE_TOOL_CLASS()

protected:
	void _LoadParam(int mode);
	void _SaveParam(int mode);
	void _SetStatus(BOOL bChecked);
protected:
	CSscBtn _btnSsc;	
	CComboBox _brushList;
	_TexCtrl _texctrl;
	CTBLImageLib _imagelib;
	
	CPinboardEdit _edit[3];
	CPinSlider _slider[3];
	CPinSpinner _spinner[3];
	
	CGuiAgent_TerrainPaint _painterAgent;

	_Param _params[4];
	BOOL _bChecked[4];

	std::string _strLib;
};

