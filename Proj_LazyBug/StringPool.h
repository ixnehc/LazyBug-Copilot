#pragma once

#include <unordered_map>
#include <vector>
#include <string>
#include <deque>
#include <time.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>

// class IFile;
class CDataPacket;

class CStringPool
{
public:
	CStringPool();
	~CStringPool();

	StringIndex AddStr(const char* str);
	StringIndex FindStr(const char* str) const;
	const char* GetTempStr(StringIndex strIndex) const;//返回的指针不能保证有效(即便返回时也有可能无效),只能用于调试信息
	void GetStr(StringIndex strIndex, std::string& ret) const;
	void GetStrSuffix(StringIndex strIndex, std::string& ret) const;
	void Clear();

	void Save(const char* folderPath);
	void Load(const char* folderPath);

	bool IsDirty() const;

	static const int NUM_BUCKETS = 32;
	static const int BUCKET_INDEX_SHIFT = 24;
	static const int MAX_INDEX_IN_BUCKET = (1 << BUCKET_INDEX_SHIFT) - 1;

private:
	struct Bucket
	{
		mutable std::mutex _mutex;
		std::unordered_map<std::string, int> _strMap;
		std::vector<std::string> _strVec;
		std::atomic<bool> _isDirty;

		Bucket() : _isDirty(false) {}
	};


	Bucket* _buckets;

	size_t GetBucketIndex(const char* str) const;
};
