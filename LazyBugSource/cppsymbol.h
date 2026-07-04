#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <deque>
#include <time.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <fstream>
#include <direct.h>
#include <io.h>
#include <shared_mutex>

#include <clang-c/Index.h>
#include <clang-c/CXString.h>
#include <clang-c/CXSourceLocation.h>
#include <clang-c/CXFile.h>

#include "concurrent/lock.h"
#include "ProjSetting.h"
#include "StringPool.h"
#include "CppSymbolParser.h"
#include "NameIndexer.h"

#include "Log/LogFile.h"

class CDataPacket;
class CSolutionFiles;
struct SolutionFile;

#define CppSymbol_Begin namespace CppSymbol{
#define CppSymbol_End }

namespace CppSymbol
{
	void SaveFileLocation(const FileLocation &loc,CDataPacket& dp);
	void LoadFileLocation(FileLocation& loc,CDataPacket& dp, CStringPool& strPool);


	struct SymbolDefineQueryResult
	{
		std::string name;
		std::string desc;
		SymbolKind kind;
		FileLocation location;
	};

	struct SymbolDefineFindResult
	{
		SymbolKind kind;
		FileLocation location;
		LineRange lineRange;// symbol实现代码开始行/结束行
	};

	class CSymbolDefine
	{
	public:
		CSymbolDefine()
		{
			_name = StringIndex_Null;
			strName = "";
			_kind = SymbolKind::Invalid;
		}

		void Save(CDataPacket& dp) const;
		void Load(CDataPacket& dp, CStringPool& strPool);

	public://consider it as protected
		SymbolKind _kind;    // symbol类型
		StringIndex _name;           // symbol名称
		const char* strName;         // symbol名称字符串指针(调试用)
		FileLocation _location;      // symbol定义位置

		LineRange _lineRange;// symbol实现代码开始行/结束行
	};

	struct Inclusion
	{
		StringIndex path;
		const char* strPath;//调试用
		time_t time;
	};

	typedef DWORD MasterParseVer;
	struct MasterIncluder
	{
		const char* strPath;//调试用,master的文件名
		MasterParseVer parseVer;
	};

	class CFileSymbolDefines
	{
	public:
		CFileSymbolDefines()
		{
			_parsedTime = 0;
			_setting = ProjSettingHandle_Null;
			_isMasterFile = false;
			_isInSln = false;
			_masterFileParseVer = 0;

			_pchTime = 0;

			_filePath = StringIndex_Null;
			_filePathStr = "";

			_pchPath = StringIndex_Null;

			_isParsing = false;
		}
		void Save(CDataPacket& dp) const;
		void Load(CDataPacket& dp, CStringPool& strPool);
		
	public:
		StringIndex _filePath;//full path
		const char* _filePathStr;//调试用

		ProjSettingHandle _setting;
		MasterParseVer _masterFileParseVer;//作为主文件每parse一次版本号加1

		time_t _parsedTime;//这个文件上一次parse时的最后修改时间
		std::deque<CSymbolDefine> _parsedDefines;//上一次parse出来的symbol定义
		std::deque<Inclusion> _parsedInclusions;//上一次parse出来的包含文件
		std::unordered_map<StringIndex, MasterIncluder> _masterIncluders;//记录被那些master文件include,已及在哪一次parse中include的自己

		time_t _pchTime;

		//以下变量不需要保存
		StringIndex _pchPath;//setting里的pch文件名
		bool _isMasterFile;//是不是主文件(cpp/c文件)
		bool _isInSln;//是不是属于solution
		bool _isParsing;
	};
	 
	const int SYMBOL_FILES_VERSION = 2;

	class CSymbolFiles
	{
	public:
		void Init(const char* folderPath)
		{
			_folderPath = folderPath;
		}
		struct Bucket
		{
			Bucket()
			{
				isDirty = false;
			}
			bool isDirty;
		};

		int BucketFromFilePath(StringIndex filePath);
		void SetDirty(StringIndex filePath);
		void SetAllDirty();

		// 验证版本号，如果版本不匹配则清除所有bucket文件
		bool VerifyVersion();

		void Save(std::unordered_map < StringIndex, CFileSymbolDefines>& defines);
		void Load(std::unordered_map < StringIndex, CFileSymbolDefines>& defines, CStringPool& strPool);

	private:
		void _ClearAllBuckets();
		void _SaveVersion();

	public:
		Bucket _buckets[CStringPool::NUM_BUCKETS];
		std::string _folderPath;
	};

	class CSymbolDB
	{
	public:
		CSymbolDB()
		{
			_projSettingLib = nullptr;
			_nextRequestId = 1;
			_initTime=0;
			_updateThreadRunning = false;
			_resetThreadLoop = false;
		}

		//用于保存到nameIndexer的symbol数据,要和NameIndexData大小一致
		struct SymbolNameData
		{
			StringIndex name;
			int index;
		};
	
		void Init(const char* folderPath,CProjSettingLib&projSettingLib);
		void Clear();
		void SetContent(const CSolutionFiles& files);
		void SetDeltaContent(const std::vector<SolutionFile*>& newFiles, const std::vector<SolutionFile*>& updatedFiles, std::vector<std::string>& removedFiles);
		void NotifyFilesChanged();

