// ====== 消息操作模块 ======

let chatContainer;

/**
 * 初始化消息模块
 */
function initMessageModule() {
    chatContainer = document.getElementById('chat-container');
}

/**
 * 创建消息元素
 * @param {string} id - 消息 ID
 * @param {string} type - 消息类型（user/ai/system）
 * @returns {HTMLElement} 消息内容元素
 */
function createMessageElement(id, type) {
    const messageElem = document.createElement('div');
    messageElem.id = id;
    messageElem.className = `message ${type}-message`;

    const contentElem = document.createElement('div');
    contentElem.className = 'message-content';
    messageElem.appendChild(contentElem);

    // 如果是用户消息，添加按钮区域
    if (type === 'user') {
        const buttonsArea = document.createElement('div');
        buttonsArea.className = 'user-message-buttons';

        const restoreBtn = document.createElement('button');
        restoreBtn.className = 'user-message-button';
        restoreBtn.textContent = '↻'; /* 只保留图标，移除文字 */
        restoreBtn.title = 'Restore';
        restoreBtn.onclick = (e) => {
            e.stopPropagation();
            window.chrome.webview.postMessage({
                action: 'userMessageRestoreClicked',
                messageId: id
            });
        };

        buttonsArea.appendChild(restoreBtn);
        messageElem.appendChild(buttonsArea);
    }

    chatContainer.appendChild(messageElem);
    return contentElem;
}

/**
 * 添加用户或系统消息
 * @param {string} id - 消息 ID
 * @param {string} textContent - 消息内容
 * @param {string} type - 消息类型（user/system）
 * @param {boolean} isFullContent - 是否为完整内容（JSON数组格式，包含标签）
 */
function addUserOrSystemMessage(id, textContent, type, isFullContent = false) {
    const contentElem = createMessageElement(id, type);

    if (isFullContent && Array.isArray(textContent)) {
        // 处理完整内容（JSON数组格式）
        renderFullContent(contentElem, textContent);
    } else {
        // 纯文本内容
        contentElem.textContent = textContent;
    }

    // 如果是系统消息且以 "Error:" 开头，设置红色背景
    if (type === 'system' && textContent.startsWith('Error:')) {
        contentElem.style.backgroundColor = 'rgba(255, 0, 0, 0.2)';
        contentElem.style.border = '1px solid rgba(255, 0, 0, 0.5)';
    }

    scrollToBottom(); /* Always scroll for user/system messages */
}

/**
 * 渲染完整内容（包含标签的JSON数组）
 * @param {HTMLElement} contentElem - 内容元素
 * @param {Array} contentArray - 内容数组
 */
function renderFullContent(contentElem, contentArray) {
    // 使用公共函数渲染内容
    renderContentWithTags(contentElem, contentArray);
}

/**
 * 开始 AI 消息
 * @param {string} id - 消息 ID
 */
function startAIMessage(id) {
    const shouldScroll = isNearBottom();
    const contentElem = createMessageElement(id, 'ai');
    if (shouldScroll) {
        scrollToBottom();
    }
}

/**
 * 追加内容到 AI 消息
 * @param {string} id - 消息 ID
 * @param {string} incrementalContent - 增量内容
 * @param {boolean} isFinalChunk - 是否为最后一块
 */
function appendToAIMessage(id, incrementalContent, isFinalChunk) {
    const shouldScroll = isNearBottom();
    const messageElem = document.getElementById(id);
    if (!messageElem) { console.error('AIMessage element not found for append:', id); return; }

    const messageContentElem = messageElem.querySelector('.message-content');
    if (!messageContentElem) { console.error('AIMessage content element (.message-content) not found for append in:', id); return; }

    // Find and remove any existing thinking containers
    const thinkingContainers = messageContentElem.querySelectorAll('.ai-thinking-container');
    thinkingContainers.forEach(container => container.remove());

    // 删除 thinking 后，合并相邻的 exploring-group
    _MergeAdjacentExploringGroups(messageContentElem);

    let targetTextContainer;
    const lastChild = messageContentElem.lastElementChild;

    if (!lastChild || !lastChild.classList.contains('ai-text-container')) {
        targetTextContainer = document.createElement('div');
        targetTextContainer.className = 'ai-text-container';
        targetTextContainer.setAttribute('data-raw-content', '');
        messageContentElem.appendChild(targetTextContainer);
    } else {
        targetTextContainer = lastChild;
    }

    let rawContent = targetTextContainer.getAttribute('data-raw-content') || '';
    rawContent += incrementalContent;
    targetTextContainer.setAttribute('data-raw-content', rawContent);

    if (typeof DOMPurify !== 'undefined' && typeof marked !== 'undefined') {
        targetTextContainer.innerHTML = DOMPurify.sanitize(marked.parse(rawContent));
        // 为代码块添加复制按钮
        addCopyButtonsToCodeBlocks(targetTextContainer);
    } else {
        console.error('DOMPurify or Marked.js not loaded.');
        targetTextContainer.textContent = rawContent; /* Fallback */
    }

    // 即时渲染已完成的 Mermaid 图表（每次增量更新都检查）
    renderCompletedMermaidBlocks(targetTextContainer, id);

    if (isFinalChunk) {
        // 收集 symbols 并发送给 C++ 查询
        collectSymbolsForMessage(id);
    }
    if (shouldScroll) {
        scrollToBottom();
    }
}

/**
 * 追加 thinking 内容到 AI 消息
 * @param {string} id - 消息 ID
 * @param {string} incrementalContent - 增量内容
 * @param {boolean} isFinalChunk - 是否为最后一块
 */
function appendToAIMessage_Thinking(id, incrementalContent, isFinalChunk) {
    const shouldScroll = isNearBottom();
    const messageElem = document.getElementById(id);
    if (!messageElem) { console.error('AIMessage element not found for thinking append:', id); return; }

    const messageContentElem = messageElem.querySelector('.message-content');
    if (!messageContentElem) { console.error('AIMessage content element (.message-content) not found for thinking append in:', id); return; }

    let targetTextContainer;
    const lastChild = messageContentElem.lastElementChild;

    if (!lastChild || !lastChild.classList.contains('ai-thinking-container')) {
        targetTextContainer = document.createElement('div');
        targetTextContainer.className = 'ai-thinking-container ai-thinking-content';
        targetTextContainer.setAttribute('data-raw-content', '');
        messageContentElem.appendChild(targetTextContainer);
    } else {
        targetTextContainer = lastChild;
    }

    let rawContent = targetTextContainer.getAttribute('data-raw-content') || '';
    rawContent += incrementalContent;
    targetTextContainer.setAttribute('data-raw-content', rawContent);

    // thinking 消息直接显示文本，不进行 markdown 渲染
    targetTextContainer.textContent = rawContent;

    if (isFinalChunk) {
        /* Optional: targetTextContainer.removeAttribute('data-raw-content'); */
    }
    if (shouldScroll) {
        scrollToBottom();
    }
}

