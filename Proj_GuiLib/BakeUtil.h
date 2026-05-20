#pragma once
#include "MapUtil.h"
#include "progress/progress.h"
#include "WorldSystem/IWorldSystem.h"
#include "WorldSystem/IBake.h"

class CBakeUtil :public CMapUtil
{
public:
	CBakeUtil(void);
	~CBakeUtil(void);

	virtual BOOL InitDlg(CWnd * pParent);
	virtual BOOL OnCommand(DWORD ctrlID ,DWORD code,LPARAM lParam,CGeActor * actor);
	virtual void OnInitDlg(CGeActor * actor);
	
	BOOL OnSetTitle(const char * title);
	BOOL OnProcess(const char * info,int cur,int full);
	BOOL OnBegin_FldBake(const char *name);
	BOOL OnEnd_FldBake();

	BOOL OnProcess_Unit(const char * info,int cur,int full);
	void BakeStaticLight();
	void BakeMultiEnvTex();
	void BakeMiniMap();
	void BakeOutlineMap();
	void GenNavMesh();
	void RegisterMode();
protected:
	
	BEGIN_DECLARE_TOOL_CLASS(CBakeUtil,TOOL_MAPCONTROL)
		GELEM_VAR_INIT(int,_nfld,0);
	END_DECLARE_TOOL_CLASS()

	friend UINT BakeThread(LPVOID pParam);
	
	CProgress _progMain;
	CProgress _progSub;
	ISceneBaker * _baker;
	int _nfld;
	int _curfld;

	float _unitSub;	// a sub unit precent range
	float _mainPro; // main process precent has completed.


	BakeQuality _bq;

	CProgressCtrl _ctrl;
	int _oldPrec;
	BOOL _bMsgUpdate;
	std::vector<pos2di> _flds;
};
