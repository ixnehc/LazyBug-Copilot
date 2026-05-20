#include "stdh.h"

#include "SymbolRefsCache.h"
#include "SolutionDBApi.h"
#include "Utils.h"
#include "stringparser/stringparser.h"
#include "datapacket/DataPacket.h"
#include "codediff/dmp_diff.h"

CSymbolRefsCache::CSymbolRefsCache() : _isCaching(false)
{
	// 使用Windows API生成临时文件名
	char tempPath[MAX_PATH];
	char tempFileName[MAX_PATH];

	// 获取系统临时目录
	DWORD tempPathLen = GetTempPathA(MAX_PATH, tempPath);
	if (tempPathLen == 0 || tempPathLen > MAX_PATH)
	{
		// 如果获取失败，使用当前目录
		strcpy_s(tempPath, MAX_PATH, ".\\");
	}

	// 生成第一个临时文件名（文件内容）
	if (GetTempFileNameA(tempPath, "SRC", 0, tempFileName) != 0)
	{
		_fileContentTempFilePath = tempFileName;
	}
	else
	{
		// 如果失败，使用备用方案
		_fileContentTempFilePath = std::string(tempPath) + "temp_file_content.tmp";
	}

	// 生成第二个临时文件名（结果）
	if (GetTempFileNameA(tempPath, "SRR", 0, tempFileName) != 0)
	{
		_resultTempFilePath = tempFileName;
	}
	else
	{
		// 如果失败，使用备用方案
		_resultTempFilePath = std::string(tempPath) + "temp_result.tmp";
	}

}

CSymbolRefsCache::~CSymbolRefsCache()
{
	Clear();

	// 清理临时文件
	if (!_fileContentTempFilePath.empty())
	{
		Utils::RemoveFile(_fileContentTempFilePath.c_str());
	}
	if (!_resultTempFilePath.empty())
	{
		Utils::RemoveFile(_resultTempFilePath.c_str());
	}

}

void CSymbolRefsCache::Init(const char* dbFolderPath)
{
	_dbFolderPath = dbFolderPath ? dbFolderPath : "";
	
}

void CSymbolRefsCache::Clear()
{
	// 等待collect线程结束
	if (_cachingThread.joinable())
	{
		_cachingThread.join();
	}
	_isCaching = false;
	
	// 清除所有entries
	{
		std::lock_guard<std::mutex> lock(_entriesMutex);
		_entries.clear();
	}

	_dbFolderPath = "";
	
}

bool CSymbolRefsCache::CheckDirty(std::string& filePath)
{
	if (_dbFolderPath.empty())
		return false;

	// 转换为小写
	std::string lowerPath = filePath;
	StringLower(lowerPath);
	
	std::lock_guard<std::mutex> lock(_entriesMutex);
	auto it = _entries.find(lowerPath);
	if (it == _entries.end())
	{
		// 没有缓存，认为是dirty的
		return true;
	}
	
	return it->second.isDirty;
}

void CSymbolRefsCache::SetDirty(std::string& filePath)
{
	// 转换为小写
	std::string lowerPath = filePath;
	StringLower(lowerPath);
	
	std::lock_guard<std::mutex> lock(_entriesMutex);
	auto it = _entries.find(lowerPath);
	if (it != _entries.end())
	{
		it->second.isDirty = true;
	}
	else
	{
		// 如果entry不存在，创建一个新的并标记为dirty
		Entry newEntry;
		newEntry.isDirty = true;
		_entries[lowerPath] = newEntry;
	}
}

bool CSymbolRefsCache::IsCaching()
{
	return _isCaching.load();
}

