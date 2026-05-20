#pragma once

#include "../common/filepackage/FileAccessMode.h"

#define PATH_SLASH "\\"
#define PATH_SLASH_C '\\'



enum FileAttr
{
	File_Default=0,
	File_Miss=1,
	File_ReadOnly=2,
};


#define Result_FileSystemBase 0x1000
enum FileSystemResult
{
	Result_Success=0,

	//File System 
	Result_TooLongPath=Result_FileSystemBase,
	Result_InvalidPath,
	Result_DirectoryAlreadyExist,
	Result_CannotCreateDirectory,
	Result_CannotCreatePackage,
	Result_CannotOpenPackage,
	Result_InvalidOpenMode,
	Result_NoSearchPath,
	Result_DirectoryInUse,
	Result_FileAlreadyExist,
	Result_WritingReadOnly,

	FileSystemResult_Force_Dword=0xffffffff,
};

//返回是否要枚举这个文件(如果返回TRUE,这个文件会被枚举出来)
typedef BOOL (*EnumFileFilter)(const char *path,BOOL bFolder,void *param);

