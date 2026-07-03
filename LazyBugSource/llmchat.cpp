#include "stdh.h"

#include "LlmChat.h"
#include <curl/curl.h>
#include <sstream>
#include <map>
#include <mutex>
#include <thread>
#include <chrono>
#include <regex>


//////////////////////////
// CLlmChat 实现
//////////////////////////

CLlmChat::CLlmChat() 
{
}

CLlmChat::~CLlmChat()
{
    Clear();
}

void CLlmChat::Init()
{
}

void CLlmChat::Clear()
{
    std::lock_guard<std::mutex> lock(m_mutex);

	// 调用Interrupt()打断会话
    if (m_activeSession)
    {
        m_activeSession->Interrupt();

        // 将会话转移到废弃会话列表
        m_discardedSessions.push_back(std::move(m_activeSession));

        // 清空活动会话
        m_activeSession.reset();
    }
}

bool CLlmChat::Request(const LlmSessionRequest& request, const LlmSessionSetting& setting)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // 如果已经有活动会话，返回错误
    if (m_activeSession)
    {
        return false;
    }

    m_setting = setting;
    
    // 创建新会话
    m_activeSession = std::make_unique<CLlmSession>(setting);
    
    // 发送请求
    bool success = m_activeSession->Request(request);
    if (!success)
    {
        m_activeSession.reset();
        return false;
    }
    
    // 分配并返回问题ID
    return true;
}

bool CLlmChat::RequestEmbedding(const std::string& input, const LlmSessionSetting& setting)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // 如果已经有活动会话，返回错误
    if (m_activeSession)
    {
        return false;
    }

    m_setting = setting;
    
    // 创建新会话
    m_activeSession = std::make_unique<CLlmSession>(setting);
    
    // 发送embedding请求
    bool success = m_activeSession->RequestEmbedding(input);
    if (!success)
    {
        m_activeSession.reset();
        return false;
    }
    
    return true;
}

bool CLlmChat::Process(LlmSessionOutput& output,bool interrupt)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // 首先检查并清理已完成的废弃会话
    for (auto it = m_discardedSessions.begin(); it != m_discardedSessions.end();)
    {
        if ((*it)->IsCompleted())
        {
            it = m_discardedSessions.erase(it);
        }
        else
        {
            ++it;
        }
    }
    
    // 没有活动会话
    if (!m_activeSession)
    {
        return false;
    }
    
    // 处理会话
    m_activeSession->Process();
    
    // 检查会话是否完成
    if (m_activeSession->IsCompleted())
    {
        // 获取回答
        m_activeSession->FetchDeltaAnswer(output.content,output.reasoning);
        m_activeSession->GetAnswer(output.fullContent);
		m_activeSession->GetUpdatedToolCalls(output.updatedToolCalls);
        
        // 检查错误
        output.hasError = m_activeSession->HasError();
        if (output.hasError)
        {
            output.errorMessage = m_activeSession->GetErrorMessage();
        }

        m_activeSession->GetTokenUsage(output.usage);
        m_activeSession->FetchEmbedding(output.embedding);

		float calculatedFee = (m_setting.api.priceInputToken * (float)output.usage.inputToken_ 
			+ m_setting.api.priceCacheRead * (float)output.usage.inputToken_CacheRead 
			+ m_setting.api.priceCacheWrite * (float)output.usage.inputToken_CacheWrite) / 1000000.0f 
			+ (m_setting.api.priceOutputToken * (float)output.usage.outputToken) / 1000000.0f;
		if (output.usage.fee <= 0.0f)
			output.usage.fee = calculatedFee;
        
        output.isCompleted = true;
        
        // 会话完成，清理
        m_activeSession.reset();
        
        return true;
    }
    else
    {
        if (!interrupt)
        {
            // 会话未完成，获取当前回答（可能是部分回答）
            m_activeSession->FetchDeltaAnswer(output.content, output.reasoning);
			m_activeSession->GetUpdatedToolCalls(output.updatedToolCalls);
			output.isCompleted = false;
            output.hasError = false;

            return true;
        }
        else
        {
            // interrupt为true的情况：取出所有数据，停止会话，并转移到废弃会话列表
            
            // 取出所有当前数据
            m_activeSession->FetchDeltaAnswer(output.content, output.reasoning);
            m_activeSession->GetAnswer(output.fullContent);
			m_activeSession->GetUpdatedToolCalls(output.updatedToolCalls);

            // 检查错误
            output.hasError = m_activeSession->HasError();
            if (output.hasError)
            {
                output.errorMessage = m_activeSession->GetErrorMessage();
            }
            
            // 调用Interrupt()打断会话
            m_activeSession->Interrupt();
            
            // 将会话转移到废弃会话列表
            m_discardedSessions.push_back(std::move(m_activeSession));
            
            // 清空活动会话
            m_activeSession.reset();
            
            // 标记为完成（虽然是被停止的）
            output.isCompleted = true;
            
            return true;
        }
    }
}


bool CLlmChat::HasActiveSession() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_activeSession != nullptr;
}
