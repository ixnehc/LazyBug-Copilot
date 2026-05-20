// 自动补全功能

// 处理自动补全
function handleAutoComplete() {
    const inputEditor = document.getElementById('inputEditor');
    const selection = window.getSelection();
    
    console.log('handleAutoComplete called');
    
    if (!selection.rangeCount) {
        console.log('No selection range, hiding autocomplete');
        hideAutoComplete();
        return;
    }

    const range = selection.getRangeAt(0);
    if (!range.collapsed) {
        console.log('Range not collapsed, hiding autocomplete');
        hideAutoComplete();
        return;
    }

    const atInfo = findAtSymbolBeforeCursor(range);
    console.log('atInfo:', atInfo);
    
    if (atInfo) {
        const query = atInfo.query;
        AutoCompleteState.query = query;
        
        console.log('Found @ symbol, query:', query);
        
        const atPosition = getAtSymbolScreenPosition(range, atInfo.atPosition);
        console.log('@ position:', atPosition);
        
        if (!AutoCompleteState.isVisible) {
            console.log('Sending autoCompleteShow message');
            sendMessageToNative({
                action: 'autoCompleteShow',
                query: query,
                position: atPosition
            });
        } else {
            console.log('Sending autoCompleteUpdate message');
            sendMessageToNative({
                action: 'autoCompleteUpdate',
                query: query,
                position: atPosition
            });
        }
    } else {
        console.log('No @ symbol found, hiding autocomplete');
        hideAutoComplete();
    }
}

// 在光标前查找@符号
function findAtSymbolBeforeCursor(range) {
    let container = range.startContainer;
    let offset = range.startOffset;
    
    console.log('findAtSymbolBeforeCursor called');
    console.log('container:', container);
    console.log('offset:', offset);
    console.log('container.nodeType:', container.nodeType);
    
    if (container.nodeType === Node.ELEMENT_NODE) {
        console.log('Container is element node, finding text node...');
        console.log('childNodes count:', container.childNodes.length);
        
        const childNodes = Array.from(container.childNodes);
        console.log('childNodes:', childNodes);
        
        for (let i = childNodes.length - 1; i >= 0; i--) {
            const child = childNodes[i];
            console.log(`Checking child ${i}:`, child, 'nodeType:', child.nodeType);
            
            if (child.nodeType === Node.TEXT_NODE && child.textContent.length > 0) {
                const text = child.textContent;
                console.log(`Text node content: "${text}"`);
                
                const atIndex = text.lastIndexOf('@');
                if (atIndex !== -1) {
                    console.log('Found @ symbol in text node at position:', atIndex);
                    
                    let query = text.substring(atIndex + 1);
                    
                    for (let j = i + 1; j < childNodes.length; j++) {
                        const nextChild = childNodes[j];
                        console.log(`Checking subsequent child ${j}:`, nextChild);
                        if (nextChild.nodeType === Node.TEXT_NODE) {
                            const nextText = nextChild.textContent;
                            console.log(`Adding text from subsequent node: "${nextText}"`);
                            query += nextText;
                            if (/[\s\n\r\t]|[^\x00-\x7F]/.test(nextText)) {
                                console.log('Found non-word or non-ASCII character, stopping collection');
                                break;
                            }
                        } else {
                            console.log('Encountered non-text node, stopping collection');
                            break;
                        }
                    }
                    
                    console.log('Final query from text nodes:', JSON.stringify(query));
                    
                    return {
                        atPosition: atIndex,
                        query: query,
                        fullText: '@' + query
                    };
                }
            }
        }
        
        console.log('No @ symbol found in any text node');
        return null;
    }
    
    if (container.nodeType !== Node.TEXT_NODE) {
        console.log('Container is not a text node after processing');
        return null;
    }

    const text = container.textContent;
    console.log('text content:', JSON.stringify(text));
    console.log('text length:', text.length);
    
    let searchPos = offset - 1;
    const maxSearchDistance = 100;
    let searchDistance = 0;
    
    while (searchPos >= 0 && searchDistance < maxSearchDistance) {
        const char = text[searchPos];
        console.log(`Checking char at pos ${searchPos}: '${char}' (code: ${char.charCodeAt(0)})`);
        
        if (char === '@') {
            let query = text.substring(searchPos + 1, offset);
            console.log('Found @ symbol at position:', searchPos);
            console.log('Query (before cursor):', JSON.stringify(query));
            
            let remainingText = text.substring(offset);
            console.log('Remaining text in current node:', JSON.stringify(remainingText));
            
            const nonWordMatch = remainingText.match(/[\s\n\r\t]|[^\x00-\x7F]/);
            if (nonWordMatch) {
                query += remainingText.substring(0, nonWordMatch.index);
                console.log('Found non-word or non-ASCII char in current node, query:', JSON.stringify(query));
            } else {
                query += remainingText;
                console.log('No non-word or non-ASCII char in current node, query:', JSON.stringify(query));
                
                if (container.nextSibling && container.nextSibling.nodeType === Node.TEXT_NODE) {
                    console.log('Checking next sibling...');
                    let nextNode = container.nextSibling;
                    while (nextNode && nextNode.nodeType === Node.TEXT_NODE) {
                        const nextText = nextNode.textContent;
                        console.log(`Adding text from next sibling: "${nextText}"`);
                        
                        const nonWordMatchNext = nextText.match(/[\s\n\r\t]|[^\x00-\x7F]/);
                        if (nonWordMatchNext) {
                            query += nextText.substring(0, nonWordMatchNext.index);
                            console.log('Found non-word char, final query:', JSON.stringify(query));
                            break;
                        } else {
                            query += nextText;
                        }
                        nextNode = nextNode.nextSibling;
                    }
                }
            }
            
            console.log('Final query:', JSON.stringify(query));
            
            return {
                atPosition: searchPos,
                query: query,
                fullText: '@' + query
            };
        } else if (char === ' ' || char === '\n' || char === '\t') {
            console.log('Found word separator, stopping search');
            break;
        }
        searchPos--;
        searchDistance++;
    }

    console.log('No @ symbol found within search range (including previous siblings)');
    return null;
}

