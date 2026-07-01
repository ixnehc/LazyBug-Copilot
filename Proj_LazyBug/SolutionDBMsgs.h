#pragma once

#include "PipeMsg.h"
#include <string>
#include <memory>

#include "CppSymbolDefines.h"

extern PipeMsgPtr CreateSolutionDBMsg(PipeMsgType id);

enum class SolutionDBMsgType
{
	None,
	RequestOpen,
	Opened,

	QueryNameItems,
	NameItems,

	CollectRefs,
	Refs,
	FindSymbolDefine,
	SymbolDefineLocations,
	
	FindInFiles,
	FindInFilesResults,

	SearchFile,
	SearchFileResults,

	SetEmbeddingModel,
	EmbeddingModelSet,

	ActivateFiles,
	ActivateFilesResult,
	//XXXXX: more SolutionDB message
};

struct SolutionDBMsg_RequestOpen : public PipeMsg
{
public:
	std::string dbFolderPath;
	std::string slnPath; // 用于DB不存在时创建新的DB

	PipeMsgType GetType() const override { return (PipeMsgType)SolutionDBMsgType::RequestOpen; }

	void Save(CDataPacket& dp) const override 
	{
		dp.Data_WriteString(dbFolderPath);
		dp.Data_WriteString(slnPath);
	}

	void Load(CDataPacket& dp) override 
	{
		dp.Data_ReadString(dbFolderPath);
		dp.Data_ReadString(slnPath);
	}
};

struct SolutionDBMsg_Opened : public PipeMsg
{
public:
	bool success;
	std::string dbFolderPath;  // 实际使用的DB文件夹路径

	SolutionDBMsg_Opened()
	{
		success = true;
	}

	PipeMsgType GetType() const override { return (PipeMsgType)SolutionDBMsgType::Opened; }

	void Save(CDataPacket& dp) const override
	{
		dp.Data_WriteSimple(success);
		dp.Data_WriteString(dbFolderPath);
	}

	void Load(CDataPacket& dp) override
	{
		dp.Data_ReadSimple(success);
		dp.Data_ReadString(dbFolderPath);
	}
};

struct SolutionDBMsg_QueryNameItems : public PipeMsg
{
public:
	std::string dbFolderPath;
	std::string query;

	PipeMsgType GetType() const override { return (PipeMsgType)SolutionDBMsgType::QueryNameItems; }

	void Save(CDataPacket& dp) const override
	{
		dp.Data_WriteString(dbFolderPath);
		dp.Data_WriteString(query);
	}

	void Load(CDataPacket& dp) override
	{
		dp.Data_ReadString(dbFolderPath);
		dp.Data_ReadString(query);
	}
};

struct SolutionDBMsg_NameItems : public PipeMsg
{
public:
	struct Item
	{
		enum Type
		{
			File_,
			Symbol,
			SystemPath,//系统
		};
		Type tp;
		char symbolKind;//SymbolKind
		std::string name;
		std::string desc;
		FileLocation fileLoc;//File类型的无效
		int score;
		std::string filePath;
	};

	std::string dbFolderPath;
	std::string query;
	std::vector<Item> items;

	PipeMsgType GetType() const override { return (PipeMsgType)SolutionDBMsgType::NameItems; }

	void Save(CDataPacket& dp) const override
	{
		dp.Data_WriteString(dbFolderPath);
		dp.Data_WriteString(query);

		int count = items.size();
		dp.Data_WriteSimple(count);
		for (auto& item : items)
		{
			dp.Data_NextByte()=item.tp;
			dp.Data_NextByte() = item.symbolKind;
			dp.Data_WriteString(item.name);
			dp.Data_WriteString(item.desc);
			dp.Data_WriteSimpleR(item.fileLoc);
			dp.Data_WriteString(item.filePath);
		}
	}

	void Load(CDataPacket& dp) override
	{
		dp.Data_ReadString(dbFolderPath);
		dp.Data_ReadString(query);

		items.clear();
		int count;
		dp.Data_ReadSimple(count);
		items.reserve(count);
		for (int i = 0; i < count; i++)
		{
			Item item;
			item.tp = (SolutionDBMsg_NameItems::Item::Type)dp.Data_NextByte();
			item.symbolKind = (int)dp.Data_NextByte();
			dp.Data_ReadString(item.name);
			dp.Data_ReadString(item.desc);
			dp.Data_ReadSimple(item.fileLoc);
			dp.Data_ReadString(item.filePath);
			items.push_back(std::move(item));
		}
	}
};

