// ====== Relay Point 接力点模块 ======

// 待处理的接力点：addRelayPoint 先于 user message 到达时（回放场景）暂存
const pendingRelayPoints = new Set();

/**
 * 创建接力槽元素（默认状态 none：仅空隙，hover 显示虚线）
 * @param {string} messageId - 锚点用户消息 ID
 * @returns {HTMLElement}
 */
function createRelaySlot(messageId) {
    const slot = document.createElement('div');
    slot.className = 'relay-slot';
    slot.dataset.messageId = messageId;
    slot.dataset.state = 'none';

    slot.addEventListener('click', (e) => {
        e.stopPropagation();
        const isSet = slot.dataset.state === 'set';
        window.chrome.webview.postMessage({
            action: isSet ? 'removeRelayPoint' : 'setRelayPoint',
            messageId: messageId
        });
    });
    return slot;
}

/**
 * 通过“前一个兄弟元素”定位槽位，避免 CSS 选择器转义问题
 */
function findRelaySlot(messageId) {
    const msg = document.getElementById(messageId);
    if (!msg) return null;
    const prev = msg.previousElementSibling;
    return (prev && prev.classList.contains('relay-slot')) ? prev : null;
}

function addRelayPoint(messageId) {
    const slot = findRelaySlot(messageId);
    if (slot) {
        slot.dataset.state = 'set';
    } else {
        // user message 尚未创建，稍后由 syncPendingRelayPoint 同步
        pendingRelayPoints.add(messageId);
    }
}

function removeRelayPoint(messageId) {
    const slot = findRelaySlot(messageId);
    if (slot) slot.dataset.state = 'none';
    pendingRelayPoints.delete(messageId);
}

/**
 * 删除接力槽元素本身（锚点消息被删除时由 C++ 推送调用）
 */
function removeRelaySlot(messageId) {
    const slot = findRelaySlot(messageId);
    if (slot) slot.remove();
    pendingRelayPoints.delete(messageId);
}

/**
 * user message 刚创建后调用：若该位置有 pending 接力点，立即置为 set
 */
function syncPendingRelayPoint(messageId) {
    if (pendingRelayPoints.delete(messageId)) {
        const slot = findRelaySlot(messageId);
        if (slot) slot.dataset.state = 'set';
    }
}

function resetRelayPoints() {
    pendingRelayPoints.clear();
}
