#pragma once

#ifdef FileSystem_EXPORT
#define FileSystem_Api __declspec(dllexport)
#else
#ifdef FileSystem_IGNORE_IMPORT
#define FileSystem_Api
#else
#define FileSystem_Api __declspec(dllimport)
#endif
#endif


class IFileSystem;
FileSystem_Api IFileSystem* GetFileSystem();


