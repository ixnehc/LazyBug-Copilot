#include "stdh.h"
#include "AiChat.h"
#include <iostream>
#include <thread>
#include <chrono>

// 示例函数：演示如何使用CAiChat与LLM交互
void AiChatExample()
{
    // 创建并初始化CAiChat
    CAiChat aiChat;
    
    AiChatSetting setting;
    setting.apiKey = "your_api_key_here"; // 请替换为您的API密钥
    setting.apiEndpoint = "https://api.openai.com/v1/chat/completions"; // OpenAI API端点
    setting.modelName = "gpt-3.5-turbo"; // 使用的模型
    setting.timeoutSeconds = 60; // 设置60秒超时
    
    aiChat.Init(setting);
    
    // 示例1：非流式响应
    {
        std::cout << "示例1：非流式响应" << std::endl;
        
        const char* question = "什么是C++？";
        const char* context = "你是一个编程助手，专注于解释编程概念。请简明扼要地回答问题。";
        bool isStreaming = false;
        
        AiChatQuestionID id = aiChat.RequestQuestion(question, context, isStreaming);
        if (id < 0)
        {
            std::cout << "请求失败！" << std::endl;
            return;
        }
        
        // 等待回答完成
        std::cout << "等待回答中..." << std::endl;
        while (!aiChat.IsAnswerComplete(id))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        // 获取回答
        std::string answer;
        if (aiChat.FetchAnswer(id, answer))
        {
            std::cout << "问题: " << question << std::endl;
            std::cout << "回答: " << answer << std::endl;
        }
        else
        {
            std::cout << "获取回答失败！" << std::endl;
        }
        
        std::cout << std::endl;
    }
    
    // 示例2：流式响应
    {
        std::cout << "示例2：流式响应" << std::endl;
        
        const char* question = "请列举5个C++的主要特性";
        const char* context = "你是一个编程助手，专注于解释编程概念。请简明扼要地回答问题。";
        bool isStreaming = true;
        
        AiChatQuestionID id = aiChat.RequestQuestion(question, context, isStreaming);
        if (id < 0)
        {
            std::cout << "请求失败！" << std::endl;
            return;
        }
        
        // 实时获取和显示流式回答
        std::cout << "问题: " << question << std::endl;
        std::cout << "流式回答: " << std::endl;
        
        std::string previousAnswer = "";
        while (!aiChat.IsAnswerComplete(id))
        {
            std::string currentAnswer;
            if (aiChat.FetchAnswer(id, currentAnswer) && currentAnswer != previousAnswer)
            {
                // 只输出新增的内容
                std::string newContent = currentAnswer.substr(previousAnswer.length());
                std::cout << newContent << std::flush; // 使用flush确保即时显示
                previousAnswer = currentAnswer;
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        // 确保获取完整的最终回答
        std::string finalAnswer;
        aiChat.FetchAnswer(id, finalAnswer);
        if (finalAnswer != previousAnswer)
        {
            std::string newContent = finalAnswer.substr(previousAnswer.length());
            std::cout << newContent << std::flush;
        }
        
        std::cout << std::endl << std::endl;
    }
    
    // 清理资源
    aiChat.Clear();
    
    std::cout << "AiChat示例结束" << std::endl;
}

// 您可以从应用程序的其他部分调用此函数
#ifdef AICHAT_EXAMPLE_MAIN
int main()
{
    AiChatExample();
    return 0;
}
#endif 