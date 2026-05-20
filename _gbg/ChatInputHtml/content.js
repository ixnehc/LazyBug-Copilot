// 内容处理（输入输出、粘贴、光标导航）

// 设置输入内容
function setInputContent(content) {
    const inputEditor = document.getElementById('inputEditor');
    if (!inputEditor) return;
    
    console.log('setInputContent called with:', content);
    console.log('Content type:', typeof content);
    console.log('Is array:', Array.isArray(content));
    
    inputEditor.innerHTML = '';
    
    let contentArray = [];
    
    if (typeof content === 'string') {
        console.log('Processing as string, attempting JSON parse...');
        try {
            contentArray = JSON.parse(content);
            console.log('JSON parse successful:', contentArray);
        } catch (e) {
            console.warn('JSON parse failed, treating as plain text:', e);
            insertTextWithLineBreaks(content);
            notifyContentChanged();
            return;
        }
    } else if (Array.isArray(content)) {
        console.log('Processing as array directly:', content);
        contentArray = content;
    } else {
        console.error('Invalid content type for setInputContent:', content);
        return;
    }

    console.log('Final contentArray:', contentArray);
    const zwsp = AppState.zwsp;
    
    // ChatInputHtml 特有：使用 ZWSP 包装标签
    for (let i = 0; i < contentArray.length; i++) {
        const item = contentArray[i];
        console.log('Processing item:', item);
        
        if (item.type === 'text') {
            // 使用公共函数处理文本
            renderTextWithTags(inputEditor, item.content);
            console.log('Added text content:', item.content);
        } else if (item.type === 'tag') {
            // ChatInputHtml 特有：添加 ZWSP 包装
            const beforeZwsp = document.createTextNode(zwsp);
            const tagElement = createInlineTagElement({
                id: item.id,
                text: item.text,
                type: item.tagType,
                tagType: item.tagType,
                data: item.data,
                removable: item.removable
            });
            const afterZwsp = document.createTextNode(zwsp);
            
            inputEditor.appendChild(beforeZwsp);
            inputEditor.appendChild(tagElement);
            inputEditor.appendChild(afterZwsp);
            console.log('Added tag element:', item.text);
        }
    }
    
    if (inputEditor.childNodes.length === 0) {
        console.log('Editor is empty, adding empty text node');
        const textNode = document.createTextNode('');
        inputEditor.appendChild(textNode);
    }
    
    const selection = window.getSelection();
    const range = document.createRange();
    range.selectNodeContents(inputEditor);
    range.collapse(false);
    selection.removeAllRanges();
    selection.addRange(range);
    
    console.log('setInputContent completed, final editor content:', inputEditor.innerHTML);
    updateWatermarkVisibility();
}

// 清空输入内容
function clearInputContent() {
    const inputEditor = document.getElementById('inputEditor');
    inputEditor.innerHTML = '';
    updateWatermarkVisibility();
}

// 获取输入内容
function getInputContent() {
    const inputEditor = document.getElementById('inputEditor');
    if (!inputEditor) return JSON.stringify([]);
    
    const contentArray = [];
    const zwsp = AppState.zwsp;
    
    function traverseNodes(node) {
        if (node.nodeType === Node.TEXT_NODE) {
            const text = node.textContent.replace(new RegExp(zwsp, 'g'), '');
            if (text.length > 0) {
                contentArray.push({
                    type: 'text',
                    content: text
                });
            }
        } else if (node.nodeType === Node.ELEMENT_NODE) {
            if (node.classList && node.classList.contains('inline-tag')) {
                contentArray.push({
                    type: 'tag',
                    id: node.getAttribute('data-tag-id') || '',
                    text: node.textContent || '',
                    tagType: node.getAttribute('data-tag-type') || node.getAttribute('class').replace('inline-tag', '').trim() || 'info',
                    data: node.getAttribute('data-tag-data') || '',
                    removable: node.getAttribute('data-tag-removable') !== 'false'
                });
            } else if (node.tagName === 'BR') {
                contentArray.push({
                    type: 'text',
                    content: '\n'
                });
            } else {
                const isBlockElement = ['DIV', 'P', 'LI', 'H1', 'H2', 'H3', 'H4', 'H5', 'H6'].includes(node.tagName);
                if (isBlockElement && contentArray.length > 0 && contentArray[contentArray.length - 1].content !== '\n') {
                    contentArray.push({ type: 'text', content: '\n' });
                }
                
                for (let child of node.childNodes) {
                    traverseNodes(child);
                }
            }
        }
    }
    
    for (let child of inputEditor.childNodes) {
        traverseNodes(child);
    }
    
    const mergedContentArray = [];
    for (let i = 0; i < contentArray.length; i++) {
        const currentItem = contentArray[i];
        if (currentItem.type === 'text') {
            if (mergedContentArray.length > 0 && mergedContentArray[mergedContentArray.length - 1].type === 'text') {
                mergedContentArray[mergedContentArray.length - 1].content += currentItem.content;
            } else {
                mergedContentArray.push({ type: 'text', content: currentItem.content });
            }
        } else {
            mergedContentArray.push(currentItem);
        }
    }
    
    return JSON.stringify(mergedContentArray);
}

