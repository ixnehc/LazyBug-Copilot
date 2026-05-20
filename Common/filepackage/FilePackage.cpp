/********************************************************************
	created:	2008/02/19
	created:	19:2:2008   13:33
	filename: 	f:\IxEngine\Common\FilePackage\FilePackage.cpp
	file path:	f:\IxEngine\Common\FilePackage
	file base:	FilePackage
	file ext:	cpp
	author:		szg
	
	purpose:	
*********************************************************************/
#include "stdh.h"
#include "FilePackage.h"
#include "DataStream.h"
#include <assert.h>
#include "../progress/progress.h"

#define BeginProgress(progress) \
	if (progress)\
		progress->SetBegin();

#define SetProgressPos(progress, cur, full) \
	if (progress)\
		progress->SetProgress(NULL, cur, full);

#define EndProgress(progress)\
	if (progress)\
		progress->SetEnd();

CFilePackage::CFilePackage() : _pStream(NULL)
{
	_exclusiveWriter.key = 0;
	_exclusiveWriter.key2 = 0;
	_pSearchFilter = new CFilePackageSearchFilter(_fileNodeList);
}

CFilePackage::~CFilePackage()
{
	Close();

	delete _pSearchFilter;
}

BOOL CFilePackage::Open(const char* pszPackage, CFileStream::OpenMode mode)
{
	if (_pStream || !pszPackage)
		return FALSE;

	char szFilePath[MAX_PATH] = { 0 };
	strcpy(szFilePath, pszPackage);
	strcat(szFilePath, FILEPACKAGEEXT);

	_pStream = new CFileStream();
	if (!_pStream->Open(szFilePath, mode))
	{
		delete _pStream;
		_pStream = NULL;
		return FALSE;
	}

	// Read the package file header
	LONG64 fileSize = _pStream->GetSize();
	if (fileSize < PACKAGE_FILEHEADER_LENGTH)
	{
		if (!_pStream->IsWriting() || (_pStream->IsWriting() && !_CreateFilePackage()))
		{
			Close();
			return FALSE;
		}
	}
	else
	{
		char szFileHeader[PACKAGE_FILEHEADER_LENGTH] = { 0 };
		_pStream->Read(szFileHeader, PACKAGE_FILEHEADER_LENGTH);

		CInDataStream in(szFileHeader, PACKAGE_FILEHEADER_LENGTH);
		in >> _fileHeader.type;
		in >> _fileHeader.ver;
		in >> _fileHeader.first;
		in >> _fileHeader.last;

		// Check file
		if ((_fileHeader.type != ('p' | 'c' << 8)) || (_fileHeader.ver != PACKAGE_VERSION))
		{
			Close();
			return FALSE;
		}
	}
	
	char szFileNode[PACKAGE_FILENODE_LENGTH] = { 0 };
	CInDataStream in(szFileNode, PACKAGE_FILENODE_LENGTH);

	int nCount = 0;
	LONG64 bytesRead;
	LONG64 pos = _fileHeader.first;
	while (pos != PACKAGE_INVALID_POSITION)
	{
		// Read node data
		_pStream->Seek(pos);
		
		bytesRead = _pStream->Read(szFileNode, PACKAGE_FILENODE_LENGTH);
		if (bytesRead != PACKAGE_FILENODE_LENGTH)
			break;
		
		PackageFileNode* node = new PackageFileNode;
		assert( node );

		in.SetData(szFileNode, PACKAGE_FILENODE_LENGTH);
		in >> node->pos;
		in >> node->front;
		in >> node->next;
		in >> node->size;
		in.Read(node->name, MAX_PATH * sizeof(char));
		node->name[MAX_PATH - 1] = '\0';
		in.Read(node->pathKeys, sizeof(PATHKEYLIST));

		// Save
		_fileNodeList.insert(PathKeyFileNodePair(node->nameKey.key, node));

		// Read the next node
		pos = node->next;
	}
	return TRUE;
}

