#ifndef __FilePackageDefines_H__
#define __FilePackageDefines_H__
#include <string>
#include <vector>
#include <map>
#include "FileAccessMode.h"

const int PACKAGE_SUBPATH_COUNT		= 10;
const int PACKAGE_PATHKEY_COUNT		= 1 + PACKAGE_SUBPATH_COUNT;

struct PathKey
{
	DWORD key;					// path hash key
	DWORD key2;					// reverse path hash key
};

typedef PathKey PATHKEYLIST[PACKAGE_PATHKEY_COUNT];
typedef char*	SUBPATHLIST[PACKAGE_SUBPATH_COUNT];

const int PACKAGE_FILENODE_LENGTH	= 4 * sizeof(LONG64) + MAX_PATH * sizeof(char) + sizeof(PATHKEYLIST);
struct PackageFileNode
{
	LONG64		pos;
	LONG64		front;
	LONG64		next;
	LONG64		size;			// data size
	char		name[MAX_PATH];	// It's so enough.
	PATHKEYLIST pathKeys;

#define nameKey pathKeys[0]
};

typedef std::multimap<DWORD, PackageFileNode*> PackageFileNodeList;
typedef std::pair<DWORD, PackageFileNode*> PathKeyFileNodePair;

const LONG64 PACKAGE_INVALID_POSITION= 0x7FFFFFFFFFFFFFFF;
const int MAX_BUFFER_LENGTH			= 1024 * 1024 * 10;	//10 MB

// package file header
const int PACKAGE_FILEHEADER_LENGTH	= 2 * sizeof(USHORT) + 2 * sizeof(LONG64);
const USHORT PACKAGE_VERSION		= 0x01;
struct PackageFileHeader
{
	USHORT	type;	// file flag, 'pc'
	USHORT	ver;	// version
	LONG64	first;	// first FileInfo node position in the package
	LONG64	last;	// last FileInfo node position
};

typedef void* HPACKAGE;
typedef void* HPACKAGEFILE;

typedef std::vector<std::string> FilePathList;

#endif