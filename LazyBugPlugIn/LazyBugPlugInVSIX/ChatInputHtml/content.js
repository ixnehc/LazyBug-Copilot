// 内容处理（输入输出、粘贴、光标导航）

// 设置输入内容
function setInputContent(content, caretTokenPos) {
    const inputEditor = document.getElementById('inputEditor');
    if (!inputEditor) return;
    
    console.log('setInputContent called with:', content);
    console.log('Content type:', typeof content);
    console.log('Is array:', Array.isArray(content));
    console.log('caretTokenPos:', caretTokenPos);
    
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
                imgSrc: item.imgSrc || '',   // image 类型恢复时带上 Base64 DataURL
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
    
    // 设置光标位置
    const selection = window.getSelection();
    const range = document.createRange();
    
    if (typeof caretTokenPos === 'number' && caretTokenPos >= 0) {
        // 根据 token 位置定位光标
        const caretLocation = _locateCaretByToken(inputEditor, caretTokenPos);
        if (caretLocation) {
            try {
                range.setStart(caretLocation.node, caretLocation.offset);
                range.collapse(true);
                selection.removeAllRanges();
                selection.addRange(range);
            } catch (e) {
                console.warn('Failed to set caret at token position:', e);
                // 回退到末尾
                range.selectNodeContents(inputEditor);
                range.collapse(false);
                selection.removeAllRanges();
                selection.addRange(range);
            }
        } else {
            // 定位失败，回退到末尾
            range.selectNodeContents(inputEditor);
            range.collapse(false);
            selection.removeAllRanges();
            selection.addRange(range);
        }
    } else {
        // 默认折叠到末尾
        range.selectNodeContents(inputEditor);
        range.collapse(false);
        selection.removeAllRanges();
        selection.addRange(range);
    }
    
    console.log('setInputContent completed, final editor content:', inputEditor.innerHTML);
    updateWatermarkVisibility();
    
    // 清除撤销栈
    // 先移除 contenteditable，再重新设为 true，这是在浏览器中清空撤销栈最简单有效的方法
    const isEditable = inputEditor.getAttribute('contenteditable');
    if (isEditable === 'true' || isEditable === '') {
        inputEditor.setAttribute('contenteditable', 'false');
        // 强制重绘
        inputEditor.offsetHeight; 
        inputEditor.setAttribute('contenteditable', 'true');
        
        // 重新聚焦并恢复光标位置
        inputEditor.focus();
        selection.removeAllRanges();
        selection.addRange(range);
    }
}