// 获取@符号屏幕位置
function getAtSymbolScreenPosition(range, atPosition) {
    try {
        const container = range.startContainer;
        
        const atRange = document.createRange();
        atRange.setStart(container, atPosition);
        atRange.setEnd(container, atPosition);
        
        const rect = atRange.getBoundingClientRect();
        
        return {
            x: Math.round(rect.left),
            y: Math.round(rect.top)
        };
    } catch (e) {
        console.error('Error calculating @ symbol position:', e);
        return { x: 0, y: 0 };
    }
}

// 处理自动补全消息
function processAutoCompleteMessage(message) {
    if (message.action === 'showAutoComplete') {
        showAutoComplete(message.items, message.position, message.config, message.selectedIndex, message.query);
    } else if (message.action === 'hideAutoComplete') {
        hideAutoComplete();
    }
}

// 更新自动补全数据
function updateAutoCompleteData(items, selectedIndex) {
    AutoCompleteState.items = items;
    AutoCompleteState.selectedIndex = selectedIndex;
    AutoCompleteState.isVisible = items.length > 0;
}

// 显示自动补全
function showAutoComplete(items, position, config, selectedIndex, query) {
    AutoCompleteState.isVisible = true;
    AutoCompleteState.items = items || [];
    AutoCompleteState.selectedIndex = selectedIndex || 0;
    AutoCompleteState.query = query || '';
    AutoCompleteState.position = position || { x: 0, y: 0 };

    const popup = document.getElementById('autoCompletePopup');
    popup.classList.remove('hidden');
    
    popup.style.left = position.x + 'px';
    popup.style.top = position.y + 'px';
    
    if (config && config.width) {
        popup.style.width = config.width + 'px';
    }

    popup.innerHTML = '';
    items.forEach((item, index) => {
        const itemElement = createAutoCompleteItem(item, index === selectedIndex);
        itemElement.addEventListener('click', () => {
            selectAutoCompleteItem(index);
            confirmAutoCompleteSelection();
        });
        popup.appendChild(itemElement);
    });

    scrollSelectedItemIntoView();
}

// 创建自动补全项
function createAutoCompleteItem(item, isSelected) {
    const itemDiv = document.createElement('div');
    itemDiv.className = 'autocomplete-item' + (isSelected ? ' selected' : '');
    
    if (item.icon) {
        const iconSpan = document.createElement('span');
        iconSpan.className = 'autocomplete-item-icon';
        iconSpan.innerHTML = item.icon;
        itemDiv.appendChild(iconSpan);
    }

    const contentDiv = document.createElement('div');
    contentDiv.className = 'autocomplete-item-content';
    
    const textDiv = document.createElement('div');
    textDiv.className = 'autocomplete-item-text';
    textDiv.textContent = item.text;
    contentDiv.appendChild(textDiv);
    
    if (item.description) {
        const descDiv = document.createElement('div');
        descDiv.className = 'autocomplete-item-description';
        descDiv.textContent = item.description;
        contentDiv.appendChild(descDiv);
    }
    
    itemDiv.appendChild(contentDiv);
    return itemDiv;
}

