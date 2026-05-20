// ====== 滚动控制工具函数 ======

/**
 * 检查聊天容器是否在底部附近
 * @param {number} threshold - 阈值（像素），默认50
 * @returns {boolean} 是否在底部附近
 */
function isNearBottom(threshold = 50) {
    const contentArea = document.getElementById('chat-container');
    if (contentArea.scrollHeight <= contentArea.clientHeight) {
        return true; /* No scrollbar, always considered at bottom */
    }
    return (contentArea.clientHeight + contentArea.scrollTop) >= (contentArea.scrollHeight - threshold);
}

/**
 * 滚动聊天容器到底部
 */
function scrollToBottom() {
    setTimeout(() => {
        const contentArea = document.getElementById('chat-container');
        contentArea.scrollTo(0, contentArea.scrollHeight);
    }, 0);
}

// ====== 代码块处理工具函数 ======

/**
 * 为代码块添加复制按钮容器和语法高亮
 * @param {HTMLElement} container - 包含代码块的容器元素
 */
function addCopyButtonsToCodeBlocks(container) {
    const preElements = container.querySelectorAll('pre');
    preElements.forEach(pre => {
        // 跳过 FileEdit 中的代码块（已经有其他交互）
        if (pre.closest('.file-edit-content')) {
            return;
        }
        
        // 如果已经包装过，跳过
        if (pre.parentElement && pre.parentElement.classList.contains('code-block-container')) {
            return;
        }
        
        // 创建容器
        const wrapper = document.createElement('div');
        wrapper.className = 'code-block-container';
        
        // 创建复制按钮
        const copyBtn = document.createElement('button');
        copyBtn.className = 'code-copy-button';
        copyBtn.textContent = 'Copy';
        copyBtn.onclick = (e) => {
            e.stopPropagation();
            copyCodeBlock(copyBtn, pre);
        };
        
        // 为代码块添加语法高亮
        addSyntaxHighlightToCodeBlock(pre);
        
        // 将 pre 包装在容器中
        pre.parentNode.insertBefore(wrapper, pre);
        wrapper.appendChild(copyBtn);
        wrapper.appendChild(pre);
    });
}

/**
 * 为单个代码块添加语法高亮
 * @param {HTMLElement} preElement - pre 元素
 */
function addSyntaxHighlightToCodeBlock(preElement) {
    // 如果已经高亮过，跳过
    if (preElement.querySelector('code.hljs')) {
        return;
    }
    
    // 查找或创建 code 元素
    let codeElement = preElement.querySelector('code');
    let codeText = '';
    let language = '';
    
    if (codeElement) {
        // 从 code 元素获取代码文本和语言
        codeText = codeElement.textContent || '';
        // 尝试从 class 属性提取语言（如 "language-cpp" 或 "lang-cpp"）
        const classMatch = codeElement.className.match(/(?:language-|lang-)(\w+)/);
        if (classMatch) {
            language = classMatch[1];
        }
    } else {
        // 没有 code 元素，获取 pre 的文本内容
        codeText = preElement.textContent || '';
        // 尝试从 pre 的 class 属性提取语言
        const classMatch = preElement.className.match(/(?:language-|lang-)(\w+)/);
        if (classMatch) {
            language = classMatch[1];
        }
    }
    
    // 如果没有 code 元素，创建一个
    if (!codeElement) {
        codeElement = document.createElement('code');
        codeElement.textContent = codeText;
        preElement.innerHTML = '';
        preElement.appendChild(codeElement);
    }
    
    // 使用 highlight.js 进行语法高亮
    if (typeof hljs !== 'undefined' && codeText.trim()) {
        try {
            let result;
            if (language && hljs.getLanguage(language)) {
                // 使用指定的语言进行高亮
                result = hljs.highlight(codeText, { language: language, ignoreIllegals: true });
            } else {
                // 自动检测语言
                result = hljs.highlightAuto(codeText);
            }
            codeElement.innerHTML = result.value;
            codeElement.classList.add('hljs');
            if (result.language) {
                codeElement.classList.add(`language-${result.language}`);
            }
        } catch (e) {
            console.warn('Syntax highlighting failed:', e);
            // 高亮失败时保留原始文本
            codeElement.textContent = codeText;
        }
    }
}

/**
 * 复制代码块内容到剪贴板
 * @param {HTMLElement} button - 复制按钮元素
 * @param {HTMLElement} preElement - pre 元素
 */
