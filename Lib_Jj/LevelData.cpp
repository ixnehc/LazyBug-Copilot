
#include "stdh.h"

#include "LevelData.h"

#include "LevelObjSrc.h"

#include "LevelClasses.h"

#include "LevelRecords.h"

#include "LevelRecordMap.h"

#include "gds/GObj.h"

#include "Random/Random.h"
#include "timer/timer.h"

#include <fstream>




//////////////////////////////////////////////////////////////////////////
//CLevelData
void CLevelData::Clear()
{
	_ndata.clear();
	_gtm.Clear();
	_gtr.Clear();

	if (_srces)
		_srces->Clear();
	Safe_Class_Delete(_srces);
	if (_basis)
		_basis->Clear();
	Safe_Class_Delete(_basis);

	for (int i=0;i<_srcesOld.size();i++)
	{
		if (_srcesOld[i])
			_srcesOld[i]->Clear();
		Safe_Class_Delete(_srcesOld[i]);
	}
	_srcesOld.clear();
	for (int i=0;i<_basisOld.size();i++)
	{
		if (_basisOld[i])
			_basisOld[i]->Clear();
		Safe_Class_Delete(_basisOld[i]);
	}
	_basisOld.clear();

	if (_grids)
		_grids->Clear();
	Safe_Class_Delete(_grids);

	for (int i=0;i<_gridsOld.size();i++)
	{
		if (_gridsOld[i])
			_gridsOld[i]->Clear();
		Safe_Class_Delete(_gridsOld[i]);
	}
	_gridsOld.clear();

}

BOOL CLevelData::_LoadGtm(const char *pathGtm)
{
	std::ifstream ifs;
	ifs.open(pathGtm,std::ios_base::in|std::ios_base::binary);

	if (!ifs.is_open())
		return FALSE;

	ifs.read((char*)&_gtm.hdr,sizeof(_gtm.hdr));
	DWORD sz;
	ifs.read((char *)&sz,sizeof(sz));
	_gtm.data.resize(sz);
	ifs.read((char *)&_gtm.data[0],sz*sizeof(_gtm.data[0]));

	ifs.close();

	return TRUE;

}

BOOL CLevelData::_LoadGtr(const char *pathGtr)
{
	std::ifstream ifs;
	ifs.open(pathGtr,std::ios_base::in|std::ios_base::binary);

	if (!ifs.is_open())
		return FALSE;

	ifs.read((char*)&_gtr._hdr,sizeof(_gtr._hdr));
	DWORD sz;

	ifs.read((char *)&sz,sizeof(sz));
	_gtr._tris.resize(sz);
	ifs.read((char *)&_gtr._tris[0],sz*sizeof(_gtr._tris[0]));

	ifs.read((char *)&sz,sizeof(sz));
	_gtr._indices.resize(sz);
	ifs.read((char *)&_gtr._indices[0],sz*sizeof(_gtr._indices[0]));

	ifs.read((char *)&sz,sizeof(sz));
	_gtr._tiles.resize(sz);
	ifs.read((char *)&_gtr._tiles[0],sz*sizeof(_gtr._tiles[0]));

	ifs.close();

	return TRUE;
}




CLevelSources *LoadLevelSources(const char *pathSrces)
{
	if (pathSrces[0])
	{
		CDataPacket dp;
		std::ifstream ifs;
		std::vector<BYTE>buf;
		ifs.open(pathSrces,std::ios_base::in|std::ios_base::binary);
		if (!ifs.is_open())
			return NULL;

		DWORD sz;
		ifs.read((char *)&sz,sizeof(sz));
		buf.resize(sz);
		ifs.read((char *)buf.data(),sz);

		ifs.close();

		CLevelSources *srces=Class_New2(CLevelSources);
		dp.SetDataBufferPointer(buf.data());
		if (FALSE==srces->Load(dp))
		{
			Safe_Class_Delete(srces);
			return NULL;
		}

		return srces;
	}

	return NULL;

}