// 获取输入纯文本
function getInputPlainText() {
    return JSON.stringify(getEditorPlainText());
}

// 获取编辑器纯文本
function getEditorPlainText() {
    const inputEditor = document.getElementById('inputEditor');
    let text = '';
    const zwsp = AppState.zwsp;
    
    for (const child of inputEditor.childNodes) {
        if (child.nodeType === Node.TEXT_NODE) {
            const textContent = child.textContent.replace(new RegExp(zwsp, 'g'), '');
            text += textContent;
        } else if (child.nodeType === Node.ELEMENT_NODE) {
            if (child.tagName === 'BR') {
                text += '\n';
            } else if (child.classList && child.classList.contains('inline-tag')) {
                const tagType = child.getAttribute('data-tag-type') || '';
                const tagData = child.getAttribute('data-tag-data') || '';
                
                if (tagType === 'file' && tagData) {
                    text += `[${tagData}]`;
                } else {
                    const tagText = child.querySelector('.inline-tag-text');
                    if (tagText) {
                        text += `[${tagText.textContent}]`;
                    }
                }
            } else {
                const textContent = child.textContent.replace(new RegExp(zwsp, 'g'), '');
                text += textContent;
            }
        }
    }
    
    return text;
}

// 获取选中文本
function getSelectedText() {
    const selection = window.getSelection();
    if (selection.rangeCount > 0) {
        return JSON.stringify(selection.toString());
    }
    return JSON.stringify('');
}

// 在光标位置插入文本
function insertTextAtCursor(text) {
    const inputEditor = document.getElementById('inputEditor');
    const selection = window.getSelection();
    
    if (!inputEditor) return;
    
    inputEditor.focus();
    
    if (!selection.rangeCount) {
        const range = document.createRange();
        range.selectNodeContents(inputEditor);
        range.collapse(false);
        selection.removeAllRanges();
        selection.addRange(range);
    }

    const range = selection.getRangeAt(0);
    const textNode = document.createTextNode(text);
    
    range.deleteContents();
    range.insertNode(textNode);
    
    range.setStartAfter(textNode);
    range.setEndAfter(textNode);
    selection.removeAllRanges();
    selection.addRange(range);
    
    notifyContentChanged();
}





// 插入带换行的文本
function insertTextWithLineBreaks(text) {
    const inputEditor = document.getElementById('inputEditor');
    const selection = window.getSelection();
    
    if (!inputEditor) return;
    
    inputEditor.focus();
    
    if (!selection.rangeCount) {
        const range = document.createRange();
        range.selectNodeContents(inputEditor);
        range.collapse(false);
        selection.removeAllRanges();
        selection.addRange(range);
    }

    // 使用 execCommand('insertText') 插入文本，使浏览器将此操作记录到
    // undo 栈中，从而支持 Ctrl+Z 撤销粘贴。
    document.execCommand('insertText', false, text);
}






// 插入内联标签
function insertInlineTag(tagData) {
    const inputEditor = document.getElementById('inputEditor');
    const selection = window.getSelection();
    
    inputEditor.focus();
    
    if (!selection.rangeCount) {
        const range = document.createRange();
        range.selectNodeContents(inputEditor);
        range.collapse(false);
        selection.removeAllRanges();
        selection.addRange(range);
    }

    // 先用 createInlineTagElement 生成 DOM 元素，再取其 outerHTML，
    // 通过 execCommand('insertHTML') 插入，使浏览器将此操作记录到
    // undo 栈中，从而支持 Ctrl+Z 撤销。
    const tagElement = createInlineTagElement(tagData);
    const zwsp = AppState.zwsp;
    const html = zwsp + tagElement.outerHTML + zwsp;
    document.execCommand('insertHTML', false, html);

    notifyContentChanged();
}

// 通知内容变化
function notifyContentChanged() {
    if (!AppState.isInitialized) return;
    
    const contentJson = getInputContent();
    
    sendMessageToNative({
        action: 'contentChanged',
        content: JSON.parse(contentJson)
    });
}

