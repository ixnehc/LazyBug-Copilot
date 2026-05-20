#pragma once

#include "MapUtil.h"

#include "CxImageWnd.h"

#include "ximage.h"

#include "PinControls.h"

class IMapFile;
class CHeighMapUtil :public CMapUtil
{
public:
	CHeighMapUtil(void);
	~CHeighMapUtil(void);

	virtual BOOL OnCommand(DWORD ctrlID ,DWORD code,LPARAM lParam,CGeActor * actor);
	virtual BOOL InitDlg(CWnd * pParent);
	virtual void OnInitDlg(CGeActor * actor);
	virtual DWORD SelectMode(){ return CGuiAgent_Draw2D::Select_Multi;}
	void RegisterMode();

	BEGIN_DECLARE_TOOL_CLASS(CHeighMapUtil,TOOL_MAPCONTROL)
		GELEM_VAR_INIT(float,_value_min,0)
		GELEM_EDITVAR("",GVT_F,GSem(GSem_Size),"")

		GELEM_VAR_INIT(float,_value_max,0)
		GELEM_EDITVAR("",GVT_F,GSem(GSem_Size),"")

	END_DECLARE_TOOL_CLASS()

	virtual void Load(CDataPacket &dp);
	virtual void Save(CDataPacket &dp);

protected:
	void _AssignHeightMap(CGeActor *actor,BOOL bOverall=FALSE);
	BOOL _IsWriteable(i_math::pos2di & block,IMapFile * pMF);

	CxImageWnd * _imageWnd;
	CPinboardEdit _edit[2];
	CPinSpinner _spinner[2];
	CPinSlider _slider[2];
	
	BOOL _hasHeighMap;
	//persistence
	float _value_min,_value_max;
};





