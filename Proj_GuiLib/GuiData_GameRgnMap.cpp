
#include "stdh.h"

#include "GuiData_GameRgnMap.h"

#include "WorldSystem/IEntitySystem.h"
#include "WorldSystem/IAssetSystem.h"

#include "FileSystem/IMapFile.h"
#include "FileSystem/IFileSystem.h"

#include "SscBase.h"

const char *GuiData_GameRgnMap::GetFullPath()
{
	IMapFile *mf=pES->GetAS()->GetSS()->mf;
	if (!mf)
		return "";
	return mf->GetUniqueSscPath(GAMERGNGRID_UNIQUEFILENAME);
}

CGameRgnGrids *GuiData_GameRgnMap::ObtainGrids()
{
	if (!_grids.IsEmpty())
		return &_grids;

	const char *path=GetFullPath();
	if (!path[0])
		return NULL;

	BOOL bExist=TRUE;

	if (!g_ssGuiLib.pFS->ExistFileAbs(path))
	{
		g_ssGuiLib.ssc->GetLatestVersion(path);
		if (!g_ssGuiLib.pFS->ExistFileAbs(path))
			bExist=FALSE;
	}

	if (!bExist)
	{
		//新建一个
		DWORD len=GAMERGNGRID_GRIDLEN;
		IMapFile *mf=pES->GetAS()->GetSS()->mf;
		i_math::recti rc=mf->GetRect();
		rc*=BLOCK_LENGTH;

		rc.scale_signed(len);

		CGameRgnGrids grids;
		grids.Init(rc,len,FALSE);
		grids.Save(path);

		if (g_ssGuiLib.pFS->ExistFileAbs(path))
			bExist=TRUE;
	}

	if (!bExist)
		return NULL;

	if (FALSE==_grids.Load(path))
		return NULL;

	return &_grids;
}