// 隐藏自动补全
function hideAutoComplete() {
    if (!AutoCompleteState.isVisible) return;
    
    AutoCompleteState.isVisible = false;
    const popup = document.getElementById('autoCompletePopup');
    popup.classList.add('hidden');
    
    sendMessageToNative({
        action: 'autoCompleteHide'
    });
}

// 选择自动补全项
function selectAutoCompleteItem(index) {
    if (index < 0 || index >= AutoCompleteState.items.length) return;
    
    AutoCompleteState.selectedIndex = index;
    updateAutoCompleteSelection();
    
    sendMessageToNative({
        action: 'autoCompleteSelect',
        index: index
    });
}

// 选择下一个自动补全项
function selectNextAutoCompleteItem() {
    if (!AutoCompleteState.isVisible || AutoCompleteState.items.length === 0) return;
    
    let newIndex = AutoCompleteState.selectedIndex + 1;
    if (newIndex >= AutoCompleteState.items.length) {
        newIndex = 0;
    }
    selectAutoCompleteItem(newIndex);
}

// 选择上一个自动补全项
function selectPreviousAutoCompleteItem() {
    if (!AutoCompleteState.isVisible || AutoCompleteState.items.length === 0) return;
    
    let newIndex = AutoCompleteState.selectedIndex - 1;
    if (newIndex < 0) {
        newIndex = AutoCompleteState.items.length - 1;
    }
    selectAutoCompleteItem(newIndex);
}

// 确认自动补全选择
function confirmAutoCompleteSelection() {
    if (!AutoCompleteState.isVisible || 
        AutoCompleteState.selectedIndex < 0 || 
        AutoCompleteState.selectedIndex >= AutoCompleteState.items.length) {
        return;
    }

    sendMessageToNative({
        action: 'autoCompleteConfirm'
    });
}

// 更新自动补全选择
function updateAutoCompleteSelection() {
    const popup = document.getElementById('autoCompletePopup');
    const items = popup.querySelectorAll('.autocomplete-item');
    
    items.forEach((item, index) => {
        if (index === AutoCompleteState.selectedIndex) {
            item.classList.add('selected');
        } else {
            item.classList.remove('selected');
        }
    });

    scrollSelectedItemIntoView();
}

// 滚动选中项到视图
function scrollSelectedItemIntoView() {
    const popup = document.getElementById('autoCompletePopup');
    const selectedItem = popup.querySelector('.autocomplete-item.selected');
    
    if (selectedItem) {
        selectedItem.scrollIntoView({
            block: 'nearest'
        });
    }
}

// 获取光标位置
function getCursorPosition() {
    const selection = window.getSelection();
    if (!selection.rangeCount) {
        return JSON.stringify({ x: 0, y: 0 });
    }

    const range = selection.getRangeAt(0);
    const rect = range.getBoundingClientRect();
    
    return JSON.stringify({
        x: Math.round(rect.left),
        y: Math.round(rect.bottom)
    });
}

// 替换自动补全为标签
// 根据 tagData 生成待插入的 HTML 片段（带两侧 ZWSP）
function _buildTagHtml(tagData) {
    const tagElement = createInlineTagElement({
        id: tagData.id || ('ac_' + Date.now()),
        text: tagData.text,
        type: tagData.type || 'autocomplete',
        data: tagData.data || '',
        imgSrc: tagData.imgSrc || '',
        removable: true
    });
    const zwsp = AppState.zwsp;
    return zwsp + tagElement.outerHTML + zwsp;
}

