#pragma once

#include <functional>
#include "ChatAgentDefines.h"

// 回调函数类型定义
using BeforeSendToLlmCallback = std::function<bool(bool)>;  // isUserMessage, return false to cancel
using AfterReceiveFromLlmCallback = std::function<void()>;
using CheckCompressCallback = std::function<int()>;  // return tokens to compress, 0 means no compress

// CChatNotify - 实现 IChatNotify 接口，响应来自 CChatAgent 的通知
class CChatNotify : public IChatNotify
{
public:
    CChatNotify();
    ~CChatNotify();

    // 设置回调
    void SetBeforeSendToLlmCallback(BeforeSendToLlmCallback callback);
    void SetAfterReceiveFromLlmCallback(AfterReceiveFromLlmCallback callback);
    void SetCheckCompressCallback(CheckCompressCallback callback);

    // IChatNotify 接口实现
    bool OnBeforeSendToLlm(bool isUserMessage) override;
    void OnAfterReceiveFromLlm() override;
    int OnCheckCompress() override;

private:
    BeforeSendToLlmCallback _beforeSendToLlmCallback;
    AfterReceiveFromLlmCallback _afterReceiveFromLlmCallback;
    CheckCompressCallback _checkCompressCallback;
};

