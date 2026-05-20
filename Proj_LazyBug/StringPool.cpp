#include "stdh.h"
#include "StringPool.h"
#include "CppSymbol.h"
#include "stringparser/stringparser.h"
#include "datapacket/DataPacket.h"
#include <functional>
#include <fstream>
#include <filesystem>

#include "Utils_File.h"

CStringPool::CStringPool()
{
	_buckets = new Bucket[NUM_BUCKETS];
}

CStringPool::~CStringPool()
{
	delete[] _buckets;
}

size_t CStringPool::GetBucketIndex(const char* str) const
{
	return std::hash<std::string>{}(str) % NUM_BUCKETS;
}

StringIndex CStringPool::AddStr(const char* str)
{
	if (!str)
	{
		return StringIndex_Null;
	}
	if (str[0] == '\0')
	{
		return StringIndex_Null;
	}

	size_t bucketIndex = GetBucketIndex(str);
	Bucket& bucket = _buckets[bucketIndex];

	// 使用 mutex 保护整个操作
	std::lock_guard<std::mutex> lock(bucket._mutex);
	
	// 检查是否存在
	auto it = bucket._strMap.find(str);
	if (it != bucket._strMap.end())
	{
		return (StringIndex)((bucketIndex << BUCKET_INDEX_SHIFT) | it->second);
	}

	// 插入新字符串
	int indexInBucket = (int)bucket._strVec.size() + 1; // 从1开始，避免返回0
	if (indexInBucket > MAX_INDEX_IN_BUCKET)
	{
		// 索引超出范围
		return StringIndex_Null;
	}
	
	bucket._strVec.push_back(str);
	bucket._strMap[bucket._strVec.back()] = indexInBucket;
	bucket._isDirty = true;

	return (StringIndex)((bucketIndex << BUCKET_INDEX_SHIFT) | indexInBucket);
}

StringIndex CStringPool::FindStr(const char* str) const
{
	if (!str)
	{
		return StringIndex_Null;
	}

	size_t bucketIndex = GetBucketIndex(str);
	const Bucket& bucket = _buckets[bucketIndex];

	std::lock_guard<std::mutex> lock(bucket._mutex);
	auto it = bucket._strMap.find(str);
	if (it != bucket._strMap.end())
	{
		return (StringIndex)((bucketIndex << BUCKET_INDEX_SHIFT) | it->second);
	}

	return StringIndex_Null;
}

void CStringPool::GetStr(StringIndex strIndex,std::string &ret) const
{
	ret = "";
	if (strIndex == StringIndex_Null)
		return;

	size_t bucketIndex = (size_t)(strIndex >> BUCKET_INDEX_SHIFT);
	int indexInBucket = strIndex & MAX_INDEX_IN_BUCKET;

	if (bucketIndex >= NUM_BUCKETS)
		return;

	const Bucket& bucket = _buckets[bucketIndex];
	std::lock_guard<std::mutex> lock(bucket._mutex);

	// indexInBucket从1开始，所以访问数组时需要-1
	int arrayIndex = indexInBucket - 1;
	if (arrayIndex < 0 || arrayIndex >= (int)bucket._strVec.size())
		return;

	ret = bucket._strVec[arrayIndex].c_str();
}

void CStringPool::GetStrSuffix(StringIndex strIndex, std::string& ret) const
{
	ret = "";
	if (strIndex == StringIndex_Null)
		return;

	size_t bucketIndex = (size_t)(strIndex >> BUCKET_INDEX_SHIFT);
	int indexInBucket = strIndex & MAX_INDEX_IN_BUCKET;

	if (bucketIndex >= NUM_BUCKETS)
		return;

	const Bucket& bucket = _buckets[bucketIndex];
	std::lock_guard<std::mutex> lock(bucket._mutex);

	// indexInBucket从1开始，所以访问数组时需要-1
	int arrayIndex = indexInBucket - 1;
	if (arrayIndex < 0 || arrayIndex >= (int)bucket._strVec.size())
		return;

	ret = GetFileSuffix(bucket._strVec[arrayIndex]);
}

