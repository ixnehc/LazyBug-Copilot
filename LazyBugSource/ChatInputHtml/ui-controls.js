// UI控制（按钮状态、菜单、进度条）

// 更新工具按钮
function updateToolButtons(buttons) {
    AppState.currentToolButtons = buttons;
    const toolbarLeft = document.getElementById('toolbarLeft');
    
    // 保留API菜单按钮和Context Usage进度条，只移除其他工具按钮
    const apiMenuButton = document.getElementById('apiMenuButton');
    const contextUsageContainer = document.getElementById('contextUsageContainer');
    
    toolbarLeft.innerHTML = '';
    
    if (apiMenuButton) {
        toolbarLeft.appendChild(apiMenuButton);
    }
    
    if (contextUsageContainer) {
        toolbarLeft.appendChild(contextUsageContainer);
    }
    
    buttons.forEach(button => {
        const buttonElement = createToolButtonElement(button);
        toolbarLeft.appendChild(buttonElement);
    });
}

// 创建工具按钮元素
function createToolButtonElement(button) {
    const buttonDiv = document.createElement('button');
    buttonDiv.className = 'tool-button';
    buttonDiv.disabled = !button.enabled;
    buttonDiv.title = button.tooltip || button.text;
    
    if (button.icon) {
        const iconSpan = document.createElement('span');
        iconSpan.className = 'tool-button-icon';
        iconSpan.innerHTML = button.icon;
        buttonDiv.appendChild(iconSpan);
    }
    
    if (button.text) {
        const textSpan = document.createElement('span');
        textSpan.textContent = button.text;
        buttonDiv.appendChild(textSpan);
    }
    
    buttonDiv.addEventListener('click', () => {
        if (!buttonDiv.disabled) {
            sendMessageToNative({
                action: 'toolButton',
                buttonId: button.id,
                buttonAction: button.action
            });
        }
    });
    
    return buttonDiv;
}

// 设置发送按钮启用状态
function setSendButtonEnabled(enabled) {
    const sendButton = document.getElementById('sendButton');
    if (sendButton) {
        sendButton.disabled = !enabled;
    }
}

// 设置发送按钮文本
function setSendButtonText(text) {
    const sendButton = document.getElementById('sendButton');
    const textSpan = sendButton.querySelector('span');
    if (textSpan) {
        textSpan.textContent = text;
    }
}

// 显示停止按钮
function showStopButton() {
    const sendButton = document.getElementById('sendButton');
    const stopButton = document.getElementById('stopButton');
    
    if (sendButton) {
        sendButton.classList.add('hidden');
    }
    if (stopButton) {
        stopButton.classList.remove('hidden');
    }
}

// 隐藏停止按钮
function hideStopButton() {
    const sendButton = document.getElementById('sendButton');
    const stopButton = document.getElementById('stopButton');
    
    if (stopButton) {
        stopButton.classList.add('hidden');
    }
    if (sendButton) {
        sendButton.classList.remove('hidden');
    }
}

// 设置占位符
function setPlaceholder(placeholder) {
    const inputEditor = document.getElementById('inputEditor');
    if (inputEditor) {
        inputEditor.setAttribute('data-placeholder', placeholder);
    }
}

// 处理API菜单按钮点击
function handleApiMenuButtonClick(event) {
    event.stopPropagation();
    
    const apiMenuButton = document.getElementById('apiMenuButton');
    const rect = apiMenuButton.getBoundingClientRect();
    
    sendMessageToNative({
        action: 'showLlmMenu',
        position: {
            x: rect.left,
            y: rect.top
        }
    });
}

// 更新MajorChat API菜单
function updateMajorChatApiMenu(currentApi) {
    const apiMenuText = document.getElementById('apiMenuText');
    if (apiMenuText) {
        apiMenuText.textContent = currentApi || 'n/a';
        apiMenuText.title = currentApi || 'n/a';
    }
}

// 压缩强度枚举值映射 (对应 ChatOpCompressIntensity)
const CompressIntensity = {
    None: 0,
    Low: 1,
    Medium: 2,
    High: 3,
    Extreme: 4
};