// 更新水印可见性
function updateWatermarkVisibility() {
    const inputEditor = document.getElementById('inputEditor');
    const watermark = document.querySelector('.input-watermark');
    const sendButton = document.getElementById('sendButton');
    
    if (!inputEditor || !watermark || !sendButton) return;
    
    const isEmpty = isEditorEmpty(inputEditor);
    
    if (isEmpty) {
        watermark.classList.remove('hidden');
        sendButton.disabled = true;
    } else {
        watermark.classList.add('hidden');
        sendButton.disabled = false;
    }
}

// 检查编辑器是否为空
function isEditorEmpty(editor) {
    if (editor.childNodes.length === 0) {
        return true;
    }
    
    const zwsp = AppState.zwsp;
    for (const node of editor.childNodes) {
        if (node.nodeType === Node.TEXT_NODE) {
            const text = node.textContent.replace(new RegExp(zwsp, 'g'), '').trim();
            if (text.length > 0) {
                return false;
            }
        } else if (node.nodeType === Node.ELEMENT_NODE) {
            if (node.classList && node.classList.contains('inline-tag')) {
                return false;
            }
            if (node.textContent.trim().length > 0) {
                return false;
            }
        }
    }
    
    return true;
}

// 处理粘贴事件
function handlePaste(event) {
    event.preventDefault();
    
    const clipboardData = event.clipboardData || window.clipboardData;
    
    // 检查是否有文件数据
    const items = clipboardData.items;
    if (items) {
        let hasFiles = false;
        let hasImage = false;
        
        // 遍历剪贴板项
        for (let i = 0; i < items.length; i++) {
            const item = items[i];
            
            // 检查是否是文件类型
            if (item.kind === 'file') {
                hasFiles = true;
                
                // 检查是否是图片
                if (item.type.startsWith('image/')) {
                    hasImage = true;
                }
            }
        }
        
        // 如果有文件或图片数据，发送事件给上层
        if (hasFiles) {
            sendMessageToNative({
                action: 'filePasted',
                fileType: hasImage ? 'image' : 'files'
            });
            return;
        }
    }
    
    
    // 如果没有文件数据，处理文本粘贴
    const pastedText = clipboardData.getData('text/plain');
    
    if (pastedText) {
        createUndoBoundary();

        // 设置标志，阻止 ensureTagIntegrity 在 input 事件中运行。
        // execCommand('insertText') 触发的 input 事件会调用 ensureTagIntegrity，
        // 其直接 DOM 修改操作不进 undo 栈，会导致 Ctrl+Z 后内容残留。
        AppState.isPasting = true;
        try {
            insertTextWithLineBreaks(pastedText);
        } finally {
            // 用 setTimeout 延迟重置，确保 input 事件的所有同步回调都已执行完毕
            setTimeout(() => { AppState.isPasting = false; }, 0);
        }
        
        updateWatermarkVisibility();
        
        setTimeout(() => {
            handleAutoComplete();
            setTimeout(() => {
                notifyContentChanged();
            }, 10);
        }, 0);
    }
}