		void Save();//返回有没有发生真正的Save

		void QuerySymbolDefine(const char* query, int maxResult,std::vector<SymbolDefineQueryResult>& results);
		void FindSymbolDefine(const char* symbolName, int maxResult, std::vector<SymbolDefineFindResult>& results);

		bool GetSymbolNamesFromLines(const char* filePath, std::vector<int>& lines, std::vector<StringIndex>& symbolNames);

		//搜集指定文件的所有引用信息
		bool CollectRefs(const CollectRefsParam &param);

		bool IsFileInDB(const char* filePath);
		CFileSymbolDefines* ObtainSymbolDefines(const char* filePath);

		void GetStr(StringIndex stringIndex, std::string& ret)		{			_strPool.GetStr(stringIndex, ret);		}
		StringIndex FindStr(const char* str)		{			return _strPool.FindStr(str);		}
		time_t GetParsedTime(StringIndex filePathIndex) const;

		// 获取指定文件的所有 symbol 类型及行范围（已排序）
		// 返回 _parsedTime（通过 outParsedTime），若文件未解析则返回 false
		bool GetSymbolLineRanges(StringIndex filePathIndex,
		                         std::vector<SymbolRangeInfo>& outRanges,
		                         time_t& outParsedTime) const;

		// 获取和设置脏标记

	public://consider it as protected
		std::string _folderPath;//db 的folder路径
// 		std::string _workspacePath;
// 		std::string _lowerCasedWorkspacePath;

		CProjSettingLib* _projSettingLib;

		void _LoadStrLib();
		bool _SaveStrLib();
		CStringPool _strPool;

		void _UpdatePchFile(CFileSymbolDefines& defines);
		void _UpdateMasterFile(CFileSymbolDefines& defines);
		void _UpdatePchGenFiles(CFileSymbolDefines& defines);
		void _UnIndex(CFileSymbolDefines& defines);
		void _Index(CFileSymbolDefines* pFileDefines, CSymbolDefine& define);
		bool _IsSlnReferenced(const CFileSymbolDefines* defines);
		void _ClearParsed(CFileSymbolDefines& define);
		void _ClearUnreferenceDefines();
		void _LoadSymbolIndexer();
		void _SaveSymbolIndexer();
		CFileSymbolDefines* _FindSymbolDefines(const char* filePath);

		std::unordered_map < StringIndex,CFileSymbolDefines> _fileSymbolDefines;//各个文件的symbol定义
		CNameIndexer _symbolIndexer;
		mutable std::shared_mutex _symbolMutex; // 保护_fileSymbolDefines和_symbolIndexer的互斥锁

		CSymbolFiles _files;
		std::unordered_set<StringIndex> _pchGenFiles;//哪些文件会用来生成pch

	private:
		// 处理解析结果
		void ProcessParseResult(const ParseResult &result);

	private:
		// 生成包含相对时间戳的请求ID(高32位为初始化后的相对时间,低32位为序号)
		ParseRequestId _GenRequestId();
		ParseRequestId _nextRequestId;
		AbsTick _initTime;

	private:
		void _InitParser();
		void _ClearParser();
		bool _Parse(ParseRequest& request);
		bool _Parse(CFileSymbolDefines& defines);
		bool _CheckReferenced(CFileSymbolDefines& defines);
		bool _CheckOutOfDate(const CFileSymbolDefines& defines);
		bool _CheckAndParsePch();
		bool _CheckAndParseNotPch(StringIndex& cursorFilePath, AbsTick budgetDur);
		bool _StepCursor(std::unordered_map<StringIndex, CFileSymbolDefines>::iterator& cursorIt, int& nSteps);//返回是否已经轮了一圈了

		CSymbolParser _parser;  // 低优先级解析池
		CSymbolParser _parserForPCH;  // 高优先级解析池

		CSymbolParser _parserForCollectRef;  // 最高优先级解析池
		std::mutex _collectRefParseMutex;

	private:
		struct Op
		{
			enum Type
			{
				NewFile,
				UpdateFileSetting,
				DeleteFile,
			};
			Type tp;
			StringIndex filePath;
			ProjSettingHandle setting;
		};

		bool _ProcessOps();//返回是否处理了op
		void _ProcessOp(const Op& op);
		void _AddOp(const Op&op);
		std::deque<Op> _ops;
		std::mutex _opsMutex; // 保护_ops的互斥锁

	private:
		// 更新线程相关
		void _StartUpdateThread();
		void _StopUpdateThread();
		void _UpdateThreadProc();
		bool _NeedResetThreadLoop();

		std::thread _updateThread;
		std::mutex _updateMutex;
		std::condition_variable _updateCV;
		std::atomic<bool> _updateThreadRunning;
		std::atomic<bool> _resetThreadLoop;
		std::atomic<int> _debugTotalParsed;
		AbsTick _debugLastParsedTime;

		LogFile _logFile;
	};

}

