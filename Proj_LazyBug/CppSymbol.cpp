#include "stdh.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <memory>
#include <cctype>
#include <mutex>

#include "Utils.h"
#include "CppSymbol.h"
#include "SolutionDB.h"
#include "datapacket/DataPacket.h"
#include "stringparser/stringparser.h"

#include <sys/stat.h>
#include <string>


CppSymbol_Begin

//////////////////////////////////////////////////////////////////////////
//CSymbolDefine

// 添加 CSymbolDefine 的序列化方法
void CSymbolDefine::Save(CDataPacket& dp) const
{
	// 保存symbol类型
	int cat = (int)_kind;
	dp.Data_WriteSimple(cat);

	// 保存名称
	dp.Data_WriteSimple(_name);

	// 保存位置信息
	SaveFileLocation(_location, dp);

	// 保存代码行范围
	dp.Data_WriteSimple(_lineRange);
}

void CSymbolDefine::Load(CDataPacket& dp, CStringPool& strPool)
{
	// 加载symbol类型
	int cat;
	dp.Data_ReadSimple(cat);
	_kind = (SymbolKind)cat;

	// 加载名称
	dp.Data_ReadSimple(_name);
	
	// 更新字符串指针
	strName = strPool.GetTempStr(_name);

	// 加载位置信息
	LoadFileLocation(_location, dp, strPool);

	// 加载代码行范围
	dp.Data_ReadSimple(_lineRange);
}

//////////////////////////////////////////////////////////////////////////
//CFileSymbolDefines
void CFileSymbolDefines::Save(CDataPacket& dp) const
{
	// 保存文件路径索引
	dp.Data_WriteSimple(_filePath);

	// 保存文件修改时间
	dp.Data_WriteSimple(_parsedTime);

	//作为主文件parse的版本号
	dp.Data_WriteSimple(_masterFileParseVer);

	// setting
	dp.Data_WriteSimple(_setting);

	// 保存symbol定义列表大小
	if(true)
	{
		int size = (int)_parsedDefines.size();
		dp.Data_WriteSimple(size);
		
		// 保存每个symbol定义
		for (const CSymbolDefine& define : _parsedDefines)
		{
			define.Save(dp);
		}
	}

	if (true)
	{
		int size=_parsedInclusions.size();
		dp.Data_WriteSimple(size);
		for(auto& inclusion:_parsedInclusions)
		{
			dp.Data_WriteSimple(inclusion.path);
			dp.Data_WriteSimple(inclusion.time);
		}
	}

	//注意目前我们不保存	_parsedRefs
	//

	if (true)
	{
		int size = _masterIncluders.size();
		dp.Data_WriteSimple(size);
		for (auto& includer : _masterIncluders)
		{
			dp.Data_WriteSimple(includer.first);
			dp.Data_WriteSimple(includer.second.parseVer);
		}
	}

	// 保存文件修改时间
	dp.Data_WriteSimple(_pchTime);
}

void CFileSymbolDefines::Load(CDataPacket& dp, CStringPool& strPool)
{
	// 加载文件路径索引
	dp.Data_ReadSimple(_filePath);
	_filePathStr = strPool.GetTempStr(_filePath);

	// 加载文件修改时间
	dp.Data_ReadSimple(_parsedTime);

	//作为主文件parse的版本号
	dp.Data_ReadSimple(_masterFileParseVer);

	// setting
	dp.Data_ReadSimple(_setting);

	// 加载symbol定义列表大小
	if(true)
	{
		int definesCount;
		dp.Data_ReadSimple(definesCount);
		_parsedDefines.resize(definesCount);
		
		// 加载每个symbol定义
		for (CSymbolDefine& define : _parsedDefines)
		{
			define.Load(dp, strPool);
		}
	}

	if(true)
	{
		int inclusionsCount;
		dp.Data_ReadSimple(inclusionsCount);
		_parsedInclusions.resize(inclusionsCount);
		for(auto& inclusion:_parsedInclusions)
		{
			dp.Data_ReadSimple(inclusion.path);
			inclusion.strPath = strPool.GetTempStr(inclusion.path);
			dp.Data_ReadSimple(inclusion.time);
		}
	}

	if (true)
	{
		int count;
		dp.Data_ReadSimple(count);
		for (int i = 0;i < count;i++)
		{
			StringIndex path;
			dp.Data_ReadSimple(path);
			MasterIncluder includer;
			includer.strPath = strPool.GetTempStr(path);
			dp.Data_ReadSimple(includer.parseVer);
			_masterIncluders[path] = includer;
		}
	}

	dp.Data_ReadSimple(_pchTime);

}

//////////////////////////////////////////////////////////////////////////
//CSymbolFiles
int CSymbolFiles::BucketFromFilePath(StringIndex filePath)
{
	return (filePath>>CStringPool::BUCKET_INDEX_SHIFT) % ARRAY_SIZE(_buckets);
}

void CSymbolFiles::SetDirty(StringIndex filePath)
{
	int bucket = BucketFromFilePath(filePath);
	_buckets[bucket].isDirty = true;
}

void CSymbolFiles::SetAllDirty()
{
	for (int i = 0;i < CStringPool::NUM_BUCKETS;i++)
		_buckets[i].isDirty = true;
}

void CSymbolFiles::_ClearAllBuckets()
{
	for (int i = 0; i < ARRAY_SIZE(_buckets); i++)
	{
		// 构造bucket文件路径
		char bucketFileName[32];
		snprintf(bucketFileName, sizeof(bucketFileName), "bucket_%02d.dat", i);
		std::string bucketPath = _folderPath + "\\_defines\\" + bucketFileName;
		
		// 删除文件（如果存在）
		if (Utils::IsFileExist(bucketPath.c_str()))
		{
			Utils::RemoveFile(bucketPath.c_str());
		}
		
		// 重置dirty标志
		_buckets[i].isDirty = false;
	}
}

void CSymbolFiles::_SaveVersion()
{
	std::string versionPath = _folderPath + "\\_defines\\ver.txt";
	
	// 写入版本号
	char versionStr[32];
	snprintf(versionStr, sizeof(versionStr), "%d", SYMBOL_FILES_VERSION);
	Utils::SaveFileContent(versionPath.c_str(), versionStr);
}

bool CSymbolFiles::VerifyVersion()
{
	std::string versionPath = _folderPath + "\\_defines\\ver.txt";
	
	int fileVersion = 0;
	std::string content;
	
	// 尝试读取版本号文件
	if (Utils::LoadFileContent(versionPath.c_str(), content))
	{
		// 解析版本号
		fileVersion = atoi(content.c_str());
	}
	
	// 如果读取失败或版本号不匹配，清除所有bucket文件
	if (fileVersion != SYMBOL_FILES_VERSION)
	{
		_ClearAllBuckets();

		if (true)
		{
			std::string path = _folderPath + "\\_defines\\.idx";

			if (Utils::IsFileExist(path.c_str()))
				Utils::RemoveFile(path.c_str());
		}

		return false;
	}
	
	return true;
}

void CSymbolFiles::Save(std::unordered_map < StringIndex, CFileSymbolDefines>& defines)
{
	// 为每个bucket收集CFileSymbolDefines
	std::vector<std::vector<CFileSymbolDefines*>> bucketDefines(ARRAY_SIZE(_buckets));
	
	// 遍历所有defines，将它们分配到对应的bucket中
	for (auto& pair : defines)
	{
		StringIndex fileIndex = pair.first;
		CFileSymbolDefines& fileDefines = pair.second;
		int bucketIndex = BucketFromFilePath(fileIndex);
		
		// 如果该bucket是dirty的，就收集这个defines
		if (_buckets[bucketIndex].isDirty)
		{
			bucketDefines[bucketIndex].push_back(&fileDefines);
		}
	}
	
	// 逐个处理每个dirty bucket
	for (int i = 0; i < ARRAY_SIZE(_buckets); i++)
	{
		if (!_buckets[i].isDirty || bucketDefines[i].empty())
			continue;

		std::vector<BYTE> buf;
		DP_BeginSave(dp, buf);
		{
			// 先写入该bucket中文件的个数
			int count = (int)bucketDefines[i].size();
			dp.Data_WriteSimple(count);

			// 再写入各个文件内容
			for (CFileSymbolDefines* fileDefines : bucketDefines[i])
				fileDefines->Save(dp);
		}
		DP_EndSave()

		// 构造bucket文件路径
		char bucketFileName[32];
		snprintf(bucketFileName, sizeof(bucketFileName), "bucket_%02d.dat", i);
		std::string bucketPath = _folderPath + "\\_defines\\" + bucketFileName;
		
		// 写入文件
		Utils::SaveFileContent(bucketPath.c_str(), buf);
		
		_buckets[i].isDirty = false;
	}
	
	// 保存版本号
	_SaveVersion();
}

