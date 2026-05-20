#pragma  once

#include "math/pos2d.h"
#include "mod/ModBase.h"
#include "FileSystem/IMapFileDefines.h"
#include "GuiLib.h"

#include "fastdelegate/FastDelegate.h"

// the first time excute redo
// Redo
//     back old version from map file
//	   Save modify to file.
//	   Reload map file

// Undo
//	   back new version from map file.
//	   save old version back data to map file
//	   Reload  map file.

// the second time excute redo
// Redo
//		back old version from map file
//		save new version back data to map file
//		Reload map file


class CGeView;
class  CModBlockBack :public CModBase
{
 	typedef fastdelegate::FastDelegate1<CModBlockBack *,void> BackupCallBack;
	typedef fastdelegate::FastDelegate1<CModBlockBack *,void> RestoreCallBack;

public:
	CModBlockBack(CGeView * view)
	{
		_view = view;
		_bFirstTime = TRUE;
	}

// frame
	virtual BOOL Undo();
	virtual BOOL Redo();

	virtual BOOL IsEmpty(){return (_iblocks.size()==0);}	

// interface	
	template<class T>
	void SetCallBack(T * pThis,void(T:: *registSel)(CModBlockBack *),void(T:: *restoreSel)(CModBlockBack *))
	{
		_dlgtBackup.bind(pThis,registSel);
		_dlgtRestore.bind(pThis,restoreSel);
	}
	BOOL BackupBlocks(const i_math::pos2di * blocks,int num);
	void SetAddOnData(void *data,int size);
	void *GetAddOnData(int &size);

	
protected:
	void Clean();
	BOOL CheckEditable();
	void NotifyReload();

	BOOL BackMapData(std::vector<MapBlockData> &data);
	void SaveData(std::vector<MapBlockData> & data);

	virtual void OnBackup()	{	}
	virtual void OnRestore()	{	}


protected:
	std::vector<i_math::pos2di> _iblocks;
	std::vector<MapBlockData> _dataBack;
	std::vector<BYTE>  _dataAddOn;

	BackupCallBack			  _dlgtBackup;
	RestoreCallBack		  _dlgtRestore;
	
	BOOL _bFirstTime;

	CGeView * _view;
};
