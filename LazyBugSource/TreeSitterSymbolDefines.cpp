#include "stdh.h"
#include "TreeSitterSymbolDefines.h"
#include "datapacket/DataPacket.h"

TreeSitterSymbol_Begin

//////////////////////////////////////////////////////////////////////////
// RawSymbolDefine

void RawSymbolDefine::Save(CDataPacket& dp) const
{
	// 保存名称
	dp.Data_WriteSimple(name);
	dp.Data_WriteSimple(showName);
	
	// 保存类型
	int kindInt = (int)kind;
	dp.Data_WriteSimple(kindInt);
	
	// 保存语言
	int langInt = (int)language;
	dp.Data_WriteSimple(langInt);
	
	// 保存位置
	dp.Data_WriteSimple(lineLoc);
	
	// 保存代码行范围
	dp.Data_WriteSimple(lineRange);
}

void RawSymbolDefine::Load(CDataPacket& dp)
{
	// 加载名称
	dp.Data_ReadSimple(name);
	dp.Data_ReadSimple(showName);
	
	// 加载类型
	int kindInt;
	dp.Data_ReadSimple(kindInt);
	kind = (SymbolKind)kindInt;
	
	// 加载语言
	int langInt;
	dp.Data_ReadSimple(langInt);
	language = (Language)langInt;
	
	// 加载位置
	dp.Data_ReadSimple(lineLoc);
	
	// 加载代码行范围
	dp.Data_ReadSimple(lineRange);
}

//////////////////////////////////////////////////////////////////////////
// RawSymbolRef

void RawSymbolRef::Save(CDataPacket& dp) const
{
	// 保存名称
	dp.Data_WriteSimple(name);
	
	// 保存位置
	dp.Data_WriteSimple(lineLoc);
	
	// 保存类型
	int kindInt = (int)kind;
	dp.Data_WriteSimple(kindInt);
	
	// 保存引用目标
	dp.Data_WriteSimple(refTarget.lineRange);
	dp.Data_WriteSimple(refTarget.fileIndex);
	int targetKindInt = (int)refTarget.kind;
	dp.Data_WriteSimple(targetKindInt);
	
	// 保存父引用目标
	dp.Data_WriteSimple(refTargetParent.lineRange);
	dp.Data_WriteSimple(refTargetParent.fileIndex);
	int parentKindInt = (int)refTargetParent.kind;
	dp.Data_WriteSimple(parentKindInt);
}

void RawSymbolRef::Load(CDataPacket& dp)
{
	// 加载名称
	dp.Data_ReadSimple(name);
	
	// 加载位置
	dp.Data_ReadSimple(lineLoc);
	
	// 加载类型
	int kindInt;
	dp.Data_ReadSimple(kindInt);
	kind = (SymbolKind)kindInt;
	
	// 加载引用目标
	dp.Data_ReadSimple(refTarget.lineRange);
	dp.Data_ReadSimple(refTarget.fileIndex);
	int targetKindInt;
	dp.Data_ReadSimple(targetKindInt);
	refTarget.kind = (SymbolKind)targetKindInt;
	
	// 加载父引用目标
	dp.Data_ReadSimple(refTargetParent.lineRange);
	dp.Data_ReadSimple(refTargetParent.fileIndex);
	int parentKindInt;
	dp.Data_ReadSimple(parentKindInt);
	refTargetParent.kind = (SymbolKind)parentKindInt;
}

//////////////////////////////////////////////////////////////////////////
// RawSymbolRefs

void RawSymbolRefs::Save(CDataPacket& dp) const
{
	// 保存引用数量
	int count = (int)refs.size();
	dp.Data_WriteSimple(count);
	
	// 保存每个引用
	for (const RawSymbolRef& ref : refs)
	{
		ref.Save(dp);
	}
	
	// 保存文件数量
	int fileCount = (int)targetFiles.size();
	dp.Data_WriteSimple(fileCount);
	
	// 保存每个文件
	for (const File& file : targetFiles)
	{
		dp.Data_WriteSimple(file.path);
		dp.Data_WriteSimple(file.time);
	}
}

void RawSymbolRefs::Load(CDataPacket& dp)
{
	// 加载引用数量
	int count;
	dp.Data_ReadSimple(count);
	refs.resize(count);
	
	// 加载每个引用
	for (RawSymbolRef& ref : refs)
	{
		ref.Load(dp);
	}
	
	// 加载文件数量
	int fileCount;
	dp.Data_ReadSimple(fileCount);
	targetFiles.resize(fileCount);
	
	// 加载每个文件
	for (File& file : targetFiles)
	{
		dp.Data_ReadSimple(file.path);
		dp.Data_ReadSimple(file.time);
	}
}