/**
 * 合并 messageContentElem 中相邻的 exploring-group
 * 当中间隔的元素（如 thinking）被删除后，原本不连续的 exploring-group 变为相邻，需要合并
 */
function _MergeAdjacentExploringGroups(messageContentElem) {
    const children = Array.from(messageContentElem.children);
    for (let i = children.length - 2; i >= 0; i--) {
        const curr = children[i];
        const next = children[i + 1];
        if (curr.classList.contains('exploring-group') && next.classList.contains('exploring-group')) {
            // 将 next 的所有 tool-call-message 移入 curr 的 content 容器
            const currContent = curr.querySelector('.exploring-group-content');
            const nextContent = next.querySelector('.exploring-group-content');
            if (currContent && nextContent) {
                while (nextContent.firstChild) {
                    currContent.appendChild(nextContent.firstChild);
                }
            }
            // 更新 curr 的计数
            const count = currContent.querySelectorAll('.tool-call-message').length;
            const countSpan = curr.querySelector('.exploring-group-count');
            if (countSpan) {
                countSpan.textContent = count;
            }
            // 删除 next
            next.remove();
        }
    }
}

/**
 * 添加 Tool Call 消息到 AI 消息（Exploring类型，支持连续消息折叠）
 * @param {string} id - 消息 ID
 * @param {string} textContent - 消息内容
 */
function addToolCallMessageToAIMessage_Exploring(id, textContent) {
    const messageElem = document.getElementById(id);
    if (!messageElem) {
        console.error('AI Message element not found for tool call message:', id);
        return;
    }

    const messageContentElem = messageElem.querySelector('.message-content');
    if (!messageContentElem) {
        console.error('AI Message content element not found for tool call message:', id);
        return;
    }

    // 创建 tool call 消息元素
    const toolCallElem = document.createElement('div');
    toolCallElem.className = 'tool-call-message';
    toolCallElem.textContent = textContent;

    // 检查最后一个子元素是否是 exploring-group，如果是则追加到该组
    const lastChild = messageContentElem.lastElementChild;
    let group = null;

    if (lastChild && lastChild.classList.contains('exploring-group')) {
        group = lastChild;
    } else {
        // 创建新的 exploring-group
        group = document.createElement('div');
        group.className = 'exploring-group';

        // 创建可点击的 label
        const label = document.createElement('div');
        label.className = 'exploring-group-label';

        const countSpan = document.createElement('span');
        countSpan.className = 'exploring-group-count';
        countSpan.textContent = '0';

        label.appendChild(document.createTextNode(' Triggers '));
        label.appendChild(countSpan);
        label.appendChild(document.createTextNode(' exploring operation(s)'));


        // 创建折叠内容容器
        const content = document.createElement('div');
        content.className = 'exploring-group-content';
        content.style.display = 'none'; // 默认折叠

        // 点击 label 切换展开/折叠
        label.onclick = function() {
            const isCollapsed = content.style.display === 'none';
            content.style.display = isCollapsed ? 'block' : 'none';
            label.classList.toggle('exploring-group-label-expanded', isCollapsed);
        };

        group.appendChild(label);
        group.appendChild(content);
        messageContentElem.appendChild(group);
    }

    // 将消息添加到组的内容容器中
    const contentContainer = group.querySelector('.exploring-group-content');
    contentContainer.appendChild(toolCallElem);

    // 更新计数
    const count = contentContainer.querySelectorAll('.tool-call-message').length;
    const countSpan = group.querySelector('.exploring-group-count');
    if (countSpan) {
        countSpan.textContent = count;
    }

    // 如果聊天窗口在底部附近，滚动到底部
    if (isNearBottom()) {
        scrollToBottom();
    }
}

/**
 * 在 AI 消息中添加用户插话
 * @param {string} messageId - AI 消息 ID
 * @param {string} content - 用户插话内容
 */
function addUserInterjectToAIMessage(messageId, content) {
    const messageElem = document.getElementById(messageId);
    if (!messageElem) {
        console.error('AI Message element not found for user interject:', messageId);
        return;
    }

    const messageContentElem = messageElem.querySelector('.message-content');
    if (!messageContentElem) {
        console.error('AI Message content element not found for user interject:', messageId);
        return;
    }

    // 创建用户插话元素，使用 user-message 样式（右侧、用户消息背景色）
    const interjectElem = document.createElement('div');
    interjectElem.className = 'user-interject';

    // 尝试解析 JSON，判断是否为完整内容数组
    try {
        const parsed = JSON.parse(content);
        if (Array.isArray(parsed)) {
            // 完整内容：使用标签渲染
            renderContentWithTags(interjectElem, parsed);
        } else {
            interjectElem.textContent = 'An interject from user: ' + content;
        }
    } catch (e) {
        // 纯文本
        interjectElem.textContent = 'An interject from user: ' + content;
    }

    // 添加到 AI 消息内容的末尾
    messageContentElem.appendChild(interjectElem);

    // 如果聊天窗口在底部附近，滚动到底部
    if (isNearBottom()) {
        scrollToBottom();
    }
}

/**
 * 清空聊天内容
 */
function clearChat() {
    chatContainer.innerHTML = '';
    // 重置标题栏名称
    setWebViewTitle(DEFAULT_CHAT_TITLE);
}

// ====== Disabled 消息相关函数 ======

/**
 * Disable 某个消息之后的所有消息
 * @param {string} messageId - 消息 ID
 */
function disableMessagesAfter(messageId) {
    const targetMessage = document.getElementById(messageId);
    if (!targetMessage) {
        console.error('Message not found:', messageId);
        return;
    }

    // 找到目标消息在容器中的位置
    const allMessages = chatContainer.querySelectorAll('.message');
    let found = false;

    for (let i = 0; i < allMessages.length; i++) {
        const message = allMessages[i];

        if (message.id === messageId) {
            // 找到目标消息，disable 它和后续所有消息
            found = true;
            message.classList.add('disabled');
            addDisabledMessageClickHandler(message);
        } else if (found) {
            // 如果已经找到目标消息，则 disable 后续所有消息
            message.classList.add('disabled');
            addDisabledMessageClickHandler(message);
        }
    }
}