void CFilePackage::Close()
{
	if (_pStream)
	{
		if (_pStream->IsOpen())
		{
			if (_pStream->IsWriting())
			{
				_pStream->Seek(2 * sizeof(USHORT));

				COutDataStream out;
				out << _fileHeader.first << _fileHeader.last;
				_pStream->Write(out.GetData(), out.GetSize());
			}
			_pStream->Close();
		}
		
		delete _pStream;
		_pStream = NULL;

		PackageFileNodeList::const_iterator it = _fileNodeList.begin();
		for (; it != _fileNodeList.end(); it++)
		{
			delete it->second;
		}
		_fileNodeList.clear();

		_exclusiveWriter.key = 0;
	}
}

CFilePackageFile* CFilePackage::CreateFile(const char* pszPath)
{
	CFilePackageFile* pFile = NULL;

	if (IsExclusive() || !pszPath || (!(_pStream && _pStream->IsWriting())))
		return NULL;

	PackageFileNode* node = _FindFileNodeByAbsPath(pszPath);
	if (node)
		return FALSE;

	node = new PackageFileNode;
	node->front = PACKAGE_INVALID_POSITION;
	node->next = PACKAGE_INVALID_POSITION;
	node->pos = _pStream->GetSize();

	// Get the last file node
	PackageFileNode* lastNode = _FindFileNodeByPosition(_fileHeader.last);
	if (lastNode)
	{
		lastNode->next = node->pos;
		// Update
		_pStream->Seek(lastNode->pos);
		_pStream->Skip(2 * sizeof(LONG64));	// pos front 'next'
		_pStream->Write(&lastNode->next, sizeof(LONG64));

		node->front = lastNode->pos;			
	}
	char szLowerFilePath[MAX_PATH] = { 0 };
	strncpy(szLowerFilePath, pszPath, MAX_PATH - 1);
	strcpy(node->name, _strlwr(szLowerFilePath));
	node->size = 0;

	memset(node->pathKeys, 0, sizeof(PATHKEYLIST));
	_pSearchFilter->CalculatePath(node->name, node->pathKeys);

	COutDataStream out;
	out << node->pos << node->front << node->next;
	out << node->size;
	out.Write(node->name, MAX_PATH * sizeof(char));
	out.Write(node->pathKeys, sizeof(PATHKEYLIST));
	_pStream->Append(out.GetData(), out.GetSize());

	if (_fileHeader.first == PACKAGE_INVALID_POSITION)
		_fileHeader.first = node->pos;
	_fileHeader.last = node->pos;

	_fileNodeList.insert(PathKeyFileNodePair(node->nameKey.key, node));

	pFile = new CFilePackageFile(this, node);
	if (pFile)
		_openFileList.push_back(pFile);
	
	return pFile;
}

CFilePackageFile* CFilePackage::OpenFile(const char* pszPath)
{
	CFilePackageFile* pFile = NULL;
	PackageFileNode* node = _FindFileNodeByAbsPath(pszPath);
	if (node)
	{
		pFile = new CFilePackageFile(this, node);
		if (pFile)
			_openFileList.push_back(pFile);
	}
	return pFile;
}

void CFilePackage::CloseFile(CFilePackageFile* pFile)
{
	std::vector<CFilePackageFile*>::iterator it = _openFileList.begin();
	for (; it != _openFileList.end(); it++)
	{
		if (*it == pFile)
		{
			delete (*it);
			_openFileList.erase(it);
			break;
		}		
	}
}

BOOL CFilePackage::FileExists(const char* pszPath) const
{
	return (NULL != _FindFileNodeByPath(pszPath));
}

BOOL CFilePackage::GetFileSize(const char* pszPath,DWORD &sz) const
{
	PackageFileNode* node = _FindFileNodeByPath(pszPath);
	if (!node)
		return FALSE;
	sz=(DWORD)node->size;
	return TRUE;
}


int CFilePackage::SearchFiles(const char* pszPath, FilePathList& rSearchResult) const
{
	return ((pszPath == NULL) ? 0 : _pSearchFilter->SearchMatchPaths(pszPath, rSearchResult));
}