// 清空输入内容
function clearInputContent() {
    const inputEditor = document.getElementById('inputEditor');
    inputEditor.innerHTML = '';
    updateWatermarkVisibility();

    // 清除撤销栈
    const isEditable = inputEditor.getAttribute('contenteditable');
    if (isEditable === 'true' || isEditable === '') {
        inputEditor.setAttribute('contenteditable', 'false');
        inputEditor.offsetHeight; 
        inputEditor.setAttribute('contenteditable', 'true');
        inputEditor.focus();
    }
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
                const tagType = node.getAttribute('data-tag-type') || node.getAttribute('class').replace('inline-tag', '').trim() || 'info';
                // 优先读 data-tag-text 属性（image 类型 textContent 为空）
                const tagText = node.getAttribute('data-tag-text') || node.textContent || '';
                const entry = {
                    type: 'tag',
                    id: node.getAttribute('data-tag-id') || '',
                    text: tagText,
                    tagType: tagType,
                    data: node.getAttribute('data-tag-data') || '',
                    removable: node.getAttribute('data-tag-removable') !== 'false'
                };
                // image 类型：保留 imgSrc（Base64 DataURL），setContent 恢复时可直接使用
                if (tagType === 'image') {
                    const imgSrc = node.getAttribute('data-tag-imgsrc') || '';
                    if (imgSrc) entry.imgSrc = imgSrc;
                }
                contentArray.push(entry);
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
                    // 优先查找 .inline-tag-text 元素
                    const tagText = child.querySelector('.inline-tag-text');
                    if (tagText) {
                        text += `[${tagText.textContent}]`;
                    } else {
                        // 对于 image 类型等没有 .inline-tag-text 元素的情况，使用 data-tag-text 属性
                        const tagTextAttr = child.getAttribute('data-tag-text') || '';
                        if (tagTextAttr) {
                            text += `[${tagTextAttr}]`;
                        }
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

    // 多行文本必须用 <br> 作为换行边界后再插入。
    // 若直接用 execCommand('insertText') 插入含 '\n' 的整段文本，浏览器会将其
    // 作为单个文本节点（靠 CSS white-space:pre-wrap 视觉换行）插入，
    // 此结构下 Chromium 的光标上下导航在跨越 '\n' 字符时存在缺陷，
    // 表现为按↑/↓键移动到相邻行时光标消失。
    // 改用 <br> + insertHTML，使换行边界与手工换行(insertLineBreak)一致，
    // insertHTML 同样会进 undo 栈，可保留 Ctrl+Z 撤销能力。
    if (text.indexOf('\n') !== -1) {
        const html = text
            .replace(/&/g, '&amp;')
            .replace(/</g, '&lt;')
            .replace(/>/g, '&gt;')
            .replace(/\r\n/g, '\n')
            .replace(/\n/g, '<br>');
        document.execCommand('insertHTML', false, html);
    } else {
        // 使用 execCommand('insertText') 插入文本，使浏览器将此操作记录到
        // undo 栈中，从而支持 Ctrl+Z 撤销粘贴。
        document.execCommand('insertText', false, text);
    }
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
        content: JSON.parse(contentJson),
        caretPos: getCaretTokenPosition(),
        isComposing: AppState.isInputComposing
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

// 处理复制事件，以支持自定义复制内容和格式
function handleCopy(event) {
    const isCut = event.type === 'cut';
    const selection = window.getSelection();
    if (!selection.rangeCount) return;

    const inputEditor = document.getElementById('inputEditor');
    if (!inputEditor || !inputEditor.contains(selection.anchorNode)) return;

    const fragment = selection.getRangeAt(0).cloneContents();
    const tempDiv = document.createElement('div');
    tempDiv.appendChild(fragment);
    
    if (!tempDiv.querySelector('.inline-tag')) {
        return; // default behavior
    }
    
    const contentArray = [];
    const zwsp = AppState.zwsp;
    
    function traverseCopyNodes(node) {
        if (node.nodeType === Node.TEXT_NODE) {
            const text = node.textContent.replace(new RegExp(zwsp, 'g'), '');
            if (text.length > 0) {
                contentArray.push({ type: 'text', content: text });
            }
        } else if (node.nodeType === Node.ELEMENT_NODE) {
            if (node.classList && node.classList.contains('inline-tag')) {
                const tagId = node.getAttribute('data-tag-id') || '';
                const tagType = node.getAttribute('data-tag-type') || node.getAttribute('class').replace('inline-tag', '').trim() || 'info';
                const tagData = node.getAttribute('data-tag-data') || '';
                const removable = node.getAttribute('data-tag-removable') !== 'false';
                // 优先用 data-tag-text，image 类型的 textContent 为空
                const textContent = node.getAttribute('data-tag-text') ||
                    (node.querySelector('.inline-tag-text') ? node.querySelector('.inline-tag-text').textContent : node.textContent);
                const entry = {
                    type: 'tag',
                    id: tagId,
                    text: textContent,
                    tagType: tagType,
                    data: tagData,
                    removable: removable
                };
                if (tagType === 'image') {
                    const imgSrc = node.getAttribute('data-tag-imgsrc') || '';
                    if (imgSrc) entry.imgSrc = imgSrc;
                }
                contentArray.push(entry);
            } else if (node.tagName === 'BR') {
                contentArray.push({ type: 'text', content: '\n' });
            } else {
                const isBlockElement = ['DIV', 'P', 'LI', 'H1', 'H2', 'H3', 'H4', 'H5', 'H6'].includes(node.tagName);
                if (isBlockElement && contentArray.length > 0 && contentArray[contentArray.length - 1].content !== '\n') {
                    contentArray.push({ type: 'text', content: '\n' });
                }
                
                for (let i = 0; i < node.childNodes.length; i++) {
                    traverseCopyNodes(node.childNodes[i]);
                }
                
                if (isBlockElement && contentArray.length > 0 && contentArray[contentArray.length - 1].content !== '\n') {
                    contentArray.push({ type: 'text', content: '\n' });
                }
            }
        }
    }
    
    traverseCopyNodes(tempDiv);
    
    let plainText = '';
    contentArray.forEach(item => {
        if (item.type === 'text') {
            plainText += item.content;
        } else if (item.type === 'tag') {
            plainText += item.data || item.text;
        }
    });
    
    event.clipboardData.setData('text/plain', plainText);
    event.clipboardData.setData('application/x-lazybug-chatinput', JSON.stringify(contentArray));
    event.clipboardData.setData('text/html', tempDiv.innerHTML);
    
    event.preventDefault();
    
    if (isCut) {
        document.execCommand('delete', false, null);
    }
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
    let customData = clipboardData.getData('application/x-lazybug-chatinput');
    let pastedHTML = clipboardData.getData('text/html');
    const pastedText = clipboardData.getData('text/plain');
    
    // 如果有自定义的复制数据，优先使用
    if (customData) {
        try {
            const contentArray = JSON.parse(customData);
            let cleanHTML = '';
            const zwsp = AppState.zwsp;
            
            contentArray.forEach(item => {
                if (item.type === 'text') {
                    const escapedText = item.content
                        .replace(/&/g, '&amp;')
                        .replace(/</g, '&lt;')
                        .replace(/>/g, '&gt;')
                        .replace(/\n/g, '<br>');
                    cleanHTML += escapedText;
                } else if (item.type === 'tag') {
                    // 统一通过 createInlineTagElement 构建，保证 image 类型渲染为 <img>
                    const span = createInlineTagElement(item);
                    span.contentEditable = false;
                    cleanHTML += `${zwsp}${span.outerHTML}${zwsp}`;
                }
            });
            
            if (cleanHTML) {
                AppState.isPasting = true;
                try {
                    if (typeof window.forceUndoBoundary === 'function') {
                        window.forceUndoBoundary();
                    }
                    document.execCommand('insertHTML', false, cleanHTML);
                    if (typeof window.forceUndoBoundary === 'function') {
                        window.forceUndoBoundary();
                    }
                } finally {
                    setTimeout(() => { AppState.isPasting = false; }, 0);
                }
                
                updateWatermarkVisibility();
                setTimeout(() => {
                    handleAutoComplete();
                    setTimeout(() => {
                        notifyContentChanged();
                    }, 10);
                }, 0);
                return;
            }
        } catch (e) {
            console.error('Failed to parse custom paste data', e);
        }
    } else if (pastedHTML && pastedHTML.includes('inline-tag')) {
        const startMatch = pastedHTML.indexOf('<!--StartFragment-->');
        const endMatch = pastedHTML.indexOf('<!--EndFragment-->');
        if (startMatch !== -1 && endMatch !== -1) {
            pastedHTML = pastedHTML.substring(startMatch + '<!--StartFragment-->'.length, endMatch);
        }
        
        const div = document.createElement('div');
        div.innerHTML = pastedHTML;
        
        const contentArray = [];
        const zwsp = AppState.zwsp;
        
        function traverseNodesPasted(node) {
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
                    const tagId = node.getAttribute('data-tag-id') || '';
                    const tagType = node.getAttribute('data-tag-type') || node.getAttribute('class').replace('inline-tag', '').trim() || 'info';
                    const tagData = node.getAttribute('data-tag-data') || '';
                    const removable = node.getAttribute('data-tag-removable') !== 'false';
                    // 优先用 data-tag-text，image 类型的 textContent 为空
                    const textContent = node.getAttribute('data-tag-text') ||
                        (node.querySelector('.inline-tag-text') ? node.querySelector('.inline-tag-text').textContent : node.textContent);
                    const entry = {
                        type: 'tag',
                        id: tagId,
                        text: textContent,
                        tagType: tagType,
                        data: tagData,
                        removable: removable
                    };
                    if (tagType === 'image') {
                        const imgSrc = node.getAttribute('data-tag-imgsrc') || '';
                        if (imgSrc) entry.imgSrc = imgSrc;
                    }
                    contentArray.push(entry);
                } else if (node.tagName === 'BR') {
                    contentArray.push({
                        type: 'text',
                        content: '\n'
                    });
                } else {
                    const isBlockElement = ['DIV', 'P', 'LI', 'H1', 'H2', 'H3', 'H4', 'H5', 'H6'].includes(node.tagName);
                    if (isBlockElement && contentArray.length > 0 && contentArray[contentArray.length - 1].content !== '\n') {
                        contentArray.push({
                            type: 'text',
                            content: '\n'
                        });
                    }
                    
                    for (let i = 0; i < node.childNodes.length; i++) {
                        traverseNodesPasted(node.childNodes[i]);
                    }
                    
                    if (isBlockElement && contentArray.length > 0 && contentArray[contentArray.length - 1].content !== '\n') {
                        contentArray.push({
                            type: 'text',
                            content: '\n'
                        });
                    }
                }
            }
        }
        
        traverseNodesPasted(div);
        
        let cleanHTML = '';
        contentArray.forEach(item => {
            if (item.type === 'text') {
                const escapedText = item.content
                    .replace(/&/g, '&amp;')
                    .replace(/</g, '&lt;')
                    .replace(/>/g, '&gt;')
                    .replace(/\n/g, '<br>');
                cleanHTML += escapedText;
            } else if (item.type === 'tag') {
                // 统一通过 createInlineTagElement 构建，保证 image 类型渲染为 <img>
                const span = createInlineTagElement(item);
                span.contentEditable = false;
                cleanHTML += `${zwsp}${span.outerHTML}${zwsp}`;
            }
        });
        
        if (cleanHTML) {
            AppState.isPasting = true;
            try {
                if (typeof window.forceUndoBoundary === 'function') {
                    window.forceUndoBoundary();
                }
                document.execCommand('insertHTML', false, cleanHTML);
                if (typeof window.forceUndoBoundary === 'function') {
                    window.forceUndoBoundary();
                }
            } finally {
                setTimeout(() => { AppState.isPasting = false; }, 0);
            }
            
            updateWatermarkVisibility();
            setTimeout(() => {
                handleAutoComplete();
                setTimeout(() => {
                    notifyContentChanged();
                }, 10);
            }, 0);
            return;
        }
    }
    
    if (pastedText) {
        // 设置标志，阻止 ensureTagIntegrity 在 input 事件中运行。
        // execCommand('insertText') 触发的 input 事件会调用 ensureTagIntegrity，
        // 其直接 DOM 修改操作不进 undo 栈，会导致 Ctrl+Z 后内容残留。
        AppState.isPasting = true;
        try {
            if (typeof window.forceUndoBoundary === 'function') {
                window.forceUndoBoundary();
            }
            insertTextWithLineBreaks(pastedText);
            if (typeof window.forceUndoBoundary === 'function') {
                window.forceUndoBoundary();
            }
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
            
            if (focusNode && focusNode.nodeType === Node.TEXT_NODE && focusOffset > 0) {
                if (focusNode.textContent.substring(focusOffset - 1, focusOffset) === zwsp) {
                    if (focusOffset === 1 && focusNode.previousSibling && 
                        focusNode.previousSibling.classList && 
                        focusNode.previousSibling.classList.contains('inline-tag')) {
                        
                        event.preventDefault();
                        const tagElement = focusNode.previousSibling;
                        const beforeZwsp = tagElement.previousSibling;
                        
                        if (beforeZwsp && beforeZwsp.nodeType === Node.TEXT_NODE && beforeZwsp.textContent.endsWith(zwsp)) {
                            selection.extend(beforeZwsp, beforeZwsp.textContent.length - 1);
                        }
                        return;
                    }
                    
                    if (focusOffset === focusNode.textContent.length && 
                        focusNode.previousSibling && 
                        focusNode.previousSibling.classList && 
                        focusNode.previousSibling.classList.contains('inline-tag')) {
                        
                        event.preventDefault();
                        const tagElement = focusNode.previousSibling;
                        const beforeZwsp = tagElement.previousSibling;
                        
                        if (beforeZwsp && beforeZwsp.nodeType === Node.TEXT_NODE && beforeZwsp.textContent.endsWith(zwsp)) {
                            selection.extend(beforeZwsp, beforeZwsp.textContent.length - 1);
                        }
                        return;
                    }
                }
            }
            
            if (focusNode && focusNode.nodeType === Node.TEXT_NODE && focusOffset === 0) {
                const prevSibling = focusNode.previousSibling;
                if (prevSibling && prevSibling.nodeType === Node.TEXT_NODE && 
                    prevSibling.textContent.endsWith(zwsp) && 
                    prevSibling.previousSibling && 
                    prevSibling.previousSibling.classList && 
                    prevSibling.previousSibling.classList.contains('inline-tag')) {
                    
                    event.preventDefault();
                    const tagElement = prevSibling.previousSibling;
                    const beforeZwsp = tagElement.previousSibling;
                    
                    if (beforeZwsp && beforeZwsp.nodeType === Node.TEXT_NODE && beforeZwsp.textContent.endsWith(zwsp)) {
                        selection.extend(beforeZwsp, beforeZwsp.textContent.length - 1);
                    }
                    return;
                }
            }
        } else {
            const container = range.startContainer;
            const offset = range.startOffset;

            if (container.nodeType === Node.TEXT_NODE && offset > 0) {
                // If the char immediately before the cursor is ZWSP
                if (container.textContent.substring(offset - 1, offset) === zwsp) {
                    // Check if this ZWSP belongs to the preceding tag
                    // It belongs to the tag if it's the very first character of the node, 
                    // and the previous sibling is a tag.
                    // Or, if it's not the first character, it shouldn't happen normally, 
                    // but we mainly care about offset === 1.
                    if (offset === 1 && container.previousSibling && 
                        container.previousSibling.classList && 
                        container.previousSibling.classList.contains('inline-tag')) {
                        
                        event.preventDefault();
                        const tagElement = container.previousSibling;
                        const beforeZwsp = tagElement.previousSibling;

                        if (beforeZwsp && beforeZwsp.nodeType === Node.TEXT_NODE && beforeZwsp.textContent.endsWith(zwsp)) {
                            range.setStart(beforeZwsp, beforeZwsp.textContent.length - 1);
                            range.collapse(true);
                            selection.removeAllRanges();
                            selection.addRange(range);
                        }
                        return;
                    }
                    
                    // Handle the old logic: if offset is at the end of the text node and it's a ZWSP
                    // This catches cases where the text node is exactly a single ZWSP, or multiple ZWSPs 
                    // but we are at the end. Actually offset === 1 covers the single ZWSP case.
                    // But just in case:
                    if (offset === container.textContent.length && 
                        container.previousSibling && 
                        container.previousSibling.classList && 
                        container.previousSibling.classList.contains('inline-tag')) {
                        
                        event.preventDefault();
                        const tagElement = container.previousSibling;
                        const beforeZwsp = tagElement.previousSibling;

                        if (beforeZwsp && beforeZwsp.nodeType === Node.TEXT_NODE && beforeZwsp.textContent.endsWith(zwsp)) {
                            range.setStart(beforeZwsp, beforeZwsp.textContent.length - 1);
                            range.collapse(true);
                            selection.removeAllRanges();
                            selection.addRange(range);
                        }
                        return;
                    }
                }
            }
        }
        
        if (range.collapsed) {
            const prevElement = getPreviousElement(range);
            if (prevElement && prevElement.nodeType === Node.TEXT_NODE && 
                prevElement.textContent.endsWith(zwsp) && 
                prevElement.previousSibling && 
                prevElement.previousSibling.classList && 
                prevElement.previousSibling.classList.contains('inline-tag')) {
                event.preventDefault();
                const tagElement = prevElement.previousSibling;
                const beforeZwsp = tagElement.previousSibling;
                if (beforeZwsp && beforeZwsp.nodeType === Node.TEXT_NODE && beforeZwsp.textContent.endsWith(zwsp)) {
                   if (event.shiftKey) {
                        selection.extend(beforeZwsp, beforeZwsp.textContent.length - 1);
                    } else {
                        range.setStart(beforeZwsp, beforeZwsp.textContent.length - 1);
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
            
            if (focusNode && focusNode.nodeType === Node.TEXT_NODE && focusOffset < focusNode.textContent.length) {
                if (focusNode.textContent.substring(focusOffset, focusOffset + 1) === zwsp) {
                    if (focusOffset === focusNode.textContent.length - 1 && 
                        focusNode.nextSibling && 
                        focusNode.nextSibling.classList && 
                        focusNode.nextSibling.classList.contains('inline-tag')) {
                        
                        event.preventDefault();
                        const tagElement = focusNode.nextSibling;
                        const afterZwsp = tagElement.nextSibling;
                        
                        if (afterZwsp && afterZwsp.nodeType === Node.TEXT_NODE && afterZwsp.textContent.startsWith(zwsp)) {
                            selection.extend(afterZwsp, 1);
                        }
                        return;
                    }
                    
                    if (focusOffset === 0 && 
                        focusNode.nextSibling && 
                        focusNode.nextSibling.classList && 
                        focusNode.nextSibling.classList.contains('inline-tag')) {
                        
                        event.preventDefault();
                        const tagElement = focusNode.nextSibling;
                        const afterZwsp = tagElement.nextSibling;
                        
                        if (afterZwsp && afterZwsp.nodeType === Node.TEXT_NODE && afterZwsp.textContent.startsWith(zwsp)) {
                            selection.extend(afterZwsp, 1);
                        }
                        return;
                    }
                }
            }
            
            if (focusNode && focusNode.nodeType === Node.TEXT_NODE && 
                focusOffset === focusNode.textContent.length) {
                const nextSibling = focusNode.nextSibling;
                if (nextSibling && nextSibling.nodeType === Node.TEXT_NODE && 
                    nextSibling.textContent.startsWith(zwsp) && 
                    nextSibling.nextSibling && 
                    nextSibling.nextSibling.classList && 
                    nextSibling.nextSibling.classList.contains('inline-tag')) {
                    
                    event.preventDefault();
                    const tagElement = nextSibling.nextSibling;
                    const afterZwsp = tagElement.nextSibling;
                    
                    if (afterZwsp && afterZwsp.nodeType === Node.TEXT_NODE && afterZwsp.textContent.startsWith(zwsp)) {
                        selection.extend(afterZwsp, 1);
                    }
                    return;
                }
            }
        } else {
            const container = range.startContainer;
            const offset = range.startOffset;
            
            if (container.nodeType === Node.TEXT_NODE && offset < container.textContent.length) {
                // If the char immediately after the cursor is ZWSP
                if (container.textContent.substring(offset, offset + 1) === zwsp) {
                    // Check if this ZWSP belongs to the following tag
                    // It belongs to the tag if it's the very last character of the node,
                    // and the next sibling is a tag.
                    if (offset === container.textContent.length - 1 && 
                        container.nextSibling && 
                        container.nextSibling.classList && 
                        container.nextSibling.classList.contains('inline-tag')) {
                        
                        event.preventDefault();
                        const tagElement = container.nextSibling;
                        const afterZwsp = tagElement.nextSibling;
                        if (afterZwsp && afterZwsp.nodeType === Node.TEXT_NODE && afterZwsp.textContent.startsWith(zwsp)) {
                            range.setStart(afterZwsp, 1);
                            range.collapse(true);
                            selection.removeAllRanges();
                            selection.addRange(range);
                        }
                        return;
                    }
                    
                    // Handle the case where offset is 0 and it's a ZWSP
                    if (offset === 0 && 
                        container.nextSibling && 
                        container.nextSibling.classList && 
                        container.nextSibling.classList.contains('inline-tag')) {
                        
                        event.preventDefault();
                        const tagElement = container.nextSibling;
                        const afterZwsp = tagElement.nextSibling;
                        if (afterZwsp && afterZwsp.nodeType === Node.TEXT_NODE && afterZwsp.textContent.startsWith(zwsp)) {
                            range.setStart(afterZwsp, 1);
                            range.collapse(true);
                            selection.removeAllRanges();
                            selection.addRange(range);
                        }
                        return;
                    }
                }
            }
        }
        
        if (range.collapsed) {
            const nextElement = getNextElement(range);
            if (nextElement && nextElement.nodeType === Node.TEXT_NODE && 
                nextElement.textContent.startsWith(zwsp) && 
                nextElement.nextSibling && 
                nextElement.nextSibling.classList && 
                nextElement.nextSibling.classList.contains('inline-tag')) {
                event.preventDefault();
                const tagElement = nextElement.nextSibling;
                const afterZwsp = tagElement.nextSibling;
                if (afterZwsp && afterZwsp.nodeType === Node.TEXT_NODE && afterZwsp.textContent.startsWith(zwsp)) {
                    if (event.shiftKey) {
                        selection.extend(afterZwsp, 1);
                    } else {
                        range.setStart(afterZwsp, 1);
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
        
        // Case 1: Cursor is inside the trailing text node, exactly after the ZWSP
        if (container.nodeType === Node.TEXT_NODE && 
            offset === 1 && 
            container.textContent.startsWith(zwsp) && 
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
        
        // Case 2: Cursor is at the end of the trailing text node (e.g. if text node is exactly 'zwsp')
        if (container.nodeType === Node.TEXT_NODE && 
            offset === container.textContent.length && 
            container.textContent.endsWith(zwsp) && 
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
        
        // Case 3: Cursor is outside the text node, but the previous node is the trailing text node
        const prevElement = getPreviousElement(range);
        if (prevElement && prevElement.nodeType === Node.TEXT_NODE && 
            prevElement.textContent.endsWith(zwsp) && 
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

    // 持续保存当前光标位置，以便失去焦点后恢复
    if (typeof saveSelection === 'function') {
        saveSelection();
    }
    
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

// ===== 删除标记：光标保存/恢复辅助 =====
// setDeletionMarks/clearDeletionMarks 会重建 DOM（拆分文本节点、包裹 span、normalize），
// 导致原有选区失效、光标跳到行首。这里按“字符偏移”保存/恢复光标。
// 由于这些操作不改变文本内容（仅包裹/展开节点、切换 class），字符偏移是稳定的。
//
// 关键：保存(_getCharOffset)与恢复(_locateCaret)必须使用【完全相同】的字符计数规则，
// 否则在中英文混合、含 tag/ZWSP/<br> 时两者计数不一致，光标会偏移。
// 统一规则：
//   - 文本节点：按 textContent.length 计（含 ZWSP）
//   - inline-tag：作为原子单位，按其 textContent.length 计
//   - <br>：计为 0（与 Range.toString() 一致）
//   - 其它元素：递归其子节点

// 计算某个节点子树的字符长度（遵循统一计数规则）
function _measureNodeChars(node) {
    if (node.nodeType === Node.TEXT_NODE) {
        return node.textContent.length;
    }
    if (node.nodeType === Node.ELEMENT_NODE) {
        if (node.classList && node.classList.contains('inline-tag')) {
            return node.textContent.length;
        }
        if (node.tagName === 'BR') {
            return 0;
        }
        let sum = 0;
        for (let i = 0; i < node.childNodes.length; i++) {
            sum += _measureNodeChars(node.childNodes[i]);
        }
        return sum;
    }
    return 0;
}

// 计算 (container, offset) 边界在 root 内的字符偏移（遵循统一计数规则）
function _getCharOffset(root, container, offset) {
    let count = 0;
    let done = false;

    function walk(node) {
        if (done) return;

        if (node === container) {
            if (node.nodeType === Node.TEXT_NODE) {
                count += offset;
            } else {
                // 元素容器：offset 为子节点索引，累加索引之前所有子节点的字符数
                for (let i = 0; i < offset && i < node.childNodes.length; i++) {
                    count += _measureNodeChars(node.childNodes[i]);
                }
            }
            done = true;
            return;
        }

        if (node.nodeType === Node.TEXT_NODE) {
            count += node.textContent.length;
        } else if (node.nodeType === Node.ELEMENT_NODE) {
            if (node.classList && node.classList.contains('inline-tag')) {
                count += node.textContent.length;
            } else if (node.tagName === 'BR') {
                // 计 0
            } else {
                for (let i = 0; i < node.childNodes.length; i++) {
                    walk(node.childNodes[i]);
                    if (done) return;
                }
            }
        }
    }

    walk(root);
    return count;
}

// 计算某个节点子树的 token 数（token 规则：字符=1，tag=1，<br>=1，跳过 ZWSP）
// 必须与 setDeletionMarks 的 token 编号规则一致
function _measureNodeTokens(node) {
    const zwsp = AppState.zwsp;
    if (node.nodeType === Node.TEXT_NODE) {
        let cnt = 0;
        const text = node.textContent;
        for (let i = 0; i < text.length; i++) {
            if (text[i] !== zwsp) cnt++;
        }
        return cnt;
    }
    if (node.nodeType === Node.ELEMENT_NODE) {
        if (node.classList && node.classList.contains('inline-tag')) {
            return 1;
        }
        if (node.tagName === 'BR') {
            return 1;
        }
        let sum = 0;
        for (let i = 0; i < node.childNodes.length; i++) {
            sum += _measureNodeTokens(node.childNodes[i]);
        }
        return sum;
    }
    return 0;
}

// 计算 (container, offset) 边界在 root 内的 token 位置（光标前的 token 数）
function _getCaretTokenPosition(root, container, offset) {
    const zwsp = AppState.zwsp;
    let count = 0;
    let done = false;

    function walk(node) {
        if (done) return;

        if (node === container) {
            if (node.nodeType === Node.TEXT_NODE) {
                // 累加该文本节点中 offset 之前的非 ZWSP 字符
                const text = node.textContent;
                for (let i = 0; i < offset && i < text.length; i++) {
                    if (text[i] !== zwsp) count++;
                }
            } else {
                // 元素容器：offset 为子节点索引，累加索引之前所有子节点的 token 数
                for (let i = 0; i < offset && i < node.childNodes.length; i++) {
                    count += _measureNodeTokens(node.childNodes[i]);
                }
            }
            done = true;
            return;
        }

        if (node.nodeType === Node.TEXT_NODE) {
            const text = node.textContent;
            for (let i = 0; i < text.length; i++) {
                if (text[i] !== zwsp) count++;
            }
        } else if (node.nodeType === Node.ELEMENT_NODE) {
            if (node.classList && node.classList.contains('inline-tag')) {
                count += 1;
            } else if (node.tagName === 'BR') {
                count += 1;
            } else {
                for (let i = 0; i < node.childNodes.length; i++) {
                    walk(node.childNodes[i]);
                    if (done) return;
                }
            }
        }
    }

    walk(root);
    return count;
}

// 获取当前光标的 token 位置（无选区/不在编辑器内返回 -1）
function getCaretTokenPosition() {
    const inputEditor = document.getElementById('inputEditor');
    if (!inputEditor) return -1;

    const selection = window.getSelection();
    if (!selection.rangeCount) return -1;

    const range = selection.getRangeAt(0);
    if (!inputEditor.contains(range.endContainer)) return -1;

    return _getCaretTokenPosition(inputEditor, range.endContainer, range.endOffset);
}



// 保存编辑器内当前光标（按 inputEditor 内的字符偏移计）
function _saveEditorCaret() {
    const inputEditor = document.getElementById('inputEditor');
    if (!inputEditor) return null;

    const selection = window.getSelection();
    if (!selection.rangeCount) return null;

    const range = selection.getRangeAt(0);
    if (!inputEditor.contains(range.startContainer) || !inputEditor.contains(range.endContainer)) {
        return null;
    }

    const startOffset = _getCharOffset(inputEditor, range.startContainer, range.startOffset);
    const endOffset = _getCharOffset(inputEditor, range.endContainer, range.endOffset);

    return { start: startOffset, end: endOffset };
}


// 在 root 内定位字符偏移对应的 (node, offset)
// 计数规则必须与 _getCharOffset 完全一致
function _locateCaret(root, targetOffset) {
    let remaining = targetOffset;
    let found = null;

    function walk(node) {
        if (found) return;
        for (let i = 0; i < node.childNodes.length; i++) {
            if (found) return;
            const child = node.childNodes[i];
            if (child.nodeType === Node.TEXT_NODE) {
                const len = child.textContent.length;
                if (remaining <= len) {
                    found = { node: child, offset: remaining };
                    return;
                }
                remaining -= len;
            } else if (child.nodeType === Node.ELEMENT_NODE) {
                if (child.classList && child.classList.contains('inline-tag')) {
                    // tag 作为原子单位：光标不进入内部，落在其前/后边界
                    const len = child.textContent.length;
                    if (remaining < len) {
                        // 落在 tag 前边界（remaining==0）或后边界
                        found = { node: node, offset: (remaining === 0) ? i : i + 1 };
                        return;
                    } else if (remaining === len) {
                        found = { node: node, offset: i + 1 };
                        return;
                    }
                    remaining -= len;
                } else if (child.tagName === 'BR') {
                    // 计 0，不递减；若偏移正好到此，落在 <br> 前
                    if (remaining === 0) {
                        found = { node: node, offset: i };
                        return;
                    }
                } else {
                    // 普通元素：递归其子节点
                    walk(child);
                }
            }
        }
    }

    walk(root);
    // 未命中（偏移超出末尾）：落到最后一个可用位置
    if (!found) {
        found = { node: root, offset: root.childNodes.length };
    }
    return found;
}


// 在 root 内定位 token 偏移对应的 (node, offset)
// 计数规则与 _getCaretTokenPosition 一致：字符=1，tag=1，<br>=1，跳过 ZWSP
function _locateCaretByToken(root, targetToken) {
    const zwsp = AppState.zwsp;
    let remaining = targetToken;
    let found = null;

    function walk(node) {
        if (found) return;
        for (let i = 0; i < node.childNodes.length; i++) {
            if (found) return;
            const child = node.childNodes[i];
            if (child.nodeType === Node.TEXT_NODE) {
                // 统计非 ZWSP 字符数
                const text = child.textContent;
                let tokenCount = 0;
                for (let j = 0; j < text.length; j++) {
                    if (text[j] !== zwsp) tokenCount++;
                }
                if (remaining < tokenCount) {
                    // 落在此文本节点内，找到第 remaining 个非 ZWSP 字符的位置
                    let nonZwspIndex = 0;
                    let charOffset = 0;
                    for (let j = 0; j < text.length; j++) {
                        if (text[j] !== zwsp) {
                            if (nonZwspIndex === remaining) {
                                found = { node: child, offset: j };
                                return;
                            }
                            nonZwspIndex++;
                        }
                        charOffset = j + 1;
                    }
                    // 不应该到达这里
                    found = { node: child, offset: text.length };
                    return;
                } else if (remaining === tokenCount) {
                    // 落在此文本节点末尾
                    found = { node: child, offset: text.length };
                    return;
                }
                remaining -= tokenCount;
            } else if (child.nodeType === Node.ELEMENT_NODE) {
                if (child.classList && child.classList.contains('inline-tag')) {
                    // tag 作为原子单位，计 1 token
                    if (remaining === 0) {
                        // 落在 tag 前边界
                        found = { node: node, offset: i };
                        return;
                    } else if (remaining === 1) {
                        // 落在 tag 后边界
                        found = { node: node, offset: i + 1 };
                        return;
                    }
                    remaining -= 1;
                } else if (child.tagName === 'BR') {
                    // <br> 计 1 token
                    if (remaining === 0) {
                        found = { node: node, offset: i };
                        return;
                    }
                    remaining -= 1;
                } else {
                    // 普通元素：递归其子节点
                    walk(child);
                }
            }
        }
    }

    walk(root);
    // 未命中（偏移超出末尾）：落到最后一个可用位置
    if (!found) {
        found = { node: root, offset: root.childNodes.length };
    }
    return found;
}


// 恢复之前保存的光标
function _restoreEditorCaret(saved) {
    if (!saved) return;

    const inputEditor = document.getElementById('inputEditor');
    if (!inputEditor) return;

    const startPos = _locateCaret(inputEditor, saved.start);
    const endPos = _locateCaret(inputEditor, saved.end);
    if (!startPos || !endPos) return;

    const selection = window.getSelection();
    const range = document.createRange();
    try {
        range.setStart(startPos.node, startPos.offset);
        range.setEnd(endPos.node, endPos.offset);
        selection.removeAllRanges();
        selection.addRange(range);
    } catch (e) {
        // 定位失败时忽略，保持当前选区
    }
}

// 设置删除标记 token 索引列表
// 每个 type:'text' 项的每个字符算1个token，每个 type:'tag' 项算1个token
function setDeletionMarks(indices) {
    const inputEditor = document.getElementById('inputEditor');
    if (!inputEditor) return;

    // 保存光标，改动 DOM 后恢复，避免光标跳到行首
    const savedCaret = _saveEditorCaret();

    // 先清除之前的标记
    clearDeletionMarks();

    if (!indices || indices.length === 0) {
        _restoreEditorCaret(savedCaret);
        return;
    }
    
    // 构建 token 索引集合，方便 O(1) 查找
    const indexSet = new Set(indices);
    
    // 遍历 DOM 节点，为每个 token 分配索引
    let tokenIndex = 0;
    const zwsp = AppState.zwsp;
    
    function processNode(node) {
        if (node.nodeType === Node.TEXT_NODE) {
            const text = node.textContent;
            let i = 0;
            const textLen = text.length;
            
            // 收集需要标记为删除的字符范围
            const ranges = [];
            let rangeStart = -1;
            
            while (i < textLen) {
                // 跳过 ZWSP
                if (text[i] === zwsp) {
                    i++;
                    continue;
                }
                
                const isMarked = indexSet.has(tokenIndex);
                
                if (isMarked && rangeStart === -1) {
                    rangeStart = i;
                } else if (!isMarked && rangeStart !== -1) {
                    ranges.push({ start: rangeStart, end: i });
                    rangeStart = -1;
                }
                
                tokenIndex++;
                i++;
            }
            
            // 处理末尾的连续标记
            if (rangeStart !== -1) {
                ranges.push({ start: rangeStart, end: textLen });
            }

            // 没有需要标记的区间，保持原节点不变
            if (ranges.length === 0) {
                return;
            }

            // 一次性构建包含所有区间的片段（正常文本与 diff-deleted span 交替），整体替换原节点。
            // 注意：不能在循环里对同一个 node 多次 insert/removeChild，否则多区间时会错乱。
            const fragment = document.createDocumentFragment();
            let cursor = 0;
            for (let r = 0; r < ranges.length; r++) {
                const range = ranges[r];
                // 区间前的正常文本
                if (range.start > cursor) {
                    fragment.appendChild(document.createTextNode(text.substring(cursor, range.start)));
                }
                // 标记为删除的文本
                const span = document.createElement('span');
                span.className = 'diff-deleted';
                span.textContent = text.substring(range.start, range.end);
                fragment.appendChild(span);
                cursor = range.end;
            }
            // 最后一个区间之后的正常文本
            if (cursor < textLen) {
                fragment.appendChild(document.createTextNode(text.substring(cursor)));
            }

            const parent = node.parentNode;
            parent.insertBefore(fragment, node);
            parent.removeChild(node);

        } else if (node.nodeType === Node.ELEMENT_NODE) {
            if (node.classList && node.classList.contains('inline-tag')) {
                // tag 算作 1 个 token
                if (indexSet.has(tokenIndex)) {
                    node.classList.add('diff-deleted');
                }
                tokenIndex++;
            } else if (node.tagName === 'BR') {
                // <br> 也算作 1 个 token（对应 \n 字符）
                // 但通常不标记 <br>
                tokenIndex++;
            } else {
                // 递归处理子节点
                // 先复制一份子节点列表，因为处理过程中会修改 DOM
                const children = Array.from(node.childNodes);
                for (const child of children) {
                    processNode(child);
                }
            }
        }
    }
    
    // 处理编辑器中的所有节点
    const children = Array.from(inputEditor.childNodes);
    for (const child of children) {
        processNode(child);
    }

    // 恢复光标位置
    _restoreEditorCaret(savedCaret);
}

// 清除删除标记
function clearDeletionMarks() {
    const inputEditor = document.getElementById('inputEditor');
    if (!inputEditor) return;

    // 保存光标，改动 DOM 后恢复
    const savedCaret = _saveEditorCaret();
    
    // 移除 .inline-tag 上的 diff-deleted class
    const taggedElements = inputEditor.querySelectorAll('.inline-tag.diff-deleted');
    taggedElements.forEach(el => el.classList.remove('diff-deleted'));
    
    // 展开 .diff-deleted span，将其文本内容恢复为普通文本节点
    const spans = inputEditor.querySelectorAll('span.diff-deleted');
    spans.forEach(span => {
        const textNode = document.createTextNode(span.textContent);
        span.parentNode.replaceChild(textNode, span);
    });
    
    // 合并相邻的文本节点
    inputEditor.normalize();

    // 恢复光标位置
    _restoreEditorCaret(savedCaret);
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
window.handleCopy = handleCopy;
window.handlePaste = handlePaste;
window.handleArrowKeyNavigation = handleArrowKeyNavigation;
window.handleTagDeletion = handleTagDeletion;
window.setDeletionMarks = setDeletionMarks;
window.clearDeletionMarks = clearDeletionMarks;
window.handleSelectionChange = handleSelectionChange;