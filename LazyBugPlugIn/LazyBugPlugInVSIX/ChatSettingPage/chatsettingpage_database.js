// ===== Database Tab - JavaScript =====

// Database 数据缓存
let cachedDatabaseData = null;

// 辅助：发送消息到 C++
function postDatabaseMsg(obj) {
    if (window.chrome && window.chrome.webview)
        window.chrome.webview.postMessage(obj);
}

// 请求 Database 数据
function requestDatabaseData() {
    postDatabaseMsg({ action: 'requestDatabaseData' });
}

// 处理 setDatabaseData 消息（由 HTML 的 message listener 调用）
function setDatabaseData(data) {
    cachedDatabaseData = data;
    renderDatabaseInfo();
}

// 创建 Database Tab 内容
function createDatabaseTabContent(contentDiv) {
    contentDiv.innerHTML = `
        <div class="database-container" id="database-container">
            <div class="database-content">
                <div class="database-actions-section">
                    <div class="database-actions-title">Clean up</div>
                    <div class="database-actions-row">
                        <select class="database-select" id="db-history-retention" disabled>
                            <option value="30" data-label="1 month">Keep 1 month of chat history</option>
                            <option value="60" data-label="2 months">Keep 2 months of chat history</option>
                            <option value="90" data-label="3 months">Keep 3 months of chat history</option>
                            <option value="180" data-label="6 months">Keep 6 months of chat history</option>
                            <option value="365" data-label="1 year">Keep 1 year of chat history</option>
                        </select>
                        <button class="database-cleanup-btn" onclick="onCleanupChatHistory()" id="btn-cleanup-chat" disabled title="Clean up old chats">🧹</button>
                    </div>
                    <div class="database-actions-buttons" style="margin-top: 20px;">
                        <button class="database-action-btn database-action-btn-danger" onclick="onCleanDatabase()" id="btn-clean-db" disabled>
                            <span class="database-btn-icon">🗑️</span>
                            <span class="database-btn-text">Clean symbol &amp; index</span>
                        </button>
                    </div>
                </div>
            </div>
        </div>
    `;
}

// 根据 DB 就绪状态启用/禁用按钮
function renderDatabaseInfo() {
    const btnClean = document.getElementById('btn-clean-db');
    const btnCleanupChat = document.getElementById('btn-cleanup-chat');
    const selRetention = document.getElementById('db-history-retention');

    const data = cachedDatabaseData;
    const ready = data && data.dbReady;

    if (btnClean) btnClean.disabled = !ready;
    if (btnCleanupChat) btnCleanupChat.disabled = !ready;
    if (selRetention) selRetention.disabled = !ready;
}

// ===== 事件处理 =====

// 打开数据库目录
function onOpenDatabaseFolder() {
    postDatabaseMsg({ action: 'openDatabaseFolder' });
}

// 清理旧聊天记录
function onCleanupChatHistory() {
    const sel = document.getElementById('db-history-retention');
    const days = sel ? parseInt(sel.value) : 30;
    const shortLabel = sel ? sel.options[sel.selectedIndex].getAttribute('data-label') : '30 days';

    showDatabaseConfirmDialog(
        'Clean up chat history',
        'This will permanently delete non-favorited chat files older than <b>' + shortLabel + '</b>.<br>The 10 most recent chats and all favorited chats will be preserved.<br><br><b>Are you sure you want to continue?</b>',
        function() {
            postDatabaseMsg({ action: 'cleanupChatHistory', days: days });
        }
    );
}

// 清空数据库
function onCleanDatabase() {
    showDatabaseConfirmDialog(
        'Clean symbol &amp; index',
        'This will delete all symbol databases (indexes, defines, PCH cache, embeddings).<br>Chat history and backups will be preserved.<br><br><b>Are you sure you want to continue?</b>',
        function() {
            postDatabaseMsg({ action: 'clearDatabase' });
        }
    );
}

// 自定义确认对话框（深色主题）
function showDatabaseConfirmDialog(title, message, onConfirm) {
    const old = document.querySelector('.custom-confirm-overlay');
    if (old) old.remove();

    const overlay = document.createElement('div');
    overlay.className = 'custom-confirm-overlay';

    overlay.addEventListener('click', function(e) {
        if (e.target === overlay) overlay.remove();
    });

    const dialog = document.createElement('div');
    dialog.className = 'custom-confirm-dialog';

    dialog.innerHTML = `
        <div class="custom-confirm-title">${title}</div>
        <div class="custom-confirm-message">${message}</div>
        <div class="custom-confirm-actions">
            <button class="custom-confirm-cancel">Cancel</button>
            <button class="custom-confirm-ok">Clear</button>
        </div>
    `;

    overlay.appendChild(dialog);
    document.body.appendChild(overlay);

    dialog.querySelector('.custom-confirm-cancel').addEventListener('click', function() {
        overlay.remove();
    });
    dialog.querySelector('.custom-confirm-ok').addEventListener('click', function() {
        overlay.remove();
        if (onConfirm) onConfirm();
    });

    function onKey(e) {
        if (e.key === 'Escape') {
            overlay.remove();
            document.removeEventListener('keydown', onKey);
        }
    }
    document.addEventListener('keydown', onKey);
}

// ===== 清理遮罩 & 结果对话框 =====

function showCleanupOverlay() {
    // 移除已有遮罩
    hideCleanupOverlay();

    const overlay = document.createElement('div');
    overlay.className = 'cleanup-overlay';
    overlay.id = 'cleanup-overlay';
    overlay.innerHTML = `
        <div class="cleanup-spinner">
            <div class="cleanup-spinner-icon"></div>
            <div class="cleanup-spinner-text">Cleaning up, please wait...</div>
        </div>
    `;
    document.body.appendChild(overlay);
}

function hideCleanupOverlay() {
    const overlay = document.getElementById('cleanup-overlay');
    if (overlay) overlay.remove();
}

function showCleanupResultDialog(jsonData) {
    let data = jsonData;
    if (typeof data === 'string') {
        try { data = JSON.parse(data); } catch(e) { return; }
    }

    const title = data.success ? 'Cleanup Complete' : 'Cleanup Failed';
    const msgHtml = data.message || '';

    const old = document.querySelector('.custom-confirm-overlay');
    if (old) old.remove();

    const overlay = document.createElement('div');
    overlay.className = 'custom-confirm-overlay';

    overlay.addEventListener('click', function(e) {
        if (e.target === overlay) overlay.remove();
    });

    const dialog = document.createElement('div');
    dialog.className = 'custom-confirm-dialog';

    dialog.innerHTML = `
        <div class="custom-confirm-title">${title}</div>
        <div class="custom-confirm-message">${msgHtml}</div>
        <div class="custom-confirm-actions">
            <button class="custom-confirm-ok" style="width:100%">OK</button>
        </div>
    `;

    overlay.appendChild(dialog);
    document.body.appendChild(overlay);

    dialog.querySelector('.custom-confirm-ok').addEventListener('click', function() {
        overlay.remove();
    });

    function onKey(e) {
        if (e.key === 'Escape' || e.key === 'Enter') {
            overlay.remove();
            document.removeEventListener('keydown', onKey);
        }
    }
    document.addEventListener('keydown', onKey);
}