BOOL CFilePackage::AddFile(const char* pszFileName, const char* pszPath)
{
	CFileStream fi;
	if (!fi.Open(pszFileName, CFileStream::OM_READ))
		return FALSE;

	CFilePackageFile* pFile = CreateFile(pszPath);
	if (!pFile)
	{
		fi.Close();
		return FALSE;
	}
	
	char* pBuffer = new char[MAX_BUFFER_LENGTH];
	LONG64 bytesRead;
	while (!fi.Eof())
	{
		bytesRead = fi.Read(pBuffer, MAX_BUFFER_LENGTH);
		pFile->Write(pBuffer, bytesRead);
	}
	fi.Close();
	delete []pBuffer;

	CloseFile(pFile);

	return TRUE;
}

BOOL CFilePackage::RemoveFile(const char* pszPath)
{
	if (!pszPath || (!(_pStream && _pStream->IsWriting())))
		return FALSE;

	PackageFileNode* node = _FindFileNodeByAbsPath(pszPath);
	if (!node)
		return FALSE;
	
	PackageFileNode* frontNode = _FindFileNodeByPosition(node->front);
	PackageFileNode* nextNode = _FindFileNodeByPosition(node->next);

	if (frontNode)
	{
		frontNode->next = node->next;

		// Modify the package source file
		_pStream->Seek(frontNode->pos);
		_pStream->Skip(2 * sizeof(LONG64));
		_pStream->Write(&frontNode->next, sizeof(LONG64));
	}
	if (nextNode)
	{
		nextNode->front = node->front;

		// Modify the package source file
		_pStream->Seek(nextNode->pos);
		_pStream->Skip(sizeof(LONG64));
		_pStream->Write(&nextNode->front, sizeof(LONG64));
	}

	if (frontNode == NULL)
		_fileHeader.first = node->next;
	if (nextNode == NULL)
		_fileHeader.last = node->front;

	_RemoveFileNodeByPathKey(node->nameKey);

	return TRUE;	
}

BOOL CFilePackage::ReplaceFile(const char* pszPath, const char* pszFileName)
{
	if (!pszPath || (!(_pStream && _pStream->IsWriting())))
		return FALSE;

	PackageFileNode* node = _FindFileNodeByAbsPath(pszPath);
	if (!node)
		return FALSE;

	CFileStream fi;
	if (!fi.Open(pszFileName, CFileStream::OM_READ))
		return FALSE;

	LONG64 fileSize = fi.GetSize();
	if (node->size > fileSize)
	{
		RemoveFile(pszPath);
		return AddFile(pszFileName, pszPath);
	}

	node->size = fileSize;

	// Update the package source file
	_pStream->Seek(node->pos);
	_pStream->Skip(3 * sizeof(LONG64));			// pos front next 'size'
	_pStream->Write(&node->size, sizeof(LONG64));
	_pStream->Skip(MAX_PATH * sizeof(char));	// skip 'name'

	char* pBuffer = new char[MAX_BUFFER_LENGTH];
	LONG64 bytesRead;
	while (!fi.Eof())
	{
		bytesRead = fi.Read(pBuffer, MAX_BUFFER_LENGTH);
		_pStream->Write(pBuffer, bytesRead);
	}
	fi.Close();
	delete []pBuffer;

	return TRUE;
}

PackageFileNode* CFilePackage::_FindFileNodeByAbsPath(const char* pszAbsPath) const
{
	return const_cast<PackageFileNode*>(_pSearchFilter->FindFileNodeByAbsPath(pszAbsPath));
}

PackageFileNode* CFilePackage::_FindFileNodeByPath(const char* pszPath) const
{
	return const_cast<PackageFileNode*>(_pSearchFilter->FindFileNodeByPath(pszPath));
}

PackageFileNode* CFilePackage::_FindFileNodeByPosition(LONG64 dwPosition) const
{
	PackageFileNode* node = NULL;
	PackageFileNodeList::const_iterator it = _fileNodeList.begin();
	for (; it != _fileNodeList.end(); it++)
	{
		if (it->second->pos == dwPosition)
		{
			node = it->second;
			break;
		}
	}
	return node;
}

PackageFileNode* CFilePackage::_FindFileNodeByPathKey(const PathKey& key)
{
	return const_cast<PackageFileNode*>(_pSearchFilter->FindFileNodeByPathKey(key));
}