/**
 * 启用所有被 disabled 的消息
 */
function enableAllDisabledMessages() {
    const disabledMessages = chatContainer.querySelectorAll('.message.disabled');

    disabledMessages.forEach(message => {
        // 移除 disabled 类
        message.classList.remove('disabled');

        // 移除点击事件处理器
        message.removeEventListener('click', handleDisabledMessageClick);
    });
}

/**
 * 删除所有被 disabled 的消息
 */
function removeDisabledMessages() {
    const disabledMessages = chatContainer.querySelectorAll('.message.disabled');

    disabledMessages.forEach(message => {
        // 从 DOM 中删除消息元素
        message.remove();
    });
}

/**
 * 为 disabled 消息添加点击事件处理
 * @param {HTMLElement} messageElement - 消息元素
 */
function addDisabledMessageClickHandler(messageElement) {
    // 移除之前可能存在的点击事件处理器
    messageElement.removeEventListener('click', handleDisabledMessageClick);

    // 添加新的点击事件处理器
    messageElement.addEventListener('click', handleDisabledMessageClick);
}

/**
 * 处理 disabled 消息的点击事件
 * @param {Event} event - 点击事件
 */
function handleDisabledMessageClick(event) {
    const messageElement = event.currentTarget;
    const messageId = messageElement.id;

    // 发送消息到 C++
    window.chrome.webview.postMessage({
        action: 'disabledMessageClicked',
        messageId: messageId
    });
}

// ====== Mermaid 即时渲染 ======

/** 已渲染过的 mermaid 预块的 Set（存储 pre 元素的引用，用 WeakSet 避免重复渲染） */
const renderedMermaidPres = new WeakSet();

/**
 * 扫描容器中已闭合（已完成）但尚未渲染的 mermaid 代码块，并即时渲染为 SVG。
 * 判断"已完成"的依据：该 ```mermaid 代码块后面已经出现了闭合的 ```。
 * @param {HTMLElement} container - 要扫描的容器元素
 * @param {string} messageId - 消息 ID，用于生成唯一的 mermaid ID
 */
function renderCompletedMermaidBlocks(container, messageId) {
    if (typeof mermaid === 'undefined') return;

    // 找到所有 <pre> 内的 mermaid 代码块
    const allMermaidCodeEls = container.querySelectorAll('code.language-mermaid');

    allMermaidCodeEls.forEach((codeEl, index) => {
        const preEl = codeEl.parentNode;

        // 已渲染过就跳过
        if (renderedMermaidPres.has(preEl)) return;

        // 检查该代码块是否已经闭合（后面是否有 ```）
        const aiTextContainer = container.closest('.ai-text-container');
        if (!aiTextContainer) return;

        const rawContent = aiTextContainer.getAttribute('data-raw-content') || '';

        // 获取代码块内容，用于在 rawContent 中定位
        const graphDefinition = codeEl.textContent.trim();
        if (!graphDefinition) return;

        // 在 rawContent 中查找该 mermaid 块，检查其后是否有闭合的 ```
        const escapedDef = graphDefinition.replace(/[.*+?^${}()|[\]\\]/g, '\\$&');
        // 匹配模式： ```mermaid\n<graphDefinition>\n```
        const pattern = new RegExp('```mermaid\\s*\\n' + escapedDef.replace(/\n/g, '\\n') + '\\s*\\n\\s*```', 's');
        const match = pattern.test(rawContent);

        if (!match) {
            // 还没闭合，继续等待后续 chunk
            return;
        }

        // 标记为已渲染（即使渲染失败也不要反复尝试）
        renderedMermaidPres.add(preEl);

        // 唯一 ID
        const mermaidId = `mermaid-${messageId}-${Date.now()}-${index}-${Math.random().toString(36).substr(2, 6)}`;

        try {
            mermaid.render(mermaidId, graphDefinition).then(({ svg }) => {
                const mermaidContainer = document.createElement('div');
                mermaidContainer.className = 'mermaid-container';
                mermaidContainer.innerHTML = svg;
                // 替换原来的 <pre> 节点
                if (preEl.parentNode) {
                    preEl.parentNode.replaceChild(mermaidContainer, preEl);
                }
            }).catch(error => {
                console.error('Mermaid rendering failed:', error);
            });
        } catch (e) {
            console.error('Mermaid render exception:', e);
        }
    });
}

// ====== 费用显示相关函数 ======

/**
 * 设置会话费用显示
 * @param {string} costText - 费用文本
 * @param {string} [messageId] - 可选，指定消息ID；不指定则使用最后一个未 disabled 的 AI 消息
 */
function setCostDisplay(costText, messageId) {
    let targetMessage = null;

    if (messageId) {
        // 通过 messageId 查找指定的 AI 消息
        targetMessage = document.getElementById(messageId);
        if (!targetMessage) {
            console.warn('setCostDisplay: message not found for messageId:', messageId);
            return;
        }
    } else {
        // 查找最后一个未 disabled 的 AI 消息
        const allMessages = chatContainer.querySelectorAll('.message.ai-message:not(.disabled)');
        if (allMessages.length === 0) {
            console.warn('No active AI messages found for cost display');
            return;
        }
        targetMessage = allMessages[allMessages.length - 1];
    }

    // 检查是否已经有费用显示元素，如果有则更新，没有则创建
    let costDisplay = targetMessage.querySelector('.cost-display');
    if (!costDisplay) {
        // 创建费用显示元素
        costDisplay = document.createElement('div');
        costDisplay.className = 'cost-display';
        targetMessage.appendChild(costDisplay);
    }

    // 更新费用文本
    costDisplay.textContent = costText;
    console.log('Cost display updated:', costText, messageId ? 'for messageId: ' + messageId : '');
}

/**
 * 创建 CLI 内容区域（包含命令、输出、可选的输入区域）
 * @param {string} cliId - CLI ID
 * @param {string} command - 命令内容
 * @param {boolean} withInputArea - 是否包含输入区域
 * @param {boolean} expanded - 是否默认展开
 * @returns {Object} { cliContent }
 */
