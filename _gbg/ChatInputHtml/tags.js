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
    }
    
    const textSpan = document.createElement('span');
    textSpan.className = 'tag-text';
    textSpan.textContent = tag.text;
    tagDiv.appendChild(textSpan);
    
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
    const tagSpan = document.createElement('span');
    tagSpan.className = `inline-tag ${tag.type || tag.tagType || 'info'}`;
    tagSpan.setAttribute('data-tag-id', tag.id || '');
    tagSpan.setAttribute('data-tag-data', tag.data || '');
    tagSpan.setAttribute('data-tag-type', tag.type || tag.tagType || 'info');
    tagSpan.setAttribute('data-tag-removable', (tag.removable !== false).toString());
    // image 类型的标签不显示普通文字 tooltip，改用原生图片预览窗口
    if ((tag.type || tag.tagType) !== 'image') {
        tagSpan.title = tag.data || tag.text;
    }
    tagSpan.contentEditable = false;

    const textSpan = document.createElement('span');
    textSpan.className = 'inline-tag-text';
    textSpan.textContent = tag.text || '';
    tagSpan.appendChild(textSpan);
    
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
        } else if (prevSibling.textContent === zwsp) {
            // 完美
        } else if (prevSibling.textContent.endsWith(zwsp)) {
            const content = prevSibling.textContent;
            const mainContent = content.slice(0, -1);
            const zwspContent = content.slice(-1);
            
            if (cursorInfo && cursorInfo.node === prevSibling) {
                if (cursorInfo.offset > mainContent.length) {
                    cursorInfo.offset = cursorInfo.offset - mainContent.length;
                    cursorInfo.needsNewNode = true;
                    cursorInfo.newNodeType = 'prevZwsp';
                    cursorInfo.relatedTag = tag;
                }
            }
            
            if (mainContent.length > 0) {
                prevSibling.textContent = mainContent;
                const separateZwsp = document.createTextNode(zwspContent);
                tag.parentNode.insertBefore(separateZwsp, tag);
                
                if (cursorInfo && cursorInfo.needsNewNode && cursorInfo.newNodeType === 'prevZwsp') {
                    cursorInfo.node = separateZwsp;
                    cursorInfo.needsNewNode = false;
                }
            } else {
                prevSibling.textContent = zwspContent;
            }
            needsRepair = true;
        } else {
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
        } else if (nextSibling.textContent === zwsp) {
            // 完美
        } else if (nextSibling.textContent.startsWith(zwsp)) {
            const content = nextSibling.textContent;
            const zwspContent = content.slice(0, 1);
            const mainContent = content.slice(1);
            
            if (cursorInfo && cursorInfo.node === nextSibling) {
                if (cursorInfo.offset === 0) {
                    cursorInfo.needsNewNode = true;
                    cursorInfo.newNodeType = 'nextZwsp';
                    cursorInfo.relatedTag = tag;
                    cursorInfo.offset = 0;
                } else if (cursorInfo.offset === 1) {
                    cursorInfo.offset = 0;
                } else {
                    cursorInfo.offset = cursorInfo.offset - 1;
                }
            }
            
            if (mainContent.length > 0) {
                nextSibling.textContent = mainContent;
                const separateZwsp = document.createTextNode(zwspContent);
                tag.parentNode.insertBefore(separateZwsp, nextSibling);
                
                if (cursorInfo && cursorInfo.needsNewNode && cursorInfo.newNodeType === 'nextZwsp') {
                    cursorInfo.node = separateZwsp;
                    cursorInfo.needsNewNode = false;
                }
            } else {
                nextSibling.textContent = zwspContent;
            }
            needsRepair = true;
        } else {
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
    
    if (beforeZwsp && beforeZwsp.textContent === AppState.zwsp) {
        startNode = beforeZwsp;
        startOffset = 0;
    } else {
        startNode = tagElement;
        startOffset = 0;
    }
    
    if (afterZwsp && afterZwsp.textContent === AppState.zwsp) {
        endNode = afterZwsp;
        endOffset = 1;
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
        if (beforeZwsp && beforeZwsp.textContent === AppState.zwsp) {
            beforeZwsp.remove();
        }
        tagElement.remove();
        if (afterZwsp && afterZwsp.textContent === AppState.zwsp) {
            afterZwsp.remove();
        }
    }
}

// Image Tag 悬停检测相关
let _imageTagHoverTimerId = null;
let _lastHoveredImageFilePath = null;


// 启动 image tag 悬停检测定时器
function startImageTagHoverDetection() {
    if (_imageTagHoverTimerId !== null) {
        return; // 已经在运行
    }
    // 每 100ms 检测一次
    _imageTagHoverTimerId = setInterval(checkImageTagHover, 100);
}

// 停止 image tag 悬停检测定时器
function stopImageTagHoverDetection() {
    if (_imageTagHoverTimerId !== null) {
        clearInterval(_imageTagHoverTimerId);
        _imageTagHoverTimerId = null;
    }
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

    // 遍历所有 image tag，检查是否有 :hover 状态
    let hoveredTag = null;
    let filePath = null;
    
    for (const tagElement of allImageTagElements) {
        if (tagElement.matches(':hover')) {
            hoveredTag = tagElement;
            
            // 获取文件路径
            if (tagElement.hasAttribute('data-image-tag-id')) {
                // 顶部 image tag：通过 tagId 从 AppState.currentTags 查找
                const tagId = tagElement.getAttribute('data-image-tag-id');
                const tag = AppState.currentTags.find(t => t.id === tagId);
                filePath = tag ? (tag.data || '') : '';
            } else {
                // inline image tag：直接从 data-tag-data 获取
                filePath = tagElement.getAttribute('data-tag-data') || '';
            }
            break;
        }
    }

    if (hoveredTag && filePath) {
        // 只有当悬停的 tag 变化时才发送消息
        if (_lastHoveredImageFilePath !== filePath) {
            _lastHoveredImageFilePath = filePath;
            const rect = hoveredTag.getBoundingClientRect();
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
        // 鼠标不在任何 image tag 上
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