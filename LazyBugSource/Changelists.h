#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <deque>

#include "FileSystem/IFileSystem.h"  // 添加对IFileSystem.h的包含

#include "timer/wuid.h"
#include "timer/timer.h"


typedef WUID FileChangeListUID;
#define FileChangeListUID_Invalid (0)

struct FileChangelist
{
	FileChangelist()
	{
		uid = FileChangeListUID_Invalid;
		parent = FileChangeListUID_Invalid;
		isCommit = false;
		isFullLoaded = false;
	}
	void LoadBrief(IFile* fl);//载入除content以外的所有内容
	void Load(IFile* fl);
	void Save(IFile* fl) const;

	bool IsFullLoaded()	const {		return isFullLoaded;	}
	void DiscardContent();

	FileChangeListUID uid;
	FileChangeListUID parent;
	bool isFullLoaded;
	bool isCommit;//这个cl是否已apply到base目录
	std::string desc;
	std::vector< FileChange> changes;
};

class CSolutionDB;
class CChangelists
{
public:
	CChangelists()
	{
		Zero();
		_ver = 0;
	}
	void Zero()
	{
		_vcxprojDB = NULL;
		_curUID = FileChangeListUID_Invalid;
	}
	void Init(CSolutionDB *vcxprojDB);
	void Clear();

	DWORD GetVer()
	{
		return _ver;
	}

	FileChangeListUID GetCur()	{		return _curUID;	}

	bool RefreshCur(const std::unordered_set<std::string> &candidateFiles);//在candidateFiles范围内,比较当前workspace下文件和base目录下的文件,生成一个uid为_curUID的change list,
	void RefreshCur();//比较当前workspace下文件和base目录下的文件,生成一个uid为_curUID的change list,
	void CommitCur();//将当前的changelist(_curUID)apply到base目录下

	bool SwitchTo(FileChangeListUID uid);//将当前的changelist切换为某个uid的changelist

	bool HasChild(FileChangeListUID uid) const;

	bool Remove(FileChangeListUID uid);

	void EnsureFullLoaded(FileChangeListUID uid);

private:
	// 应用 changelist 的变更到基准目录
	void _ApplyChangelist(IFileSystem* pFS, const FileChangelist& cl, const std::string& baseFolder);
	
	// 撤销 changelist 的变更到基准目录
	void _UnapplyChangelist(IFileSystem* pFS, const FileChangelist& cl, const std::string& baseFolder);

	void _Load(const char *folder);

	bool _LoadCur();
	void _SaveCur();
	void _SaveChangelist(IFileSystem* pFS, const FileChangelist& cl);
	void _LoadChangelist(IFileSystem* pFS, FileChangeListUID uid, FileChangelist& cl);

	void _BuildCurChangelist(IFileSystem* pFS, std::vector<FileChange>& collectedChanges);

	FileChangelist* _FindChangelist(FileChangeListUID uid);

	CSolutionDB* _vcxprojDB;

	FileChangeListUID _curUID;
	std::unordered_map< FileChangeListUID, FileChangelist> _briefs;//保存FileChangelist的brief部分

	DWORD _ver;//每次内容修改后就加1,永远递增

	friend class CGuiView_Changelists;
	friend class CChangelistsDialog;

};