void CSymbolFiles::Load(std::unordered_map < StringIndex, CFileSymbolDefines>& defines, CStringPool& strPool)
{
	// 遍历所有可能的bucket文件
	for (int i = 0; i < ARRAY_SIZE(_buckets); i++)
	{
		// 构造bucket文件路径
		char bucketFileName[32];
		snprintf(bucketFileName, sizeof(bucketFileName), "bucket_%02d.dat", i);
		std::string bucketPath = _folderPath + "\\_defines\\" + bucketFileName;

		// 检查文件是否存在
		if (!Utils::IsFileExist(bucketPath.c_str()))
			continue;
			
		std::vector<BYTE> buf;
		// 读取文件
		if (!Utils::LoadFileContent(bucketPath.c_str(), buf))
			continue;

		if (true)
		{
			CDataPacket dp;
			dp.SetDataBufferPointer(buf.data());

			int count;
			dp.Data_ReadSimple(count);

			// 读取指定数量的CFileSymbolDefines
			for (int j = 0; j < count; j++)
			{
				CFileSymbolDefines tempDefines;
				tempDefines.Load(dp, strPool);

				// 将加载的defines添加到map中
				defines[tempDefines._filePath] = std::move(tempDefines);
			}
		}
		_buckets[i].isDirty = false;
	}
}

//////////////////////////////////////////////////////////////////////////
//CSymbolDB

void CSymbolDB::_LoadStrLib()
{
	std::string strlibPath = _folderPath + "\\_strlib";
	_strPool.Load(strlibPath.c_str());
}

bool CSymbolDB::_SaveStrLib()
{
	std::string strlibPath = _folderPath + "\\_strlib";
	_strPool.Save(strlibPath.c_str());
	return true;
}

void CSymbolDB::_InitParser()
{
	_parser.Init(10, ThreadPriority::LOWEST);  // 异步模式下初始化解析池
	_parserForPCH.Init(10, ThreadPriority::LOWEST);  // 异步模式下初始化解析池
	_parserForCollectRef.Init(4, ThreadPriority::HIGHEST);
}

void CSymbolDB::_ClearParser()
{
	_parser.Close();
	_parserForPCH.Close();
	_parserForCollectRef.Close();
}

void CSymbolDB::Init(const char* folderPath, CProjSettingLib& projSettingLib)
{
	_projSettingLib = &projSettingLib;
	_folderPath= folderPath;
	_initTime = GetAbsTick();
	_files.Init(folderPath);

	_debugTotalParsed = 0;
	_debugLastParsedTime = 0;

	if (true)
	{
		std::string path = _folderPath + "\\_log\\SymbolDB.txt";
		Utils::RemoveFile(path.c_str());
		_logFile.Create(path.c_str());
	}

	if (true)
	{
		std::unique_lock< std::shared_mutex> lock(_symbolMutex);
		_symbolIndexer.Init();
	}
}

void CSymbolDB::Clear()
{
	_ClearParser();
	_StopUpdateThread();

	Save();

	_strPool.Clear();

	if (true)
	{
		std::unique_lock< std::shared_mutex> lock(_symbolMutex);
		_fileSymbolDefines.clear();
		_symbolIndexer.Clear();
	}

	_projSettingLib = nullptr;
}

bool CSymbolDB::_Parse(ParseRequest &request)
{
	if (request.NeedGenPCH())
	{
		_parser.DiscardAll(_GenRequestId());
		_parserForPCH.Request(request);
	}
	else
	{
		const int maxActives = 16;
		if (_parser.GetActiveCount() >= maxActives)
			return false;

		_parser.Request(request);
	}
	return true;
}

bool CSymbolDB::_Parse(CFileSymbolDefines& defines)
{
	if (defines._isParsing)
		return false;
	 
	ParseRequest request;
	 _strPool.GetStr(defines._filePath, request.lowerCasedParseFilePath);

// 	 if (!request.lowerCasedParseFilePath.empty())
// 	 {
// 		 std::string lowerCasedFolderPath = _folderPath;
// 		 StringLower(lowerCasedFolderPath);
// 		 lowerCasedFolderPath = GetFileTitle(lowerCasedFolderPath);
// 		 if (request.lowerCasedParseFilePath.c_str()[0] != lowerCasedFolderPath.c_str()[0])
// 		 {
// 			 MessageBoxA(NULL, request.lowerCasedParseFilePath.c_str(), lowerCasedFolderPath.c_str(), MB_OK);
// 		 }
// 	 }

	request.requestId = _GenRequestId();
	request.setting = _projSettingLib->Get(defines._setting);
#ifdef ENABLE_PARSER_DEBUG
	request.debugStartTime = GetAbsTick();
#endif

	if (!_Parse(request))
		return false;
	defines._isParsing = true;
	return true;
}


// SymbolNameStartsWithFilter 的 userData 结构体
// （已废弃，改用 lambda 捕获，保留注释供参考）

// 过滤器回调函数，用于检查符号名称是否以查询字符串开头
// currentResultCount：当前已加入结果集的条目数量
// 返回值：
//   pass      = 该条目是否通过过滤加入结果集
//   shouldStop= 是否要立即终止整个搜索遍历
//
// 逻辑：
//   1. 若 _IsSlnReferenced 不通过，直接 pass=false，不停止
//   2. 判断该条目是否与 query 完全相等（完整名称或某一 '.' 分段完全等于 query，忽略大小写）
//   3. 完全相等的条目：始终 pass=true，记录 hasExactMatch=true，不停止
//   4. 不完全相等（仅前缀匹配）的条目：
//      - 若 currentResultCount < maxResults：pass=true，不停止
//      - 若 currentResultCount >= maxResults：
//          * 若 hasExactMatch==true：已有完全相等且结果已满，shouldStop=true（停止整个搜索）
//          * 否则：pass=false，继续搜索（可能后面还有完全相等的）