function createCliContentArea(cliId, command, withInputArea, expanded) {
    const cliContent = document.createElement('div');
    cliContent.className = 'cli-collapsible-content';
    cliContent.id = `${cliId}-content`;
    cliContent.style.display = expanded ? 'block' : 'none';
    
    // 创建命令内容容器
    const commandContentWrapper = document.createElement('div');
    commandContentWrapper.className = 'cli-content-wrapper';
    
    const commandContent = document.createElement('div');
    commandContent.className = 'cli-command-content';
    commandContent.textContent = command;
    
    const commandCopyBtn = document.createElement('button');
    commandCopyBtn.className = 'cli-copy-button';
    commandCopyBtn.innerHTML = '<svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><rect x="9" y="9" width="13" height="13" rx="2" ry="2"></rect><path d="M5 15H4a2 2 0 0 1-2-2V4a2 2 0 0 1 2-2h9a2 2 0 0 1 2 2v1"></path></svg>';
    commandCopyBtn.title = 'Copy command';
    commandCopyBtn.onclick = function(e) {
        e.stopPropagation();
        copyCliContent(commandCopyBtn, command);
    };
    
    commandContentWrapper.appendChild(commandContent);
    commandContentWrapper.appendChild(commandCopyBtn);
    
    // 创建输出内容容器
    const outputContentWrapper = document.createElement('div');
    outputContentWrapper.className = 'cli-content-wrapper';
    
    const outputContent = document.createElement('div');
    outputContent.className = 'cli-output-content';
    outputContent.id = `${cliId}-output`;
    
    const outputCopyBtn = document.createElement('button');
    outputCopyBtn.className = 'cli-copy-button';
    outputCopyBtn.innerHTML = '<svg width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><rect x="9" y="9" width="13" height="13" rx="2" ry="2"></rect><path d="M5 15H4a2 2 0 0 1-2-2V4a2 2 0 0 1 2-2h9a2 2 0 0 1 2 2v1"></path></svg>';
    outputCopyBtn.title = 'Copy output';
    outputCopyBtn.onclick = function(e) {
        e.stopPropagation();
        const outputText = outputContent.textContent || '';
        copyCliContent(outputCopyBtn, outputText);
    };
    
    outputContentWrapper.appendChild(outputContent);
    outputContentWrapper.appendChild(outputCopyBtn);
    
    cliContent.appendChild(commandContentWrapper);
    cliContent.appendChild(outputContentWrapper);
    
    // 可选：创建输入区域
    if (withInputArea) {
        const inputArea = createCliInputArea(cliId);
        cliContent.appendChild(inputArea);
    }
    
    return { cliContent };
}

/**
 * 创建 CLI 输入区域
 * @param {string} cliId - CLI ID
 * @returns {HTMLElement} 输入区域元素
 */
function createCliInputArea(cliId) {
    const inputArea = document.createElement('div');
    inputArea.className = 'cli-input-area hidden';
    inputArea.id = `${cliId}-input-area`;
    
    const inputPrompt = document.createElement('span');
    inputPrompt.className = 'cli-input-prompt';
    inputPrompt.textContent = '⟩';
    
    const inputField = document.createElement('input');
    inputField.type = 'text';
    inputField.className = 'cli-input-field';
    inputField.id = `${cliId}-input-field`;
    inputField.placeholder = 'Type input and press Enter...';
    
    const sendBtn = document.createElement('button');
    sendBtn.className = 'cli-send-btn';
    sendBtn.textContent = 'Send';
    sendBtn.onclick = function() {
        sendCliInput(cliId, null);
    };
    
    inputField.onkeypress = function(e) {
        if (e.key === 'Enter') {
            sendCliInput(cliId, null);
        }
    };
    
    inputArea.appendChild(inputPrompt);
    inputArea.appendChild(inputField);
    inputArea.appendChild(sendBtn);
    
    return inputArea;
}

function addCliDisplay(messageId, cliId, command, desc, status = "none", shellType = "") {
    const shouldScroll = isNearBottom();
    
    // 查找对应的 AI 消息元素
    const messageElem = document.getElementById(messageId);
    if (!messageElem) {
        console.warn('AI message not found for messageId:', messageId);
        return;
    }
    
    const messageContentElem = messageElem.querySelector('.message-content');
    if (!messageContentElem) {
        console.warn('Message content element not found for messageId:', messageId);
        return;
    }
    
    // 创建新的 CLI 显示容器
    const cliContainer = document.createElement('div');
    cliContainer.className = 'cli-display-container';
    cliContainer.id = cliId;
    cliContainer.setAttribute('data-message-id', messageId);
    cliContainer.setAttribute('data-status', status);
    
    // 创建可折叠的标题栏
    const cliHeader = document.createElement('div');
    cliHeader.className = 'cli-collapsible-header';
    cliHeader.onclick = function() {
        toggleCliContent(cliId);
    };
    
    // 创建展开按钮（初始为收起状态）
    const expandIcon = document.createElement('span');
    expandIcon.className = 'cli-expand-icon';
    expandIcon.textContent = '+';
    
    // 创建 shell type 标签
    const shellTypeLabel = document.createElement('span');
    shellTypeLabel.className = 'cli-shell-type';
    if (shellType && shellType.trim()) {
        let shellName = shellType.trim();
        if (shellName.toLowerCase().endsWith('.exe')) {
            shellName = shellName.slice(0, -4);
        }
        shellTypeLabel.textContent = shellName.toUpperCase();
    } else {
        shellTypeLabel.textContent = 'CLI';
    }
    
    // 创建标题文本
    const headerText = document.createElement('span');
    headerText.className = 'cli-header-text';
    headerText.textContent = (desc && desc.trim()) ? desc : command;
    
    cliHeader.appendChild(expandIcon);
    cliHeader.appendChild(shellTypeLabel);
    cliHeader.appendChild(headerText);
    
    // 根据 status 添加不同的按钮和内容
    const isPending = (status === "pending");
    const isAccepted = (status === "accepted");
    const withInputArea = isPending || isAccepted;
    
    if (isPending) {
        // pending 状态：显示播放、禁止、白名单三个按钮
        cliHeader.appendChild(createPlayButton(cliId));
        cliHeader.appendChild(createRejectButton(cliId));
        cliHeader.appendChild(createWhitelistButton(cliId));
    } else if (isAccepted) {
        // accepted 状态：显示停止、白名单按钮
        cliHeader.appendChild(createStopButton(cliId));
        cliHeader.appendChild(createWhitelistButton(cliId));
    } else {
        // none 状态：只显示白名单按钮
        cliHeader.appendChild(createWhitelistButton(cliId));
    }
    
    // 创建内容区域（pending 状态自动展开）
    const { cliContent } = createCliContentArea(cliId, command, withInputArea, isPending);
    if (isPending) {
        expandIcon.textContent = '-';
    }
    
    cliContainer.appendChild(cliHeader);
    cliContainer.appendChild(cliContent);
    messageContentElem.appendChild(cliContainer);
    
    // 自动滚动到底部
    if (shouldScroll) {
        scrollToBottom();
    }
}