// 处理键盘导航（箭头键）
function handleArrowKeyNavigation(event) {
    const selection = window.getSelection();
    if (!selection.rangeCount) return;

    if (!event.shiftKey && !selection.isCollapsed) {
        return;
    }

    const range = selection.getRangeAt(0);
    const zwsp = AppState.zwsp;
    
    if (event.key === 'ArrowLeft') {
        if (event.shiftKey) {
            const focusNode = selection.focusNode;
            const focusOffset = selection.focusOffset;
            
            if (focusNode && focusNode.nodeType === Node.TEXT_NODE && 
                focusNode.textContent === zwsp && 
                focusNode.previousSibling && 
                focusNode.previousSibling.classList && 
                focusNode.previousSibling.classList.contains('inline-tag')) {
                
                event.preventDefault();
                const tagElement = focusNode.previousSibling;
                const beforeZwsp = tagElement.previousSibling;
                
                if (beforeZwsp && beforeZwsp.textContent === zwsp) {
                    selection.extend(beforeZwsp, 0);
                }
                return;
            }
            
            if (focusNode && focusNode.nodeType === Node.TEXT_NODE && focusOffset === 0) {
                const prevSibling = focusNode.previousSibling;
                if (prevSibling && prevSibling.nodeType === Node.TEXT_NODE && 
                    prevSibling.textContent === zwsp && 
                    prevSibling.previousSibling && 
                    prevSibling.previousSibling.classList && 
                    prevSibling.previousSibling.classList.contains('inline-tag')) {
                    
                    event.preventDefault();
                    const tagElement = prevSibling.previousSibling;
                    const beforeZwsp = tagElement.previousSibling;
                    
                    if (beforeZwsp && beforeZwsp.textContent === zwsp) {
                        selection.extend(beforeZwsp, 0);
                    }
                    return;
                }
            }
        } else {
            const container = range.startContainer;
            const offset = range.startOffset;

            if (container.nodeType === Node.TEXT_NODE && 
                container.textContent === zwsp && 
                container.previousSibling && 
                container.previousSibling.classList && 
                container.previousSibling.classList.contains('inline-tag')) {
                
                event.preventDefault();
                const tagElement = container.previousSibling;
                const beforeZwsp = tagElement.previousSibling;

                if (beforeZwsp && beforeZwsp.textContent === zwsp) {
                    range.setStartBefore(beforeZwsp);
                    range.collapse(true);
                    selection.removeAllRanges();
                    selection.addRange(range);
                }
                return;
            }
        }
        
        if (range.collapsed) {
            const prevElement = getPreviousElement(range);
            if (prevElement && prevElement.nodeType === Node.TEXT_NODE && 
                prevElement.textContent === zwsp && 
                prevElement.previousSibling && 
                prevElement.previousSibling.classList && 
                prevElement.previousSibling.classList.contains('inline-tag')) {
                event.preventDefault();
                const tagElement = prevElement.previousSibling;
                const beforeZwsp = tagElement.previousSibling;
                if (beforeZwsp && beforeZwsp.textContent === zwsp) {
                   if (event.shiftKey) {
                        selection.extend(beforeZwsp, 0);
                    } else {
                        range.setStartBefore(beforeZwsp);
                        range.collapse(true);
                        selection.removeAllRanges();
                        selection.addRange(range);
                    }
                }
            }
        }
    } else if (event.key === 'ArrowRight') {
        if (event.shiftKey) {
            const focusNode = selection.focusNode;
            const focusOffset = selection.focusOffset;
            
            if (focusNode && focusNode.nodeType === Node.TEXT_NODE && 
                focusNode.textContent === zwsp && 
                focusNode.nextSibling && 
                focusNode.nextSibling.classList && 
                focusNode.nextSibling.classList.contains('inline-tag')) {
                
                event.preventDefault();
                const tagElement = focusNode.nextSibling;
                const afterZwsp = tagElement.nextSibling;
                
                if (afterZwsp && afterZwsp.textContent === zwsp) {
                    selection.extend(afterZwsp, 1);
                }
                return;
            }
            
            if (focusNode && focusNode.nodeType === Node.TEXT_NODE && 
                focusOffset === focusNode.textContent.length) {
                const nextSibling = focusNode.nextSibling;
                if (nextSibling && nextSibling.nodeType === Node.TEXT_NODE && 
                    nextSibling.textContent === zwsp && 
                    nextSibling.nextSibling && 
                    nextSibling.nextSibling.classList && 
                    nextSibling.nextSibling.classList.contains('inline-tag')) {
                    
                    event.preventDefault();
                    const tagElement = nextSibling.nextSibling;
                    const afterZwsp = tagElement.nextSibling;
                    
                    if (afterZwsp && afterZwsp.textContent === zwsp) {
                        selection.extend(afterZwsp, 1);
                    }
                    return;
                }
            }
        } else {
            const container = range.startContainer;
            const offset = range.startOffset;
            
            if (container.nodeType === Node.TEXT_NODE && 
                container.textContent === zwsp && 
                container.nextSibling && 
                container.nextSibling.classList && 
                container.nextSibling.classList.contains('inline-tag')) {
                
                event.preventDefault();
                const tagElement = container.nextSibling;
                const afterZwsp = tagElement.nextSibling;
                if (afterZwsp && afterZwsp.textContent === zwsp) {
                    range.setStartAfter(afterZwsp);
                    range.collapse(true);
                    selection.removeAllRanges();
                    selection.addRange(range);
                }
                return;
            }
        }
        
        if (range.collapsed) {
            const nextElement = getNextElement(range);
            if (nextElement && nextElement.nodeType === Node.TEXT_NODE && 
                nextElement.textContent === zwsp && 
                nextElement.nextSibling && 
                nextElement.nextSibling.classList && 
                nextElement.nextSibling.classList.contains('inline-tag')) {
                event.preventDefault();
                const tagElement = nextElement.nextSibling;
                const afterZwsp = tagElement.nextSibling;
                if (afterZwsp && afterZwsp.textContent === zwsp) {
                    if (event.shiftKey) {
                        selection.extend(afterZwsp, 1);
                    } else {
                        range.setStartAfter(afterZwsp);
                        range.collapse(true);
                        selection.removeAllRanges();
                        selection.addRange(range);
                    }
                }
            }
        }
    }
}