// 当前压缩强度
let currentCompressIntensity = CompressIntensity.Low;

// 流光期间保存的原始 tooltip
let _compressOriginalTooltip = null;

// 设置压缩强度 (0-4, 对应 None, Low, Medium, High, Extreme)
// 显示下标: None=∞, Low=4, Medium=3, High=2, Extreme=1
function setCompressIntensity(intensity, tooltip) {
    const compressButton = document.getElementById('compressButton');
    const levelBadge = document.getElementById('compressLevelBadge');
    
    if (!compressButton || !levelBadge) return;
    
    currentCompressIntensity = intensity;
    
    // 更新按钮状态
    if (intensity === CompressIntensity.None) {
        compressButton.classList.add('compress-none');
        levelBadge.textContent = '∞';
    } else {
        compressButton.classList.remove('compress-none');
        // 显示下标转换: Low(1)->4, Medium(2)->3, High(3)->2, Extreme(4)->1
        const displayLevel = 5 - intensity;
        levelBadge.textContent = displayLevel;
    }
    
    // 设置 tooltip
    if (tooltip) {
        _compressOriginalTooltip = tooltip;
        if (!compressButton.classList.contains('flowing')) {
            compressButton.title = tooltip;
        }
    }
}

// 开始压缩按钮(Context Level)流光效果
function startCompressFlowing() {
    const compressButton = document.getElementById('compressButton');
    if (compressButton) {
        if (_compressOriginalTooltip === null) {
            _compressOriginalTooltip = compressButton.title || '';
        }
        compressButton.classList.add('flowing');
        compressButton.title = 'Compressing...';
    }
}

// 停止压缩按钮(Context Level)流光效果
function stopCompressFlowing() {
    const compressButton = document.getElementById('compressButton');
    if (compressButton) {
        compressButton.classList.remove('flowing');
        if (_compressOriginalTooltip !== null) {
            compressButton.title = _compressOriginalTooltip;
            _compressOriginalTooltip = null;
        }
    }
}

// 获取下一个压缩强度 (循环切换)

function getNextCompressIntensity() {
    const values = [CompressIntensity.None, CompressIntensity.Extreme, 
                    CompressIntensity.High, CompressIntensity.Medium, 
                    CompressIntensity.Low];
    const currentIndex = values.indexOf(currentCompressIntensity);
    const nextIndex = (currentIndex + 1) % values.length;
    return values[nextIndex];
}

// 处理压缩按钮点击
function handleCompressButtonClick() {
    const nextIntensity = getNextCompressIntensity();
    setCompressIntensity(nextIntensity);
    
    // 通知C++压缩强度已改变
    sendMessageToNative({
        action: 'compressIntensityChanged',
        intensity: nextIntensity
    });
}

// 设置压缩后大小显示 (如 "18K", "1.21M", "0B" 等)
function setCompressedSize(sizeText, tooltip) {
    const sizeElement = document.getElementById('compressSize');
    if (sizeElement) {
        sizeElement.textContent = sizeText || '0B';
        if (tooltip) {
            sizeElement.title = tooltip;
        }
    }
}

// 处理@按钮点击
function handleAtButtonClick(event) {
    if (AppState.currentTags.length === 0) {
        return;
    }
    
    event.stopPropagation();
    
    const atButton = document.getElementById('atButton');
    const rect = atButton.getBoundingClientRect();
    
    sendMessageToNative({
        action: 'showTagMenu',
        position: {
            x: rect.left,
            y: rect.top
        }
    });
}

// 处理发送按钮点击
function handleSendClick() {
    const contentJson = getInputContent();
    const plainText = getEditorPlainText();
    
    if (plainText.trim()) {
        sendMessageToNative({
            action: 'send',
            content: JSON.parse(contentJson),
            plainText: plainText
        });
        
        clearInputContent();
    }
}

