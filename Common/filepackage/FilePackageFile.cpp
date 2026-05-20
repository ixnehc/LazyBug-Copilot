/********************************************************************
	created:	2008/02/19
	created:	19:2:2008   13:35
	filename: 	f:\IxEngine\Common\FilePackage\FilePackageFile.cpp
	file path:	f:\IxEngine\Common\FilePackage
	file base:	FilePackageFile
	file ext:	cpp
	author:		szg
	
	purpose:	
*********************************************************************/
#include "stdh.h"
#include "FilePackageFile.h"
#include "FilePackage.h"

CFilePackageFile::CFilePackageFile(CFilePackage* package, PackageFileNode* node) 
	: _pPackage(package), _node(node), _curFileCursor(0)
{
}

CFilePackageFile::~CFilePackageFile()
{
}

LONG64 CFilePackageFile::Read(void* lpBuf, LONG64 size)
{
	LONG64 bytesRead = 0;
	if (_node && (_curFileCursor < _node->size))
	{
		LONG64 leftSize = _node->size - _curFileCursor;
		if (size <= leftSize)
		{
			LONG64 pos = _node->pos + PACKAGE_FILENODE_LENGTH + _curFileCursor;
			_pPackage->_pStream->Seek(pos);
			bytesRead = _pPackage->_pStream->Read(lpBuf, size);

			_curFileCursor += bytesRead;
		}
	}
	return bytesRead;
}

LONG64 CFilePackageFile::Write(const void* lpBuf, LONG64 size)
{
	LONG64 bytesWritten = 0;
	if (!_node || !_pPackage->_pStream->IsWriting())
		return bytesWritten;

	if (_pPackage->IsExclusive() && !_IsExclusiveWriter())
	{
		return bytesWritten;
	}

	LONG64 pos;
	if ((size + _curFileCursor) <= _node->size)
	{
		pos = _node->pos + PACKAGE_FILENODE_LENGTH + _curFileCursor;
		_pPackage->_pStream->Seek(pos);
		bytesWritten = _pPackage->_pStream->Write(lpBuf, size);

		_curFileCursor += bytesWritten;
	}
	else
	{
		if (_pPackage->_IsLastFile(_node))
		{
			pos = _node->pos + PACKAGE_FILENODE_LENGTH + _curFileCursor;
			_pPackage->_pStream->Seek(pos);
			bytesWritten = _pPackage->_pStream->Append(lpBuf, size);

			_node->size += bytesWritten;

			// Update the file size
			_UpdateSizeToFile(_node);

			// at the file end
			_curFileCursor = _node->size;
		}
		else
		{
			// Remove the node from the list and not release it, so it can't be found.
			_pPackage->_RemoveFileNodeByPathKey(_node->nameKey, FALSE);

			// Create a new file at the package end
			CFilePackageFile* pFile = _pPackage->CreateFile(_node->name);
			if (!pFile)
			{
				// Failed to create a new package file.
				_pPackage->_AddFileNode(_node);
				return bytesWritten;
			}

			// Important
			_pPackage->_exclusiveWriter = pFile->_node->nameKey;

			// Copy the source file data
			char* pBuffer = new char[MAX_BUFFER_LENGTH];
			
			// From the beginning
			LONG64 dwTail = _node->pos + PACKAGE_FILENODE_LENGTH + _node->size;
			LONG64 bytesRead = 0;
			pos = _node->pos + PACKAGE_FILENODE_LENGTH;
			while (pos < dwTail)
			{
				_pPackage->_pStream->Seek(pos);
				bytesRead = _pPackage->_pStream->Read(pBuffer, size);
				pFile->Write(pBuffer, bytesRead);
				pos += bytesRead;
			}

			delete pBuffer;

			// Write data
			bytesWritten = pFile->Write(lpBuf, size);

			// Release the PackageFileNode instance
			delete _node;

			// Store the new path key
			_node = pFile->_node;
			
			// Close the new open package file
			_pPackage->CloseFile(pFile);			
		}
	}

	return bytesWritten;
}

BOOL CFilePackageFile::IsReading()
{
	return _pPackage->_pStream->IsReading();
}

BOOL CFilePackageFile::IsWriting()
{
	return _pPackage->_pStream->IsWriting();
}

const char* CFilePackageFile::GetPath()
{
	return (_node ? _node->name : NULL);
}

LONG64 CFilePackageFile::GetSize()
{
	return (_node ? _node->size : 0);
}

void CFilePackageFile::Reset()
{
	_curFileCursor = 0;
}

LONG64 CFilePackageFile::Seek(LONG64 position)
{
	LONG64 oldPosition = _curFileCursor;
	if (_node && (position <= _node->size))
	{
		_curFileCursor = position;
	}
	return oldPosition;
}

LONG64 CFilePackageFile::GetCurPos()
{
	return _curFileCursor;
}


void CFilePackageFile::Close()
{
	if (_IsExclusiveWriter())
	{
		_pPackage->_exclusiveWriter.key = 0;
		_pPackage->_exclusiveWriter.key2 = 0;
	}

	_curFileCursor = 0;
	_node = NULL;
}

BOOL CFilePackageFile::_IsExclusiveWriter() const
{
	return ( _node && 
			(_pPackage->_exclusiveWriter.key == _node->nameKey.key) && 
			(_pPackage->_exclusiveWriter.key2 == _node->nameKey.key2) );
}

inline void CFilePackageFile::_UpdateSizeToFile(PackageFileNode* node)
{
	LONG64 pos = node->pos + 3 * sizeof(LONG64);	//pos front next 'size' name
	_pPackage->_pStream->Seek(pos);
	_pPackage->_pStream->Write(&node->size, sizeof(LONG64));
}