// 处理标签删除
function handleTagDeletion(event) {
    const selection = window.getSelection();
    if (!selection.rangeCount) return;

    const range = selection.getRangeAt(0);
    const zwsp = AppState.zwsp;

    if (event.key === 'Backspace' && range.collapsed) {
        const container = range.startContainer;
        const offset = range.startOffset;
        
        if (container.nodeType === Node.TEXT_NODE && 
            container.textContent === zwsp && 
            container.previousSibling && 
            container.previousSibling.classList && 
            container.previousSibling.classList.contains('inline-tag')) {
            event.preventDefault();
            const tagElement = container.previousSibling;
            const beforeZwsp = tagElement.previousSibling;
            const afterZwsp = container;
            
            selectTagCombination(beforeZwsp, tagElement, afterZwsp);
            document.execCommand('delete');
            
            notifyContentChanged();
            return;
        }
        
        const prevElement = getPreviousElement(range);
        if (prevElement && prevElement.nodeType === Node.TEXT_NODE && 
            prevElement.textContent === zwsp && 
            prevElement.previousSibling && 
            prevElement.previousSibling.classList && 
            prevElement.previousSibling.classList.contains('inline-tag')) {
            event.preventDefault();
            const tagElement = prevElement.previousSibling;
            const beforeZwsp = tagElement.previousSibling;
            const afterZwsp = prevElement;
            
            selectTagCombination(beforeZwsp, tagElement, afterZwsp);
            document.execCommand('delete');
            
            notifyContentChanged();
            return;
        }
    } else if (event.key === 'Delete' && range.collapsed) {
        const container = range.startContainer;
        
        if (container.nodeType === Node.TEXT_NODE && 
            container.textContent === zwsp && 
            container.nextSibling && 
            container.nextSibling.classList && 
            container.nextSibling.classList.contains('inline-tag')) {
            event.preventDefault();
            const beforeZwsp = container;
            const tagElement = container.nextSibling;
            const afterZwsp = tagElement.nextSibling;
            
            selectTagCombination(beforeZwsp, tagElement, afterZwsp);
            document.execCommand('delete');
            
            notifyContentChanged();
            return;
        }
        
        const nextElement = getNextElement(range);
        if (nextElement && nextElement.nodeType === Node.TEXT_NODE && 
            nextElement.textContent === zwsp && 
            nextElement.nextSibling && 
            nextElement.nextSibling.classList && 
            nextElement.nextSibling.classList.contains('inline-tag')) {
            event.preventDefault();
            const beforeZwsp = nextElement;
            const tagElement = nextElement.nextSibling;
            const afterZwsp = tagElement.nextSibling;
            
            selectTagCombination(beforeZwsp, tagElement, afterZwsp);
            document.execCommand('delete');
            
            notifyContentChanged();
            return;
        }
    }
}

// 处理选择变化
function handleSelectionChange() {
    const inputEditor = document.getElementById('inputEditor');
    if (!inputEditor) return;
    
    const allTags = inputEditor.querySelectorAll('.inline-tag');
    allTags.forEach(tag => tag.classList.remove('selected'));
    
    const selection = window.getSelection();
    if (selection.rangeCount === 0 || selection.isCollapsed) {
        return;
    }
    
    const range = selection.getRangeAt(0);
    
    allTags.forEach(tag => {
        if (isNodeInRange(tag, range)) {
            tag.classList.add('selected');
        }
    });
}

// 导出到全局
window.setInputContent = setInputContent;
window.clearInputContent = clearInputContent;
window.getInputContent = getInputContent;
window.getInputPlainText = getInputPlainText;
window.getEditorPlainText = getEditorPlainText;
window.getSelectedText = getSelectedText;
window.insertTextAtCursor = insertTextAtCursor;
window.insertTextWithLineBreaks = insertTextWithLineBreaks;
window.insertInlineTag = insertInlineTag;
window.notifyContentChanged = notifyContentChanged;
window.updateWatermarkVisibility = updateWatermarkVisibility;
window.isEditorEmpty = isEditorEmpty;
window.handlePaste = handlePaste;
window.handleArrowKeyNavigation = handleArrowKeyNavigation;
window.handleTagDeletion = handleTagDeletion;
window.handleSelectionChange = handleSelectionChange;