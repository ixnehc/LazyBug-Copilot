// 主入口、初始化、事件监听

// 强制切分 Undo 栈
function forceUndoBoundary() {
    const editor = document.getElementById('inputEditor');
    if (!editor) return;
    
    const sel = window.getSelection();
    if (!sel.rangeCount) return;
    
    const range = sel.getRangeAt(0).cloneRange();
    
    // 通过让输入框瞬间失去焦点并重新获取焦点，强迫浏览器原生提交当前的撤销分组，不产生多余撤销步骤
    editor.blur();
    editor.focus();
    
    sel.removeAllRanges();
    sel.addRange(range);
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
        if (event.ctrlKey || event.metaKey) {
            event.preventDefault();
            handleSendClick();
        } else if (!event.shiftKey) {
            event.preventDefault();
            handleSendClick();
        } else {
            // 拦截原生的换行行为，并在前后插入强制 Undo 边界
            event.preventDefault();
            forceUndoBoundary();
            document.execCommand('insertLineBreak', false, null);
            forceUndoBoundary();
        }
    } else if (event.key === 'Escape') {
        console.warn('Escape received');
        event.preventDefault();
        sendMessageToNative({
            action: 'escape'
        });
    } else if (event.key === 'Backspace' || event.key === 'Delete') {
        const selection = window.getSelection();
        if (selection.rangeCount > 0 && !selection.isCollapsed) {
            // 当存在多选区并删除时，让它成为一个独立的撤销点
            forceUndoBoundary();
            
            // 延迟一点再次截断，使得随后的新输入不和刚才的删除动作混在同一个 Undo 历史里
            setTimeout(() => {
                forceUndoBoundary();
            }, 0);
            return;
        }
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
    
    // 复制、剪切和粘贴事件监听
    inputEditor.addEventListener('copy', handleCopy);
    inputEditor.addEventListener('cut', handleCopy);
    inputEditor.addEventListener('paste', handlePaste);

    // 文本选择监听
    document.addEventListener('selectionchange', handleSelectionChange);
    inputEditor.addEventListener('mouseup', handleSelectionChange);
    inputEditor.addEventListener('keyup', handleSelectionChange);

    // 发送按钮点击
    sendButton.addEventListener('click', handleSendClick);

    // Skill按钮点击
    const skillButton = document.getElementById('skillButton');
    if (skillButton) {
        skillButton.addEventListener('click', handleSkillClick);
    }

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

    // 压缩强度按钮点击
    const compressButton = document.getElementById('compressButton');
    if (compressButton) {
        compressButton.addEventListener('click', handleCompressButtonClick);
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
    window.replaceAutoCompleteWithTag = replaceAutoCompleteTag;
    window.focusEditor = focusEditor;
    window.setCompressIntensity = setCompressIntensity;
    window.setCompressedSize = setCompressedSize;
    
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
window.forceUndoBoundary = forceUndoBoundary;
window.handleKeyDown = handleKeyDown;
window.initializeEventListeners = initializeEventListeners;
