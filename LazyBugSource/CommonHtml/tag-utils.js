/**
 * 公共标签工具函数
 * 用于 ChatInputHtml 和 ChatCtrlHtml
 */

/**
 * 创建内联标签元素（简化版本，用于消息显示）
 * @param {Object} tagData - 标签数据
 * @param {string} tagData.id - 标签ID
 * @param {string} tagData.text - 标签文本
 * @param {string} tagData.tagType - 标签类型
 * @param {string} tagData.data - 标签数据
 * @param {boolean} tagData.removable - 是否可移除
 * @returns {HTMLElement} 标签元素
 */
function createInlineTagElement(tagData) {
    const tagType = tagData.tagType || 'info';
    const tagElement = document.createElement('span');
    tagElement.className = 'inline-tag';
    if (tagType) {
        tagElement.classList.add(tagType);
    }
    tagElement.setAttribute('data-tag-id', tagData.id || '');
    tagElement.setAttribute('data-tag-type', tagType);
    tagElement.setAttribute('data-tag-data', tagData.data || '');
    tagElement.setAttribute('data-tag-removable', tagData.removable !== false ? 'true' : 'false');

    if (tagType === 'image') {
        // image 类型：直接渲染缩略图
        // imgSrc 是 Base64 DataURL（由 C++ 生成），data 保留原始文件路径供 hover 预览
        const imgSrc = tagData.imgSrc || tagData.data || '';
        if (imgSrc) {
            const img = document.createElement('img');
            img.src = imgSrc;
            img.className = 'inline-tag-image';
            img.alt = tagData.text || '';
            img.draggable = false;
            tagElement.appendChild(img);
        } else {
            tagElement.textContent = tagData.text || '';
        }
    } else {
        tagElement.textContent = tagData.text || '';
    }
    
    return tagElement;
}

/**
 * 渲染带有标签的内容数组
 * @param {HTMLElement} container - 容器元素
 * @param {Array} contentArray - 内容数组 [{type: 'text', content: '...'}, {type: 'tag', ...}]
 */
function renderContentWithTags(container, contentArray) {
    for (let i = 0; i < contentArray.length; i++) {
        const item = contentArray[i];
        
        if (item.type === 'text') {
            renderTextWithTags(container, item.content);
        } else if (item.type === 'tag') {
            const tagElement = createInlineTagElement(item);
            container.appendChild(tagElement);
        }
    }
}

/**
 * 渲染文本内容（支持换行，用于标签系统）
 * @param {HTMLElement} container - 容器元素
 * @param {string} text - 文本内容
 */
function renderTextWithTags(container, text) {
    if (text.includes('\n')) {
        const lines = text.split('\n');
        for (let j = 0; j < lines.length; j++) {
            if (j > 0) {
                const brElement = document.createElement('br');
                container.appendChild(brElement);
            }
            if (lines[j].length > 0) {
                const textNode = document.createTextNode(lines[j]);
                container.appendChild(textNode);
            }
        }
    } else {
        const textNode = document.createTextNode(text);
        container.appendChild(textNode);
    }
}

// 导出到全局
window.createInlineTagElement = createInlineTagElement;
window.renderContentWithTags = renderContentWithTags;
window.renderTextWithTags = renderTextWithTags;

