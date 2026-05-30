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
function updateMajorChatApiMenu(currentApi, apis) {
    const currentApiObject = apis.find(api => api.name === currentApi);
    const isCurrentApiAvailable = currentApiObject ? currentApiObject.available : true;
    
    const apiMenuText = document.getElementById('apiMenuText');
    if (apiMenuText) {
        apiMenuText.textContent = currentApi || 'n/a';
        apiMenuText.title = currentApi || 'n/a';

        if (!isCurrentApiAvailable) {
            apiMenuText.classList.add('unavailable');
        } else {
            apiMenuText.classList.remove('unavailable');
        }
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
        compressButton.title = tooltip;
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

// 焦点编辑器
function focusEditor() {
    const inputEditor = document.getElementById('inputEditor');
    if (inputEditor) {
        inputEditor.focus();
        
        const selection = window.getSelection();
        const range = document.createRange();
        range.selectNodeContents(inputEditor);
        range.collapse(false);
        selection.removeAllRanges();
        selection.addRange(range);
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
window.setCompressedSize = setCompressedSize;
window.handleCompressButtonClick = handleCompressButtonClick;
window.handleAtButtonClick = handleAtButtonClick;
window.handleSendClick = handleSendClick;
window.handleStopClick = handleStopClick;
window.handleSkillClick = handleSkillClick;
window.focusEditor = focusEditor;