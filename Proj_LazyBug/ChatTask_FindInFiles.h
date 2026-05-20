#pragma once
#include "ChatTaskMgr.h"
#include "LlmChat.h"
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>


class CChatTask_FindInFiles : public CChatTask
{
public:
	CChatTask_FindInFiles();
	~CChatTask_FindInFiles();
	
	const char* GetType() override { return "FindInFiles"; }
	void Start() override;
	void Update() override;
	void Interrupt() override;
	bool NeedLlmSession() override { return false; }

	bool DependsOn(CChatTask* task) override;

private:
	void _Fail();
	void _Succeed();
	void _ThreadFunc();

	void _TestCases();

	std::string _dbFolderPath;
	
	std::thread* _workerThread;
	std::atomic<bool> _shouldStop;
	std::atomic<bool> _threadFinished;
	std::mutex _resultMutex;

	std::string _threadResult;
	std::string _threadMessage;
	bool _threadSuccess;

};