async function copyCodeBlock(button, preElement) {
    // 获取代码文本（去除高亮标记，只保留纯文本）
    let codeText = '';
    
    // 尝试获取 code 元素
    const codeElement = preElement.querySelector('code');
    if (codeElement) {
        codeText = codeElement.textContent || codeElement.innerText || '';
    } else {
        codeText = preElement.textContent || preElement.innerText || '';
    }
    
    try {
        await navigator.clipboard.writeText(codeText);
        // 显示复制成功反馈
        button.textContent = 'Copied!';
        button.classList.add('copied');
        
        setTimeout(() => {
            button.textContent = 'Copy';
            button.classList.remove('copied');
        }, 2000);
    } catch (err) {
        console.error('Failed to copy code:', err);
        button.textContent = 'Failed';
        setTimeout(() => {
            button.textContent = 'Copy';
        }, 2000);
    }
}

// ====== FileEdit 内容处理工具函数 ======

/**
 * 处理 FileEdit 内容中的行高亮标记，同时保持语法高亮
 * @param {string} content - 原始内容
 * @returns {string} 处理后的 HTML
 */
function processFileEditContentWithHighlights(content) {
    if (!content) return '';
    
    // 按行分割内容
    const lines = content.split('\n');
    const processedLines = [];
    
    // 先收集所有行的信息（包括高亮类型和原始内容）
    const lineInfos = [];
    for (let i = 0; i < lines.length; i++) {
        let line = lines[i];
        let highlightClass = '';
        
        // 检查行开头的特殊标记
        if (line.startsWith('[+]')) {
            highlightClass = 'line-highlight-added';
            line = line.substring(3); // 移除标记
        } else if (line.startsWith('[-]')) {
            highlightClass = 'line-highlight-removed';
            line = line.substring(3); // 移除标记
        } else if (line.startsWith('[*]')) {
            highlightClass = 'line-highlight-modified';
            line = line.substring(3); // 移除标记
        } else if (line.startsWith('[i]')) {
            highlightClass = 'line-highlight-info';
            line = line.substring(3); // 移除标记
        } else if (line.startsWith('[!]')) {
            highlightClass = 'line-highlight-warning';
            line = line.substring(3); // 移除标记
        }
        
        lineInfos.push({
            originalLine: line,
            highlightClass: highlightClass
        });
    }
    
    // 重新组合没有标记的内容用于语法高亮
    const cleanContent = lineInfos.map(info => info.originalLine).join('\n');
    
    // 应用语法高亮
    let highlightedContent = cleanContent;
    if (typeof hljs !== 'undefined') {
        try {
            highlightedContent = hljs.highlight(cleanContent, {language: 'cpp'}).value;
        } catch (e) {
            console.warn('语法高亮失败:', e);
            highlightedContent = escapeHtml(cleanContent);
        }
    } else {
        highlightedContent = escapeHtml(cleanContent);
    }
    
    // 将语法高亮后的内容按行分割
    const highlightedLines = highlightedContent.split('\n');
    
    // 为需要行高亮的行添加 span 包装
    for (let i = 0; i < lineInfos.length && i < highlightedLines.length; i++) {
        const lineInfo = lineInfos[i];
        if (lineInfo.highlightClass) {
            processedLines.push(`<span class="${lineInfo.highlightClass}">${highlightedLines[i]}</span>`);
        } else {
            processedLines.push(highlightedLines[i]);
        }
    }
    
    return processedLines.join('\n');
}

/**
 * HTML 转义函数
 * @param {string} text - 原始文本
 * @returns {string} 转义后的 HTML
 */
function escapeHtml(text) {
    const div = document.createElement('div');
    div.textContent = text;
    return div.innerHTML;
}

/**
 * 计算内容实际高度的辅助函数
 * @param {string} content - 内容文本
 * @returns {number} 计算出的高度（像素）
 */
function calculateContentHeight(content) {
    // 创建临时元素来测量内容高度
    const tempDiv = document.createElement('div');
    tempDiv.style.position = 'absolute';
    tempDiv.style.visibility = 'hidden';
    tempDiv.style.width = '600px'; // 使用固定宽度而不是 100%，避免宽度计算问题
    tempDiv.style.fontFamily = 'Consolas, "Courier New", monospace';
    tempDiv.style.fontSize = '0.85em';
    tempDiv.style.lineHeight = '1.6';
    tempDiv.style.whiteSpace = 'pre';
    tempDiv.style.padding = '12px';
    tempDiv.style.boxSizing = 'border-box'; // 确保 padding 包含在尺寸计算中
    
    // 处理内容
    const processedContent = processFileEditContentWithHighlights(content || '');
    tempDiv.innerHTML = processedContent;
    
    document.body.appendChild(tempDiv);
    const height = tempDiv.offsetHeight;
    document.body.removeChild(tempDiv);
    
    // 添加 5px 的缓冲区域，避免边界情况导致高度计算误差
    return height + 5;
}