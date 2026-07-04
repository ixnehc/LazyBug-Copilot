#include "stdh.h"
#include "CppSymbolDefines.h"
#include "datapacket/DataPacket.h"
#include "stringparser/stringparser.h"

#include "timer/timer.h"


CppSymbol_Begin

//////////////////////////////////////////////////////////////////////////
//RawSymbolDefine

void RawSymbolDefine::Save(CDataPacket& dp) const
{
	dp.Data_WriteString(name);
	dp.Data_WriteString(showName);
	int k = (int)kind;
	dp.Data_WriteSimple(k);

	// 保存location
	dp.Data_WriteSimpleR(lineLoc);

	dp.Data_WriteSimpleR(lineRange);
}

void RawSymbolDefine::Load(CDataPacket& dp)
{
	dp.Data_ReadString(name);
	dp.Data_ReadString(showName);
	int k;
	dp.Data_ReadSimple(k);
	kind = (SymbolKind)k;

	// 加载location
	dp.Data_ReadSimple(lineLoc);

	dp.Data_ReadSimple(lineRange);
}

//////////////////////////////////////////////////////////////////////////
//RawSymbolRef

void RawSymbolRef::Save(CDataPacket& dp) const
{
	dp.Data_WriteString(name);

	// 保存kind
	char k = (char)kind;
	dp.Data_WriteSimple(k);

	// 保存引用位置
	dp.Data_WriteSimpleR(lineLoc);

	dp.Data_WriteSimpleR(refTarget);
	dp.Data_WriteSimpleR(refTargetParent);
}

void RawSymbolRef::Load(CDataPacket& dp)
{
	dp.Data_ReadString(name);

	char k;
	dp.Data_ReadSimple(k);
	kind = (SymbolKind)k;

	dp.Data_ReadSimple(lineLoc);


	dp.Data_ReadSimple(refTarget);
	dp.Data_ReadSimple(refTargetParent);
}

//////////////////////////////////////////////////////////////////////////
//RawSymbolRefs

void RawSymbolRefs::Save(CDataPacket& dp) const
{
	// 保存refs
	int refCount = static_cast<int>(refs.size());
	dp.Data_WriteSimple(refCount);
	for (const RawSymbolRef& ref : refs)
	{
		ref.Save(dp);
	}

	// 保存targetFiles
	int targetCount = static_cast<int>(targetFiles.size());
	dp.Data_WriteSimple(targetCount);
	for (const File& file : targetFiles)
	{
		dp.Data_WriteString(file.path);
		dp.Data_WriteSimple(file.time);
	}
}

void RawSymbolRefs::Load(CDataPacket& dp)
{
	// 读取refs
	int refCount;
	dp.Data_ReadSimple(refCount);
	refs.resize(refCount);
	for (int i = 0; i < refCount; i++)
	{
		refs[i].Load(dp);
	}

	// 读取targetFiles
	int targetCount;
	dp.Data_ReadSimple(targetCount);
	targetFiles.resize(targetCount);
	for (int i = 0; i < targetCount; i++)
	{
		dp.Data_ReadString(targetFiles[i].path);
		dp.Data_ReadSimple(targetFiles[i].time);
	}
}


//////////////////////////////////////////////////////////////////////////
//RawInclusion

//////////////////////////////////////////////////////////////////////////
//CollectRefParam

void CollectRefsParam::Save(CDataPacket& dp) const
{
	dp.Data_WriteString(filePath);
	dp.Data_WriteString(unsavedContentTempFilePath);
	dp.Data_WriteString(resultTempFilePath);
}

void CollectRefsParam::Load(CDataPacket& dp)
{
	dp.Data_ReadString(filePath);
	dp.Data_ReadString(unsavedContentTempFilePath);
	dp.Data_ReadString(resultTempFilePath);
}



void RawInclusion::Save(CDataPacket& dp) const
{
	dp.Data_WriteSimple(time);
}

