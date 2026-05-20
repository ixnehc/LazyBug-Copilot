// 主入口、初始化、事件监听

function createUndoBoundary() {
    const inputEditor = document.getElementById('inputEditor');
    if (!inputEditor) return;

    inputEditor.focus();

    // 通过插入再删除一个零宽字符，显式切分 undo 分组。
    document.execCommand('insertText', false, AppState.zwsp);
    document.execCommand('delete', false, null);
}

// 处理键盘按下
function handleKeyDown(event) {
    // 如果自动补全列表可见，优先处理自动补全按键
    if (AutoCompleteState.isVisible) {
        if (event.key === 'ArrowDown') {
            event.preventDefault();
            selectNextAutoCompleteItem();
            return;
        } else if (event.key === 'ArrowUp') {
            event.preventDefault();
            selectPreviousAutoCompleteItem();
            return;
        } else if (event.key === 'Enter' || event.key === 'Tab') {
            event.preventDefault();
            confirmAutoCompleteSelection();
            return;
        } else if (event.key === 'Escape') {
            event.preventDefault();
            hideAutoComplete();
            return;
        }
    }
    
    if (event.key === 'Enter') {
        createUndoBoundary();
        if (event.ctrlKey || event.metaKey) {
            event.preventDefault();
            handleSendClick();
        } else if (!event.shiftKey) {
            event.preventDefault();
            handleSendClick();
        }
    } else if (event.key === 'Escape') {
        console.warn('Escape received');
        event.preventDefault();
        sendMessageToNative({
            action: 'escape'
        });
    } else if (event.key === 'Backspace' || event.key === 'Delete') {
        handleTagDeletion(event);
    } else if (event.key === 'ArrowLeft' || event.key === 'ArrowRight') {
        handleArrowKeyNavigation(event);
    } else if (event.key === 'PageUp') {
        event.preventDefault();
        const contentJson = getInputContent();
        sendMessageToNative({
            action: 'pageNavigation',
            direction: 'up',
            content: JSON.parse(contentJson)
        });
    } else if (event.key === 'PageDown') {
        event.preventDefault();
        const contentJson = getInputContent();
        sendMessageToNative({
            action: 'pageNavigation',
            direction: 'down',
            content: JSON.parse(contentJson)
        });
    }
}



// 初始化事件监听器
function initializeEventListeners() {
    const inputEditor = document.getElementById('inputEditor');
    const sendButton = document.getElementById('sendButton');
    const stopButton = document.getElementById('stopButton');
    const atButton = document.getElementById('atButton');

    // 输入内容变化监听
    inputEditor.addEventListener('input', debounce(notifyContentChanged, 300));
    inputEditor.addEventListener('input', ensureTagIntegrity);
    inputEditor.addEventListener('input', handleAutoComplete);
    inputEditor.addEventListener('input', updateWatermarkVisibility);
    inputEditor.addEventListener('keydown', handleKeyDown);
    
    // 粘贴事件监听
    inputEditor.addEventListener('paste', handlePaste);

    // 文本选择监听
    document.addEventListener('selectionchange', handleSelectionChange);
    inputEditor.addEventListener('mouseup', handleSelectionChange);
    inputEditor.addEventListener('keyup', handleSelectionChange);

    // 发送按钮点击
    sendButton.addEventListener('click', handleSendClick);

    // Stop按钮点击
    if (stopButton) {
        stopButton.addEventListener('click', handleStopClick);
    }

    // @按钮点击
    if (atButton) {
        atButton.addEventListener('click', handleAtButtonClick);
    }

    // API菜单按钮点击
    const apiMenuButton = document.getElementById('apiMenuButton');
    if (apiMenuButton) {
        apiMenuButton.addEventListener('click', handleApiMenuButtonClick);
    }

    // 监听来自C++的消息
    if (window.chrome && window.chrome.webview) {
        window.chrome.webview.addEventListener('message', handleNativeMessage);
    }
}

// 初始化
document.addEventListener('DOMContentLoaded', function() {
    initializeEventListeners();
    AppState.isInitialized = true;
    
    // 初始化@按钮状态
    const atButton = document.getElementById('atButton');
    if (atButton) {
        atButton.disabled = true;
    }
    
    // 暴露函数到全局作用域，供C++的ExecuteScript调用
    window.setInputContent = setInputContent;
    window.getInputContent = getInputContent;
    window.getInputPlainText = getInputPlainText;
    window.getSelectedText = getSelectedText;
    window.processAutoCompleteMessage = processAutoCompleteMessage;
    window.getCursorPosition = getCursorPosition;
    window.replaceAutoCompleteWithTag = replaceAutoCompleteWithTag;
    window.focusEditor = focusEditor;
    window.setContextUsage = setContextUsage;
    
    // 初始化水印显示状态
    updateWatermarkVisibility();
    
    // 通知C++初始化完成
    if (window.chrome && window.chrome.webview) {
        window.chrome.webview.postMessage(JSON.stringify({
            action: 'initialized'
        }));
    }
});

// 导出到全局
window.handleKeyDown = handleKeyDown;
window.initializeEventListeners = initializeEventListeners;