/**
 * 处理 CLI 按钮点击动作
 * @param {string} cliId - CLI 容器的 ID
 * @param {string} action - 动作类型（"accept", "reject", "whitelist"）
 */
function onCliAction(cliId, action) {
    const container = document.getElementById(cliId);
    if (!container) return;
    
    // 根据操作类型更新状态
    let newStatus = container.getAttribute('data-status');
    if (action === 'accept') {
        newStatus = 'accept';
    } else if (action === 'reject') {
        newStatus = 'reject';
    } else if (action === 'stop') {
        newStatus = 'stop';
    }
    
    // 更新容器状态属性
    container.setAttribute('data-status', newStatus);
    
    // 根据状态更新按钮显示
    updateCliButtonsByStatus(container, cliId, newStatus);
    
    // 发送消息到 C++
    // accept 和 reject 直接发送给 CChatUi，whitelist 发送给 CChatDialogA
    if (action === 'whitelist') {
        // 白名单消息发送到 CChatDialogA
        window.chrome.webview.postMessage({
            action: 'cliWhitelist',
            cliId: cliId
        });
    } else if (action !== 'whitelist') {
        // accept, reject, stop 消息发送到 CChatUi
        window.chrome.webview.postMessage({
            action: 'cliStatusChange',
            cliId: cliId,
            cliStatus: action
        });
    }
}

/**
 * 根据 CliStatus 更新按钮显示
 * @param {HTMLElement} container - CLI 容器元素
 * @param {string} cliId - CLI 容器的 ID
 * @param {string} status - 状态 ('none', 'pending', 'accept', 'reject', 'stop')
 */
function updateCliButtonsByStatus(container, cliId, status) {
    // 获取或创建按钮
    let playBtn = container.querySelector('.cli-play-button');
    let rejectBtn = container.querySelector('.cli-reject-button');
    let stopBtn = container.querySelector('.cli-stop-button');
    let whitelistBtn = container.querySelector('.cli-whitelist-button');
    
    // 获取标题栏元素（类名是 cli-collapsible-header）
    const cliHeader = container.querySelector('.cli-collapsible-header');
    
    // 确保白名单按钮存在
    if (!whitelistBtn) {
        whitelistBtn = createWhitelistButton(cliId);
        cliHeader.appendChild(whitelistBtn);
    }
    
    // 根据状态显示/隐藏按钮
    switch (status) {
        case 'pending':
            // Pending 状态：显示 Play、Reject、Whitelist
            if (!playBtn) {
                playBtn = createPlayButton(cliId);
                cliHeader.appendChild(playBtn);
            }
            if (!rejectBtn) {
                rejectBtn = createRejectButton(cliId);
                cliHeader.appendChild(rejectBtn);
            }
            if (playBtn) playBtn.style.display = 'inline-block';
            if (rejectBtn) rejectBtn.style.display = 'inline-block';
            if (stopBtn) stopBtn.style.display = 'none';
            if (whitelistBtn) whitelistBtn.style.display = 'inline-block';
            break;
            
        case 'accept':
        case 'accepted':
            // Running 状态（Accept 后或白名单命令）：显示 Stop、Whitelist
            if (playBtn) playBtn.style.display = 'none';
            if (rejectBtn) rejectBtn.style.display = 'none';
            if (!stopBtn) {
                stopBtn = createStopButton(cliId);
                if (whitelistBtn && whitelistBtn.parentNode === cliHeader) {
                    cliHeader.insertBefore(stopBtn, whitelistBtn);
                } else {
                    cliHeader.appendChild(stopBtn);
                }
            }
            if (stopBtn) stopBtn.style.display = 'inline-block';
            if (whitelistBtn) whitelistBtn.style.display = 'inline-block';
            break;
            
        case 'reject':
        case 'stop':
        case 'none':
        default:
            // 完成/停止/初始状态：只显示 Whitelist
            if (playBtn) playBtn.style.display = 'none';
            if (rejectBtn) rejectBtn.style.display = 'none';
            if (stopBtn) stopBtn.style.display = 'none';
            if (whitelistBtn) whitelistBtn.style.display = 'inline-block';
            break;
    }
}

/**
 * 创建播放按钮
 * @param {string} cliId - CLI 容器的 ID
 * @returns {HTMLElement} 播放按钮元素
 */
function createPlayButton(cliId) {
    const playBtn = document.createElement('span');
    playBtn.className = 'cli-action-button cli-play-button';
    playBtn.innerHTML = '▶';
    playBtn.title = 'Play';
    playBtn.onclick = function(e) {
        e.stopPropagation();
        onCliAction(cliId, 'accept');
    };
    return playBtn;
}

/**
 * 创建禁止按钮
 * @param {string} cliId - CLI 容器的 ID
 * @returns {HTMLElement} 禁止按钮元素
 */
function createRejectButton(cliId) {
    const rejectBtn = document.createElement('span');
    rejectBtn.className = 'cli-action-button cli-reject-button';
    rejectBtn.innerHTML = '⛔';
    rejectBtn.title = 'Reject';
    rejectBtn.onclick = function(e) {
        e.stopPropagation();
        onCliAction(cliId, 'reject');
    };
    return rejectBtn;
}

/**
 * 创建停止按钮
 * @param {string} cliId - CLI 容器的 ID
 * @returns {HTMLElement} 停止按钮元素
 */
function createStopButton(cliId) {
    const stopBtn = document.createElement('span');
    stopBtn.className = 'cli-action-button cli-stop-button';
    stopBtn.innerHTML = '⏹';
    stopBtn.title = 'Stop';
    stopBtn.onclick = function(e) {
        e.stopPropagation();
        onCliAction(cliId, 'stop');
    };
    return stopBtn;
}