struct SolutionDBMsg_CollectRefs : public PipeMsg
{
	std::string dbFolderPath;
	CppSymbol::CollectRefsParam collectRefParam;

	PipeMsgType GetType() const override { return (PipeMsgType)SolutionDBMsgType::CollectRefs; }

	void Save(CDataPacket& dp) const override
	{
		dp.Data_WriteString(dbFolderPath);
		collectRefParam.Save(dp);
	}

	void Load(CDataPacket& dp) override
	{
		dp.Data_ReadString(dbFolderPath);
		collectRefParam.Load(dp);
	}
};

struct SolutionDBMsg_Refs : public PipeMsg
{
	bool success;

	PipeMsgType GetType() const override { return (PipeMsgType)SolutionDBMsgType::Refs; }

	void Save(CDataPacket& dp) const override
	{
		dp.Data_NextByte() = success;
	}

	void Load(CDataPacket& dp) override
	{
		success=dp.Data_NextByte();
	}
};

struct SolutionDBMsg_FindSymbolDefine : public PipeMsg
{
public:
	std::string dbFolderPath;
	std::string symbolName;
	int maxResult = 32;

	PipeMsgType GetType() const override { return (PipeMsgType)SolutionDBMsgType::FindSymbolDefine; }

	void Save(CDataPacket& dp) const override
	{
		dp.Data_WriteString(dbFolderPath);
		dp.Data_WriteString(symbolName);
		dp.Data_WriteSimple(maxResult);
	}

	void Load(CDataPacket& dp) override
	{
		dp.Data_ReadString(dbFolderPath);
		dp.Data_ReadString(symbolName);
		dp.Data_ReadSimple(maxResult);
	}
};

struct SolutionDBMsg_SymbolDefines : public PipeMsg
{
public:
	struct Location
	{
		char symbolKind; // SymbolKind
		FileLocation fileLoc;
		LineRange lineRange;
		std::string filePath;
	};

	std::string symbolName;
	std::vector<Location> locations;

	PipeMsgType GetType() const override { return (PipeMsgType)SolutionDBMsgType::SymbolDefineLocations; }

	void Save(CDataPacket& dp) const override
	{
		dp.Data_WriteString(symbolName);

		int count = locations.size();
		dp.Data_WriteSimple(count);
		for (auto& loc : locations)
		{
			dp.Data_NextByte() = loc.symbolKind;
			dp.Data_WriteSimpleR(loc.fileLoc);
			dp.Data_WriteSimpleR(loc.lineRange);
			dp.Data_WriteString(loc.filePath);
		}
	}

	void Load(CDataPacket& dp) override
	{
		dp.Data_ReadString(symbolName);

		locations.clear();
		int count;
		dp.Data_ReadSimple(count);
		locations.reserve(count);
		for (int i = 0; i < count; i++)
		{
			Location loc;
			loc.symbolKind = (int)dp.Data_NextByte();
			dp.Data_ReadSimple(loc.fileLoc);
			dp.Data_ReadSimple(loc.lineRange);
			dp.Data_ReadString(loc.filePath);
			locations.push_back(std::move(loc));
		}
	}
};

struct SolutionDBMsg_FindInFiles : public PipeMsg
{
public:
	std::string dbFolderPath;
	std::string keyword;
	int maxResults = 100; // 最大结果数

	PipeMsgType GetType() const override { return (PipeMsgType)SolutionDBMsgType::FindInFiles; }

	void Save(CDataPacket& dp) const override
	{
		dp.Data_WriteString(dbFolderPath);
		dp.Data_WriteString(keyword);
		dp.Data_WriteSimple(maxResults);
	}

	void Load(CDataPacket& dp) override
	{
		dp.Data_ReadString(dbFolderPath);
		dp.Data_ReadString(keyword);
		dp.Data_ReadSimple(maxResults);
	}
};

struct SolutionDBMsg_FindInFilesResults : public PipeMsg
{
public:

	std::string dbFolderPath;
	std::string keyword;
	FindInFileResults results;

	PipeMsgType GetType() const override { return (PipeMsgType)SolutionDBMsgType::FindInFilesResults; }

	void Save(CDataPacket& dp) const override
	{
		dp.Data_WriteString(dbFolderPath);
		dp.Data_WriteString(keyword);
		results.Save(dp);
	}

	void Load(CDataPacket& dp) override
	{
		dp.Data_ReadString(dbFolderPath);
		dp.Data_ReadString(keyword);

		results.Load(dp);
	}
};

