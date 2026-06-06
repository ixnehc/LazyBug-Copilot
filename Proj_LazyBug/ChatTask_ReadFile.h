#pragma once
#include "ChatTaskMgr.h"
#include "LlmChat.h"
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>


class CChatTask_ReadFile : public CChatTask
{
public:
	CChatTask_ReadFile();
	~CChatTask_ReadFile();
	
	const char* GetType() override { return "ReadFile"; }
	void Start() override;
	void Update() override;
	void Interrupt() override;
	bool NeedLlmSession() override { return false; }

	bool DependsOn(CChatTask* task) override;

private:
	void _Fail();
	void _Succeed();
	void _ThreadFunc();

	std::thread* _workerThread;
	std::atomic<bool> _shouldStop;
	std::atomic<bool> _threadFinished;
	std::mutex _resultMutex;

	std::string _threadResult;
	std::string _threadResultSimple;
	std::string _threadMessage;
	bool _threadSuccess;
	int _threadSimpleStartLine;
	int _threadSimpleEndLine;

};