/**
 * 创建白名单按钮
 * @param {string} cliId - CLI 容器的 ID
 * @returns {HTMLElement} 白名单按钮元素
 */
function createWhitelistButton(cliId) {
    const whitelistBtn = document.createElement('span');
    whitelistBtn.className = 'cli-action-button cli-whitelist-button';
    whitelistBtn.innerHTML = '📋';
    whitelistBtn.title = 'Edit Command Whitelist';
    whitelistBtn.onclick = function(e) {
        e.stopPropagation();
        onCliAction(cliId, 'whitelist');
    };
    return whitelistBtn;
}

/**
 * 发送 CLI 输入到 C++
 * @param {string} cliId - CLI 容器的 ID
 * @param {string} messageId - 消息 ID
 */
function sendCliInput(cliId, messageId) {
    const inputField = document.getElementById(`${cliId}-input-field`);
    if (!inputField) return;
    
    const input = inputField.value;
    if (!input.trim()) return;
    
    // 发送到 C++
    window.chrome.webview.postMessage({
        action: 'cliInput',
        messageId: messageId,
        cliId: cliId,
        input: input
    });
    
    // 在输出区域显示用户输入（回显）
    const outputContent = document.getElementById(`${cliId}-output`);
    if (outputContent) {
        outputContent.textContent += `\n> ${input}\n`;
    }
    
    // 清空输入框
    inputField.value = '';
    
    // 滚动到底部
    scrollToBottom();
}


/**
 * 显示/隐藏 CLI 输入区域
 * @param {string} cliId - CLI 容器的 ID
 * @param {boolean} show - 是否显示
 */
function showCliInputArea(cliId, show) {
    console.log('showCliInputArea called:', cliId, 'show:', show);
    const inputArea = document.getElementById(`${cliId}-input-area`);
    console.log('inputArea element:', inputArea);
    if (inputArea) {
        if (show) {
            inputArea.classList.remove('hidden');
            // 聚焦输入框
            const inputField = document.getElementById(`${cliId}-input-field`);
            if (inputField) inputField.focus();
            
            // 自动展开 CLI 内容
            const content = document.getElementById(`${cliId}-content`);
            const container = document.getElementById(cliId);
            if (content && container) {
                content.style.display = 'block';
                const icon = container.querySelector('.cli-expand-icon');
                if (icon) icon.textContent = '-';  // 展开状态显示 -
            }
        } else {
            inputArea.classList.add('hidden');
        }
    } else {
        console.warn('inputArea not found for cliId:', cliId);
        // 尝试查找所有 cli-input-area
        const allInputAreas = document.querySelectorAll('.cli-input-area');
        console.log('all input areas:', allInputAreas);
    }
}

/**
 * 完成 CLI 显示
 * @param {string} cliId - CLI 容器的 ID
 * @param {number} exitCode - 退出码（-1 表示被中断）
 */
function completeCliDisplay(cliId, exitCode) {
    // 隐藏输入区域
    const inputArea = document.getElementById(`${cliId}-input-area`);
    if (inputArea) {
        inputArea.classList.add('hidden');
    }
    
    // 根据状态更新按钮（CLI 完成后进入 none 状态，只显示白名单按钮）
    const container = document.getElementById(cliId);
    if (container) {
        container.setAttribute('data-status', 'none');
        updateCliButtonsByStatus(container, cliId, 'none');
    }
}

function addQuestion(messageId, questionId, question, options) {
    const shouldScroll = isNearBottom();
    
    // 查找对应的 AI 消息元素
    const messageElem = document.getElementById(messageId);
    if (!messageElem) {
        console.warn('AI message not found for messageId:', messageId);
        return;
    }
    
    const messageContentElem = messageElem.querySelector('.message-content');
    if (!messageContentElem) {
        console.warn('Message content element not found for messageId:', messageId);
        return;
    }
    
    // 创建问题容器
    const questionContainer = document.createElement('div');
    questionContainer.className = 'question-container';
    questionContainer.id = `question-${questionId}`;
    
    // 创建问题文本
    const questionText = document.createElement('div');
    questionText.className = 'question-text';
    questionText.textContent = question;
    questionContainer.appendChild(questionText);
    
    // 判断是否有选项
    if (options && options.length > 0) {
        // 有选项 - 创建选择窗口
        const optionsContainer = document.createElement('div');
        optionsContainer.className = 'question-options';
        
        // 添加选项按钮
        options.forEach(function(option) {
            const optionBtn = document.createElement('button');
            optionBtn.className = 'question-option-btn';
            optionBtn.textContent = option;
            optionBtn.onclick = function() {
                submitQuestionAnswer(questionId, option);
            };
            optionsContainer.appendChild(optionBtn);
        });
        
        // 添加取消按钮
        const cancelBtn = document.createElement('button');
        cancelBtn.className = 'question-cancel-btn';
        cancelBtn.textContent = 'Cancel';
        cancelBtn.onclick = function() {
            submitQuestionAnswer(questionId, 'Cancel');
        };
        optionsContainer.appendChild(cancelBtn);
        
        questionContainer.appendChild(optionsContainer);
    } else {
        // 没有选项 - 创建输入窗口
        const inputContainer = document.createElement('div');
        inputContainer.className = 'question-input-container';
        
        const inputField = document.createElement('input');
        inputField.type = 'text';
        inputField.className = 'question-input-field';
        inputField.placeholder = 'Please enter your answer...';
        
        const btnContainer = document.createElement('div');
        btnContainer.className = 'question-btn-container';
        
        const submitBtn = document.createElement('button');
        submitBtn.className = 'question-submit-btn';
        submitBtn.textContent = 'OK';
        submitBtn.onclick = function() {
            const answer = inputField.value.trim();
            if (answer) {
                submitQuestionAnswer(questionId, answer);
            }
        };
        
        const cancelBtn = document.createElement('button');
        cancelBtn.className = 'question-cancel-btn';
        cancelBtn.textContent = 'Cancel';
        cancelBtn.onclick = function() {
            submitQuestionAnswer(questionId, 'Cancel');
        };
        
        // 支持回车提交
        inputField.onkeypress = function(e) {
            if (e.key === 'Enter') {
                const answer = inputField.value.trim();
                if (answer) {
                    submitQuestionAnswer(questionId, answer);
                }
            }
        };
        
        btnContainer.appendChild(submitBtn);
        btnContainer.appendChild(cancelBtn);
        inputContainer.appendChild(inputField);
        inputContainer.appendChild(btnContainer);
        questionContainer.appendChild(inputContainer);
    }
    
    messageContentElem.appendChild(questionContainer);
    
    // 自动滚动到底部
    if (shouldScroll) {
        scrollToBottom();
    }
}

