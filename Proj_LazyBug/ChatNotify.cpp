#include "stdh.h"
#include "ChatNotify.h" 

CChatNotify::CChatNotify()
{
}

CChatNotify::~CChatNotify()
{
}

void CChatNotify::SetBeforeSendToLlmCallback(BeforeSendToLlmCallback callback)
{
    _beforeSendToLlmCallback = callback;
}

void CChatNotify::SetAfterReceiveFromLlmCallback(AfterReceiveFromLlmCallback callback)
{
    _afterReceiveFromLlmCallback = callback;
}

void CChatNotify::SetCheckCompressCallback(CheckCompressCallback callback)
{
    _checkCompressCallback = callback;
}

bool CChatNotify::OnBeforeSendToLlm(bool isUserMessage)
{
    if (_beforeSendToLlmCallback)
    {
        return _beforeSendToLlmCallback(isUserMessage);
    }
    return true;
}

void CChatNotify::OnAfterReceiveFromLlm()
{
    if (_afterReceiveFromLlmCallback)
    {
        _afterReceiveFromLlmCallback();
    }
}

int CChatNotify::OnCheckCompress()
{
    if (_checkCompressCallback)
    {
        return _checkCompressCallback();
    }
    return 0;
}

