// ====== 入口初始化模块 ======

let loadingOverlay;
let pauseOverlay;

/**
 * 初始化应用
 */
function initApp() {
    // 设置 Marked 选项
    marked.setOptions({
        gfm: true,
        breaks: false, /* MODIFIED: Set to false to avoid single newlines becoming <br> */
        smartLists: true,
        smartypants: false,
        highlight: function (code, lang) { return code; }
    });
    
    // 获取 DOM 元素
    loadingOverlay = document.getElementById('loading-overlay');
    pauseOverlay = document.getElementById('pause-overlay');
    
    // 初始化各模块
    initTitlebar();
    initMessageModule();
    initSymbolModule();
    
    // 初始化 Favorite 按钮
    if (typeof initFavoriteButton === 'function') {
        initFavoriteButton();
    }
    
    // 设置全局事件监听
    setupGlobalEvents();
    
    // 设置 WebView 消息监听
    setupWebViewMessageListener();
    
    // 通知 C++ JavaScript 已初始化
    if (window.chrome && window.chrome.webview) {
        window.chrome.webview.postMessage({ action: 'jsInitialized' });
    } else {
        console.error('window.chrome.webview not available for initialization message.');
    }
}

/**
 * 设置全局事件监听
 */
function setupGlobalEvents() {
    // 全局点击事件，用于隐藏菜单
    document.addEventListener('click', (e) => {
        // 如果点击的不是标题栏或菜单内容，则隐藏菜单
        if (!document.getElementById('webview-titlebar').contains(e.target)) {
            hideTitlebarMenu();
        }
        
        // 隐藏 FileEdit 选项菜单（除非点击在菜单或按钮上）
        let shouldHideFileEditMenus = true;
        
        // 检查点击是否在任何 FileEdit 选项按钮或菜单上
        if (e.target.classList.contains('file-edit-options-btn') || 
            e.target.closest('.file-edit-options-btn') ||
            e.target.classList.contains('file-edit-options-menu') ||
            e.target.closest('.file-edit-options-menu')) {
            shouldHideFileEditMenus = false;
        }
        
        if (shouldHideFileEditMenus) {
            hideAllFileEditOptionMenus();
        }
    });

    // 拦截所有链接点击，阻止默认导航行为
    document.addEventListener('click', (e) => {
        // 查找被点击的元素或其父元素是否为链接
        let target = e.target;
        while (target && target.tagName !== 'A') {
            target = target.parentElement;
        }
        
        // 如果点击的是链接
        if (target && target.tagName === 'A' && target.href) {
            e.preventDefault(); // 阻止默认导航行为
            e.stopPropagation(); // 阻止事件冒泡
            
            // 发送消息到 C++，请求用外部浏览器打开链接
            if (window.chrome && window.chrome.webview) {
                window.chrome.webview.postMessage({
                    action: 'openExternalUrl',
                    url: target.href
                });
            } else {
                console.error('WebView not available to open URL:', target.href);
            }
        }
    }, true); // 使用捕获阶段，确保在其他事件处理之前拦截
}

/**
 * 设置 WebView 消息监听
 */