CGameRgnGrids*CLevelData::_LoadGrids(const char *pathGrids)
{
	if (pathGrids[0])
	{
		CGameRgnGrids*grids=Class_New2(CGameRgnGrids);
		if(FALSE==grids->Load(pathGrids))
		{
			Safe_Class_Delete(grids);
			return NULL;
		}

		return grids;
	}

	return NULL;

}


BOOL CLevelData::Load(const char *pathNavData,const char *pathSrces,const char *pathGrids,const char *pathGtm,const char *pathGtr,CLevelRecords *records)
{
	if (false==_ndata.load(pathNavData))
		return FALSE;

	if (FALSE==_LoadGtm(pathGtm))
		goto _fail;

	if (FALSE==_LoadGtr(pathGtr))
		goto _fail;

	_srces=LoadLevelSources(pathSrces);
	if (!_srces)
		goto _fail;
	_basis=Class_New2(CLevelBasis);
	_basis->Build(_srces,_pathSrces.c_str(),records);

	_grids=_LoadGrids(pathGrids);
	if (!_grids)
		goto _fail;

	_pathNavData=pathNavData;
	_pathSrces=pathSrces;
	_pathGrids=pathGrids;
	_pathGtm=pathGtm;
	_pathGtr=pathGtr;

	return TRUE;

_fail:
	Clear();

	return FALSE;
}

void CLevelData::ReloadSources(CLevelRecords *records)
{
	CLevelSources *srces=LoadLevelSources(_pathSrces.c_str());
	if (!srces)
		return;
	if (_srces)
		_srcesOld.push_back(_srces);
	_srces=srces;

	if (_basis)
		_basisOld.push_back(_basis);

	_basis=Class_New2(CLevelBasis);
	_basis->Build(_srces,_pathSrces.c_str(),records);

}

void CLevelData::ReloadGrids()
{
	CGameRgnGrids*grids=_LoadGrids(_pathGrids.c_str());
	if (!grids)
		return;
	if (_grids)
		_gridsOld.push_back(_grids);
	_grids=grids;
}


//////////////////////////////////////////////////////////////////////////
//CWorldData
void CWorldData::Clear()
{
	std::unordered_map<RecordID,CLevelData*>::iterator it;
	for (it=_datas.begin();it!=_datas.end();it++)
	{
		CLevelData *p=(*it).second;
		p->Clear();
		Safe_Class_Delete(p);
	}
	_datas.clear();

	_dbSlatesB.Clear();

	if (_basis)
	{
		_basis->Clear();
		Safe_Class_Delete(_basis);
	}

	for (int i=0;i<_basisOld.size();i++)
	{
		if (_basisOld[i])
			_basisOld[i]->Clear();
		Safe_Class_Delete(_basisOld[i]);
	}
	_basisOld.clear();


}

BOOL CWorldData::AddLevel(RecordID idMap,const char *pathDataRoot,CLevelRecords *records)
{
	if (idMap==RecordID_Invalid)
		return FALSE;
	if (_datas.find(idMap)!=_datas.end())
		return FALSE;
	CLevelData *ld=Class_New2(CLevelData);

	std::string pathNvd,pathSrces,pathGrids,pathGtm,pathGtr;
	pathNvd=pathDataRoot;
	pathNvd+="\\"+records->GetMap(idMap)->path;
	pathSrces=pathGrids=pathGtm=pathGtr=pathNvd;

	pathNvd+="\\1.nvd";
	pathSrces+="\\1.lsd";
	pathGrids+="\\1.rgd";
	pathGtm+="\\1.gti";
	pathGtr+="\\1.gtr";
	
	if (!ld->Load(pathNvd.c_str(),pathSrces.c_str(),pathGrids.c_str(),pathGtm.c_str(),pathGtr.c_str(),records))
	{
		Safe_Class_Delete(ld);
		return FALSE;
	}

	_datas[idMap]=ld;

	return TRUE;
}

void CWorldData::ReloadSources(CLevelRecords *records)
{
	std::unordered_map<RecordID,CLevelData*>::iterator it;
	for (it=_datas.begin();it!=_datas.end();it++)
	{
		CLevelData *p=(*it).second;
		p->ReloadSources(records);
	}

	if (_basis)
	{
		_basisOld.push_back(_basis);
		_basis=NULL;
	}
	BuildBasis(records);
}