//////////////////////////////////////////////////////////////////////////
// CollectRefsParam

void CollectRefsParam::Save(CDataPacket& dp) const
{
	dp.Data_WriteSimple(filePath);
	dp.Data_WriteSimple(unsavedContentTempFilePath);
	dp.Data_WriteSimple(resultTempFilePath);
}

void CollectRefsParam::Load(CDataPacket& dp)
{
	dp.Data_ReadSimple(filePath);
	dp.Data_ReadSimple(unsavedContentTempFilePath);
	dp.Data_ReadSimple(resultTempFilePath);
}

//////////////////////////////////////////////////////////////////////////
// ParseRequest

void ParseRequest::Save(CDataPacket& dp) const
{
	dp.Data_WriteSimple(lowerCasedParseFilePath);
	dp.Data_WriteSimple(requestId);
	
	collectRefParam.Save(dp);
	
	// 保存语言
	int langInt = (int)language;
	dp.Data_WriteSimple(langInt);
	
	// setting 不需要保存，在子进程中会重新设置
}

void ParseRequest::Load(CDataPacket& dp, ProjSetting& setting)
{
	dp.Data_ReadSimple(lowerCasedParseFilePath);
	dp.Data_ReadSimple(requestId);
	
	collectRefParam.Load(dp);
	
	// 加载语言
	int langInt;
	dp.Data_ReadSimple(langInt);
	language = (Language)langInt;
	
	// setting 从外部传入
	setting = setting;
}

//////////////////////////////////////////////////////////////////////////
// ParseResult

void ParseResult::Save(CDataPacket& dp) const
{
	dp.Data_WriteSimple(parseFilePath);
	dp.Data_WriteSimple(success);
	dp.Data_WriteSimple(requestId);
	dp.Data_WriteSimple(discarded);
	
	// 保存definesByFile
	int fileCount = (int)definesByFile.size();
	dp.Data_WriteSimple(fileCount);
	
	for (const auto& pair : definesByFile)
	{
		dp.Data_WriteSimple(pair.first);
		
		int defineCount = (int)pair.second.size();
		dp.Data_WriteSimple(defineCount);
		
		for (const RawSymbolDefine& define : pair.second)
		{
			define.Save(dp);
		}
	}
	
	// 保存fileTimes
	int timeCount = (int)fileTimes.size();
	dp.Data_WriteSimple(timeCount);
	
	for (const auto& pair : fileTimes)
	{
		dp.Data_WriteSimple(pair.first);
		dp.Data_WriteSimple(pair.second);
	}
}

void ParseResult::Load(CDataPacket& dp, bool appendDebug)
{
	dp.Data_ReadSimple(parseFilePath);
	dp.Data_ReadSimple(success);
	dp.Data_ReadSimple(requestId);
	dp.Data_ReadSimple(discarded);
	
	// 加载definesByFile
	int fileCount;
	dp.Data_ReadSimple(fileCount);
	
	for (int i = 0; i < fileCount; i++)
	{
		std::string filePath;
		dp.Data_ReadSimple(filePath);
		
		int defineCount;
		dp.Data_ReadSimple(defineCount);
		
		std::vector<RawSymbolDefine> defines;
		defines.resize(defineCount);
		
		for (int j = 0; j < defineCount; j++)
		{
			defines[j].Load(dp);
		}
		
		definesByFile[filePath] = std::move(defines);
	}
	
	// 加载fileTimes
	int timeCount;
	dp.Data_ReadSimple(timeCount);
	
	for (int i = 0; i < timeCount; i++)
	{
		std::string filePath;
		time_t fileTime;
		dp.Data_ReadSimple(filePath);
		dp.Data_ReadSimple(fileTime);
		
		fileTimes[filePath] = fileTime;
	}
}

#ifdef ENABLE_TREESITTER_PARSER_DEBUG
void ParseResult::DumpDebugString(std::string& s) const
{
	s = "ParseResult: ";
	s += parseFilePath;
	s += ", success=";
	s += success ? "true" : "false";
	s += ", requestId=";
	s += std::to_string(requestId);
	s += ", definesByFile.size()=";
	s += std::to_string(definesByFile.size());
	
	for (const auto& pair : definesByFile)
	{
		s += "\n  File: ";
		s += pair.first;
		s += ", defines=";
		s += std::to_string(pair.second.size());
	}
}

void ParseResult::AddDebugTime(const char* name)
{
	DebugEntry entry;
	entry.name = name;
	entry.t = GetAbsTick();
	debugTimes.push_back(entry);
}
#endif

TreeSitterSymbol_End