function setupWebViewMessageListener() {
    window.chrome.webview.addEventListener('message', event => {
        try {
            const message = event.data;
            if (typeof message !== 'object' || message === null) {
                console.error('Received non-object message or C++ sent stringified JSON:', event.data);
                return;
            }

            switch (message.action) {
                // ====== 消息相关 ======
                case 'addUserMessage':
                    addUserOrSystemMessage(message.id, message.content, 'user', message.isFullContent);
                    break;
                case 'startAIMessage':
                    startAIMessage(message.id);
                    break;
                case 'addToAIMessage':
                    appendToAIMessage(message.id, message.content, message.isComplete);
                    break;
                case 'addToAIMessage_Thinking':
                    appendToAIMessage_Thinking(message.id, message.content, message.isComplete);
                    break;
                case 'addSystemMessage':
                    addUserOrSystemMessage(message.id, message.content, 'system');
                    break;
                case 'addToolCallMessageToAIMessage_Exploring':
                    addToolCallMessageToAIMessage_Exploring(message.id, message.content);
                    break;
                case 'clearChat': 
                    clearChat(); 
                    break;
                
                // ====== FileEdit 相关 ======
                case 'addFileEdit':
                    createFileEditWindow(message.fileEditId, message.messageId, message.title, message.content, message.buttons, message.isCollapsed, message.diffContent, message.fullPath);
                    break;
                case 'updateFileEditTitle':
                    updateFileEditWindow(message.fileEditId, { title: message.title });
                    break;
                case 'updateFileEditContent':
                    updateFileEditWindow(message.fileEditId, { 
                        content: message.content, 
                        diffContent: message.diffContent 
                    });
                    break;
                case 'updateFileEditHeight':
                    // height 已废弃，使用自适应高度；此 action 保留以兼容旧客户端
                    break;
                case 'updateFileEditButtons':
                    updateFileEditWindow(message.fileEditId, { buttons: message.buttons });
                    break;
                case 'toggleFileEditCollapse':
                    updateFileEditWindow(message.fileEditId, { isCollapsed: message.isCollapsed });
                    break;
                case 'startFileEditModification':
                    startFileEditModification(message.fileEditId);
                    break;
                case 'stopFileEditModification':
                    stopFileEditModification(message.fileEditId);
                    break;
                case 'showFileEditProgressLabel':
                    showFileEditProgressLabel(message.messageId, message.fileName, message.fullPath);
                    break;
                case 'hideFileEditProgressLabel':
                    hideFileEditProgressLabel(message.messageId);
                    break;
                
                // ====== FileSummarize 相关 =====
                case 'addFileSummarize':
                    createFileSummarizeWindow(message.messageId, message.filePath);
                    break;
                
                // ====== User Interject 相关 =====
                case 'addUserInterjectToAIMessage':
                    addUserInterjectToAIMessage(message.messageId, message.content);
                    break;
                
                // ====== 标题栏相关 ======
                case 'setWebViewTitle':
                    setWebViewTitle(message.title);
                    break;
                // ====== Disabled 消息相关 ======
                case 'disableMessagesAfter':
                    disableMessagesAfter(message.messageId);
                    break;
                case 'enableAllDisabledMessages':
                    enableAllDisabledMessages();
                    break;
                case 'removeDisabledMessages':
                    removeDisabledMessages();
                    break;
                
                // ====== 加载遮罩层相关 ======
                case 'showLoadingOverlay':
                    showLoadingOverlay();
                    break;
                case 'hideLoadingOverlay':
                    hideLoadingOverlay();
                    break;
                
                // ====== 费用显示相关 ======
                case 'setCostDisplay':
                    setCostDisplay(message.costText, message.messageId, message.cacheRateColor);
                    break;
                
                // ====== Symbol 链接相关 ======
                case 'collectSymbols':
                    collectSymbols();
                    break;
                case 'applySymbolLinks':
                    applySymbolLinks(message.messageId, message.symbols);
                    break;
                
                // ====== 暂停状态边框相关 ======
                case 'showPause':
                    showPauseOverlay(message.show, message.flow);
                    break;
                case 'stopPauseFlow':
                    stopPauseFlow(message.stop);
                    break;
                
// ====== CLI 显示相关 ======
                case 'addCliDisplay':
                    addCliDisplay(message.messageId, message.cliId, message.command, message.desc, message.status, message.shellType);
                    break;
                case 'appendCliOutput':
                    appendCliOutput(message.messageId, message.output);
                    break;
                case 'showCliInput':
                    // 显示/隐藏 CLI 输入框
                    showCliInputArea(message.cliId, message.show);
                    break;
                case 'completeCliDisplay':
                    // CLI 执行完成
                    completeCliDisplay(message.cliId, message.exitCode);
                    break;
                
                // ====== MCP 显示相关 ======
                case 'addMcpDisplay':
                    addMcpDisplay(message.messageId, message.mcpId, message.mcpName, message.toolName, message.arguments, message.argsSummary);
                    break;
                case 'setMcpResult':
                    setMcpResult(message.messageId, message.result);
                    break;
                
                // ====== Question 相关 ======
                case 'addQuestion':
                    addQuestion(message.messageId, message.questionId, message.question, message.options);
                    break;
                case 'addQuestionDisplay':
                    addQuestionDisplay(message.messageId, message.question, message.answer);
                    break;
                case 'clearQuestion':
                    clearQuestion();
                    break;

                // ====== Favorite 按钮相关 ======
                case 'setFavoriteStatus':
                    if (typeof setFavoriteStatus === 'function') {
                        setFavoriteStatus(message.isFavorite);
                    }
                    break;

                default:
                    console.warn('Unknown message action:', message.action); 
                    break;
            }
        } catch (e) {
            console.error('Error processing message from C++:', e, 'Raw data:', event.data);
        }
    });
}

// ====== 加载遮罩层相关函数 ======

/**
 * 显示加载遮罩层
 */
function showLoadingOverlay() {
    loadingOverlay.classList.add('show');
}

/**
 * 隐藏加载遮罩层
 */
function hideLoadingOverlay() {
    loadingOverlay.classList.remove('show');
}

// ====== 暂停状态边框相关函数 ======

/**
 * 显示/隐藏暂停状态边框
 * @param {boolean} show - 是否显示
 * @param {boolean} flow - 是否流动 (默认true)
 */
function showPauseOverlay(show, flow = true) {
    if (show) {
        pauseOverlay.classList.add('show');
        // 根据 flow 参数决定是否添加 stopped 类
        if (flow) {
            pauseOverlay.classList.remove('stopped');
        } else {
            pauseOverlay.classList.add('stopped');
        }
    } else {
        pauseOverlay.classList.remove('show');
        pauseOverlay.classList.remove('stopped');
    }
}

/**
 * 停止/恢复暂停边框的流动动画
 * @param {boolean} stop - true: 停止流动, false: 恢复流动
 */
function stopPauseFlow(stop) {
    if (stop) {
        pauseOverlay.classList.add('stopped');
    } else {
        pauseOverlay.classList.remove('stopped');
    }
}

// ====== DOM 加载完成后初始化 ======
document.addEventListener('DOMContentLoaded', initApp);