/**
 * 添加问题和答案显示框
 * @param {string} messageId - 消息ID
 * @param {string} question - 问题文本
 * @param {string} answer - 答案文本
 */
function addQuestionDisplay(messageId, question, answer) {
    const shouldScroll = isNearBottom();
    
    // 查找对应的 AI 消息元素
    const messageElem = document.getElementById(messageId);
    if (!messageElem) {
        console.warn('AI message not found for messageId:', messageId);
        return;
    }
    
    const messageContentElem = messageElem.querySelector('.message-content');
    if (!messageContentElem) {
        console.warn('Message content element not found for messageId:', messageId);
        return;
    }
    
    // 创建问题显示容器
    const displayContainer = document.createElement('div');
    displayContainer.className = 'question-display-container';
    
    // 创建问题文本
    const questionText = document.createElement('div');
    questionText.className = 'question-display-text';
    questionText.textContent = question;
    displayContainer.appendChild(questionText);
    
    // 创建分隔线
    const divider = document.createElement('div');
    divider.className = 'question-display-divider';
    displayContainer.appendChild(divider);
    
    // 创建答案文本
    const answerText = document.createElement('div');
    answerText.className = 'question-display-answer';
    answerText.textContent = answer;
    displayContainer.appendChild(answerText);
    
    messageContentElem.appendChild(displayContainer);
    
    // 自动滚动到底部
    if (shouldScroll) {
        scrollToBottom();
    }
}

/**
 * 清除当前显示的问题
 */
function clearQuestion() {
    // 查找所有问题容器
    const questionContainers = document.querySelectorAll('.question-container');
    
    // 移除所有问题容器
    questionContainers.forEach(function(container) {
        container.remove();
    });
    
}

function submitQuestionAnswer(questionId, answer) {
    // 发送答案到 C++
    const message = {
        action: 'questionAnswer',
        questionId: questionId,
        answer: answer
    };
    window.chrome.webview.postMessage(JSON.stringify(message));
    
    // 移除问题窗口
    const questionContainer = document.getElementById(`question-${questionId}`);
    if (questionContainer) {
        questionContainer.remove();
    }
}

/**
 * 切换 CLI 内容的展开/折叠状态
 * @param {string} cliId - CLI 容器的 ID
 */
function toggleCliContent(cliId) {
    const content = document.getElementById(`${cliId}-content`);
    const container = document.getElementById(cliId);
    
    if (!content || !container) return;
    
    const icon = container.querySelector('.cli-expand-icon');
    
    if (content.style.display === 'none' || content.style.display === '') {
        // 展开
        content.style.display = 'block';
        if (icon) icon.textContent = '-';  // 展开状态显示 -
    } else {
        // 折叠
        content.style.display = 'none';
        if (icon) icon.textContent = '+';  // 收起状态显示 +
    }
}

/**
 * 在最后一个 CLI 显示元素中追加输出
 * @param {string} messageId - 消息 ID
 * @param {string} deltaOutput - 增量输出内容
 */
/**
 * 启动等待中的 CLI（用户点击播放按钮）
 * @param {string} cliId - CLI 容器的 ID
 */
function startPendingCli(cliId) {
    // 发送到 C++
    window.chrome.webview.postMessage({
        action: 'startPendingCli',
        cliId: cliId
    });
}

function appendCliOutput(messageId, deltaOutput) {
    const shouldScroll = isNearBottom();
    
    // 查找对应的 AI 消息元素
    const messageElem = document.getElementById(messageId);
    if (!messageElem) {
        console.warn('AI message not found for messageId:', messageId);
        return;
    }
    
    const messageContentElem = messageElem.querySelector('.message-content');
    if (!messageContentElem) {
        console.warn('Message content element not found for messageId:', messageId);
        return;
    }
    
    // 查找该 messageId 的最后一个 CLI 容器
    const cliContainers = messageContentElem.querySelectorAll(`.cli-display-container[data-message-id="${messageId}"]`);
    
    if (cliContainers.length === 0) {
        console.warn('No CLI display found for messageId:', messageId);
        return;
    }
    
    // 获取最后一个 CLI 容器
    const lastCliContainer = cliContainers[cliContainers.length - 1];
    
    // 查找输出内容元素
    const outputContent = lastCliContainer.querySelector('.cli-output-content');
    if (outputContent) {
        // 检查 CLI 输出区域是否在底部附近
        const isOutputNearBottom = outputContent.scrollHeight - outputContent.scrollTop - outputContent.clientHeight < 50;
        
        // 追加输出内容
        outputContent.textContent += deltaOutput;
        
        // 只有在底部附近时才自动滚动
        if (isOutputNearBottom) {
            outputContent.scrollTop = outputContent.scrollHeight;
        }
    }
    
    if (shouldScroll) {
        scrollToBottom();
    }
}

// ====== MCP Display 相关 ======

/**
 * 添加 MCP 工具调用显示
 * @param {string} messageId - 消息 ID
 * @param {string} mcpId - MCP 显示容器的唯一 ID
 * @param {string} mcpName - MCP 名称
 * @param {string} toolName - 工具名称
 * @param {string} arguments - 工具参数 JSON 字符串
 * @param {string} argsSummary - 参数摘要 JSON 字符串
 */
