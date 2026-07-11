// 与C++通信接口

// 发送消息到原生代码
function sendMessageToNative(message) {
    if (window.chrome && window.chrome.webview) {
        window.chrome.webview.postMessage(message);
    }
}

// 处理来自C++的消息
function handleNativeMessage(event) {
    try {
        const data = event.data;
        processNativeMessage(data);
    } catch (e) {
        console.error('Failed to process message:', e);
    }
}

// 处理原生消息
function processNativeMessage(data) {
    console.log('processNativeMessage called with:', data);
    switch (data.action) {
        case 'updateTags':
            if (window.updateTags) window.updateTags(data.tags || []);
            break;
        case 'updateToolButtons':
            if (window.updateToolButtons) window.updateToolButtons(data.buttons || []);
            break;
        case 'setContent':
            console.log('setContent action received, data.content:', data.content);
            if (window.setInputContent) window.setInputContent(data.content || '[]', data.caretPos);
            break;
        case 'clearContent':
            if (window.clearInputContent) window.clearInputContent();
            break;
        case 'insertText':
            if (window.insertTextAtCursor) window.insertTextAtCursor(data.text || '');
            break;
        case 'insertInlineTag':
            if (window.insertInlineTag) window.insertInlineTag(data.tag || {});
            break;
        case 'setSendButtonEnabled':
            if (window.setSendButtonEnabled) window.setSendButtonEnabled(data.enabled !== false);
            break;
        case 'setSendButtonText':
            if (window.setSendButtonText) window.setSendButtonText(data.text || '发送');
            break;
        case 'showStopButton':
            if (window.showStopButton) window.showStopButton();
            break;
        case 'hideStopButton':
            if (window.hideStopButton) window.hideStopButton();
            break;
        case 'setPlaceholder':
            if (window.setPlaceholder) window.setPlaceholder(data.placeholder || '请输入您的消息...');
            break;
        case 'updateMajorChatApiMenu':
            if (window.updateMajorChatApiMenu) updateMajorChatApiMenu(data.current || '');
            break;
        case 'updateAutoComplete':
            if (window.updateAutoCompleteData) window.updateAutoCompleteData(data.items || [], data.selectedIndex);
            break;
        case 'hideAutoComplete':
            AutoCompleteState.isVisible = false;
            break;
        case 'replaceAutoCompleteWithTag':
            if (window.replaceAutoCompleteWithTag) {
                window.replaceAutoCompleteWithTag(data.prefix || '', data.tag || {});
            }
            break;
        case 'showCompressSummarizeTip':
            if (window.showCompressSummarizeTip) {
                window.showCompressSummarizeTip(data.success, data.message, data.logPath);
            }
            break;
        case 'hideCompressSummarizeTip':
            if (window.hideCompressSummarizeTip) {
                window.hideCompressSummarizeTip();
            }
            break;
        case 'setDeletionMarks':
            if (window.setDeletionMarks) window.setDeletionMarks(data.indices || []);
            break;
        case 'clearDeletionMarks':
            if (window.clearDeletionMarks) window.clearDeletionMarks();
            break;
        case 'setInputHintToggleButtonState':
            if (window.setInputHintToggleButtonState) {
                window.setInputHintToggleButtonState(data.enabled !== false);
            }
            break;
    }
}

// 导出到全局
window.sendMessageToNative = sendMessageToNative;
window.handleNativeMessage = handleNativeMessage;
window.processNativeMessage = processNativeMessage;