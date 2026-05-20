#pragma once

#include "GuiLib.h"
#include "FileSystem/IFileSystem.h"
#include "FileSystem/ISscSystem.h"


typedef DWORD SscUID;

#define SscUID_Invalid (0)


class CSscUIDs
{
public:
	CSscUIDs()
	{
		_bLocked=FALSE;
	}
	BOOL BeginGen();
	SscUID Gen();
	void EndGen();

protected:
	BOOL _bLocked;
	const char *_GetPath();
	ISscSystem *_GetSS();

};

GuiLib_Api SscUID SscUID_SafeGen();