//#define QuerySymbolDefine_Log 
void CSymbolDB::QuerySymbolDefine(const char* query, int maxResults, std::vector<SymbolDefineQueryResult>& results)
{
	results.clear();
	
	if (!query || strlen(query) == 0)
		return;

	// 从nameIndexer中查询匹配的符号，使用过滤器
	std::vector<NameIndexerData> indexerResults;

	std::shared_lock<std::shared_mutex> lock(_symbolMutex);

	bool hasExactMatch = false; // 是否已经找到过完全相等的 symbol（由 filter 自身维护）
	auto symbolNameStartsWithFilter = [this, maxResults, query, &hasExactMatch](const NameIndexerData& data, const char* q, int currentResultCount) -> CNameIndexer::FilterResult
	{
		CNameIndexer::FilterResult ret{ false, false };

		if (!q)
			return ret;

		// 将NameIndexerData转换为SymbolNameData
		const CSymbolDB::SymbolNameData* symbolData = reinterpret_cast<const CSymbolDB::SymbolNameData*>(&data);

		// symbolData->name 是文件路径索引，symbolData->index 是该文件中符号定义的索引
		StringIndex filePathIndex = symbolData->name;
		int symbolIndex = symbolData->index;

		// 在_fileSymbolDefines中查找对应的文件
		auto it = _fileSymbolDefines.find(filePathIndex);
		if (it == _fileSymbolDefines.end())
			return ret;

		const CFileSymbolDefines& fileDefines = it->second;

		// 检查索引是否有效
		if (symbolIndex < 0 || symbolIndex >= static_cast<int>(fileDefines._parsedDefines.size()))
			return ret;

		const CSymbolDefine& symbolDefine = fileDefines._parsedDefines[symbolIndex];

		// 获取符号名称
		std::string symbolName;
		GetStr(symbolDefine._name, symbolName);

		// 辅助函数：检查名称是否以查询字符串开头（不区分大小写）
		auto startsWithIgnoreCase = [](const std::string& name, const char* q) -> bool {
			size_t queryLen = strlen(q);
			size_t nameLen = name.length();
			if (nameLen < queryLen)
				return false;
			for (size_t i = 0; i < queryLen; ++i)
			{
				if (std::tolower((unsigned char)name[i]) != std::tolower((unsigned char)q[i]))
					return false;
			}
			return true;
		};

		// 判断该条目是否通过 sln 引用过滤
		if (!_IsSlnReferenced(&fileDefines))
			return ret; // pass=false, shouldStop=false

		// 辅助函数：检查名称是否与查询字符串完全相等（区分大小写）
		auto equalsCaseSensitive = [](const char* name, const char* q) -> bool {
			return strcmp(name, q) == 0;
		};

		// 检查完整名称或各分段是否匹配（前缀或完全相等）
		// exactMatched：某一分段（或完整名称）与 query 大小写完全一致
		bool matched      = false; // 是否至少前缀匹配（忽略大小写）
		bool exactMatched = false; // 是否完全相等（区分大小写）

		// 检查完整名称
		if (startsWithIgnoreCase(symbolName, q))
			matched = true;
		if (equalsCaseSensitive(symbolName.c_str(), query))
			exactMatched = true;

		// 检查每个 '.' 分段名称（无论完整名称是否已匹配，分段都要检查）
		{
			const char* fullName = symbolName.c_str();
			size_t dotPos = symbolName.rfind('.');
			while (dotPos != std::string::npos)
			{
				const char* partialName = fullName + dotPos + 1;
				if (startsWithIgnoreCase(partialName, q))
					matched = true;
				if (equalsCaseSensitive(partialName, query))
				{
					exactMatched = true;
					break; // 已找到完全相等的分段，无需继续
				}
				if (dotPos == 0)
					break;
				dotPos = symbolName.rfind('.', dotPos - 1);
			}
		}

		if (!matched)
			return ret; // pass=false, shouldStop=false

		if (exactMatched)
		{
			// 完全相等：始终加入结果，记录已找到完全相等
			hasExactMatch = true;
			ret.pass      = true;
			ret.shouldStop = false;
		}
		else
		{
			// 仅前缀匹配（非完全相等）
			if (currentResultCount >= maxResults)
			{
				// 结果集已满：
				// 若已存在完全相等的 symbol，则停止搜索；
				// 否则继续搜索，期待找到完全相等的
				ret.pass      = false;
				ret.shouldStop = hasExactMatch;
			}
			else
			{
				ret.pass      = true;
				ret.shouldStop = false;
			}
		}

		return ret;
	};
	_symbolIndexer.Query(query, maxResults, indexerResults, symbolNameStartsWithFilter);

#ifdef QuerySymbolDefine_Log
	// 将所有搜集到的symbol名字字符串拼成一个大的字符串
	std::string allSymbolNames;
#endif
	
	// 遍历indexer返回的结果，获取详细的符号定义信息
	for (const NameIndexerData& indexerData : indexerResults)
	{
		// 将NameIndexerData转换为SymbolNameData
		const SymbolNameData* symbolData = reinterpret_cast<const SymbolNameData*>(&indexerData);
		
		StringIndex filePathIndex = symbolData->name;
		int symbolIndex = symbolData->index; 

		// 在_fileSymbolDefines中查找对应的文件
		auto it = _fileSymbolDefines.find(filePathIndex);
		if (it == _fileSymbolDefines.end())
			continue;

		const CFileSymbolDefines& fileDefines = it->second;
		
		// 检查索引是否有效
		if (symbolIndex < 0 || symbolIndex >= static_cast<int>(fileDefines._parsedDefines.size()))
			continue;

		const CSymbolDefine& symbolDefine = fileDefines._parsedDefines[symbolIndex];

		// 创建查询结果
		SymbolDefineQueryResult result;
		
		// 设置符号名称
		_strPool.GetStr(symbolDefine._name, result.name);
		
#ifdef QuerySymbolDefine_Log
		// 将符号名称添加到总字符串中
		if (!allSymbolNames.empty()) {
			allSymbolNames += ", ";
		}
		allSymbolNames += result.name;
#endif
		
		// 设置文件路径
		result.location = symbolDefine._location;

		result.kind = symbolDefine._kind;
		
		// 生成描述信息（包含符号类型、位置等信息）
		std::ostringstream desc;
// 		
// 		// 添加符号类型描述
// 		switch (symbolDefine._category)
// 		{
// 		case SymbolCategory::FUNCTION:
// 			desc << "Function";
// 			break;
// 		case SymbolCategory::CLASS:
// 			desc << "Class";
// 			break;
// 		case SymbolCategory::STRUCT:
// 			desc << "Struct";
// 			break;
// 		case SymbolCategory::ENUM:
// 			desc << "Enum";
// 			break;
// 		case SymbolCategory::VARIABLE:
// 			desc << "Variable";
// 			break;
// 		case SymbolCategory::TYPEDEF:
// 			desc << "Typedef";
// 			break;
// 		case SymbolCategory::MACRO:
// 			desc << "Macro";
// 			break;
// 		case SymbolCategory::NAMESPACE:
// 			desc << "Namespace";
// 			break;
// 		default:
// 			desc << "Symbol";
// 			break;
// 		}
// 		
// 		// 添加位置信息
// 		desc << " at line " << symbolDefine._location.line;
// 		if (symbolDefine._location.columnStart > 0)
// 		{
// 			desc << ", column " << symbolDefine._location.columnStart;
// 			if (symbolDefine._location.columnEnd > symbolDefine._location.columnStart)
// 				desc << "-" << symbolDefine._location.columnEnd;
// 		}
// 		
// 		// 如果有代码行范围信息，也添加进去
// 		if (symbolDefine._startLine > 0 && symbolDefine._endLine > symbolDefine._startLine)
// 		{
// 			desc << " (lines " << symbolDefine._startLine << "-" << symbolDefine._endLine << ")";
// 		}
		
		result.desc = desc.str();
		
		// 添加到结果列表
		results.push_back(result);
	}
	
#ifdef QuerySymbolDefine_Log
	// 输出到_logFile
	if (!allSymbolNames.empty()) {
		_logFile.Dump("QuerySymbolDefine: query=\"%s\", symbols=[%s]", query, allSymbolNames.c_str());
	} else {
		_logFile.Dump("QuerySymbolDefine: query=\"%s\", no symbols found", query);
	}
#endif
}

