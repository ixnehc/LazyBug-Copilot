// 标签管理（顶部标签、内联标签）

// 更新标签显示
function updateTagsDisplay() {
    const tagsBar = document.getElementById('tagsBar');
    const tagsContainer = document.getElementById('tagsContainer');
    const atButton = document.getElementById('atButton');
    
    const visibleTags = AppState.currentTags.filter(tag => tag.visible !== false);
    
    if (atButton) {
        atButton.disabled = AppState.currentTags.length === 0;
    }
    
    if (visibleTags.length === 0) {
        tagsBar.classList.add('empty');
        tagsContainer.innerHTML = '';
        return;
    }
    
    tagsBar.classList.remove('empty');
    tagsContainer.innerHTML = '';
    
    visibleTags.forEach(tag => {
        const tagElement = createTagElement(tag);
        tagsContainer.appendChild(tagElement);
    });
}

// 创建标签元素
function createTagElement(tag) {
    const tagDiv = document.createElement('div');
    tagDiv.className = `tag ${tag.type || 'info'}`;
    // image 类型的标签不显示普通文字 tooltip，改用原生图片预览窗口
    if (tag.type !== 'image') {
        tagDiv.title = tag.data || tag.text;
    }
    tagDiv.style.cursor = 'pointer';
    tagDiv.addEventListener('click', (e) => {
        e.stopPropagation();
        sendMessageToNative({
            action: 'tagClicked',
            tagId: tag.id
        });
    });
    // image 类型：标记 data 属性以便后续查询
    if (tag.type === 'image') {
        tagDiv.setAttribute('data-image-tag-id', tag.id);
        
        const imgSrc = tag.imgSrc || tag.data || '';
        if (imgSrc) {
            const img = document.createElement('img');
            img.src = imgSrc;
            img.className = 'tag-image';
            img.alt = tag.text || '';
            img.draggable = false;
            tagDiv.appendChild(img);
        } else {
            const textSpan = document.createElement('span');
            textSpan.className = 'tag-text';
            textSpan.textContent = tag.text;
            tagDiv.appendChild(textSpan);
        }
    } else {
        const textSpan = document.createElement('span');
        textSpan.className = 'tag-text';
        textSpan.textContent = tag.text;
        tagDiv.appendChild(textSpan);
    }
    
    if (tag.removable !== false) {
        const removeBtn = document.createElement('button');
        removeBtn.className = 'tag-remove';
        removeBtn.innerHTML = '×';
        removeBtn.title = '移除标签';
        removeBtn.addEventListener('click', (e) => {
            e.stopPropagation();
            removeTag(tag.id);
        });
        tagDiv.appendChild(removeBtn);
    }
    
    return tagDiv;
}

// 移除标签
function removeTag(tagId) {
    sendMessageToNative({
        action: 'hideTag',
        tagId: tagId
    });
}

// 切换标签可见性
function toggleTagVisibility(tagId) {
    const tag = AppState.currentTags.find(t => t.id === tagId);
    if (tag) {
        tag.visible = !tag.visible;
        updateTagsDisplay();
    }
    
    sendMessageToNative({
        action: 'toggleTagVisibility',
        tagId: tagId
    });
}

// 更新标签数据
function updateTags(tags) {
    AppState.currentTags = tags;
    updateTagsDisplay();
}