void RawInclusion::Load(CDataPacket& dp)
{
	dp.Data_ReadSimple(time);
}

//////////////////////////////////////////////////////////////////////////
//ParseRequest

void ParseRequest::Save(CDataPacket& dp) const
{
	dp.Data_WriteString(lowerCasedParseFilePath);
	dp.Data_WriteSimple(requestId);

	// 保存 collectRefParam
	collectRefParam.Save(dp);

	// 序列化ProjSetting（如果存在）
	if (setting)
	{
		dp.Data_WriteSimple(true); // 标记有setting
		
		// 序列化additionalIncludeFullPathes
		int pathCount = (int)setting->additionalIncludeFullPathes.size();
		dp.Data_WriteSimple(pathCount);
		for (const std::string& path : setting->additionalIncludeFullPathes)
		{
			dp.Data_WriteString(path);
		}
		
		// 序列化pch路径
		dp.Data_WriteString(setting->lowerCasedPchFullPath);
		dp.Data_WriteString(setting->lowerCasedPchOutputFullPath);
	}
	else
	{
		dp.Data_WriteSimple(false); // 标记没有setting
	}

#ifdef ENABLE_PARSER_DEBUG
	dp.Data_WriteSimple(debugStartTime);
#endif
}

void ParseRequest::Load(CDataPacket& dp, ProjSetting& projSetting)
{
	dp.Data_ReadString(lowerCasedParseFilePath);
	dp.Data_ReadSimple(requestId);

	// 加载 collectRefParam
	collectRefParam.Load(dp);

	bool hasSetting;
	dp.Data_ReadSimple(hasSetting);
	
	if (hasSetting)
	{
		// 读取additionalIncludeFullPathes
		int pathCount;
		dp.Data_ReadSimple(pathCount);
		projSetting.additionalIncludeFullPathes.clear();
		projSetting.additionalIncludeFullPathes.resize(pathCount);
		for (int i = 0; i < pathCount; i++)
		{
			dp.Data_ReadString(projSetting.additionalIncludeFullPathes[i]);
		}
		
		// 读取pch路径
		dp.Data_ReadString(projSetting.lowerCasedPchFullPath);
		dp.Data_ReadString(projSetting.lowerCasedPchOutputFullPath);
		
		// 设置setting指针指向传入的对象
		setting = &projSetting;
	}
	else
	{
		// 没有setting数据
		setting = nullptr;
	}

#ifdef ENABLE_PARSER_DEBUG
	dp.Data_ReadSimple(debugStartTime);
#endif
}


//////////////////////////////////////////////////////////////////////////
//ParseResult

void ParseResult::Save(CDataPacket& dp) const
{
	dp.Data_WriteString(parseFilePath);
	dp.Data_WriteSimple(success);
	dp.Data_WriteSimple(isPCHGenerated);
	dp.Data_WriteSimple(pchTime);
	dp.Data_WriteSimple(requestId);
	dp.Data_WriteSimple(discarded);
	
	// 序列化definesByFile
	if (true)
	{
		int fileCount = (int)definesByFile.size();
		dp.Data_WriteSimple(fileCount);
		for (const auto& filePair : definesByFile)
		{
			dp.Data_WriteString(filePair.first); // 文件路径
			int defineCount = (int)filePair.second.size();
			dp.Data_WriteSimple(defineCount);
			for (const RawSymbolDefine& define : filePair.second)
			{
				define.Save(dp);
			}
		}
	}
	
	// 序列化fileTimes
	if (true)
	{
		int timeCount = (int)fileTimes.size();
		dp.Data_WriteSimple(timeCount);
		for (const auto& timePair : fileTimes)
		{
			dp.Data_WriteString(timePair.first); // 文件路径
			dp.Data_WriteSimple(timePair.second); // 时间
		}
	}
	
	// 序列化inclusions
	if (true)
	{
		int inclusionCount = (int)inclusions.size();
		dp.Data_WriteSimple(inclusionCount);
		for (const auto& inclusionPair : inclusions)
		{
			dp.Data_WriteString(inclusionPair.first); // 文件路径（键）
			inclusionPair.second.Save(dp); // RawInclusion对象（值）
		}
	}
	
	// 序列化debugTimes
#ifdef ENABLE_PARSER_DEBUG
	dp.Data_WriteSimple(debugBaseTime);
	int debugCount = (int)debugTimes.size();
	dp.Data_WriteSimple(debugCount);
	for (const DebugEntry& entry : debugTimes)
	{
		dp.Data_WriteString(entry.name);
		dp.Data_WriteSimple(entry.t);
	}
#endif
}