void CSymbolDB::FindSymbolDefine(const char* symbolName, int maxResult, std::vector<SymbolDefineFindResult>& results)
{
	results.clear();
	
	if (!symbolName || strlen(symbolName) == 0)
		return;

	// 从nameIndexer中查询匹配的符号，使用完全匹配
	std::vector<NameIndexerData> indexerResults;
	
	std::shared_lock<std::shared_mutex> lock(_symbolMutex);
	
	// 使用完全匹配过滤器
	auto exactMatchFilter = [this, maxResult](const NameIndexerData& data, const char* query, int currentResultCount) -> CNameIndexer::FilterResult {
		CNameIndexer::FilterResult ret{ false, false };

		if (!query)
			return ret;

		// 已达到最大数量限制，终止遍历
		if (currentResultCount >= maxResult)
		{
			ret.shouldStop = true;
			return ret;
		}

		CSymbolDB* symbolDB = this;

		// 将NameIndexerData转换为SymbolNameData
		const CSymbolDB::SymbolNameData* symbolData = reinterpret_cast<const CSymbolDB::SymbolNameData*>(&data);
		
		// symbolData->name 是文件路径索引，symbolData->index 是该文件中符号定义的索引
		StringIndex filePathIndex = symbolData->name;
		int symbolIndex = symbolData->index;

		// 在_fileSymbolDefines中查找对应的文件
		auto it = symbolDB->_fileSymbolDefines.find(filePathIndex);
		if (it == symbolDB->_fileSymbolDefines.end())
			return ret;

		const CFileSymbolDefines& fileDefines = it->second;

		// 检查索引是否有效
		if (symbolIndex < 0 || symbolIndex >= static_cast<int>(fileDefines._parsedDefines.size()))
			return ret;

		const CSymbolDefine& symbolDefine = fileDefines._parsedDefines[symbolIndex];

		// 获取符号名称
		std::string defineName;
		symbolDB->GetStr(symbolDefine._name, defineName);

		// 判断是否完全匹配（区分大小写，完整名称或某一 '.' 分段严格等于 query）
		bool matched = false;

		if (defineName == query)
			matched = true;

		if (!matched)
		{
			const char* fullName = defineName.c_str();
			size_t dotPos = defineName.rfind('.');
			while (dotPos != std::string::npos)
			{
				const char* partialName = fullName + dotPos + 1;
				if (strcmp(partialName, query) == 0)
				{
					matched = true;
					break;
				}
				if (dotPos == 0)
					break;
				dotPos = defineName.rfind('.', dotPos - 1);
			}
		}

		if (!matched)
			return ret; // pass=false, shouldStop=false

		if (!symbolDB->_IsSlnReferenced(&fileDefines))
			return ret; // pass=false, shouldStop=false

		// 完全匹配且通过引用过滤：加入结果
		ret.pass = true;
		ret.shouldStop = false;
		return ret;
	};

	// 查询完全匹配的符号
	_symbolIndexer.Query(symbolName, maxResult, indexerResults, exactMatchFilter);
	
	// 遍历indexer返回的结果，获取详细的符号定义信息
	for (const NameIndexerData& indexerData : indexerResults)
	{
		// 检查是否已达到maxResult限制
		if (static_cast<int>(results.size()) >= maxResult)
			break;

		// 将NameIndexerData转换为SymbolNameData
		const SymbolNameData* symbolData = reinterpret_cast<const SymbolNameData*>(&indexerData);
		
		StringIndex filePathIndex = symbolData->name;
		int symbolIndex = symbolData->index; 

		// 在_fileSymbolDefines中查找对应的文件
		auto it = _fileSymbolDefines.find(filePathIndex);
		if (it == _fileSymbolDefines.end())
			continue;

		const CFileSymbolDefines& fileDefines = it->second;
		
		// 检查索引是否有效
		if (symbolIndex < 0 || symbolIndex >= static_cast<int>(fileDefines._parsedDefines.size()))
			continue;

		const CSymbolDefine& symbolDefine = fileDefines._parsedDefines[symbolIndex];

		// 创建查询结果
		SymbolDefineFindResult result;
		
		// 设置符号类型
		result.kind = symbolDefine._kind;
		
		// 设置符号位置
		result.location = symbolDefine._location;
		
		// 设置代码行范围
		result.lineRange = symbolDefine._lineRange;
		
		// 添加到结果列表
		results.push_back(result);
	}
}

bool CSymbolDB::GetSymbolNamesFromLines(const char* filePath, std::vector<int>& lines, std::vector<StringIndex>& symbolNames)
{
	symbolNames.clear();
	
	if (!filePath || lines.empty())
		return false;
	
	// 首先找到该文件的symbol定义
	std::shared_lock<std::shared_mutex> lock(_symbolMutex);
	
	CFileSymbolDefines* fileDefines = _FindSymbolDefines(filePath);
	if (!fileDefines)
	{
		// 文件不存在于数据库中，为所有行返回StringIndex_Null
		symbolNames.resize(lines.size(), StringIndex_Null);
		return false;
	}
	
	// 预分配结果数组
	symbolNames.resize(lines.size(), StringIndex_Null);
	
	// 如果没有symbol定义，直接返回
	if (fileDefines->_parsedDefines.empty())
		return true;
	
	// 找到文件的最大行数，用于创建行标记数组
	int maxLine = 0;
	for (int line : lines)
	{
		if (line > maxLine)
			maxLine = line;
	}
	
	// 还需要考虑symbol定义的最大行号
	for (const CSymbolDefine& define : fileDefines->_parsedDefines)
	{
		if (define._lineRange.IsValid() && define._lineRange.end > maxLine)
			maxLine = define._lineRange.end;
	}
	
	// 创建行标记数组，初始化为StringIndex_Null
	std::vector<StringIndex> lineSymbols(maxLine + 1, StringIndex_Null);
	
	// 收集所有有效的symbol定义，并计算它们的range大小
	struct SymbolWithRange
	{
		const CSymbolDefine* define;
		int rangeSize;
	};
	
	std::vector<SymbolWithRange> symbols;
	for (const CSymbolDefine& define : fileDefines->_parsedDefines)
	{
		if (define._lineRange.IsValid())
		{
			int rangeSize = define._lineRange.end - define._lineRange.start;
			symbols.push_back({&define, rangeSize});
		}
	}
	
	// 按range大小从大到小排序
	std::sort(symbols.begin(), symbols.end(), 
		[](const SymbolWithRange& a, const SymbolWithRange& b) {
			return a.rangeSize > b.rangeSize; // 从大到小排序
		});
	
	// 依次将symbol的range标记到行标记数组中
	for (const auto& symbol : symbols)
	{
		const CSymbolDefine& define = *symbol.define;
		for (int line = define._lineRange.start; line < define._lineRange.end; ++line)
		{
			if (line >= 0 && line < static_cast<int>(lineSymbols.size()))
			{
				// 如果该行还没有被标记，或者当前symbol的range更大（由于排序，先处理range大的）
				// 这里我们直接覆盖，因为已经按range大小排序了
				lineSymbols[line] = define._name;
			}
		}
	}
	
	// 从行标记数组中直接读取每行的symbol
	for (size_t i = 0; i < lines.size(); ++i)
	{
		int line = lines[i];
		if (line >= 0 && line < static_cast<int>(lineSymbols.size()))
		{
			symbolNames[i] = lineSymbols[line];
		}
		// 否则保持为StringIndex_Null
	}

	return true;
}


bool CSymbolDB::CollectRefs(const CollectRefsParam& param)
{
	if (param.filePath.empty())
		return false;

	// 在字符串池中找到该文件的索引
	StringIndex fileIndex = _strPool.FindStr(param.filePath.c_str());
	if (fileIndex == StringIndex_Null)
		return false;

	ParseRequest request;
	{
		std::shared_lock<std::shared_mutex> lock(_symbolMutex);

		auto it = _fileSymbolDefines.find(fileIndex);
		if (it == _fileSymbolDefines.end())
			return false;

		const CFileSymbolDefines& fileDefines = it->second;
		if (fileDefines._isMasterFile)
		{
			request.lowerCasedParseFilePath = param.filePath;
			request.setting = _projSettingLib->Get(fileDefines._setting);
		}
		else
		{
			for (std::unordered_map<StringIndex, MasterIncluder>::const_iterator it = fileDefines._masterIncluders.begin();it != fileDefines._masterIncluders.end();it++)
			{
				StringIndex includerFilePath = it->first;
				std::unordered_map<StringIndex, CFileSymbolDefines>::iterator it2 = _fileSymbolDefines.find(includerFilePath);
				if (it2 != _fileSymbolDefines.end())
				{
					if (it2->second._masterFileParseVer == it->second.parseVer)
					{
						CFileSymbolDefines& includerDefines = (*it2).second;
						_strPool.GetStr(includerDefines._filePath, request.lowerCasedParseFilePath);
						request.setting = _projSettingLib->Get(includerDefines._setting);
						break;
					}
				}
			}
			if (request.lowerCasedParseFilePath.empty())
			{
				request.lowerCasedParseFilePath = param.filePath;
				request.setting = _projSettingLib->Get(fileDefines._setting);
			}
		}

#ifdef ENABLE_PARSER_DEBUG
		request.debugStartTime = GetAbsTick();
#endif
	}

	if (request.lowerCasedParseFilePath.empty())
		return false;

	request.requestId = _GenRequestId();
	request.collectRefParam = param;

	std::lock_guard<std::mutex> lock(_collectRefParseMutex);
	_parserForCollectRef.Request(request);

	ParseResult result;
	while (!_parserForCollectRef.FetchResult(result))
		Sleep(1);

	return result.success;
}