// 创建内联标签元素
function createInlineTagElement(tag) {
    const tagType = tag.type || tag.tagType || 'info';
    const tagSpan = document.createElement('span');
    tagSpan.className = `inline-tag ${tagType}`;
    tagSpan.setAttribute('data-tag-id', tag.id || '');
    tagSpan.setAttribute('data-tag-data', tag.data || '');
    tagSpan.setAttribute('data-tag-type', tagType);
    tagSpan.setAttribute('data-tag-removable', (tag.removable !== false).toString());
    // 始终存储 text，方便 getInputContent 读取（避免 img 元素 textContent 为空的问题）
    tagSpan.setAttribute('data-tag-text', tag.text || '');
    tagSpan.contentEditable = false;

    if (tagType === 'image') {
        // image 类型：直接显示缩略图，不显示文字 tooltip（由原生预览窗口负责）
        // imgSrc 是 C++ 端传来的 Base64 DataURL，data 保留原始文件路径供 hover 预览
        const imgSrc = tag.imgSrc || tag.data || '';
        if (imgSrc) {
            // 将 imgSrc 存入 DOM 属性，getInputContent 序列化时可恢复
            tagSpan.setAttribute('data-tag-imgsrc', imgSrc);
            const img = document.createElement('img');
            img.src = imgSrc;
            img.className = 'inline-tag-image';
            img.alt = tag.text || '';
            img.draggable = false;
            tagSpan.appendChild(img);
        } else {
            // 无图片数据时退化为文字显示
            const textSpan = document.createElement('span');
            textSpan.className = 'inline-tag-text';
            textSpan.textContent = tag.text || '';
            tagSpan.appendChild(textSpan);
        }
    } else {
        // 普通类型：显示文字
        tagSpan.title = tag.data || tag.text;
        const textSpan = document.createElement('span');
        textSpan.className = 'inline-tag-text';
        textSpan.textContent = tag.text || '';
        tagSpan.appendChild(textSpan);
    }
    
    return tagSpan;
}

// 移除内联标签
function removeInlineTag(tagElement) {
    if (tagElement && tagElement.parentNode) {
        tagElement.parentNode.removeChild(tagElement);
        notifyContentChanged();
    }
}

// 确保标签完整性（ZWSP包装）
function ensureTagIntegrity() {
    // paste 期间跳过：execCommand('insertText') 触发的 input 事件会走到这里，
    // 而 ensureTagIntegrity 对 DOM 的直接修改不进 undo 栈，会导致 Ctrl+Z 后内容残留。
    if (AppState.isPasting) return;

    const inputEditor = document.getElementById('inputEditor');
    const zwsp = AppState.zwsp;
    let changed = false;
    
    const selection = window.getSelection();
    let cursorInfo = null;
    if (selection.rangeCount > 0) {
        const range = selection.getRangeAt(0);
        cursorInfo = {
            node: range.startContainer,
            offset: range.startOffset,
            isCollapsed: range.collapsed
        };
    }
    
    const walker = document.createTreeWalker(
        inputEditor,
        NodeFilter.SHOW_ELEMENT,
        {
            acceptNode: function(node) {
                return node.classList && node.classList.contains('inline-tag') 
                    ? NodeFilter.FILTER_ACCEPT 
                    : NodeFilter.FILTER_REJECT;
            }
        }
    );
    
    const tags = [];
    let currentNode;
    while (currentNode = walker.nextNode()) {
        tags.push(currentNode);
    }
    
    tags.forEach(tag => {
        let needsRepair = false;
        
        const prevSibling = tag.previousSibling;
        if (!prevSibling || prevSibling.nodeType !== Node.TEXT_NODE) {
            const beforeZwsp = document.createTextNode(zwsp);
            tag.parentNode.insertBefore(beforeZwsp, tag);
            needsRepair = true;
        } else if (!prevSibling.textContent.endsWith(zwsp)) {
            // 不再修改当前 textNode 的 textContent，以防破坏原生 Undo 栈
            // 只要缺失 ZWSP，我们就直接在 tag 前面补充一个全新的 ZWSP 文本节点
            const beforeZwsp = document.createTextNode(zwsp);
            tag.parentNode.insertBefore(beforeZwsp, tag);
            needsRepair = true;
        }
        
        const nextSibling = tag.nextSibling;
        if (!nextSibling || nextSibling.nodeType !== Node.TEXT_NODE) {
            const afterZwsp = document.createTextNode(zwsp);
            if (nextSibling) {
                tag.parentNode.insertBefore(afterZwsp, nextSibling);
            } else {
                tag.parentNode.appendChild(afterZwsp);
            }
            needsRepair = true;
        } else if (!nextSibling.textContent.startsWith(zwsp)) {
            // 同理，只要存在起始 ZWSP 即可，无需将它与其他输入字符拆分为两个 Node
            const afterZwsp = document.createTextNode(zwsp);
            tag.parentNode.insertBefore(afterZwsp, nextSibling);
            needsRepair = true;
        }
        
        if (needsRepair) {
            changed = true;
        }
    });
    
    if (changed && cursorInfo) {
        try {
            const range = document.createRange();
            if (cursorInfo.node && cursorInfo.node.parentNode) {
                range.setStart(cursorInfo.node, Math.min(cursorInfo.offset, cursorInfo.node.textContent ? cursorInfo.node.textContent.length : 0));
                range.collapse(true);
                selection.removeAllRanges();
                selection.addRange(range);
            } else {
                range.selectNodeContents(inputEditor);
                range.collapse(false);
                selection.removeAllRanges();
                selection.addRange(range);
            }
        } catch (e) {
            const range = document.createRange();
            range.selectNodeContents(inputEditor);
            range.collapse(false);
            selection.removeAllRanges();
            selection.addRange(range);
        }
    }
}