void ParseResult::Load(CDataPacket& dp,bool appendDebug)
{
	dp.Data_ReadString(parseFilePath);
	dp.Data_ReadSimple(success);
	dp.Data_ReadSimple(isPCHGenerated);
	dp.Data_ReadSimple(pchTime);
	dp.Data_ReadSimple(requestId);
	dp.Data_ReadSimple(discarded);
	
	// 加载definesByFile
	if (true)
	{
		definesByFile.clear();
		int fileCount;
		dp.Data_ReadSimple(fileCount);
		for (int i = 0; i < fileCount; i++)
		{
			std::string filePath;
			dp.Data_ReadString(filePath);
			int defineCount;
			dp.Data_ReadSimple(defineCount);
			std::vector<RawSymbolDefine>& defines = definesByFile[filePath];
			defines.resize(defineCount);
			for (int j = 0; j < defineCount; j++)
			{
				defines[j].Load(dp);
			}
		}
	}
	
	// 加载fileTimes
	if (true)
	{
		fileTimes.clear();
		int timeCount;
		dp.Data_ReadSimple(timeCount);
		for (int i = 0; i < timeCount; i++)
		{
			std::string filePath;
			dp.Data_ReadString(filePath);
			time_t time;
			dp.Data_ReadSimple(time);
			fileTimes[filePath] = time;
		}
	}
	
	// 加载inclusions
	if (true)
	{
		inclusions.clear();
		int inclusionCount;
		dp.Data_ReadSimple(inclusionCount);
		for (int i = 0; i < inclusionCount; i++)
		{
			std::string filePath;
			dp.Data_ReadString(filePath); // 读取文件路径（键）
			RawInclusion inclusion;
			inclusion.Load(dp); // 读取RawInclusion对象（值）
			inclusions[filePath] = inclusion; // 存储到map中
		}
	}
	
	// 加载debugTimes
#ifdef ENABLE_PARSER_DEBUG
	AbsTick loadedBaseTime;
	dp.Data_ReadSimple(loadedBaseTime);
	if (!appendDebug) 
	{
		debugBaseTime = loadedBaseTime;
		debugTimes.clear();
	} 
	int debugCount;
	dp.Data_ReadSimple(debugCount);
	size_t currentSize = debugTimes.size();
	debugTimes.resize(currentSize + debugCount);
	for (int i = 0; i < debugCount; i++)
	{
		dp.Data_ReadString(debugTimes[currentSize + i].name);
		dp.Data_ReadSimple(debugTimes[currentSize + i].t);
	}
#endif
}

#ifdef ENABLE_PARSER_DEBUG

void ParseResult::AddDebugTime(const char* name)
{
	DebugEntry entry;
	entry.name = name;
	entry.t = GetAbsTick();
	debugTimes.push_back(entry);
}

void ParseResult::DumpDebugString(std::string& s) const
{
    for (size_t i = 0; i < debugTimes.size(); ++i)
    {
        const DebugEntry& entry = debugTimes[i];
        AbsTick diff = (i==0)?entry.t - debugBaseTime: entry.t- debugTimes[i-1].t;
        s += entry.name + ": " + std::to_string(diff);
        if (i < debugTimes.size() - 1)
        {
            s += ", ";
        }
    }
}

#endif



CppSymbol_End