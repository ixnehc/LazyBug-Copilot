#pragma once

#include "SolutionDBDefines.h"

#include <unordered_map>
#include <unordered_set>
#include <deque>

#include "timer/timer.h"

class CDataPacket;

//#define ENABLE_TREESITTER_PARSER_DEBUG 

#define TreeSitterSymbol_Begin namespace TreeSitterSymbol{
#define TreeSitterSymbol_End }


namespace TreeSitterSymbol
{
	typedef unsigned __int64 ParseRequestId;
	const ParseRequestId ParseRequestId_Invalid = 0;


	struct RawSymbolDefine
	{
		std::string name;
		std::string showName;
		SymbolKind kind;    // 统一符号类型
		Language language;  // 语言标识
		SingleLineLoc lineLoc;
		LineRange lineRange;// symbol实现代码开始行/结束行

		void Save(CDataPacket& dp) const;
		void Load(CDataPacket& dp);
	};

	struct RawSymbolRef
	{
		std::string name;
		SingleLineLoc lineLoc;//引用部分的位置
		SymbolKind kind;

		struct RefTarget
		{
			RefTarget()
			{
				Zero();
			}
			void Zero()
			{
				fileIndex = 0xffff;
				kind = SymbolKind::Invalid;
				lineRange.start = lineRange.end = 0;
			}
			bool IsValid() const
			{
				return fileIndex != 0xffff;
			}
			LineRange lineRange;
			WORD fileIndex;//0xffff表示无效
			SymbolKind kind : 16;
		};

		//被引用部分的范围
		RefTarget refTarget;
		RefTarget refTargetParent;

		void Save(CDataPacket& dp) const;
		void Load(CDataPacket& dp);
	};

	struct RawSymbolRefs
	{
		std::vector<RawSymbolRef> refs;

		struct File
		{
			std::string path;
			time_t time;
		};
		std::vector<File> targetFiles;

		void Save(CDataPacket& dp) const;
		void Load(CDataPacket& dp);
	};

	struct CollectRefsParam
	{
		bool IsValid() const
		{
			return !filePath.empty();
		}
		std::string filePath;//collect哪个文件的Refs
		std::string unsavedContentTempFilePath;//要collect的文件的未保存内容保存在哪个临时文件
		std::string resultTempFilePath;//结果保存在哪个临时文件,结果为一个RawSymbolRefs对象

		void Save(CDataPacket& dp) const;
		void Load(CDataPacket& dp);
	};

	struct ParseRequest
	{
		ParseRequest()
		{
			requestId = 0;
			setting = nullptr;
			language = Language::Unknown;
		}
		void MoveFrom(ParseRequest& src)
		{
			lowerCasedParseFilePath = std::move(src.lowerCasedParseFilePath);
			collectRefParam = std::move(src.collectRefParam);
			setting = src.setting;
			requestId = src.requestId;
			language = src.language;
#ifdef ENABLE_TREESITTER_PARSER_DEBUG
			debugStartTime = src.debugStartTime;
#endif
		}
		bool NeedParse() const
		{
			return !lowerCasedParseFilePath.empty();
		}

		// 序列化方法
		void Save(CDataPacket& dp) const;
		void Load(CDataPacket& dp, ProjSetting& setting);

		std::string lowerCasedParseFilePath;//parse哪个文件

		CollectRefsParam collectRefParam;

		const ProjSetting* setting;

		ParseRequestId requestId;  // 请求序号

		Language language; // 语言类型

#ifdef ENABLE_TREESITTER_PARSER_DEBUG
		AbsTick debugStartTime;
#endif
	};

	struct ParseResult
	{
		ParseResult()
		{
			success = false;
			requestId = 0;
			discarded = false;
#ifdef ENABLE_TREESITTER_PARSER_DEBUG
			debugBaseTime = 0;
#endif
		}

		// 序列化方法
		void Save(CDataPacket& dp) const;
		void Load(CDataPacket& dp, bool appendDebug);

		//注意ParseResult里的所有文件路径名都是小写的
		std::string parseFilePath;      //parse哪个文件

		std::unordered_map<std::string, std::vector<RawSymbolDefine>> definesByFile;
		std::unordered_map<std::string, time_t> fileTimes;

		bool success;             // 是否解析成功

		ParseRequestId requestId;       // 请求序号
		bool discarded;           // 是否被丢弃

#ifdef ENABLE_TREESITTER_PARSER_DEBUG
		void DumpDebugString(std::string& s) const;
		void AddDebugTime(const char* name);
		struct DebugEntry
		{
			std::string name;
			AbsTick t;
		};
		AbsTick debugBaseTime;
		std::vector<DebugEntry> debugTimes;
#else
		void AddDebugTime(const char* name) { }

#endif
	};

}