// 选中标签组合
function selectTagCombination(beforeZwsp, tagElement, afterZwsp) {
    const selection = window.getSelection();
    const range = document.createRange();
    
    let startNode, startOffset;
    let endNode, endOffset;
    
    if (beforeZwsp && beforeZwsp.nodeType === Node.TEXT_NODE && beforeZwsp.textContent.endsWith(AppState.zwsp)) {
        startNode = beforeZwsp;
        startOffset = beforeZwsp.textContent.length - 1;
    } else if (tagElement && tagElement.parentNode) {
        startNode = tagElement.parentNode;
        startOffset = Array.prototype.indexOf.call(tagElement.parentNode.childNodes, tagElement);
    } else {
        startNode = tagElement;
        startOffset = 0;
    }
    
    if (afterZwsp && afterZwsp.nodeType === Node.TEXT_NODE && afterZwsp.textContent.startsWith(AppState.zwsp)) {
        endNode = afterZwsp;
        endOffset = 1;
    } else if (tagElement && tagElement.parentNode) {
        endNode = tagElement.parentNode;
        endOffset = Array.prototype.indexOf.call(tagElement.parentNode.childNodes, tagElement) + 1;
    } else {
        endNode = tagElement;
        endOffset = 1;
    }
    
    try {
        range.setStart(startNode, startOffset);
        range.setEnd(endNode, endOffset);
        selection.removeAllRanges();
        selection.addRange(range);
    } catch (e) {
        console.warn('Failed to select tag combination, falling back to direct removal');
        if (beforeZwsp && beforeZwsp.nodeType === Node.TEXT_NODE && beforeZwsp.textContent.endsWith(AppState.zwsp)) {
            beforeZwsp.textContent = beforeZwsp.textContent.slice(0, -1);
        }
        tagElement.remove();
        if (afterZwsp && afterZwsp.nodeType === Node.TEXT_NODE && afterZwsp.textContent.startsWith(AppState.zwsp)) {
            afterZwsp.textContent = afterZwsp.textContent.slice(1);
        }
    }
}

// Image Tag 悬停检测相关
let _isImageTagHoverDetectionRunning = false;
let _lastHoveredImageFilePath = null;

// 启动 image tag 悬停/选中检测
function startImageTagHoverDetection() {
    if (_isImageTagHoverDetectionRunning) {
        return; // 已经在运行
    }
    _isImageTagHoverDetectionRunning = true;
    document.addEventListener('selectionchange', checkImageTagHover);
    document.addEventListener('mousemove', checkImageTagHover);
    document.addEventListener('mouseleave', checkImageTagHover);
}

