#pragma once

//#include "GuiLib.h"
#include "afxwin.h"
#include "afxcmn.h" // 包含CRichEditCtrl定义

#include "LlmChat.h"

#include "RepairWnd.h"
#include "SmartRepairRefCollector.h"

#include "SmartRepairTrigger.h"

#include <vector>
#include <memory>

struct RepairRequest
{
	RepairRequest()
	{
		Zero();
	}
	void Zero()
	{
		sessionId = SmartRepairSessionID_Invalid;
		cursorLine = 0;
		cursorColumn = 0;
	}
	void Clear()
	{
		filePath.clear();
		fileName.clear();
		fileContent.clear();
		fileLines.clear();
		cursorLinePrefix.clear();
		cursorLineSuffix.clear();

		Zero();
	}
	SmartRepairSessionID sessionId;
	std::string filePath;
	std::string fileName;
	std::string fileContent;
	std::vector<std::string> fileLines;
	int cursorLine;
	int cursorColumn;
	std::string cursorLinePrefix;//cursor所在行中cursor前的部分
	std::string cursorLineSuffix;//cursor所在行中cursor后的部分
};

struct RepairTarget
{
	RepairTarget()
	{
		Zero();
	}
	void Zero()
	{
		rewriteStartLine = rewriteEndLine = 0;
		debugStartTime = 0;
		debugDur = 0;
	}
	void Clear()
	{
		Zero();
	}
	bool IsValid()
	{
		return rewriteEndLine > rewriteStartLine;
	}
	std::string debugUserMessage;
	AbsTick debugStartTime;
	AbsTick debugDur;

	int rewriteStartLine;
	int rewriteEndLine;

};

class CSmartRepairChat :public CLlmChat
{
public:
	CSmartRepairChat()
	{
		_sessionId = SmartRepairSessionID_Invalid;
	}
	void SetSessionID(SmartRepairSessionID sessionId)
	{
		_sessionId = sessionId;
	}
	SmartRepairSessionID FetchSessionID()
	{
		SmartRepairSessionID sessionId = _sessionId;
		_sessionId = SmartRepairSessionID_Invalid;
		return sessionId;
	}

public:
	SmartRepairSessionID _sessionId;
};

struct CodeComparingOldChars
{
	struct Line
	{
		int line;
		std::vector<int> indices;
	};
	std::vector<Line> lines;
};

struct CodeComparingChars;

class CSmartRepair
{
public:
	CSmartRepair();
	virtual ~CSmartRepair();

public:

	void Init(CWnd *pMainWnd);
	void Clear();

	void Update();

	CSmartRepairTrigger& GetTrigger()	{		return _trigger;	}

	void StartPrepareContext(RepairRequest &request);
	void StartAIRequest();
	void ShowSuggestion(const RECT &focusRect);
	void HideSuggestion();

protected:

	void _Reset();

	CSmartRepairTrigger _trigger;

	RepairRequest _repairRequest;

	//RefCollector
	void _ProcessRefCollect();
	CSmartRepairRefCollector _refCollector;
	CSmartRepairRefCollector::Result _collectedRef;

	// LLM
	void _InitLlmChat();
	void _ProcessLlmChat();
	LlmSessionSetting _defaultSettings;
	CSmartRepairChat _llmChat;
	RepairTarget _repairTarget;
	std::string _suggest;

	//Repair window
	CRepairWnd _resultWnd;

	//Old chars(旧文本中哪些字符会被删除)
	CodeComparingOldChars _oldChars;
	
	// 解析差异字符串
	void _ParseDiffString(const char* diffStr, CodeComparingChars& comparingChars);
	
};



