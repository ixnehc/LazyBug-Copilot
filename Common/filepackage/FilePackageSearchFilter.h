#ifndef __FilePackageSearchFilter_H__
#define __FilePackageSearchFilter_H__
#include "FilePackageDefines.h"
#include "stringparser/stringparser.h"

class CFilePackageSearchFilter
{
public:
	CFilePackageSearchFilter(const PackageFileNodeList& fileNodeList) 
		: _fileNodeList(fileNodeList)
	{
	}

public:
	const char* MatchPath(const char* pszPath) const
	{
		const PackageFileNode* node = FindFileNodeByPath(pszPath);
		return (node ? node->name : NULL);
	}
	const char* MatchAbsPath(const char* pszAbsPath) const
	{
		const PackageFileNode* node = FindFileNodeByAbsPath(pszAbsPath);
		return (node ? node->name : NULL);
	}
	int SearchMatchPaths(const char* pszPath, FilePathList& rSearchResult) const
	{
		int nCount = 0;

		// Get all files
		const char* const STAR			= "*";
		const char* const STARDOTSTAR	= "*.*";
		if (strcmp(pszPath, STAR) == 0 || strcmp(pszPath, STARDOTSTAR) == 0)
		{
			PackageFileNodeList::const_iterator it = _fileNodeList.begin();
			for (; it != _fileNodeList.end(); it++)
			{
				rSearchResult.push_back(it->second->name);
			}
			return (nCount = static_cast<int>(rSearchResult.size()));
		}

		// _strlwr
		char szLowerPath[MAX_PATH] = { 0 };
		strncpy(szLowerPath, pszPath, MAX_PATH - 1);
		_strlwr(szLowerPath);

		DWORD nHash = CalcHashCode(szLowerPath);
		DWORD nHash2 = CalcHashCodeReverse(szLowerPath);

		PackageFileNode* node;
		PackageFileNodeList::const_iterator it = _fileNodeList.begin();
		for (; it != _fileNodeList.end(); it++)
		{
			node = it->second;
			for (int i = 0; i < PACKAGE_PATHKEY_COUNT; i++)
			{
				if ((nHash == node->pathKeys[i].key) && 
					(nHash2 == node->pathKeys[i].key2))
				{
					rSearchResult.push_back(node->name);
					break;
				}
			}
		}
		return (nCount = static_cast<int>(rSearchResult.size()));
	}

public:
	const PackageFileNode* FindFileNodeByAbsPath(const char* pszAbsPath) const
	{
		const PackageFileNode* node;

		// _strlwr
		char szLowerPath[MAX_PATH] = { 0 };
		strncpy(szLowerPath, pszAbsPath, MAX_PATH - 1);

		PathKey pk;
        pk.key = CalcHashCode(_strlwr(szLowerPath));
		pk.key2 = CalcHashCodeReverse(szLowerPath);
			
		return (node = FindFileNodeByPathKey(pk));
	}
	const PackageFileNode* FindFileNodeByPath(const char* pszPath) const
	{
		const PackageFileNode* node;

		// _strlwr
		char szLowerPath[MAX_PATH] = { 0 };
		strncpy(szLowerPath, pszPath, MAX_PATH - 1);
		_strlwr(szLowerPath);

		PathKey pk;
		pk.key = CalcHashCode(_strlwr(szLowerPath));
		pk.key2 = CalcHashCodeReverse(szLowerPath);

		if (NULL != (node = FindFileNodeByPathKey(pk)))
			return node;

		// Not found, match all sub paths
		PackageFileNodeList::const_iterator it = _fileNodeList.begin();
		for (; it != _fileNodeList.end(); it++)
		{
			node = it->second;
			for (int i = 1; i < PACKAGE_PATHKEY_COUNT; i++)
			{
				if ((pk.key == node->pathKeys[i].key) && 
					(pk.key2 == node->pathKeys[i].key2))
				{
					return node;
				}
			}
		}
		return (node = NULL);
	}
	const PackageFileNode* FindFileNodeByPathKey(const PathKey& key) const
	{
		const PackageFileNode* node = NULL;
		int count = static_cast<int>(_fileNodeList.count(key.key));
		if (count > 0)
		{
			PackageFileNodeList::const_iterator it = _fileNodeList.find(key.key);
			for (int i = 0; (i < count && it != _fileNodeList.end()); i++, it++)
			{
				if (it->second->nameKey.key2 == key.key2)
				{
					node =it->second;
					break;
				}
			}
		}		
		return node;
	}

public: 
	static void CalculatePath(const char* pszPath, PATHKEYLIST& pathKeys)
	{
		// Save the 'pszPath' hash and it's reverse hash
		PathKey* pKeys = pathKeys;
		(*pKeys).key = CalcHashCode(pszPath);
		(*pKeys).key2 = CalcHashCodeReverse(pszPath);
		pKeys++;

		// Split the path to get the sub path list
		SUBPATHLIST subpaths = { 0 };
		int count = SplitPath(pszPath, subpaths);
		for (int i = 0; i < count; i++, pKeys++)
		{
			(*pKeys).key = CalcHashCode(subpaths[i]);
			(*pKeys).key2 = CalcHashCodeReverse(subpaths[i]);
			delete []subpaths[i];
		}
	}

	static int SplitPath(const char* path, SUBPATHLIST& subpaths)
	{
		int count = 0;
		char* p = const_cast<char*>(path);
		char* q = (char*)strchr(path, '\\');
		int size = static_cast<int>(strlen(path));
		int len = 0;
		while (q)
		{
			len = static_cast<int>(q - path);
			if (len > 0 && len < size)
			{
				char* p2 = new char[len + 1];
				if (p2)
				{
					strncpy(p2, path, len);
					p2[len] = '\0';
					subpaths[count] = p2;
					if (count++ >= PACKAGE_SUBPATH_COUNT)
						break;
				}				
			}
			p = ++q;
			q = strchr(p, '\\');
		}
		return count;
	}

private:
	const PackageFileNodeList& _fileNodeList;
};
#endif