const char* CStringPool::GetTempStr(StringIndex strIndex) const
{
	if (strIndex == StringIndex_Null)
	{
		return "";
	}

	size_t bucketIndex = (size_t)(strIndex >> BUCKET_INDEX_SHIFT);
	int indexInBucket = strIndex & MAX_INDEX_IN_BUCKET;

	if (bucketIndex >= NUM_BUCKETS)
	{
		return "";
	}

	const Bucket& bucket = _buckets[bucketIndex];
	std::lock_guard<std::mutex> lock(bucket._mutex);

	// indexInBucket从1开始，所以访问数组时需要-1
	int arrayIndex = indexInBucket - 1;
	if (arrayIndex < 0 || arrayIndex >= (int)bucket._strVec.size())
	{
		return "";
	}

	return bucket._strVec[arrayIndex].c_str();
}



void CStringPool::Clear()
{
	for (int i = 0; i < NUM_BUCKETS; ++i)
	{
		Bucket& bucket = _buckets[i];
		std::lock_guard<std::mutex> lock(bucket._mutex);
		bucket._strMap.clear();
		bucket._strVec.clear();
		bucket._isDirty = false;
	}
}

void CStringPool::Save(const char* folderPath)
{
	for (int i = 0; i < NUM_BUCKETS; ++i)
	{
		Bucket& bucket = _buckets[i];
		if (!bucket._isDirty)
		{
			continue;
		}

		std::lock_guard<std::mutex> lock(bucket._mutex);
		if (!bucket._isDirty) // Double check after lock
		{
			continue;
		}
		
		// 使用CDataPacket序列化bucket数据
		std::vector<BYTE> buf;
		CDataPacket dp;
		DP_BeginSave(dp, buf);
		{
			// 写入字符串数量
			int size = (int)bucket._strVec.size();
			dp.Data_WriteSimple(size);

			// 写入每个字符串
			for (const auto& str : bucket._strVec)
			{
				dp.Data_WriteString(str.c_str());
			}
		}
		DP_EndSave();
		
		// 格式化为三位数，前面补零
		char indexStr[4];
		snprintf(indexStr, sizeof(indexStr), "%03d", i);
		std::string bucketPath = std::string(folderPath) + "\\" + indexStr + ".bucket";

		// 将序列化后的数据写入文件
		std::ofstream ofs;
		Utils::OpenOFStream(ofs,bucketPath.c_str(), std::ios::binary | std::ios::trunc);

		if (ofs.is_open())
		{
			ofs.write((const char*)buf.data(), buf.size());
			ofs.close();
			bucket._isDirty = false;
		}
	}
}

void CStringPool::Load(const char* folderPath)
{
	Clear();

	for (int i = 0; i < NUM_BUCKETS; ++i)
	{
		Bucket& bucket = _buckets[i];
		std::lock_guard<std::mutex> lock(bucket._mutex);

		// 格式化为三位数，前面补零
		char indexStr[4];
		snprintf(indexStr, sizeof(indexStr), "%03d", i);
		std::string bucketPath = std::string(folderPath) + "\\" + indexStr + ".bucket";

		// 读取文件数据
		std::ifstream ifs;
		Utils::OpenIFStream(ifs, bucketPath.c_str());
		if (ifs.is_open())
		{
			// 获取文件大小
			ifs.seekg(0, std::ios::end);
			size_t fileSize = ifs.tellg();
			ifs.seekg(0, std::ios::beg);

			// 读取所有数据到buffer
			std::vector<BYTE> buf(fileSize);
			ifs.read((char*)buf.data(), fileSize);
			ifs.close();

			// 使用CDataPacket反序列化
			CDataPacket dp;
			dp.SetDataBufferPointer(buf.data());
			
			// 读取字符串数量
			int count = 0;
			dp.Data_ReadSimple(count);

			bucket._strVec.resize(count);
			bucket._strMap.reserve(count);

			// 读取每个字符串
			for (int j = 0; j < count; ++j)
			{
				std::string str;
				dp.Data_ReadString(str);
				bucket._strVec[j] = str;
				bucket._strMap[bucket._strVec[j]] = j + 1; // indexInBucket从1开始
			}
		}
		bucket._isDirty = false;
	}
}

bool CStringPool::IsDirty() const
{
	for (int i = 0; i < NUM_BUCKETS; ++i)
	{
		if (_buckets[i]._isDirty)
		{
			return true;
		}
	}
	return false;
}

