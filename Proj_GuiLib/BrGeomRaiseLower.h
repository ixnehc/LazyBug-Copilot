#pragma once

#include "BrushUtil.h"

#include "PinControls.h"

#include "GuiAgent_TerrainRLOp.h"

class CBrGeomRaiseLower :public CBrushUtil
{
public:
	CBrGeomRaiseLower(void);
	~CBrGeomRaiseLower(void);

	virtual BOOL DlgProc(UINT message,WPARAM wParam,LPARAM lParam,CGeActor * actor,int mode);
	virtual BOOL BeginParam(CWnd * pParent,int mode,CGeActor * actor,int level,const char * nameView,int priority = AGENTPRIORITY_STANDARD);
	virtual void EndParam(int mode);
	virtual void RegisterMode();

	virtual BOOL InitDlg(CWnd * pParent);
	virtual BOOL OnCommand(DWORD ctrlID ,DWORD code,LPARAM lParam,CGeActor * actor);
	virtual void OnInitDlg(CGeActor * actor);
	virtual void RegisterAgent();
	
	friend class CGuiAgent_TerrainRLOp;
protected:
	struct _BrushParam
	{
		_BrushParam()
		{
			speed = 10.0f;
			hardness = 1.0f;
			radius = 1.0f;
			radius2 = 6.0f;
			GConstructor();
		}
		~_BrushParam()
		{
			GDestructor();
		}
		float speed;
		float hardness;
		float radius;
		float radius2;
		float height;
		float height2;
		BEGIN_GOBJ(_BrushParam,1)
			GELEM_VAR_INIT(float,speed,10.0f)
			GELEM_VAR_INIT(float,hardness,1.0f)
			GELEM_VAR_INIT(float,radius,1.0f)
			GELEM_VAR_INIT(float,radius2,6.0f)
			GELEM_VAR_INIT(float,height,0.0f);
			GELEM_VAR_INIT(float,height2,0.0f);
		END_GOBJ();	
	};

	BEGIN_DECLARE_TOOL_CLASS(CBrGeomRaiseLower,TOOL_TERRAINBRUSH)
		GELEM_OBJARRAY(_BrushParam,_params)
	END_DECLARE_TOOL_CLASS()

	void SaveParam(int i);
	void LoadParam(int i);

protected:

	CPinSlider _slider_ir;
	CPinboardEdit _edit_ir;
	CPinSpinner _spinner_ir;


	CPinSlider _slider_or;
	CPinboardEdit _edit_or;
	CPinSpinner _spinner_or;


	CPinSlider _slider_speed;
	CPinboardEdit _edit_speed;
	CPinSpinner _spinner_speed;
	
	CPinSlider _slider_hardness;
	CPinboardEdit	_edit_hardness;
	CPinSpinner _spinner_hardness;
	
	CPinboardEdit _edit_height;  //Flatten模式设置 高度
	CButton _check_height;   //
	CPinboardEdit _edit_height2;  //Flatten模式设置 高度2
	CButton _check_height2;   //


	_BrushParam _params[6];
	//
	CGuiAgent_TerrainRLOp _agentTerrainRLOp;
};
