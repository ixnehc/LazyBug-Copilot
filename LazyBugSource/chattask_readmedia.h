#pragma once
#include "ChatTaskMgr.h"
#include "LlmChat.h"
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>


class CChatTask_ReadMedia : public CChatTask
{
public:
	CChatTask_ReadMedia();
	~CChatTask_ReadMedia();
	
	const char* GetType() override { return "ReadMedia"; }
	void Start() override;
	void Update() override;
	void Interrupt() override;
	int GetLlmSessionCount() override { return 0; }

	bool DependsOn(CChatTask* task) override;

private:
	void _Fail();
	void _Succeed();
	void _ThreadFunc();

	static const char* _GetMimeType(const std::string& ext);
	static bool _IsSupportedImage(const std::string& ext);

	std::thread* _workerThread;
	std::atomic<bool> _shouldStop;
	std::atomic<bool> _threadFinished;
	std::mutex _resultMutex;

	std::string _threadResult;
	std::string _threadResultSimple;
	std::string _threadMessage;
	bool _threadSuccess;
};