void CFilePackage::_RemoveFileNodeByPathKey(const PathKey& key, BOOL bRelease)
{
	int count = static_cast<int>(_fileNodeList.count(key.key));
	if (count > 0)
	{
		PackageFileNodeList::iterator it = _fileNodeList.find(key.key);
		for (int i = 0; (i < count && it != _fileNodeList.end()); i++, it++)
		{
			if (it->second->nameKey.key2 = key.key2)
			{
				if (bRelease)
					delete it->second;
				_fileNodeList.erase(it);
				break;
			}
		}
	}	
}

void CFilePackage::_AddFileNode(PackageFileNode* node)
{
	PackageFileNode* p = _FindFileNodeByPathKey(node->nameKey);
	if (!p)
	{
		_fileNodeList.insert(PathKeyFileNodePair(node->nameKey.key, node));
	}
}

BOOL CFilePackage::_RemoveFile(PackageFileNode* node)
{
	if (!(_pStream && _pStream->IsWriting()))
		return FALSE;

	PackageFileNode* frontNode = _FindFileNodeByPosition(node->front);
	PackageFileNode* nextNode = _FindFileNodeByPosition(node->next);

	if (frontNode)
	{
		frontNode->next = node->next;

		// Modify the package source file
		_pStream->Seek(frontNode->pos);
		_pStream->Skip(2 * sizeof(LONG64));
		_pStream->Write(&frontNode->next, sizeof(LONG64));
	}
	if (nextNode)
	{
		nextNode->front = node->front;

		// Modify the package source file
		_pStream->Seek(nextNode->pos);
		_pStream->Skip(sizeof(LONG64));
		_pStream->Write(&nextNode->front, sizeof(LONG64));
	}

	_RemoveFileNodeByPathKey(node->nameKey);

	if (frontNode == NULL)
		_fileHeader.first = node->next;
	if (nextNode == NULL)
		_fileHeader.last = PACKAGE_INVALID_POSITION;

	return TRUE;	
}

inline BOOL CFilePackage::_CreateFilePackage()
{
	_fileHeader.type = 'p' | 'c' << 8;
	_fileHeader.ver = PACKAGE_VERSION;
	_fileHeader.first = PACKAGE_INVALID_POSITION;
	_fileHeader.last = PACKAGE_INVALID_POSITION;

	COutDataStream out;
	out << _fileHeader.type;
	out << _fileHeader.ver;
	out << _fileHeader.first;
	out << _fileHeader.last;
	return (PACKAGE_FILEHEADER_LENGTH == _pStream->Append(out.GetData(), out.GetSize()));
}

LONG64 CFilePackage::_GetActualSize() const
{
	LONG64 liPackageSize = PACKAGE_FILEHEADER_LENGTH;
	int nCount = static_cast<int>(_fileNodeList.size());
	PackageFileNodeList::const_iterator it = _fileNodeList.begin();
	for (; it != _fileNodeList.end(); it++)
	{
		liPackageSize += PACKAGE_FILENODE_LENGTH;
		liPackageSize += it->second->size;
	}
	return liPackageSize;
}

BOOL CFilePackage::IsFilePackage(const char* pszPackage)
{
	char szFilePath[MAX_PATH] = { 0 };
	if (pszPackage)
	{
		strncpy(szFilePath, pszPackage, MAX_PATH - 1);
		strcat(szFilePath, FILEPACKAGEEXT);
	}
	return CFileStream::FileExists(szFilePath);
}