void CWorldData::ReloadGrids()
{
	std::unordered_map<RecordID,CLevelData*>::iterator it;
	for (it=_datas.begin();it!=_datas.end();it++)
	{
		CLevelData *p=(*it).second;
		p->ReloadGrids();
	}
}

void CWorldData::_LoadSlatesB(const char *path,SlatesBData&data)
{
	std::ifstream ifs;
	ifs.open(path,std::ios_base::in|std::ios_base::binary);

	if (!ifs.is_open())
		return;

	DWORD ver;
	ifs.read((char *)&ver,sizeof(ver));

	ifs.read((char *)&data.w,sizeof(data.w));
	ifs.read((char *)&data.h,sizeof(data.h));

	DWORD sz;
	ifs.read((char *)&sz,sizeof(sz));
	data.buf.resize(sz);
	ifs.read((char *)&data.buf[0],sz*sizeof(data.buf[0]));

	ifs.close();

}


void CWorldData::_LoadSlatesB(const char *pathFolder,std::deque<SlatesBData>&buf)
{
	std::string pathToFind;
	std::string pathSub;
	pathToFind=pathFolder;
	pathToFind+="\\*.slsb";
	HANDLE hFindFile;
	WIN32_FIND_DATAA fd;
	hFindFile=FindFirstFileA(pathToFind.c_str(),&fd);
	if (hFindFile!=INVALID_HANDLE_VALUE)
	{
		do
		{
			if ((fd.cFileName[0] == '.')&&(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
			{
				if (fd.cFileName[1] =='\0'||(fd.cFileName[1]=='.'&&fd.cFileName[2] == '\0'))
					continue;//ignore the dots
			}
			if (fd.dwFileAttributes&FILE_ATTRIBUTE_HIDDEN)
				continue;//忽略隐藏文件
			if (fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
				continue;

			pathSub=pathFolder;
			pathSub=pathSub+"\\"+fd.cFileName;

			buf.resize(buf.size()+1);
			_LoadSlatesB(pathSub.c_str(),buf[buf.size()-1]);

		}while(FindNextFileA(hFindFile,&fd));
		::FindClose(hFindFile);
	}

}


void CWorldData::LoadDatabaseSlatesB(const char *pathRoot)
{
	_dbSlatesB.Clear();

	std::string path;

	path=pathRoot;
	path+="\\easies";
	_LoadSlatesB(path.c_str(),_dbSlatesB.easies);

	path=pathRoot;
	path+="\\hards";
	_LoadSlatesB(path.c_str(),_dbSlatesB.hards);
}

SlatesBData *CWorldData::PickUpSlatesB(BOOL bEasy)
{
	std::deque<SlatesBData> *buf=NULL;
	if (bEasy)
		buf=&_dbSlatesB.easies;
	else
		buf=&_dbSlatesB.hards;

	DWORD c=buf->size();
	if (c<=0)
		return NULL;

	CSysRandom::Srand((DWORD)GetTSC());
	int idx=CSysRandom::RandRangeInt(0,(int)c);
	return &(*buf)[idx];
}



CLevelData *CWorldData::FindLevel(RecordID idMap)
{
	std::unordered_map<RecordID,CLevelData*>::iterator it=_datas.find(idMap);

	if (it==_datas.end())
		return NULL;

	return (*it).second;
}

void CWorldData::BuildBasis(CLevelRecords *records)
{
	_basis=Class_New2(CWorldBasis);

	std::vector<RecordID>idMaps;
	std::vector<CLevelSources*>sourcesAll;
	sourcesAll.reserve(_datas.size());
	idMaps.reserve(_datas.size());
	std::unordered_map<RecordID,CLevelData*>::iterator it;
	for (it=_datas.begin();it!=_datas.end();it++)
	{
		RecordID idMap=(*it).first;
		CLevelData *ld=(*it).second;

		CLevelSources *sources=ld->GetSources();
		if (sources)
		{
			sourcesAll.push_back(sources);
			idMaps.push_back(idMap);
		}
	}

	_basis->Build(sourcesAll.data(),idMaps.data(),sourcesAll.size(),records);
}
