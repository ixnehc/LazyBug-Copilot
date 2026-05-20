
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "GameRgnGrid.h"

#include "stdio.h"

#define CUR_VER 1


BOOL CGameRgnGrids::Init(i_math::recti &rc,DWORD lenGrid,BOOL bRT)
{
	_hdr.rc=rc;
	_hdr.lenGrid=lenGrid;
	_hdr.bRT=bRT;

	if (bRT)
	{
		_gridsRT.resize(_hdr.rc.getArea());
		VEC_SET(_gridsRT,0);
	}
	else
	{
		_grids.resize(_hdr.rc.getArea());
		VEC_SET(_grids,0);
	}

	return TRUE;
}

void CGameRgnGrids::Clear()
{
	_grids.clear();
	_gridsRT.clear();
	_hdr.Zero();
}

BOOL CGameRgnGrids::Load(const char * file)
{
	Clear();

	FILE *fp = fopen(file,"rb");
	if (!fp)
		return FALSE;

	DWORD ver;
	fread(&ver,sizeof(ver),1,fp);
	if (ver==CUR_VER)
	{
		fread(&_hdr,sizeof(_hdr),1,fp);

		if (_hdr.bRT)
		{
			_gridsRT.resize(_hdr.rc.getArea());
			fread(_gridsRT.data(), sizeof(GameRgnGridRT), _hdr.rc.getArea(), fp);
		}
		else
		{
			_grids.resize(_hdr.rc.getArea());
			fread(_grids.data(),sizeof(GameRgnGrid),_hdr.rc.getArea(),fp);
		}
		
	}
	fclose(fp);

	return TRUE;
}


BOOL CGameRgnGrids::Save(const char * file)
{
	FILE * fp = fopen(file,"wb");
	if(!fp)
		return FALSE;

	DWORD ver=CUR_VER;
	fwrite(&ver,sizeof(ver),1,fp);

	fwrite(&_hdr,sizeof(_hdr),1,fp);

	if (_hdr.bRT)
		fwrite(_gridsRT.data(),sizeof(GameRgnGridRT),_hdr.rc.getArea(),fp);
	else
		fwrite(_grids.data(),sizeof(GameRgnGrid),_hdr.rc.getArea(),fp);

	fclose(fp);
	return TRUE;
}

BOOL CGameRgnGrids::SaveRT(const char * file)
{
	if (_hdr.bRT)
		return Save(file);

	FILE * fp = fopen(file,"wb");
	if(!fp)
		return FALSE;

	DWORD ver=CUR_VER;
	fwrite(&ver,sizeof(ver),1,fp);

	Header hdr=_hdr;
	hdr.bRT=TRUE;

	fwrite(&hdr,sizeof(hdr),1,fp);

	GameRgnGridRT gridRT;

	for (int i=0;i<_grids.size();i++)
	{
		gridRT.From(_grids[i]);
		fwrite(&gridRT,sizeof(GameRgnGridRT),1,fp);
	}

	fclose(fp);
	return TRUE;
}