void CSymbolDB::_UnIndex(CFileSymbolDefines& defines)
{
	always_assert(sizeof(SymbolNameData) == sizeof(NameIndexerData));
	for (int i = 0;i < defines._parsedDefines.size();i++)
	{
		NameIndexerData data;
		SymbolNameData *pData = (SymbolNameData *)&data;
		pData->name = defines._filePath;
		pData->index = i;

		std::string nameStr;
		_strPool.GetStr(defines._parsedDefines[i]._name, nameStr);
		const char* fullName = nameStr.c_str();
		
		// Remove full name entry
		_symbolIndexer.RemoveEntry(fullName, data);
		
		// Remove each hierarchical level of the name
		size_t dotPos = nameStr.rfind('.');
		while (dotPos != std::string::npos) 
		{
			const char* partialName = fullName + dotPos + 1;
			_symbolIndexer.RemoveEntry(partialName, data);
			if (dotPos == 0) 
				break;
			dotPos = nameStr.rfind('.', dotPos - 1);
		}
	}
}

void CSymbolDB::_Index(CFileSymbolDefines* pFileDefines, CSymbolDefine& define)
{
	//添加到nameIndexer - 添加每个层级的分段名称
	NameIndexerData data;
	SymbolNameData* pData = (SymbolNameData*)&data;
	pData->name = pFileDefines->_filePath;
	pData->index = (int)pFileDefines->_parsedDefines.size() - 1;

	std::string nameStr;
	_strPool.GetStr(define._name, nameStr);
	const char* fullName = nameStr.c_str();

	// 添加完整名称
	_symbolIndexer.AddEntry(fullName, data);

	// 添加每个层级的分段名称
	size_t dotPos = nameStr.rfind('.');
	while (dotPos != std::string::npos)
	{
		const char* partialName = fullName + dotPos + 1;
		_symbolIndexer.AddEntry(partialName, data);
		if (dotPos == 0)
			break;
		dotPos = nameStr.rfind('.', dotPos - 1);
	}

	// 如果名称中没有点，上面已经添加了完整名称
}

void CSymbolDB::_ClearParsed(CFileSymbolDefines& defines)
{
	_UnIndex(defines);
	defines._parsedDefines.clear();
	defines._parsedInclusions.clear();
	defines._parsedTime = 0;
}
 
void CSymbolDB::_ClearUnreferenceDefines()
{
	std::unique_lock<std::shared_mutex> lock(_symbolMutex);
	
	// 收集需要删除的文件路径索引
	std::vector<StringIndex> toRemove;
	
	for (auto& pair : _fileSymbolDefines)
	{
		const CFileSymbolDefines* defines = &pair.second;
		if (!_IsSlnReferenced(defines))
		{
			toRemove.push_back(pair.first);
			_files.SetDirty(pair.first);
		}
	}
	
	// 删除未被引用的define
	for (StringIndex filePathIdx : toRemove)
	{
		auto it = _fileSymbolDefines.find(filePathIdx);
		if (it != _fileSymbolDefines.end())
		{
			_UnIndex(it->second);
			_fileSymbolDefines.erase(it);
		}
	}
}

bool CSymbolDB::_IsSlnReferenced(const CFileSymbolDefines* defines)
{
	if (!defines)
		return false;
		
	// 如果文件本身就在解决方案中，直接返回 true
	if (defines->_isInSln)
		return true;

	if (defines->_isMasterFile)
		return false;
		
	// 检查包含它的 master 文件是否在解决方案中
	for (const auto& includerPair : defines->_masterIncluders)
	{
		StringIndex includerFilePath = includerPair.first;
		const MasterIncluder& includer = includerPair.second;
		
		// 在文件定义中查找包含者
		auto it = _fileSymbolDefines.find(includerFilePath);
		if (it != _fileSymbolDefines.end())
		{
			const CFileSymbolDefines& includerDefines = it->second;
			
			// 检查版本号是否匹配（确保是有效的包含关系）
			if (includerDefines._masterFileParseVer == includer.parseVer)
			{
				// 如果包含它的 master 文件在解决方案中，返回 true
				if (includerDefines._isInSln)
					return true;
			}
		}
	}
	
	// 都不满足条件，返回 false
	return false;
}


void CSymbolDB::ProcessParseResult(const ParseResult& result) 
{
	std::unique_lock< std::shared_mutex> lock(_symbolMutex);

	CFileSymbolDefines* mainFileDefines = _FindSymbolDefines(result.parseFilePath.c_str());

	if (mainFileDefines)
		mainFileDefines->_isParsing = false;
	 
#ifdef ENABLE_PARSER_DEBUG
	if (true)
	{
		std::string s = result.parseFilePath.c_str();
		s += "--";
		result.DumpDebugString(s);
		_logFile.Dump("Parsed:\"%s\"", s.c_str());
	}
#endif

	if (result.discarded)
		return;

	if (!result.success)
	{
		if (mainFileDefines)
		{
			mainFileDefines->_masterFileParseVer++;

			_ClearParsed(*mainFileDefines);

			mainFileDefines->_parsedTime = Utils::GetFileTimeT(result.parseFilePath.c_str());
			if (mainFileDefines->_parsedTime == 0)//这个文件可能不存在,使用当前时间
				mainFileDefines->_parsedTime = Utils::GetCurFileTimeT();

			mainFileDefines->_pchTime = result.pchTime;
			_files.SetDirty(mainFileDefines->_filePath);
		}
		return;
	}

	//对于这一次parse涉及到的文件,先清除旧数据
	if (mainFileDefines)
	{
		_ClearParsed(*mainFileDefines);

		for (const auto& inclusionPair : result.inclusions)
		{
			const std::string& filePath = inclusionPair.first;
			const RawInclusion& rawInclusion = inclusionPair.second;

			Inclusion inclusion;
			inclusion.path = _strPool.AddStr(filePath.c_str());
			if (inclusion.path != mainFileDefines->_filePath)
			{
				std::unordered_map < StringIndex, CFileSymbolDefines>::iterator it = _fileSymbolDefines.find(inclusion.path);
				if (it != _fileSymbolDefines.end())
				{
					CFileSymbolDefines& includingDefines = (*it).second;

					if (rawInclusion.time > includingDefines._parsedTime)
						_ClearParsed(includingDefines);
				}
			}
		}
	}

	// 遍历本次解析涉及的所有文件
	for (const auto& pair : result.definesByFile)
	{
		const std::string& filePath = pair.first;
		const std::vector<RawSymbolDefine>& rawDefines = pair.second;

		CFileSymbolDefines* pFileDefines = ObtainSymbolDefines(filePath.c_str());
		if (!pFileDefines)
			continue;

		time_t fileTime = result.fileTimes.at(filePath);

		// 只有当本次解析比记录的要新时，才更新
		if (fileTime > pFileDefines->_parsedTime)
		{
			_ClearParsed(*pFileDefines);

			_files.SetDirty(pFileDefines->_filePath);

			StringIndex filePathStringIndex = pFileDefines->_filePath;

			// 转换原始符号定义为最终格式
			for (const RawSymbolDefine& rawDefine : rawDefines)
			{
				CSymbolDefine define;

				// 设置symbol名称
				define._name = _strPool.AddStr(rawDefine.name.c_str());
				define.strName = _strPool.GetTempStr(define._name);

				// 设置symbol类型
				define._kind = rawDefine.kind;

				// 设置symbol位置
				define._location.filePath = filePathStringIndex;
				define._location.strFilePath = _strPool.GetTempStr(filePathStringIndex);
				define._location.lineLoc = rawDefine.lineLoc;

				// 设置代码行范围
				define._lineRange = rawDefine.lineRange;

				// 添加到定义列表
				pFileDefines->_parsedDefines.push_back(define);

				_Index(pFileDefines, define);
			}

			// 更新该文件的时间戳
			pFileDefines->_parsedTime = fileTime;
			_debugTotalParsed++;
			_debugLastParsedTime = GetAbsTick();
		}
	}

	// 单独处理主文件的包含关系和PCH时间
	if (mainFileDefines)
	{
		mainFileDefines->_masterFileParseVer++;

		mainFileDefines->_parsedInclusions.clear();
		MasterIncluder includer;
		includer.parseVer = mainFileDefines->_masterFileParseVer;
		includer.strPath = mainFileDefines->_filePathStr;
		for (const auto& inclusionPair : result.inclusions)
		{
			const std::string& filePath = inclusionPair.first;
			const RawInclusion& rawInclusion = inclusionPair.second;
			
			Inclusion inclusion;
			inclusion.path = _strPool.AddStr(filePath.c_str());
			if (inclusion.path != mainFileDefines->_filePath)
			{
				inclusion.strPath = _strPool.GetTempStr(inclusion.path);
				inclusion.time = rawInclusion.time;
				mainFileDefines->_parsedInclusions.push_back(inclusion);

				std::unordered_map < StringIndex, CFileSymbolDefines>::iterator it = _fileSymbolDefines.find(inclusion.path);
				if (it != _fileSymbolDefines.end())
				{
					CFileSymbolDefines& includingDefines = (*it).second;
					includingDefines._parsedTime = rawInclusion.time;

					//非master file,记录includers(注意: pch文件不记录includers)
					if (!includingDefines._isMasterFile)
						includingDefines._masterIncluders[mainFileDefines->_filePath] = includer;
				}
			}
		}

		mainFileDefines->_pchTime = result.pchTime;

		_files.SetDirty(mainFileDefines->_filePath);
	}

}

