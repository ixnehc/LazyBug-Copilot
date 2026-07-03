// ====== Symbol 链接模块 ======

/**
 * 收集页面中所有消息的 symbols 并发送给 C++ 查询
 * 此函数供 C++ 端调用
 */
function collectSymbols() {
    const chatContainer = document.getElementById('chat-container');
    if (!chatContainer) {
        console.warn('collectSymbols: chat-container not found');
        return;
    }

    // 遍历所有消息元素
    const messages = chatContainer.querySelectorAll('.message');
    messages.forEach(message => {
        if (message.id) {
            collectSymbolsForMessage(message.id);
        }
    });
}

/**
 * 收集指定消息中所有行内 <code> 元素的文本内容（排除多行代码块）
 * 并发送给 C++ 端进行查询
 * @param {string} messageId - 消息 ID，用于标识是哪条消息
 */
function collectSymbolsForMessage(messageId) {
    const messageElem = document.getElementById(messageId);
    if (!messageElem) {
        console.warn('collectSymbolsForMessage: Message element not found:', messageId);
        return;
    }

    // 收集所有行内 <code> 元素（不在 <pre> 内的）
    const inlineCodes = messageElem.querySelectorAll('code');
    const symbols = [];

    inlineCodes.forEach(code => {
        // 排除多行代码块内的 <code>
        if (code.closest('pre')) {
            return;
        }
        // 排除已经有 symbol-link 的（避免重复收集）
        if (code.classList.contains('symbol-link')) {
            return;
        }
        const text = code.textContent.trim();
        if (text.length > 0) {
            symbols.push(text);
        }
    });

    // 去重
    const uniqueSymbols = [...new Set(symbols)];

    if (uniqueSymbols.length > 0) {
        // 发送给 C++ 端
        window.chrome.webview.postMessage({
            action: 'querySymbolLocations',
            messageId: messageId,
            symbols: uniqueSymbols
        });
    }
}

/**
 * 应用 Symbol 链接样式
 * 将指定的 symbols 列表中的 <code> 元素添加 symbol-link 类，并存储解析结果
 * @param {string} messageId - 消息 ID
 * @param {Object[]} symbolsWithResults - 包含 symbol 和 results 的对象数组
 * @param {string} symbolsWithResults[].symbol - 符号名称
 * @param {Object[]} symbolsWithResults[].results - 解析结果数组
 * @param {string} symbolsWithResults[].results[].filePath - 文件路径
 * @param {number} symbolsWithResults[].results[].lineNumber - 行号（-1表示无行号）
 */
function applySymbolLinks(messageId, symbolsWithResults) {
    const messageElem = document.getElementById(messageId);
    if (!messageElem) {
        console.warn('applySymbolLinks: Message element not found:', messageId);
        return;
    }

    if (!Array.isArray(symbolsWithResults) || symbolsWithResults.length === 0) {
        return;
    }

    // 构建 symbol -> results 的映射
    const symbolMap = new Map();
    symbolsWithResults.forEach(item => {
        if (item.symbol && Array.isArray(item.results)) {
            symbolMap.set(item.symbol, item.results);
        }
    });

    // 遍历所有行内 <code> 元素
    const inlineCodes = messageElem.querySelectorAll('code');
    inlineCodes.forEach(code => {
        // 排除多行代码块内的 <code>
        if (code.closest('pre')) {
            return;
        }
        // 排除已经是 symbol-link 的
        if (code.classList.contains('symbol-link')) {
            return;
        }

        const text = code.textContent.trim();
        if (symbolMap.has(text)) {
            code.classList.add('symbol-link');
            // 将解析结果存储在 data 属性中
            const results = symbolMap.get(text);
            code.dataset.symbolResults = JSON.stringify(results);
        }
    });
}

/**
 * 初始化 Symbol 模块的事件监听
 * 使用事件委托处理所有 symbol-link 的点击
 */
function initSymbolModule() {
    const chatContainer = document.getElementById('chat-container');
    if (!chatContainer) {
        console.error('initSymbolModule: chat-container not found');
        return;
    }

    // 使用事件委托监听点击
    chatContainer.addEventListener('click', handleSymbolLinkClick);
}

/**
 * 处理 symbol-link 的点击事件
 * @param {Event} event - 点击事件
 */
function handleSymbolLinkClick(event) {
    // 查找被点击的元素或其父元素是否为 symbol-link
    const code = event.target.closest('code.symbol-link');
    if (!code) {
        return;
    }

    // 阻止事件冒泡
    event.stopPropagation();

    // 获取 symbol 文本
    const symbolText = code.textContent.trim();

    // 获取存储的解析结果
    let results = null;
    if (code.dataset.symbolResults) {
        try {
            results = JSON.parse(code.dataset.symbolResults);
        } catch (e) {
            console.warn('Failed to parse symbol results:', e);
        }
    }

    // 发送给 C++ 端
    window.chrome.webview.postMessage({
        action: 'symbolLinkClicked',
        symbol: symbolText,
        results: results
    });
}
