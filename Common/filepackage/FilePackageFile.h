#ifndef __PackageFile_H__	
#define __PackageFile_H__
#include "FilePackageDefines.h"

class CFilePackage;
class CFilePackageFile
{
public:
	explicit CFilePackageFile(CFilePackage* package, PackageFileNode* node);
	~CFilePackageFile();

public:
	LONG64 Read(void* lpBuf, LONG64 dwSize);
	LONG64 Write(const void* lpBuf, LONG64 dwSize);

	BOOL IsReading();			//Read ?
	BOOL IsWriting();			//Writing?

	const char* GetPath();		//a lower cased path,linked by "\\"
	LONG64 GetSize();			//Only valid when reading
	void Reset();				//Seek to begin
	LONG64 Seek(LONG64 position);//From beginning
	LONG64 GetCurPos();
	void Close();

protected:
	inline void _UpdateSizeToFile(PackageFileNode* node);
	inline BOOL _IsExclusiveWriter() const;

private:
	PackageFileNode* _node;
	CFilePackage* _pPackage;
	LONG64 _curFileCursor;
};
#endif