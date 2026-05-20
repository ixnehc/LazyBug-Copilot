#include "stdh.h"
#include "Cursors.h"

#include "../stringparser/stringparser.h"

#include "WorldSystem/IWorldSystem.h"
#include "WorldSystem/IAssetSystem.h"
#include "WorldSystem/IAssetShell.h"

CCursors::CCursors()
{
	_shell=NULL;
}

CCursors::~CCursors()
{
	Clear();
}

BOOL CCursors::Init(IAssetSystem*pAS)
{
	_shell=pAS->GetShell();

	const char *path=pAS->GetWS()->GetPath(WSPath_HwCursor);

	std::string pathPttn;
	pathPttn=path;
	pathPttn+="\\*.ani";

	HANDLE hFindFile;
	WIN32_FIND_DATA fd;
	hFindFile = FindFirstFile(fromMBCS(pathPttn.c_str()), &fd);
	if (hFindFile!=INVALID_HANDLE_VALUE)
	{
		do
		{
			if (fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)
				continue;//ignore the folder

			std::string s=path;
			s = s + "\\" + toMBCS(fd.cFileName);
			HCURSOR handle = ::LoadCursorFromFile(fromMBCS(s.c_str()));
			if (handle)
			{
				std::string title = GetFileTitle(std::string(toMBCS(fd.cFileName)));
				StringLower(title);
				_cursors[title]=handle;
			}

		}while(FindNextFile(hFindFile,&fd));
		::FindClose(hFindFile);
	}

	return TRUE;
}


void CCursors::Clear()
{
	std::unordered_map<std::string,HCURSOR>::iterator it;
	for (it=_cursors.begin();it!=_cursors.end();it++)
		DestroyCursor( (*it).second);
	_cursors.clear();
}


HCURSOR CCursors::GetActive()
{
	std::string name=_shell->GetCursorName();
	std::unordered_map<std::string,HCURSOR>::iterator it=_cursors.find(name);
	if (it==_cursors.end())
		it=_cursors.find(std::string("_default"));
	if (it!=_cursors.end())
		return (*it).second;
	return NULL;

}
