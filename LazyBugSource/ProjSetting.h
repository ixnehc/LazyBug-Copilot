#pragma once

#include "timer/wuid.h"
#include <mutex>
#include <atomic>
#include <fstream>
#include <vector>
#include <unordered_map>

#include "SolutionDBDefines.h"

class CDataPacket;

class CProjSettingLib
{
private:
	std::string m_fullPath;
	mutable std::mutex m_mutex; // 保护多线程访问
	bool _isDirty; // 标识数据是否被修改

public:
	std::unordered_map< ProjSettingHandle, const ProjSetting*> _uniqueSettings;

	CProjSettingLib() : _isDirty(false) {}
	~CProjSettingLib() { Clear(); }

	void Init(const char *fullPath);
	void Clear();

	void Load();
	void Save();

	// 检查数据是否被修改
	bool IsDirty() const;

	//Add()和Get()要支持多线程访问
	ProjSettingHandle Add(const ProjSetting& setting);//返回的handle可以被永久保存(比如可以被保存在文件里)
	const ProjSetting* Get(ProjSettingHandle);//返回的指针永久有效,不会因为Add()新的setting而失效

private:
	// 序列化到DataPacket
	void SerializeToDataPacket(CDataPacket& dp) const;
	// 从DataPacket反序列化
	void DeserializeFromDataPacket(CDataPacket& dp);
};