// 处理停止按钮点击
function handleStopClick() {
    sendMessageToNative({
        action: 'stopButtonClicked'
    });
}

// 保存编辑器失去焦点时的选区
let _savedSelectionRange = null;
// 正在恢复焦点期间，暂停保存，避免获焦时浏览器默认选区覆盖已保存的位置
let _restoringFocus = false;

// 保存当前选区（在编辑器内光标变化时持续调用，以便恢复焦点时能回到原位）
function saveSelection() {
    if (_restoringFocus) return;

    const inputEditor = document.getElementById('inputEditor');
    if (!inputEditor) return;

    const selection = window.getSelection();
    if (selection.rangeCount === 0) return;

    const range = selection.getRangeAt(0);
    // 只保存在 inputEditor 内部的选区（包含折叠光标）
    if (inputEditor.contains(range.commonAncestorContainer)) {
        _savedSelectionRange = range.cloneRange();
    }
}


// 焦点编辑器
function focusEditor() {
    const inputEditor = document.getElementById('inputEditor');
    if (inputEditor) {
        // 先取出要恢复的位置，防止 focus 引发的事件覆盖已保存的选区
        const rangeToRestore = (_savedSelectionRange && inputEditor.contains(_savedSelectionRange.commonAncestorContainer))
            ? _savedSelectionRange.cloneRange()
            : null;

        _restoringFocus = true;
        inputEditor.focus();

        const selection = window.getSelection();
        if (rangeToRestore) {
            // 恢复之前保存的选区位置
            selection.removeAllRanges();
            selection.addRange(rangeToRestore);
        } else {
            // 没有保存的选区，则将光标移到末尾
            const range = document.createRange();
            range.selectNodeContents(inputEditor);
            range.collapse(false);
            selection.removeAllRanges();
            selection.addRange(range);
        }
        _restoringFocus = false;
    }
}

// 处理Skill按钮点击
function handleSkillClick() {
    const skillButton = document.getElementById('skillButton');
    const rect = skillButton ? skillButton.getBoundingClientRect() : null;
    sendMessageToNative({
        action: 'skillButtonClicked',
        rect: rect ? {
            left: rect.left,
            top: rect.top,
            right: rect.right,
            bottom: rect.bottom,
            width: rect.width,
            height: rect.height
        } : null
    });
}

// 处理MCP按钮点击
function handleMcpClick() {
    const mcpButton = document.getElementById('mcpButton');
    const rect = mcpButton ? mcpButton.getBoundingClientRect() : null;
    sendMessageToNative({
        action: 'mcpButtonClicked',
        rect: rect ? {
            left: rect.left,
            top: rect.top,
            right: rect.right,
            bottom: rect.bottom,
            width: rect.width,
            height: rect.height
        } : null
    });
}

// 压缩结果提示标签相关
let _compressSummarizeTipTimer = null;
let _isTipHovered = false;
let _tipEventListenersAttached = false;

