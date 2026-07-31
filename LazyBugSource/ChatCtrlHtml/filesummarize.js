// ====== FileSummarize 功能模块 ======

/**
 * 更新所有 FileSummarize 按钮的 tooltip 状态
 */
function updateFileSummarizeTooltips() {
    const buttons = document.querySelectorAll('.file-summarize-button');
    buttons.forEach(button => {
        const filePath = button.textContent;
        if (button.scrollWidth > button.clientWidth) {
            button.title = filePath;
        } else {
            button.title = '';
        }
    });
}

/**
 * 根据容器当前模式渲染文件列表
 */
function renderFileSummarizeList(container) {
    const list = container.querySelector('.file-summarize-list');
    if (!list) return;

    const messageId = container._messageId;
    const mode = container.dataset.mode || 'recent';

    // 清空列表
    list.innerHTML = '';

    // 统一从 _sofarFiles 派生
    const all = container._sofarFiles || [];
    const files = (mode === 'sofar') ? all : all.filter(e => e.recent);

    files.forEach(entry => {
        const item = document.createElement('div');
        item.className = 'file-summarize-item';
        item.setAttribute('data-file-path', entry.path);

        const button = document.createElement('button');
        button.className = 'file-summarize-button';
        // So Far 模式下：非 recent 文件显示浅色，recent 文件保持正常风格
        if (mode === 'sofar' && !entry.recent) {
            button.classList.add('faded');
        }
        button.textContent = entry.path;

        button.onclick = (e) => {
            e.stopPropagation();
            const msg = {
                action: 'fileSummarizeClicked',
                messageId: messageId,
                filePath: entry.path
            };
            if (mode === 'sofar') {
                msg.sofar = true;
            }
            window.chrome.webview.postMessage(msg);
        };

        item.appendChild(button);
        list.appendChild(item);

        requestAnimationFrame(() => {
            if (button.scrollWidth > button.clientWidth) {
                button.title = entry.path;
            }
        });
    });

    // 更新标题栏
    const header = container.querySelector('.file-summarize-container-header');
    if (header) {
        if (mode === 'sofar') {
            header.textContent = 'Modified Files So Far';
        } else {
            header.textContent = 'Modified Files';
        }
    }
}

/**
 * 创建或获取 Modified Files 容器
 * @param {string} messageId - 所属消息 ID
 * @returns {HTMLElement} 容器元素
 */
function getOrCreateFileSummarizeContainer(messageId) {
    const containerId = 'filesummarize-container-' + messageId;
    let container = document.getElementById(containerId);
    
    if (!container) {
        const messageElem = document.getElementById(messageId);
        if (!messageElem) {
            console.error('Message element not found for FileSummarize:', messageId);
            return null;
        }

        const contentElem = messageElem.querySelector('.message-content');
        if (!contentElem) {
            console.error('Message content element not found for FileSummarize:', messageId);
            return null;
        }

        container = document.createElement('div');
        container.id = containerId;
        container.className = 'file-summarize-container';
        container._messageId = messageId;
        container._sofarFiles = [];
        container._hasToggleBtn = false;
        container.dataset.mode = 'recent';
        
        // 标题栏（可点击切换模式）
        const header = document.createElement('div');
        header.className = 'file-summarize-container-header';
        header.textContent = 'Modified Files';
        container.appendChild(header);
        
        // 切换按钮占位移除（不再需要单独的 toggle button）
        container._hasToggleBtn = false;
        
        // 文件列表
        const list = document.createElement('div');
        list.className = 'file-summarize-list';
        container.appendChild(list);
        
        contentElem.appendChild(container);
    }
    
    return container;
}

/**
 * 创建 FileSummarize 按钮（添加到容器中）
 *   - listType === "sofar"：files 数组[{path, recent}, ...]
 *   - 默认（旧数据）：单文件，当作 recent 项追加
 */
function createFileSummarizeWindow(messageId, filePath, listType, filesArray) {
    const chatContainer = document.getElementById('chat-container');
    
    const container = getOrCreateFileSummarizeContainer(messageId);
    if (!container) return;
    
    if (listType === 'sofar') {
        container._sofarFiles = filesArray || [];

        // 首次收到 sofar 数据时，启用标题栏点击切换模式
        if (!container._hasToggleBtn) {
            container._hasToggleBtn = true;
            const header = container.querySelector('.file-summarize-container-header');
            if (header) {
                header.style.cursor = 'pointer';
                header.onclick = (e) => {
                    e.stopPropagation();
                    const currentMode = container.dataset.mode || 'recent';
                    const nextMode = (currentMode === 'sofar') ? 'recent' : 'sofar';
                    container.dataset.mode = nextMode;
                    renderFileSummarizeList(container);
                };
            }
        }
    } else {
        // 旧数据回放：单文件作为 recent 条目
        if (!container._sofarFiles.some(e => e.path === filePath)) {
            container._sofarFiles.push({ path: filePath, recent: true });
        }
    }

    // 按当前模式渲染
    renderFileSummarizeList(container);

    // 滚动到底部
    chatContainer.scrollTop = chatContainer.scrollHeight;
}

// 监听窗口大小变化，更新所有按钮的 tooltip
window.addEventListener('resize', () => {
    requestAnimationFrame(() => {
        updateFileSummarizeTooltips();
    });
});