function replaceAutoCompleteWithTag(prefix, tagData) {
    const inputEditor = document.getElementById('inputEditor');
    
    if (!inputEditor) return;
    
    const textNodes = [];
    const walk = document.createTreeWalker(inputEditor, NodeFilter.SHOW_TEXT);
    let node;
    while ((node = walk.nextNode())) {
        textNodes.push(node);
    }
    
    for (let i = 0; i < textNodes.length; i++) {
        const textNode = textNodes[i];
        const text = textNode.textContent;
        
        const atPos = text.lastIndexOf('@');
        if (atPos !== -1) {
            const afterAt = text.substring(atPos + 1);
            
            let accumulatedText = afterAt;
            let matchEndNode = textNode;
            let matchEndOffset = text.length;
            let j = i + 1;
            
            while (j < textNodes.length && accumulatedText.length < prefix.length) {
                const nextTextNode = textNodes[j];
                if (isTextNodeContinuous(matchEndNode, nextTextNode)) {
                    accumulatedText += nextTextNode.textContent;
                    matchEndNode = nextTextNode;
                    matchEndOffset = nextTextNode.textContent.length;
                    j++;
                } else {
                    break;
                }
            }
            
            if (prefix.indexOf(accumulatedText) === 0 || accumulatedText.indexOf(prefix) === 0) {
                if (matchEndNode !== textNode) {
                    replaceAcrossNodes(textNode, atPos, matchEndNode, matchEndOffset, prefix, tagData);
                    return;
                }
                break;
            }
        }
    }
    
    const selection = window.getSelection();
    if (!selection.rangeCount) return;
    
    const range = selection.getRangeAt(0);
    const container = range.startContainer;
    
    if (container.nodeType !== Node.TEXT_NODE) return;

    const text = container.textContent;
    const offset = range.startOffset;
    
    let atPos = -1;
    
    if (prefix.length > 0) {
        const searchText = text.substring(0, offset);
        const targetPattern = '@' + prefix;
        atPos = searchText.lastIndexOf(targetPattern);
        
        if (atPos === -1) {
            atPos = searchText.lastIndexOf('@');
        }
    } else {
        atPos = text.lastIndexOf('@', offset - 1);
    }
    
    if (atPos === -1) {
        atPos = text.lastIndexOf('@');
        if (atPos === -1) return;
        
        const afterAt = text.substring(atPos + 1);
        if (prefix.length > 0 && afterAt.indexOf(prefix) !== 0) {
            return;
        }
    }

    // 用 Range 精确选中 "@prefix"，再通过 execCommand('insertHTML') 替换，
    // 使浏览器将整个"删除触发词 + 插入标签"记录为一条 undo 操作。
    const replaceRange = document.createRange();
    replaceRange.setStart(container, atPos);
    replaceRange.setEnd(container, atPos + 1 + prefix.length);
    selection.removeAllRanges();
    selection.addRange(replaceRange);

    document.execCommand('insertHTML', false, _buildTagHtml(tagData));
    
    hideAutoComplete();
    notifyContentChanged();
}

// 跨节点替换
function replaceAcrossNodes(startNode, startOffset, endNode, endOffset, prefix, tagData) {
    const selection = window.getSelection();

    // 用 Range 精确选中跨节点的 "@prefix"，再通过 execCommand('insertHTML') 替换，
    // 使浏览器将整个"删除触发词 + 插入标签"记录为一条 undo 操作。
    const replaceRange = document.createRange();
    replaceRange.setStart(startNode, startOffset);
    replaceRange.setEnd(endNode, endOffset);
    selection.removeAllRanges();
    selection.addRange(replaceRange);

    document.execCommand('insertHTML', false, _buildTagHtml(tagData));
    
    hideAutoComplete();
    notifyContentChanged();
}

// 导出到全局
window.handleAutoComplete = handleAutoComplete;
window.findAtSymbolBeforeCursor = findAtSymbolBeforeCursor;
window.getAtSymbolScreenPosition = getAtSymbolScreenPosition;
window.processAutoCompleteMessage = processAutoCompleteMessage;
window.updateAutoCompleteData = updateAutoCompleteData;
window.showAutoComplete = showAutoComplete;
window.createAutoCompleteItem = createAutoCompleteItem;
window.hideAutoComplete = hideAutoComplete;
window.selectAutoCompleteItem = selectAutoCompleteItem;
window.selectNextAutoCompleteItem = selectNextAutoCompleteItem;
window.selectPreviousAutoCompleteItem = selectPreviousAutoCompleteItem;
window.confirmAutoCompleteSelection = confirmAutoCompleteSelection;
window.updateAutoCompleteSelection = updateAutoCompleteSelection;
window.scrollSelectedItemIntoView = scrollSelectedItemIntoView;
window.getCursorPosition = getCursorPosition;
window.replaceAutoCompleteWithTag = replaceAutoCompleteWithTag;
window.replaceAcrossNodes = replaceAcrossNodes;