BOOL CFilePackage::CleanupPackage(const char* pszPackage, CProgress* progress)
{
	const LONG64 ALLOWABLE_SIZE		= 64 * 1024 * 1024;
	const int MAX_PACKAGEFILE_SIZE	= 128 * 1024 * 1024;

	BeginProgress(progress);

	CFilePackage pack;
	if (!pack.Open(pszPackage, CFileStream::OM_READ_AND_WRITE) || 
		((pack._GetActualSize() + ALLOWABLE_SIZE) >= pack._pStream->GetSize()))
	{
		EndProgress(progress);
		return FALSE;
	}

	// Sort the file node by position in package
	std::map<LONG64, PackageFileNode*> fileNodeList;
	PackageFileNodeList::const_iterator it = pack._fileNodeList.begin();
	for (; it != pack._fileNodeList.end(); it++)
	{
		fileNodeList[it->second->pos] = it->second;
	}

	LONG64 nextPos = PACKAGE_INVALID_POSITION;
	LONG64 writePos = PACKAGE_FILEHEADER_LENGTH;
	LONG64 readPos;
	LONG64 bytesRead;
	LONG64 bytesWritten;

	COutDataStream out;
	std::vector<char> vecBuffer(MAX_PACKAGEFILE_SIZE);

	int nCount = static_cast<int>(fileNodeList.size());
	int nCountAdd = 0;

	char szTemp[1024] = { 0 };

	PackageFileNode* node;
	PackageFileNode* frontNode;
	PackageFileNode* nextNode;
	std::map<LONG64, PackageFileNode*>::iterator itFront;
	std::map<LONG64, PackageFileNode*>::iterator itNext;
	std::map<LONG64, PackageFileNode*>::iterator itNode = fileNodeList.begin();
	for (; itNode != fileNodeList.end(); itNode++)
	{
		node = itNode->second;

		if (writePos == node->pos)
		{
			writePos += PACKAGE_FILENODE_LENGTH;
			writePos += node->size;
			++nCountAdd;
			SetProgressPos(progress, nCountAdd, nCount);
			continue;
		}

		// Allocate memory
		vecBuffer.resize((size_t) node->size);

		// Read the file data
		readPos = node->pos;
		readPos += PACKAGE_FILENODE_LENGTH;
		pack._pStream->Seek(readPos);
		bytesRead = pack._pStream->Read(vecBuffer.data(), node->size);
		if (bytesRead != node->size)
			break;

		// Write the file node data
		out.Clear();
		out << writePos << node->front << node->next;
		out << node->size;
		out.Write(node->name, MAX_PATH * sizeof(char));
		out.Write(node->pathKeys, sizeof(PATHKEYLIST));
		pack._pStream->Seek(writePos);
		bytesWritten = pack._pStream->Write(out.GetData(), out.GetSize());
		if (bytesWritten != PACKAGE_FILENODE_LENGTH)
			break;

		// Modify itself node data
		node->pos = writePos;

		// Write the file data
		bytesWritten = pack._pStream->Write(vecBuffer.data(), bytesRead);
		if (bytesWritten != bytesRead)
			break;

		// next->front = pos;
		itNext = itNode;
		if (++itNext != fileNodeList.end())
		{
			nextNode = itNext->second;

			itNext->second->front = node->pos;

			pack._pStream->Seek(itNext->second->pos);
			pack._pStream->Skip(sizeof(LONG64));
			pack._pStream->Write(&itNext->second->front, sizeof(LONG64));
		}		

		// front->next = pos;
		itFront = itNode;
		if (itFront != fileNodeList.begin() && (--itFront != fileNodeList.end()))
		{
			frontNode = itFront->second;

			itFront->second->next = node->pos;

			pack._pStream->Seek(itFront->second->pos);
			pack._pStream->Skip(2 * sizeof(LONG64));
			pack._pStream->Write(&itFront->second->next, sizeof(LONG64));
		}	

		writePos += PACKAGE_FILENODE_LENGTH;
		writePos += node->size;		
		++nCountAdd;

		// Modify the file header 'first' field
		if (nCountAdd == 1)
		{
			pack._fileHeader.first = PACKAGE_FILEHEADER_LENGTH;

			pack._pStream->Seek(2 * sizeof(USHORT));
			pack._pStream->Write(&pack._fileHeader.first, sizeof(LONG64));
		}

		SetProgressPos(progress, nCountAdd, nCount);
	}
	// Modify the file header 'last' field
	if ((nCountAdd == nCount) && (--itNode != fileNodeList.end()))
	{
		pack._fileHeader.last = itNode->second->pos;
		pack._pStream->Seek(2 * sizeof(USHORT) + sizeof(LONG64));
		pack._pStream->Write(&pack._fileHeader.last, sizeof(LONG64));

		pack._pStream->Close();

		// Reset the file size		
		char szFilePath[MAX_PATH] = { 0 };
		strcpy(szFilePath, pszPackage);
		strcat(szFilePath, FILEPACKAGEEXT);
		CFileStream::SetFileSize(szFilePath, writePos);
	}

	EndProgress(progress);

	return TRUE;
}