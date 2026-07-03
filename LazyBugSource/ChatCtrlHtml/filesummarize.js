// ====== FileSummarize 功能模块 ======

/**
 * 更新所有 FileSummarize 按钮的 tooltip 状态
 */
function updateFileSummarizeTooltips() {
    const buttons = document.querySelectorAll('.file-summarize-button');
    buttons.forEach(button => {
        const filePath = button.textContent;
        // 检查文本是否溢出，只有溢出时才设置 tooltip
        if (button.scrollWidth > button.clientWidth) {
            button.title = filePath;
        } else {
            button.title = '';
        }
    });
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
        // 创建容器
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

        // 创建容器
        container = document.createElement('div');
        container.id = containerId;
        container.className = 'file-summarize-container';
        
        // 创建标题栏
        const header = document.createElement('div');
        header.className = 'file-summarize-container-header';
        header.textContent = 'Modified Files';
        container.appendChild(header);
        
        // 创建文件列表
        const list = document.createElement('div');
        list.className = 'file-summarize-list';
        container.appendChild(list);
        
        // 插入到消息内容区域的末尾
        contentElem.appendChild(container);
    }
    
    return container;
}

/**
 * 创建 FileSummarize 按钮（添加到容器中）
 * @param {string} messageId - 所属消息 ID
 * @param {string} filePath - 文件完整路径（显示文本）
 */
function createFileSummarizeWindow(messageId, filePath) {
    const chatContainer = document.getElementById('chat-container');
    
    // 获取或创建容器
    const container = getOrCreateFileSummarizeContainer(messageId);
    if (!container) {
        return;
    }
    
    const list = container.querySelector('.file-summarize-list');
    if (!list) {
        console.error('File list not found in container');
        return;
    }
    
    // 检查是否已存在相同的文件（避免重复添加）
    const existingItem = list.querySelector(`[data-file-path="${filePath}"]`);
    if (existingItem) {
        console.log('File already exists in container:', filePath);
        return;
    }
    
    // 创建文件项
    const item = document.createElement('div');
    item.className = 'file-summarize-item';
    item.setAttribute('data-file-path', filePath);
    
    // 创建按钮
    const button = document.createElement('button');
    button.className = 'file-summarize-button';
    button.textContent = filePath; // 显示完整路径
    
    // 点击事件处理
    button.onclick = (e) => {
        e.stopPropagation();
        console.log('FileSummarize clicked:', messageId, filePath);
        window.chrome.webview.postMessage({
            action: 'fileSummarizeClicked',
            messageId: messageId,
            filePath: filePath
        });
    };
    
    item.appendChild(button);
    list.appendChild(item);
    
    // 初始检查 tooltip
    requestAnimationFrame(() => {
        if (button.scrollWidth > button.clientWidth) {
            button.title = filePath;
        }
    });
    
    // 滚动到底部
    chatContainer.scrollTop = chatContainer.scrollHeight;
}

// 监听窗口大小变化，更新所有按钮的 tooltip
window.addEventListener('resize', () => {
    requestAnimationFrame(() => {
        updateFileSummarizeTooltips();
    });
});