bool CSymbolDB::IsFileInDB(const char* filePath)
{
	std::shared_lock<std::shared_mutex> lock(_symbolMutex);

	return _FindSymbolDefines(filePath) != nullptr;
}

time_t CSymbolDB::GetParsedTime(StringIndex filePathIndex) const
{
	std::shared_lock<std::shared_mutex> lock(_symbolMutex);
	auto it = _fileSymbolDefines.find(filePathIndex);
	if (it == _fileSymbolDefines.end())
		return 0;
	return it->second._parsedTime;
}

bool CSymbolDB::GetSymbolLineRanges(StringIndex filePathIndex,
                                    std::vector<SymbolRangeInfo>& outRanges,
                                    time_t& outParsedTime) const
{
	std::shared_lock<std::shared_mutex> lock(_symbolMutex);
	auto it = _fileSymbolDefines.find(filePathIndex);
	if (it == _fileSymbolDefines.end())
		return false;

	const CFileSymbolDefines& defines = it->second;
	outParsedTime = defines._parsedTime;
	for (const CSymbolDefine& sym : defines._parsedDefines)
	{
		if (sym._lineRange.IsValid())
			outRanges.push_back({ sym._kind, sym._lineRange });
	}

	// 按起始行排序
	std::sort(outRanges.begin(), outRanges.end(),
	          [](const SymbolRangeInfo& a, const SymbolRangeInfo& b)
	          { return a._lineRange.start < b._lineRange.start; });
	return true;
}

CFileSymbolDefines* CSymbolDB::_FindSymbolDefines(const char* filePath) 
{
	StringIndex index = _strPool.FindStr(filePath);
	if (index == StringIndex_Null)
		return nullptr;

	auto it = _fileSymbolDefines.find(index);
	if (it == _fileSymbolDefines.end())
		return nullptr;
	return &it->second;
}

CFileSymbolDefines* CSymbolDB::ObtainSymbolDefines(const char* filePath)
{
	StringIndex index = _strPool.AddStr(filePath);
	if (index == StringIndex_Null)
		return nullptr;
	CFileSymbolDefines*ret= &_fileSymbolDefines[index];
	if (ret->_filePath == StringIndex_Null)
	{
		ret->_filePath = index;
		ret->_filePathStr = _strPool.GetTempStr(index);
	}
	return ret;
}

bool CSymbolDB::_CheckReferenced(CFileSymbolDefines& defines)
{
	if (defines._isMasterFile)
		return true;

	for (auto it = defines._masterIncluders.begin();it != defines._masterIncluders.end();)
	{
		StringIndex includerFilePath = it->first;
		auto it2 = _fileSymbolDefines.find(includerFilePath);
		if(it2 != _fileSymbolDefines.end())
		{
			if(it2->second._masterFileParseVer == it->second.parseVer)
				return true;
		}
		//这个includer已经不存在或者不再include自己了,删掉它
		it = defines._masterIncluders.erase(it);
	}

	return false;
}

bool CSymbolDB::_CheckOutOfDate(const CFileSymbolDefines& defines)
{
	if (defines._parsedTime == 0)
		return true;
	const char* filePath = _strPool.GetTempStr(defines._filePath);
	
	time_t fileTime = Utils::GetFileTimeT(filePath);
	if (fileTime == 0)
		return false;//文件有异常情况(比如文件不存在)
	if (fileTime != defines._parsedTime)//文件发生了修改
		return true;

	return false;
}


// bool CSymbolDB::_CheckOutOfDateWithInclusions(const CFileSymbolDefines& defines)
// {
// 	if (defines._parsedTime == 0)
// 		return true;
// 
// 	//检查依赖的pch是否发生了变化
// 	if (defines._isMasterFile)
// 	{
// 		if (defines._pchTime != 0)
// 		{
// 			if (defines._pchPath != StringIndex_Null)
// 			{
// 				if (defines._pchPath != defines._filePath)//使用pch而不是Generate pch
// 				{
// 					std::unordered_map < StringIndex, CFileSymbolDefines>::iterator it = _fileSymbolDefines.find(defines._pchPath);
// 					if (it != _fileSymbolDefines.end())
// 					{
// 						CFileSymbolDefines& pchDefines = (*it).second;
// 						if (pchDefines._pchTime != defines._pchTime)
// 							return true;
// 					}
// 					else
// 						return true;
// 				}
// 			}
// 		}
// 	}
// 
// 	const char* filePath = _strPool.GetTempStr(defines._filePath);
// 	time_t fileTime = Utils::GetFileTimeT(filePath);
// 
// 	if (fileTime != defines._parsedTime)//文件发生了修改
// 		return true;
// 
// 	const std::deque<Inclusion>* inclusions = &defines._parsedInclusions;
// 
// 	// 遍历所有包含的文件,检查它们的修改时间
// 	for (const auto& inclusion : *inclusions)
// 	{
// 		// 获取包含文件的当前修改时间
// 		const char* includedPath = inclusion.strPath;
// 		time_t currentIncludedTime = Utils::GetFileTimeT(includedPath);
// 
// 		// 如果包含的文件比数据库中记录的新,则认为文件已过期
// 		if (currentIncludedTime > inclusion.time)
// 			return true;
// 	}
// 
// 	return false;
// }

ParseRequestId CSymbolDB::_GenRequestId()		
{			
	// 使用相对时间而非绝对时间,防止溢出
	AbsTick curTime = GetAbsTick();
	
	// 计算从初始化以来经过的时间(毫秒)
	AbsTick relativeTime = curTime - _initTime;
	
	// 将相对时间截断为32位
	ParseRequestId timeStamp = (ParseRequestId)(relativeTime & 0xFFFFFFFF);
	
	// 确保序号部分不超过32位
	_nextRequestId = (_nextRequestId + 1) & 0xFFFFFFFF;
	
	// 防止生成无效ID
	if (_nextRequestId == 0)
		_nextRequestId = 1;
	
	// 组合时间戳和序号
	return (timeStamp << 32) | _nextRequestId;
}

void CSymbolDB::_UpdatePchFile(CFileSymbolDefines& defines)
{
	defines._pchPath = StringIndex_Null;

	always_assert(_projSettingLib);

	const ProjSetting* settingPtr = _projSettingLib->Get(defines._setting);
	if (settingPtr)
	{
		if (!settingPtr->lowerCasedPchFullPath.empty())
			defines._pchPath = _strPool.AddStr(settingPtr->lowerCasedPchFullPath.c_str());
	}
}

void CSymbolDB::_UpdateMasterFile(CFileSymbolDefines& defines)
{
	defines._isMasterFile = false;
	if (defines._pchPath != StringIndex_Null)
	{
		if (defines._pchPath == defines._filePath)
		{
			defines._isMasterFile = true;
		}
	}
	if (!defines._isMasterFile)
	{
		std::string suffix;
		_strPool.GetStrSuffix(defines._filePath, suffix);
		StringLower(suffix);
		if ((suffix=="cpp"|| suffix == "c"|| suffix == "cc"|| suffix == "cxx"))
			defines._isMasterFile = true;
	}

	//Master 文件不记录那些include自己的文件
	if (defines._isMasterFile)
		defines._masterIncluders.clear();

}