// 显示压缩结果提示
// success: 是否成功
// message: 显示的消息
// logPath: 日志文件路径（成功时显示链接）
function showCompressSummarizeTip(success, message, logPath) {
    const tipElement = document.getElementById('compressSummarizeTip');
    const textElement = document.getElementById('compressSummarizeTipText');
    const linkElement = document.getElementById('compressSummarizeTipLink');
    
    if (!tipElement || !textElement) return;
    
    // 清除之前的定时器
    if (_compressSummarizeTipTimer) {
        clearTimeout(_compressSummarizeTipTimer);
        _compressSummarizeTipTimer = null;
    }
    
    // 只绑定一次事件监听器
    if (!_tipEventListenersAttached) {
        tipElement.addEventListener('mouseenter', function() {
            _isTipHovered = true;
            // 清除当前的隐藏定时器
            if (_compressSummarizeTipTimer) {
                clearTimeout(_compressSummarizeTipTimer);
                _compressSummarizeTipTimer = null;
            }
        });
        
        tipElement.addEventListener('mouseleave', function() {
            _isTipHovered = false;
            // 鼠标离开后，延迟2秒再隐藏
            _compressSummarizeTipTimer = setTimeout(function() {
                hideCompressSummarizeTip();
            }, 2000);
        });
        
        _tipEventListenersAttached = true;
    }
    
    // 设置样式
    tipElement.classList.remove('hidden', 'success', 'error');
    tipElement.classList.add(success ? 'success' : 'error');
    
    // 设置文本和链接
    if (linkElement) {
        if (success && logPath && message) {
            // 解析消息，分离 "view detail" 部分
            const viewDetailIndex = message.indexOf(', view detail');
            if (viewDetailIndex !== -1) {
                const mainMessage = message.substring(0, viewDetailIndex);
                textElement.textContent = mainMessage + ', ';
                linkElement.textContent = 'view detail';
                linkElement.classList.remove('hidden');
            linkElement.onclick = function(e) {
                    e.preventDefault();
                    e.stopPropagation();
                    sendMessageToNative({
                        action: 'openLogFile',
                        path: logPath
                    });
                };
            } else {
                textElement.textContent = message || '';
                linkElement.classList.add('hidden');
            }
        } else {
            textElement.textContent = message || '';
            linkElement.classList.add('hidden');
        }
    } else {
        textElement.textContent = message || '';
    }
    
    // 5秒后自动隐藏（如果未被悬停）
    _compressSummarizeTipTimer = setTimeout(function() {
        if (!_isTipHovered) {
            hideCompressSummarizeTip();
        }
    }, 5000);
}

// 隐藏压缩结果提示
function hideCompressSummarizeTip() {
    const tipElement = document.getElementById('compressSummarizeTip');
    if (tipElement) {
        tipElement.classList.add('hidden');
    }
    if (_compressSummarizeTipTimer) {
        clearTimeout(_compressSummarizeTipTimer);
        _compressSummarizeTipTimer = null;
    }
}

// 设置输入提示开关按钮状态
function setInputHintToggleButtonState(enabled) {
    const button = document.getElementById('inputHintToggleButton');
    if (button) {
        if (enabled) {
            button.classList.remove('disabled');
        } else {
            button.classList.add('disabled');
        }
    }
}

// 处理输入提示开关按钮点击
function handleInputHintToggleClick() {
    const button = document.getElementById('inputHintToggleButton');
    if (button) {
        const isCurrentlyEnabled = !button.classList.contains('disabled');
        // 切换状态 - 如果当前是启用状态，点击后变为禁用（添加disabled类）
        // 如果当前是禁用状态，点击后变为启用（移除disabled类）
        sendMessageToNative({
            action: 'inputHintToggleClicked',
            enabled: !isCurrentlyEnabled
        });
    }
}

// 导出到全局
window.updateToolButtons = updateToolButtons;
window.createToolButtonElement = createToolButtonElement;
window.setSendButtonEnabled = setSendButtonEnabled;
window.setSendButtonText = setSendButtonText;
window.showStopButton = showStopButton;
window.hideStopButton = hideStopButton;
window.setPlaceholder = setPlaceholder;
window.handleApiMenuButtonClick = handleApiMenuButtonClick;
window.updateMajorChatApiMenu = updateMajorChatApiMenu;
window.setCompressIntensity = setCompressIntensity;
window.startCompressFlowing = startCompressFlowing;
window.stopCompressFlowing = stopCompressFlowing;
window.setCompressedSize = setCompressedSize;
window.handleCompressButtonClick = handleCompressButtonClick;
window.handleAtButtonClick = handleAtButtonClick;
window.handleSendClick = handleSendClick;
window.handleStopClick = handleStopClick;
window.handleSkillClick = handleSkillClick;
window.handleMcpClick = handleMcpClick;
window.focusEditor = focusEditor;
window.showCompressSummarizeTip = showCompressSummarizeTip;
window.hideCompressSummarizeTip = hideCompressSummarizeTip;
window.setInputHintToggleButtonState = setInputHintToggleButtonState;
window.handleInputHintToggleClick = handleInputHintToggleClick;