// 停止 image tag 悬停/选中检测
function stopImageTagHoverDetection() {
    if (!_isImageTagHoverDetectionRunning) {
        return;
    }
    _isImageTagHoverDetectionRunning = false;
    document.removeEventListener('selectionchange', checkImageTagHover);
    document.removeEventListener('mousemove', checkImageTagHover);
    document.removeEventListener('mouseleave', checkImageTagHover);
}

// 检查鼠标是否在 image tag 上（包括顶部 image tags 和 inline image tags）
function checkImageTagHover() {
    // 查找所有 image tag 元素（顶部标签 + inline 标签）
    const imageTags = document.querySelectorAll('[data-image-tag-id]');
    const inlineImageTags = document.querySelectorAll('.inline-tag[data-tag-type="image"]');
    
    // 合并所有可能的 image tag 元素
    const allImageTagElements = [...imageTags, ...inlineImageTags];
    
    if (allImageTagElements.length === 0) {
        // 没有 image tag，如果之前有悬停状态则发送 leave
        if (_lastHoveredImageFilePath !== null) {
            _lastHoveredImageFilePath = null;
            sendMessageToNative({
                action: 'imageTagHoverResult',
                filePath: null
            });
        }
        return;
    }

    let activeTag = null;
    let filePath = null;
    
    // 首先检查选中的内容中是否包含 image tag
    const selection = window.getSelection();
    let selectedImageTags = [];
    if (selection && selection.rangeCount > 0 && !selection.isCollapsed) {
        for (const tagElement of allImageTagElements) {
            if (selection.containsNode(tagElement, true)) {
                selectedImageTags.push(tagElement);
            }
        }
    }
    
    if (selectedImageTags.length === 1) {
        // 如果只选中了一个 image tag，优先显示
        activeTag = selectedImageTags[0];
    } else {
        // 如果没有选中或选中多个，降级到 hover 逻辑
        for (const tagElement of allImageTagElements) {
            if (tagElement.matches(':hover')) {
                activeTag = tagElement;
                break;
            }
        }
    }

    if (activeTag) {
        // 获取文件路径
        if (activeTag.hasAttribute('data-image-tag-id')) {
            // 顶部 image tag：通过 tagId 从 AppState.currentTags 查找
            const tagId = activeTag.getAttribute('data-image-tag-id');
            const tag = AppState.currentTags.find(t => t.id === tagId);
            filePath = tag ? (tag.data || '') : '';
        } else {
            // inline image tag：直接从 data-tag-data 获取
            filePath = activeTag.getAttribute('data-tag-data') || '';
        }
    }

    if (activeTag && filePath) {
        // 只有当悬停或选中的 tag 变化时才发送消息
        if (_lastHoveredImageFilePath !== filePath) {
            _lastHoveredImageFilePath = filePath;
            const rect = activeTag.getBoundingClientRect();
            sendMessageToNative({
                action: 'imageTagHoverResult',
                filePath: filePath,
                position: {
                    x: Math.round(rect.left + rect.width / 2),
                    y: Math.round(rect.bottom)
                }
            });
        }
    } else {
        // 鼠标不在任何 image tag 上，也没有唯一选中的 image tag
        if (_lastHoveredImageFilePath !== null) {
            _lastHoveredImageFilePath = null;
            sendMessageToNative({
                action: 'imageTagHoverResult',
                filePath: null
            });
        }
    }
}

// 页面加载后自动启动检测
if (document.readyState === 'complete') {
    startImageTagHoverDetection();
} else {
    window.addEventListener('load', startImageTagHoverDetection);
}

// 导出到全局
window.updateTagsDisplay = updateTagsDisplay;
window.createTagElement = createTagElement;
window.removeTag = removeTag;
window.toggleTagVisibility = toggleTagVisibility;
window.updateTags = updateTags;
window.createInlineTagElement = createInlineTagElement;
window.removeInlineTag = removeInlineTag;
window.ensureTagIntegrity = ensureTagIntegrity;
window.selectTagCombination = selectTagCombination;
window.startImageTagHoverDetection = startImageTagHoverDetection;
window.stopImageTagHoverDetection = stopImageTagHoverDetection;