void CSymbolDB::_UpdatePchGenFiles(CFileSymbolDefines& defines)
{
	bool isGenPch = false;

	if (defines._pchPath != StringIndex_Null)
	{
		if (defines._pchPath == defines._filePath)
		{
			isGenPch = true;
		}
	}

	if (isGenPch)
		_pchGenFiles.insert(defines._filePath);
	else
		_pchGenFiles.erase(defines._filePath);
}

void CSymbolDB::_LoadSymbolIndexer()
{
	std::string path = _folderPath + "\\_defines\\.idx";
	_symbolIndexer.LoadFromFile(path.c_str());
}

void CSymbolDB::_SaveSymbolIndexer()
{
	std::string path = _folderPath + "\\_defines\\.idx";
	_symbolIndexer.SaveToFile(path.c_str());
}

 
void CSymbolDB::SetContent(const CSolutionFiles& files)
{
	if (_fileSymbolDefines.size() > 0)
		return;

	if (files._lowerCasedFiles.size() <= 0)
		return;

	// 初始化解析器
	_InitParser();

	_LoadStrLib();

	std::unique_lock< std::shared_mutex> lock(_symbolMutex);
	if (true)
	{
		// 验证版本号，如果不匹配会清除所有bucket文件
		_files.VerifyVersion();
		_files.Load(_fileSymbolDefines, _strPool);
		_LoadSymbolIndexer();
	}

	bool isAnyDirty = false;

	std::string suffix;

	std::unordered_map<std::string, SolutionFile>::const_iterator it;
	for(it=files._lowerCasedFiles.begin();it!=files._lowerCasedFiles.end();++it)
	{
		const SolutionFile& file = it->second;

		suffix = GetFileSuffix(file.lowerCasedFilePath);
		StringLower(suffix);
		if (!Utils::IsCppFile(suffix))
			continue;

		StringIndex index = _strPool.AddStr(file.lowerCasedFilePath.c_str());
		if(index == StringIndex_Null)
			continue;

		CFileSymbolDefines* defines;
		std::unordered_map<StringIndex, CFileSymbolDefines>::iterator it = _fileSymbolDefines.find(index);
		if (it == _fileSymbolDefines.end())
		{
			isAnyDirty = true;

			defines = &_fileSymbolDefines[index];
			if (defines->_filePath != index)
			{
				defines->_filePath = index;
				defines->_filePathStr = _strPool.GetTempStr(index);
			}
		}
		else
			defines = &(*it).second;

		always_assert(defines);
		always_assert(defines->_filePath == index);

		defines->_isInSln = true;

		if (defines->_setting != file.setting)
		{
			//setting发生变化,需要重新 parse
			defines->_parsedTime = 0;
			defines->_setting = file.setting;
// 			_files.SetDirty(index);
		}

		_UpdatePchFile(*defines);
		_UpdateMasterFile(*defines);
		_UpdatePchGenFiles(*defines);

	}

	if (isAnyDirty)
		_files.SetAllDirty();

	// 内容创建完毕后启动更新线程
	_StartUpdateThread();
}

void CSymbolDB::SetDeltaContent(const std::vector<SolutionFile*>& newFiles, const std::vector<SolutionFile*>& updatedFiles, std::vector<std::string>& removedFiles)
{
	std::string suffix;
	for(int i=0;i<newFiles.size();i++)
	{
		SolutionFile* file = newFiles[i];

		suffix = GetFileSuffix(file->lowerCasedFilePath);
		StringLower(suffix);
		if (!Utils::IsCppFile(suffix))
			continue;

		StringIndex index = _strPool.AddStr(file->lowerCasedFilePath.c_str());
		if(index == StringIndex_Null)
			continue;

		Op op;
		op.tp = Op::NewFile;
		op.filePath = index;
		op.setting = file->setting;
		_AddOp(op);
	}

	for(int i=0;i<updatedFiles.size();i++)
	{
		SolutionFile* file = updatedFiles[i];

		suffix = GetFileSuffix(file->lowerCasedFilePath);
		StringLower(suffix);
		if (!Utils::IsCppFile(suffix))
			continue;

		StringIndex index = _strPool.FindStr(file->lowerCasedFilePath.c_str());
		if(index == StringIndex_Null)
			continue;

		Op op;
		op.tp = Op::UpdateFileSetting;
		op.filePath = index;
		op.setting = file->setting;
		_AddOp(op);
	}

	for(int i=0;i<removedFiles.size();i++)
	{
		const std::string& path = removedFiles[i];
		StringIndex index = _strPool.FindStr(path.c_str());
		if(index == StringIndex_Null)
			continue;

		Op op;
		op.tp = Op::DeleteFile;
		op.filePath = index;
		_AddOp(op);
	}

	_SaveStrLib();
}

void CSymbolDB::NotifyFilesChanged()
{
	_resetThreadLoop.store(true);
	_updateCV.notify_all();
}

void CSymbolDB::Save()
{
	_SaveStrLib();

	_ClearUnreferenceDefines();

	_files.Save(_fileSymbolDefines);
	_SaveSymbolIndexer();
	
}


void CSymbolDB::_AddOp(const Op&op)
{
	{
		std::lock_guard<std::mutex> lock(_opsMutex);
		_ops.push_back(op);
	}
	_resetThreadLoop.store(true);
	_updateCV.notify_all();
}

void CSymbolDB::_ProcessOp(const Op& op)
{
	switch (op.tp)
	{
		case Op::NewFile:
		{
			std::unique_lock< std::shared_mutex> lock(_symbolMutex);
			CFileSymbolDefines* defines;
			std::unordered_map<StringIndex, CFileSymbolDefines>::iterator it = _fileSymbolDefines.find(op.filePath);
			if (it == _fileSymbolDefines.end())
			{

				defines = &_fileSymbolDefines[op.filePath];
				defines->_filePath = op.filePath;
				defines->_filePathStr = _strPool.GetTempStr(op.filePath);
			}
			else
				defines = &(*it).second;

			always_assert(defines);
			always_assert(defines->_filePath == op.filePath);

			if (defines->_setting != op.setting)
			{
				//setting发生变化,需要重新 parse
				defines->_parsedTime = 0;
				defines->_setting = op.setting;
			}

			defines->_isInSln = true;

			_UpdatePchFile(*defines);
			_UpdateMasterFile(*defines);
			_UpdatePchGenFiles(*defines);
			_files.SetDirty(op.filePath);
			break;
		}
		case Op::UpdateFileSetting:
		{
			std::unique_lock< std::shared_mutex> lock(_symbolMutex);
			std::unordered_map<StringIndex, CFileSymbolDefines>::iterator it = _fileSymbolDefines.find(op.filePath);
			if(it != _fileSymbolDefines.end())
			{
				CFileSymbolDefines& defines = it->second;
				defines._isInSln = true;
				if (defines._setting != op.setting)
				{
					defines._parsedTime = 0;
					defines._setting = op.setting;

					_UpdatePchFile(defines);
					_UpdateMasterFile(defines);
					_UpdatePchGenFiles(defines);

					_files.SetDirty(op.filePath);
				}
			}
			break;
		}
		case Op::DeleteFile:
		{
			std::unique_lock< std::shared_mutex> lock(_symbolMutex);
			_pchGenFiles.erase(op.filePath);
			std::unordered_map<StringIndex, CFileSymbolDefines>::iterator it = _fileSymbolDefines.find(op.filePath);
			if (it != _fileSymbolDefines.end())
			{
				CFileSymbolDefines& defines = it->second;
				defines._isInSln = false;
			}
			_files.SetDirty(op.filePath);
			break;
		}
	}
}

bool CSymbolDB::_ProcessOps()
{
	std::deque<Op> localOps;
	
	// 在锁保护下快速移动操作到本地队列
	{
		std::lock_guard<std::mutex> lock(_opsMutex);
		localOps = std::move(_ops);
		_ops.clear();
	}
	
	// 在锁外处理操作，避免长时间持有锁
	for (size_t i = 0; i < localOps.size(); i++)
		_ProcessOp(localOps[i]);
	return localOps.size() > 0;
}