function addMcpDisplay(messageId, mcpId, mcpName, toolName, arguments, argsSummary) {
    const shouldScroll = isNearBottom();
    
    const messageElem = document.getElementById(messageId);
    if (!messageElem) return;
    
    const messageContentElem = messageElem.querySelector('.message-content');
    if (!messageContentElem) return;
    
    // 解析参数摘要
    let parsedArgs = [];
    try {
        parsedArgs = JSON.parse(argsSummary || '[]');
    } catch (e) {
        parsedArgs = [];
    }
    
    // 创建 MCP 显示容器
    const mcpContainer = document.createElement('div');
    mcpContainer.className = 'mcp-display-container';
    mcpContainer.id = mcpId;
    mcpContainer.setAttribute('data-message-id', messageId);
    
    // 创建可折叠的标题栏
    const mcpHeader = document.createElement('div');
    mcpHeader.className = 'mcp-collapsible-header';
    mcpHeader.onclick = function() {
        toggleMcpContent(mcpId);
    };
    
    // 展开按钮
    const expandIcon = document.createElement('span');
    expandIcon.className = 'mcp-expand-icon';
    expandIcon.textContent = '+';
    
    // MCP 名称标签（左边方框）
    const mcpLabel = document.createElement('span');
    mcpLabel.className = 'mcp-name-label';
    mcpLabel.textContent = '🛠️ ' + (mcpName || 'MCP');
    
    // Tool 名称标签（右边方框）
    const toolLabel = document.createElement('span');
    toolLabel.className = 'mcp-tool-name-label';
    toolLabel.textContent = toolName;
    
    // 标题文本（带参数摘要，key灰色 value白色）
    const headerText = document.createElement('span');
    headerText.className = 'mcp-header-text';
    if (parsedArgs.length > 0) {
        parsedArgs.forEach((arg, idx) => {
            if (idx > 0) headerText.innerHTML += '<span class="mcp-arg-sep">, </span>';
            headerText.innerHTML += '<span class="mcp-arg-key">' + escapeHtml(arg.k) + '=</span><span class="mcp-arg-val">' + escapeHtml(arg.v) + '</span>';
        });
    }
    
    // 停止按钮（在结果未设置时显示）
    const stopBtn = document.createElement('span');
    stopBtn.className = 'cli-action-button cli-stop-button mcp-stop-button';
    stopBtn.innerHTML = '⏹';
    stopBtn.title = 'Stop';
    stopBtn.id = mcpId + '-stop-btn';
    stopBtn.onclick = function(e) {
        e.stopPropagation();
        onMcpAction(mcpId, 'stop');
    };
    
    mcpHeader.appendChild(expandIcon);
    mcpHeader.appendChild(mcpLabel);
    mcpHeader.appendChild(toolLabel);
    mcpHeader.appendChild(headerText);
    mcpHeader.appendChild(stopBtn);
    
    // 创建内容区域
    const mcpContent = document.createElement('div');
    mcpContent.className = 'mcp-collapsible-content';
    mcpContent.id = mcpId + '-content';
    mcpContent.style.display = 'none';
    
    // 上半部分: 参数区域
    const argsSection = document.createElement('div');
    argsSection.className = 'mcp-args-section';
    
    const argsTitle = document.createElement('div');
    argsTitle.className = 'mcp-section-title';
    argsTitle.textContent = 'Arguments';
    argsSection.appendChild(argsTitle);
    
    const argsContent = document.createElement('div');
    argsContent.className = 'mcp-args-content';
    // 每个参数一行，key蓝色 value绿色
    if (parsedArgs.length > 0) {
        parsedArgs.forEach(arg => {
            const line = document.createElement('div');
            line.className = 'mcp-arg-line';
            line.innerHTML = '<span class="mcp-arg-key">' + escapeHtml(arg.k) + '=</span><span class="mcp-arg-val">' + escapeHtml(arg.v) + '</span>';
            argsContent.appendChild(line);
        });
    } else {
        // 回退: 尝试格式化 JSON
        try {
            const parsed = JSON.parse(arguments);
            argsContent.textContent = JSON.stringify(parsed, null, 2);
        } catch (e) {
            argsContent.textContent = arguments;
        }
    }
    argsSection.appendChild(argsContent);
    
    // 下半部分: 结果区域
    const resultSection = document.createElement('div');
    resultSection.className = 'mcp-result-section';
    
    const resultTitle = document.createElement('div');
    resultTitle.className = 'mcp-section-title';
    resultTitle.textContent = 'Result';
    resultSection.appendChild(resultTitle);
    
    const resultContent = document.createElement('div');
    resultContent.className = 'mcp-result-content';
    resultContent.id = mcpId + '-result';
    resultContent.textContent = '...';
    resultSection.appendChild(resultContent);
    
    mcpContent.appendChild(argsSection);
    mcpContent.appendChild(resultSection);
    
    mcpContainer.appendChild(mcpHeader);
    mcpContainer.appendChild(mcpContent);
    messageContentElem.appendChild(mcpContainer);
    
    if (shouldScroll) {
        scrollToBottom();
    }
}

/**
 * 设置 MCP 工具调用的结果
 * @param {string} messageId - 消息 ID
 * @param {string} result - 结果内容
 */
function setMcpResult(messageId, result) {
    // 查找该 messageId 下最后一个 MCP 容器
    const messageElem = document.getElementById(messageId);
    if (!messageElem) return;
    
    const mcpContainers = messageElem.querySelectorAll('.mcp-display-container[data-message-id="' + messageId + '"]');
    if (!mcpContainers || mcpContainers.length === 0) return;
    
    const lastContainer = mcpContainers[mcpContainers.length - 1];
    const resultContent = lastContainer.querySelector('.mcp-result-content');
    if (!resultContent) return;
    
    // 隐藏停止按钮
    const stopBtn = lastContainer.querySelector('.mcp-stop-button');
    if (stopBtn) {
        stopBtn.style.display = 'none';
    }
    
    // 尝试格式化 JSON
    try {
        const parsed = JSON.parse(result);
        resultContent.textContent = JSON.stringify(parsed, null, 2);
    } catch (e) {
        resultContent.textContent = result;
    }
}

/**
 * 处理 MCP 操作（如停止）
 * @param {string} mcpId - MCP 容器的 ID
 * @param {string} action - 操作类型（stop）
 */
function onMcpAction(mcpId, action) {
    if (action === 'stop') {
        // 发送停止消息到 C++
        window.chrome.webview.postMessage({
            action: 'mcpStatusChange',
            mcpId: mcpId,
            mcpStatus: 'stop'
        });
    }
}

/**
 * 切换 MCP 显示的展开/折叠状态
 * @param {string} mcpId - MCP 容器的 ID
 */
function toggleMcpContent(mcpId) {
    const content = document.getElementById(mcpId + '-content');
    const container = document.getElementById(mcpId);
    
    if (!content || !container) return;
    
    const icon = container.querySelector('.mcp-expand-icon');
    
    if (content.style.display === 'none' || content.style.display === '') {
        content.style.display = 'block';
        if (icon) icon.textContent = '-';
    } else {
        content.style.display = 'none';
        if (icon) icon.textContent = '+';
    }
}