bool CSymbolRefsCache::CacheRefs(std::string& filePath, std::string& fileContent)
{
	if (_dbFolderPath.empty())
		return false;

	// 检查是否已经有collect线程在运行
	if (_isCaching.load())
	{
		return false; // 已经有线程在运行
	}
	
	// 如果之前的线程已经结束，先join它
	if (_cachingThread.joinable())
	{
		_cachingThread.join();
	}
	
	// 转换为小写
	std::string lowerPath = filePath;
	StringLower(lowerPath);
	
	// 清除文件的cache失效标记
	{
		std::lock_guard<std::mutex> lock(_entriesMutex);
		auto it = _entries.find(lowerPath);
		if (it != _entries.end())
		{
			it->second.isDirty = false;
		}
		else
		{
			// 创建新的entry
			Entry newEntry;
			newEntry.isDirty = false;
			_entries[lowerPath] = newEntry;
		}
	}
	
	// 启动collect线程
	_isCaching = true;
	
	_cachingThread = std::thread([this, lowerPath, filePath, fileContent]() {
		// 保存文件内容到临时文件
		if (!Utils::SaveFileContent(_fileContentTempFilePath.c_str(), fileContent))
		{
			_isCaching = false;
			return;
		}
			
		// 准备CollectRefsParam
		CppSymbol::CollectRefsParam param;
		param.filePath = filePath;
		param.unsavedContentTempFilePath = _fileContentTempFilePath;
		param.resultTempFilePath = _resultTempFilePath;
			
		// 调用SolutionDB_CollectRef
		bool success = SolutionDB_CollectRefs(_dbFolderPath.c_str(), param);
			
		if (success)
		{
			// 读取结果文件
			std::vector<BYTE> resultData;
			if (Utils::LoadFileContent(_resultTempFilePath.c_str(), resultData))
			{
				// 解析结果
				CDataPacket dp;
				dp.SetDataBufferPointer(resultData.data());
					
				CppSymbol::RawSymbolRefs refs;
				refs.Load(dp);
					
				// 更新cache
				{
					std::lock_guard<std::mutex> lock(_entriesMutex);
					auto it = _entries.find(lowerPath);
					if (it != _entries.end())
					{
						it->second.refs = std::move(refs);
						it->second.fileContent = fileContent;
						it->second.lines.clear();
					}
				}
			}
		}
// 		else
// 		{
// 			std::lock_guard<std::mutex> lock(_entriesMutex);
// 			auto it = _entries.find(lowerPath);
// 			if (it != _entries.end())
// 				it->second.isDirty = true;
// 		}
		
		_isCaching = false;
	});
	
	return true;
}


int CSymbolRefsCache::_FindClosestLineInCache(Entry* entry,const std::vector<std::string>& lines, int lineNum,std::atomic<bool>& cancelFlag)
{
	// 检查边界
	if (lineNum <= 0 || lineNum > (int)lines.size())
		return -1;

	// 检查是否被取消
	if (cancelFlag)
		return -1;

	// 分割成行
	if (entry->lines.empty())
		SplitLines(entry->fileContent, entry->lines);

	// 使用 MyersDiff 进行行级别的差异分析
	MyersDiff<std::vector<std::string>> diff(entry->lines, lines);
	const auto& diffs = diff.diffs();

	int oldLine = 0;
	int newLine = 0;
	int closestOldLine = -1;
	int bestDist = INT_MAX;

	for (const auto& d : diffs)
	{
		int len = d.text.size();
		if (d.operation == DiffOp_Equal)
		{
			// 区间对应: [newLine, newLine+len) <-> [oldLine, oldLine+len)
			if (lineNum >= newLine && lineNum < newLine + len)
			{
				// lineNum 直接对应某一行
				return oldLine + (lineNum - newLine);
			}
			else
			{
				// 不直接落在Equal区间，找最近边界
				int distToStart = abs(lineNum - newLine);
				if (distToStart < bestDist)
				{
					bestDist = distToStart;
					closestOldLine = oldLine;
				}
				int distToEnd = abs(lineNum - (newLine + len - 1));
				if (distToEnd < bestDist)
				{
					bestDist = distToEnd;
					closestOldLine = oldLine + len - 1;
				}
			}
			oldLine += len;
			newLine += len;
		}
		else if (d.operation == DiffOp_Insert)
		{
			newLine += len;
		}
		else if (d.operation == DiffOp_Delete)
		{
			oldLine += len;
		}

		// 检查取消
		if (cancelFlag)
			return -1;
	}

	return closestOldLine;
}

