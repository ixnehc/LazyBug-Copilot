#pragma once
#include "GuiLib.h"

class CXTPDockingPaneManager;

class GuiLib_Api CPaneFlasher
{
public:
	CPaneFlasher()
	{
		_id=0;
		_mgr=NULL;
		_bFlashing=FALSE;
		_state=0;
	}
	void Init(int id,CXTPDockingPaneManager *mgr,UINT idIcon);
	void Flash();
	void Update();

protected:

	CXTPDockingPaneManager *_mgr;
	int _id;
	CXTPImageManagerIconHandle _hIcon[3];

	BOOL _bFlashing;

	DWORD _tStart;

	int _state;//0或者1

};