struct SolutionDBMsg_SearchFile : public PipeMsg
{
public:
	std::string dbFolderPath;
	std::string keyword;
	int maxResults = 100; // 最大结果数

	PipeMsgType GetType() const override { return (PipeMsgType)SolutionDBMsgType::SearchFile; }

	void Save(CDataPacket& dp) const override
	{
		dp.Data_WriteString(dbFolderPath);
		dp.Data_WriteString(keyword);
		dp.Data_WriteSimple(maxResults);
	}

	void Load(CDataPacket& dp) override
	{
		dp.Data_ReadString(dbFolderPath);
		dp.Data_ReadString(keyword);
		dp.Data_ReadSimple(maxResults);
	}
};

struct SolutionDBMsg_SearchFileResult : public PipeMsg
{
public:
	std::string dbFolderPath;
	std::string keyword;
	SearchFileResult results;

	PipeMsgType GetType() const override { return (PipeMsgType)SolutionDBMsgType::SearchFileResults; }

	void Save(CDataPacket& dp) const override
	{
		dp.Data_WriteString(dbFolderPath);
		dp.Data_WriteString(keyword);
		results.Save(dp);
	}

	void Load(CDataPacket& dp) override
	{
		dp.Data_ReadString(dbFolderPath);
		dp.Data_ReadString(keyword);
		results.Load(dp);
	}
};

struct SolutionDBMsg_SetEmbeddingModel : public PipeMsg
{
public:
	std::string dbFolderPath;
	std::string modelName;
	std::string endpoint;
	std::string apiKey;
	int timeoutSeconds = 600;

	PipeMsgType GetType() const override { return (PipeMsgType)SolutionDBMsgType::SetEmbeddingModel; }

	void Save(CDataPacket& dp) const override
	{
		dp.Data_WriteString(dbFolderPath);
		dp.Data_WriteString(modelName);
		dp.Data_WriteString(endpoint);
		dp.Data_WriteString(apiKey);
		dp.Data_WriteSimple(timeoutSeconds);
	}

	void Load(CDataPacket& dp) override
	{
		dp.Data_ReadString(dbFolderPath);
		dp.Data_ReadString(modelName);
		dp.Data_ReadString(endpoint);
		dp.Data_ReadString(apiKey);
		dp.Data_ReadSimple(timeoutSeconds);
	}
};

struct SolutionDBMsg_EmbeddingModelSet : public PipeMsg
{
public:
	bool success;
	std::string dbFolderPath;

	SolutionDBMsg_EmbeddingModelSet()
	{
		success = true;
	}

	PipeMsgType GetType() const override { return (PipeMsgType)SolutionDBMsgType::EmbeddingModelSet; }

	void Save(CDataPacket& dp) const override
	{
		dp.Data_WriteSimple(success);
		dp.Data_WriteString(dbFolderPath);
	}

	void Load(CDataPacket& dp) override
	{
		dp.Data_ReadSimple(success);
		dp.Data_ReadString(dbFolderPath);
	}
};

struct SolutionDBMsg_ActivateFiles : public PipeMsg
{
public:
	std::string dbFolderPath;
	std::vector<std::string> filePaths;

	PipeMsgType GetType() const override { return (PipeMsgType)SolutionDBMsgType::ActivateFiles; }

	void Save(CDataPacket& dp) const override
	{
		dp.Data_WriteString(dbFolderPath);
		int count = (int)filePaths.size();
		dp.Data_WriteSimple(count);
		for (const std::string& path : filePaths)
			dp.Data_WriteString(path);
	}

	void Load(CDataPacket& dp) override
	{
		dp.Data_ReadString(dbFolderPath);
		int count;
		dp.Data_ReadSimple(count);
		filePaths.resize(count);
		for (int i = 0; i < count; i++)
			dp.Data_ReadString(filePaths[i]);
	}
};

struct SolutionDBMsg_ActivateFilesResult : public PipeMsg
{
public:
	bool success;
	std::string dbFolderPath;

	SolutionDBMsg_ActivateFilesResult()
	{
		success = true;
	}

	PipeMsgType GetType() const override { return (PipeMsgType)SolutionDBMsgType::ActivateFilesResult; }

	void Save(CDataPacket& dp) const override
	{
		dp.Data_WriteSimple(success);
		dp.Data_WriteString(dbFolderPath);
	}

	void Load(CDataPacket& dp) override
	{
		dp.Data_ReadSimple(success);
		dp.Data_ReadString(dbFolderPath);
	}
};

//XXXXX: more SolutionDB message