void CSymbolDB::_StartUpdateThread()
{
	// 如果线程已经在运行，直接返回
	if (_updateThreadRunning.load())
		return;

	// 启动更新线程
	_updateThreadRunning.store(true);
	_updateThread = std::thread(&CSymbolDB::_UpdateThreadProc, this);
	
	// 设置线程为最低优先级
	HANDLE threadHandle = _updateThread.native_handle();
	SetThreadPriority(threadHandle, THREAD_PRIORITY_LOWEST);
}

void CSymbolDB::_StopUpdateThread()
{
	// 如果线程没有在运行，直接返回
	if (!_updateThreadRunning.load())
		return;

	// 停止线程
	_updateThreadRunning.store(false);
	_updateCV.notify_all();

	// 等待线程结束
	if (_updateThread.joinable())
	{
		_updateThread.join();
	}
}

bool CSymbolDB::_CheckAndParsePch()
{
	bool isAnyParse = false;
	std::unordered_map<StringIndex, time_t> pchIncludeTimes;//
	for (StringIndex pchIndex : _pchGenFiles)
	{
		auto it = _fileSymbolDefines.find(pchIndex);
		if (it == _fileSymbolDefines.end())
			continue;

		CFileSymbolDefines& defines = it->second;
		if (defines._isParsing)
			continue;

		if (_CheckOutOfDate(defines))
		{
			isAnyParse = true;
			_Parse(defines);
			continue;
		}

		//这个pch本身没有修改,搜集它的所有的includes
		{
			const std::deque<Inclusion>* inclusions = &defines._parsedInclusions;

			// 遍历所有包含的文件,检查它们的修改时间
			for (const auto& inclusion : *inclusions)
			{
				time_t currentIncludedTime = 0;
				std::unordered_map<StringIndex, time_t>::iterator it = pchIncludeTimes.find(inclusion.path);
				if (it != pchIncludeTimes.end())
				{
					currentIncludedTime = it->second;
				}
				else
				{
					// 获取包含文件的当前修改时间
					const char* includedPath = _strPool.GetTempStr(inclusion.path);
					currentIncludedTime = Utils::GetFileTimeT(includedPath);
					pchIncludeTimes[inclusion.path] = currentIncludedTime;
				}

				// 如果包含的文件比数据库中记录的新,则认为文件已过期
				if (currentIncludedTime > inclusion.time)
				{
					isAnyParse = true;
					_Parse(defines);
					break;
				}
			}
		}

		if (_NeedResetThreadLoop())
			break;
	}

	return isAnyParse;
}

bool CSymbolDB::_StepCursor(std::unordered_map<StringIndex, CFileSymbolDefines>::iterator& cursorIt, int& nSteps)
{
	if (cursorIt == _fileSymbolDefines.end())
		return true;
	cursorIt++;
	nSteps++;
	if (cursorIt == _fileSymbolDefines.end())
		cursorIt = _fileSymbolDefines.begin();
	return nSteps>=_fileSymbolDefines.size();
}

bool CSymbolDB::_CheckAndParseNotPch(StringIndex& cursorFilePath, AbsTick budgetDur)
{
	AbsTick startT = GetAbsTick();

	//Locate the cursor
	std::unordered_map<StringIndex, CFileSymbolDefines>::iterator cursorIt;
	if (cursorFilePath != StringIndex_Null)
	{
		cursorIt = _fileSymbolDefines.find(cursorFilePath);
		if (cursorIt == _fileSymbolDefines.end())
			cursorIt = _fileSymbolDefines.begin();
	}
	else
		cursorIt = _fileSymbolDefines.begin();

	bool isAnyAction = false;
	int nSteps = 0;
	while (cursorIt != _fileSymbolDefines.end())
	{
		if (_NeedResetThreadLoop())
			break;

		if (GetAbsTick() > startT + budgetDur)
			break;

		CFileSymbolDefines& defines = (*cursorIt).second;

		if (defines._isParsing)
		{
			if (_StepCursor(cursorIt, nSteps))
				break;
			continue;
		}
		if (!_CheckReferenced(defines))
		{
			if (_StepCursor(cursorIt, nSteps))
				break;
			continue;
		}
		if (!_CheckOutOfDate(defines))
		{
			if (_StepCursor(cursorIt, nSteps))
				break;
			continue;
		}
		isAnyAction = true;
		if (defines._isMasterFile)
		{
			if (_Parse(defines))
			{
				if (_StepCursor(cursorIt, nSteps))
					break;
			}
			else
				break;
		}
		else
		{
			//所有include自己的master都需要parse
			bool needAbort = false;
			for (std::unordered_map<StringIndex, MasterIncluder>::iterator it = defines._masterIncluders.begin();it != defines._masterIncluders.end();)
			{
				StringIndex includerFilePath = it->first;
				std::unordered_map<StringIndex, CFileSymbolDefines>::iterator it2 = _fileSymbolDefines.find(includerFilePath);
				if (it2 != _fileSymbolDefines.end())
				{
					if (it2->second._masterFileParseVer == it->second.parseVer)
					{
						CFileSymbolDefines& includerDefines = (*it2).second;
						if (includerDefines._isParsing)
						{
							it++;
							continue;
						}
						if (_Parse(includerDefines))
						{
							it++;
							continue;
						}
						else
						{
							needAbort = true;
							break;
						}
					}
				}
				//这个includer已经不存在或者不再include自己了,删掉它
				it = defines._masterIncluders.erase(it);
				continue;
			}
			if (needAbort)
				break;
			if (_StepCursor(cursorIt, nSteps))//all includer processed, move to next cursor
				break;
		}
	}

	if (cursorIt == _fileSymbolDefines.end())
		cursorFilePath = StringIndex_Null;
	else
		cursorFilePath = (*cursorIt).first;

	return isAnyAction;
}

bool CSymbolDB::_NeedResetThreadLoop()
{
	if (!_updateThreadRunning.load())
		return true;
	if (_resetThreadLoop.load())
		return true;
	return false;
}

void CSymbolDB::_UpdateThreadProc()
{
	StringIndex cursorParseFile = StringIndex_Null;
	while (_updateThreadRunning.load())
	{
		_resetThreadLoop = false;

		bool isAnyAction = false;
		AbsTick startT = GetAbsTick();

		isAnyAction = isAnyAction | _ProcessOps();
		if (_NeedResetThreadLoop())
			continue;

		//Fetch pch result,无budget(允许花任意长的时间)
		if (true)
		{
			ParseResult result;
			while (_parserForPCH.FetchResult(result))
			{
				ProcessParseResult(result);
				isAnyAction = true;
			}
			if (_NeedResetThreadLoop())
				continue;
		}

		//Fetch result
		if (true)
		{
			const AbsTick budgetDur = 20;
			AbsTick startT = GetAbsTick();
			ParseResult result;
			while (_parser.FetchResult(result))
			{
				ProcessParseResult(result);
				isAnyAction = true;
				if (GetAbsTick() > startT + budgetDur)
					break;
				if (_NeedResetThreadLoop())
					break;
			}
			if (_NeedResetThreadLoop())
				continue;
		}

		if (!_parserForPCH.IsFlushed())
			continue;

		//检查是否要更新pch
		if (_CheckAndParsePch())
			continue;

		if (_NeedResetThreadLoop())
			continue;

		//非pch文件
		{
			const AbsTick budgetDur = 2000000000000;
			if (_CheckAndParseNotPch(cursorParseFile, budgetDur))
				isAnyAction = true;
		}

		if (_NeedResetThreadLoop())
			continue;

		if (!_parser.IsFlushed())
			continue;
		if (isAnyAction)
			continue;

		Save();

		std::unique_lock<std::mutex> lock(_updateMutex);
		_updateCV.wait(lock, [this] 
		{
			return _NeedResetThreadLoop();
		});


// 		AbsTick costTick = GetAbsTick() - startT;
// 		if (costTick<50)
// 		{
// 			std::unique_lock<std::mutex> lock(_updateMutex);
// 			_updateCV.wait_for(lock, std::chrono::milliseconds(50-costTick), [this] {
// 				return _NeedResetThreadLoop();
// 			});
// 		}
	}
}


CppSymbol_End


