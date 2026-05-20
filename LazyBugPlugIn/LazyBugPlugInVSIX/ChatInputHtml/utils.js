// 工具函数

// 防抖函数
function debounce(func, wait) {
    let timeout;
    return function executedFunction(...args) {
        const later = () => {
            clearTimeout(timeout);
            func(...args);
        };
        clearTimeout(timeout);
        timeout = setTimeout(later, wait);
    };
}

// 检查节点是否在选择范围内
function isNodeInRange(node, range) {
    try {
        if (range.intersectsNode) {
            return range.intersectsNode(node);
        }
        
        const nodeRange = document.createRange();
        nodeRange.selectNode(node);
        
        return range.compareBoundaryPoints(Range.START_TO_END, nodeRange) >= 0 &&
               range.compareBoundaryPoints(Range.END_TO_START, nodeRange) <= 0;
    } catch (e) {
        return false;
    }
}

// 获取前一个元素
function getPreviousElement(range) {
    const container = range.startContainer;
    const offset = range.startOffset;
    
    if (container.nodeType === Node.TEXT_NODE) {
        if (offset === 0 && container.previousSibling) {
            return container.previousSibling;
        }
    } else if (container.nodeType === Node.ELEMENT_NODE) {
        if (offset > 0) {
            return container.childNodes[offset - 1];
        }
    }
    return null;
}

// 获取后一个元素
function getNextElement(range) {
    const container = range.startContainer;
    const offset = range.startOffset;
    
    if (container.nodeType === Node.TEXT_NODE) {
        if (offset === container.textContent.length && container.nextSibling) {
            return container.nextSibling;
        }
    } else if (container.nodeType === Node.ELEMENT_NODE) {
        if (offset < container.childNodes.length) {
            return container.childNodes[offset];
        }
    }
    return null;
}

// 检查两个文本节点是否连续
function isTextNodeContinuous(node1, node2) {
    if (!node1 || !node2) return false;
    let current = node1.nextSibling;
    while (current) {
        if (current === node2) return true;
        if (current.nodeType === Node.TEXT_NODE && current.textContent.length > 0) {
            return false;
        }
        if (current.nodeType === Node.ELEMENT_NODE) {
            if (current.tagName === 'BR') return false;
            if (current.classList && current.classList.contains('inline-tag')) return false;
        }
        current = current.nextSibling;
    }
    return false;
}

// 导出到全局
window.debounce = debounce;
window.isNodeInRange = isNodeInRange;
window.getPreviousElement = getPreviousElement;
window.getNextElement = getNextElement;
window.isTextNodeContinuous = isTextNodeContinuous;