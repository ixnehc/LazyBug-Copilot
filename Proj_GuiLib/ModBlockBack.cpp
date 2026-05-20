
#include "stdh.h"
#include ".\ModBlockBack.h"

#include "GuiData.h"
#include "WorldSystem/IEntitySystem.h"
#include "FileSystem/IMapFile.h"

BOOL CModBlockBack::CheckEditable()
{
	if(!_view) 
		return FALSE;

	GuiData_System *ss = (GuiData_System *)_view->FindData("system");
	if((!ss)||(!ss->sevent))
		return FALSE;

	IMapFile * pMF = ss->mf;
	if(!pMF)
		return FALSE;

	i_math::recti rcMap;
	rcMap=pMF->GetRect();

	for(int i = 0;i< _iblocks.size();i++){
		i_math::pos2di & iblock = _iblocks[i];
		if (!rcMap.isPointInside(iblock))
			continue;//忽略不在地图范围内的block
		if(!pMF->BlockCheckedOut(iblock))
			return FALSE;
	}

	return TRUE;
}

BOOL CModBlockBack::Redo()
{ 
	//如果存在不能编辑的地块 回滚到修改的前一状态
	if(!CheckEditable()){
		SaveData(_dataBack);
		NotifyReload();
		Clean();
		return TRUE;
	}

	std::vector<BYTE> temp;
	if(TRUE)  // 保存场景的当前状态
	{
		temp.swap(_dataAddOn);
		if(!_dlgtBackup.empty())
			_dlgtBackup(this);
		OnBackup();
	}
	
	if(!_bFirstTime)
	{
		std::vector<MapBlockData> tempdata; //temply back the new version data
		tempdata.swap(_dataBack);
		if(FALSE == BackMapData(_dataBack))// load old version data from map file.
			return FALSE;
		SaveData(tempdata);		// save the new version data to map file.
	}

	NotifyReload();	// 重载数据

	if (!_bFirstTime)
	{
		temp.swap(_dataAddOn);
		if(!_dlgtRestore.empty())  //恢复到上一次场景状态
			_dlgtRestore(this); 
		OnRestore();
		temp.swap(_dataAddOn);
	}

	assert(_view);
	_view->Draw();

	_bFirstTime = FALSE;

	return TRUE;
}
void CModBlockBack::SaveData(std::vector<MapBlockData> & data)
{
	GuiData_System *ss = (GuiData_System *)_view->FindData("system");
	assert(ss);
	IMapFile * pMF = ss->mf;
	assert(pMF);
	assert(data.size()==_iblocks.size());

	for(int i = 0;i< _iblocks.size();i++)
	{
		i_math::pos2di & iblock = _iblocks[i];
		if(pMF->BlockCheckedOut(iblock))
			MapFile_SetBlockData(pMF,data[i]);
	}	
}

BOOL CModBlockBack::Undo()
{
	GuiData_System *ss = (GuiData_System *)_view->FindData("system");
	assert(ss);
	IMapFile * pMF = ss->mf;
	assert(pMF);

	std::vector<BYTE> temp;
	if(TRUE)  // 保存场景的当前状态
	{
		temp.swap(_dataAddOn);

		if(!_dlgtBackup.empty())
			_dlgtBackup(this);
		OnBackup();
	}
    
	if(TRUE)
	{
		std::vector<MapBlockData> tempdata;  //temply back old version data.
		tempdata.swap(_dataBack);		 
		if(FALSE == BackMapData(_dataBack))	// load new version data from map file
			return FALSE;
		SaveData(tempdata);	   // save old version data to map file	
	}


	NotifyReload(); // 重载数据
	
	temp.swap(_dataAddOn);
	if(!_dlgtRestore.empty())
		_dlgtRestore(this); //恢复到上一次场景状态
	OnRestore();
	temp.swap(_dataAddOn);

	assert(_view);
	_view->Draw();

	return TRUE;
}
BOOL CModBlockBack::BackMapData(std::vector<MapBlockData> &data)
{
	GuiData_System *ss = (GuiData_System *)_view->FindData("system");
	assert(ss);
	IMapFile * pMF = ss->mf;
	assert(pMF);

	data.clear();
	data.resize(_iblocks.size());

	for(int i = 0;i< _iblocks.size();i++)
	{
		i_math::pos2di & iblock = _iblocks[i];
		MapFile_GetBlockData(pMF,iblock,data[i]);
	}

	return TRUE;
}

BOOL CModBlockBack::BackupBlocks(const i_math::pos2di * blocks,int num) //备份修改前的数据
{
	Clean();
	if(num<=0)
		return TRUE;

	assert(blocks);
	_iblocks.resize(num);

	for(int i = 0;i<num;i++)		//提交发生变化的地块坐标
		_iblocks[i] = blocks[i];
	

	return BackMapData(_dataBack);
}

void CModBlockBack::Clean()
{
	_iblocks.clear();
	_dataBack.clear();
}

void CModBlockBack::NotifyReload()
{
	GuiData_System *ss = (GuiData_System *)_view->FindData("system");
	assert(ss);
	if(ss->pES)
		ss->pES->ReloadMap(_iblocks.data(),_iblocks.size());
}

void CModBlockBack::SetAddOnData(void *data,int size)
{
	_dataAddOn.clear();
	if (data)
	{
		_dataAddOn.resize(size);
		memcpy(_dataAddOn.data(),data,size);
	}
}

void *CModBlockBack::GetAddOnData(int &size)
{
	size=_dataAddOn.size();
	return (void*)_dataAddOn.data();
}




