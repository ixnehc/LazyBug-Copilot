#pragma once

#include <deque>
#include <unordered_set>

#include "UnifiedDiff.h"

extern int GetProcessStart(const std::string& workingStr, const std::string& expectTag);
extern bool CullMarkDown(std::string& workingStr, const std::string& startMark, const std::string& endMark, std::string& content,bool &isFullResult);
extern bool CullHeadMarkLine(std::string& workingStr, const std::string& startMark, std::string& content);
extern void AddLineNumber(const std::string& fileContent, std::string& fileContentWithLineNumber);
extern void AddLineSuffix(const std::string& fileContent, std::string& fileContentWithLineNumber);


struct LlmProcessorsResult
{
	void Clear()
	{
		raw.clear();
		outputTotal.clear();
		outputDelta.clear();
	}
	void AddOutputDelta(const std::string &delta)
	{
		outputTotal += delta;
		outputDelta += delta;
	}
	std::string raw;//原始的LLM的返回字符串
	std::string outputTotal;//输出到聊天窗口的所有内容
	std::string outputDelta;//输出到聊天窗口的增量内容
};

struct LlmProcessorsContext
{

};

class CLlmProcessor
{
public:
	CLlmProcessor()
	{
	}
	virtual const char* GetName() = 0;
	virtual void Start() {}
	virtual void Stop(const LlmProcessorsContext& context, LlmProcessorsResult& result) {}
	virtual void Interrupt(const LlmProcessorsContext& context, LlmProcessorsResult& result) {}
	virtual int GetProcessStart(const std::string &workingStr, const LlmProcessorsContext &context) const = 0;//返回可以从workingStr哪里开始处理,返回-1表示无法处理
	virtual bool Process(std::string& workingStr, const LlmProcessorsContext& context,LlmProcessorsResult& result) = 0;//从workingStr中挖取一部分进行处理,返回true表示确实挖取了一部分进行了处理

};


//这个类用来处理LLM的返回字符串
class CLlmProcessors
{
public:
	void Start(const std::vector<std::string> &enabledProcessorNames);
	void Stop();
	virtual void Interrupt();
	void Add(const char* str);

	void FetchDelta(std::string& delta);

	CLlmProcessor* FindProcessor(const char* name);

private:
	std::string _workingStr;
	std::unordered_set<std::string> _enables;

	virtual LlmProcessorsResult& _GetResult() = 0;
	virtual const LlmProcessorsContext& _GetContext() = 0;
	virtual void _GetProcessors(std::vector<CLlmProcessor*>&processors)=0;

	void _PrepareProcessors(std::vector<CLlmProcessor*>& processors);
};