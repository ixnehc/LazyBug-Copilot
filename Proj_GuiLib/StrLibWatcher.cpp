
#include "stdh.h"

#include "StrLibWatcher.h"

#include "stringparser/stringparser.h"


#include "FileSystem/IFileSystem.h"

#include "strlib/strlib.h"


//////////////////////////////////////////////////////////////////////////
//

void CStrLibWatcher::Init(IFileSystem *pFS)
{
	std::string path=StrLib_Get()->GetPath();
	path=GetFileFolderPath(path);

	_watcher.Start(path.c_str(),WNF_CHANGE_FILE_NAME|WNF_CHANGE_LAST_WRITE|WNF_CHANGE_CREATION);
}

void CStrLibWatcher::Clear()
{
	_watcher.Stop();
}

extern BOOL LoadStrLib();

void CStrLibWatcher::Update()
{
	ChangedFileInformation *info=NULL;
	DWORD c=_watcher.FetchChangedFiles((const ChangedFileInformation *&)info);

	for (int i=0;i<c;i++)
	{
		ChangedFileAction action=info[i].action;
		switch(action)
		{
		case FA_REMOVED:
		case FA_MODIFIED:
		case FA_RENAMED_OLD_NAME:
		case FA_RENAMED_NEW_NAME:
			break;
		default:
			continue;
		}
		if (strcmp(info[i].name,"default.strlib")==0)
			LoadStrLib();
	}

}