static void TryAddRef(CSymbolRefsCache::RefSnippets& resultSnippets, const CppSymbol::RawSymbolRef& ref)
{
	switch (ref.refTarget.kind)
	{
		case SymbolKind::Class:
		case SymbolKind::Struct:
		case SymbolKind::Enum:
		case SymbolKind::Field:
		case SymbolKind::Macro:
		{
			resultSnippets.Add(ref.refTarget);
			break;
		}
		case SymbolKind::Method:
		{
			if (ref.kind == SymbolKind::CallExpr)
			{
				resultSnippets.Add(ref.refTarget);
			}
			break;
		}
	}

	switch (ref.refTargetParent.kind)
	{
		case SymbolKind::Class:
		case SymbolKind::Struct:
		case SymbolKind::Enum:
		case SymbolKind::Field:
		case SymbolKind::Macro:
		{
			resultSnippets.Add(ref.refTargetParent);
			break;
		}
	}
}

bool CSymbolRefsCache::_FindNearbyRefs(const Entry* entry, int lineNum, int range, RefSnippets& resultSnippets, std::atomic<bool>& cancelFlag)
{
	// 清理输出
	resultSnippets.Clear();

	if (!entry)
		return false;

	const CppSymbol::RawSymbolRefs& rs = entry->refs;
	if (rs.refs.empty())
		return false;

	if (range < 0) range = 0;
	int winStart = lineNum - range;
	if (winStart < 0) winStart = 0;
	int winEnd = lineNum + range; // 包含边界

	RefSnippets snippets;
	// 选择窗口内的引用，将其目标行区间加入结果
	for (const CppSymbol::RawSymbolRef& r : rs.refs)
	{
		int refLine = r.lineLoc.line; // 假定 SingleLineLoc 有 line 字段（0-based）
		if (refLine < winStart || refLine > winEnd)
			continue;

		TryAddRef(snippets, r);
	}

	if (cancelFlag)
		return false;

	// 合并每个文件内的重叠/相邻区间，保持有序
	for (auto& kv : snippets.fileRanges)
	{
		auto& ranges = kv.second;
		if (ranges.empty()) continue;
		std::sort(ranges.begin(), ranges.end(), [](const LineRange& a, const LineRange& b){
			if (a.start != b.start) return a.start < b.start;
			return a.end < b.end;
		});
		std::vector<LineRange> merged;
		merged.reserve(ranges.size());
		LineRange cur = ranges[0];
		for (size_t i = 1; i < ranges.size(); ++i)
		{
			if (ranges[i].start <= cur.end + 1)
			{
				cur.end = (std::max)(cur.end, ranges[i].end);
			}
			else
			{
				merged.push_back(cur);
				cur = ranges[i];
			}
		}
		merged.push_back(cur);
		ranges.swap(merged);

		if (cancelFlag)
			return false;
	}

	snippets.files = rs.targetFiles; // 保存路径+时间

	resultSnippets = std::move(snippets);

	return true;
}


bool CSymbolRefsCache::FindNearbyRefs(std::string& filePath, const std::vector<std::string>& lines, int lineNum, int range, RefSnippets& snippets, std::atomic<bool>& cancelFlag)
{
	std::lock_guard<std::mutex> lock(_entriesMutex);

	Entry* entry = nullptr;
	if (true)
	{
		auto it = _entries.find(filePath);
		if (it != _entries.end())
			entry = &it->second;
	}

	if (!entry)
		return false;

	int projLineNum = _FindClosestLineInCache(entry, lines, lineNum, cancelFlag);
	if (projLineNum < 0)
		return false;

	return _FindNearbyRefs(entry, projLineNum, range, snippets, cancelFlag);
}
