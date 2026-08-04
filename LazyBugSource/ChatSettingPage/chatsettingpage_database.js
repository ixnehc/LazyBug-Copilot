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
                <div class="database-info-section" id="database-info-section">
                </div>
                <div class="database-actions-section">
                    <div class="database-actions-title">Clean up</div>
                    <div class="database-actions-row">
                        <select class="database-select" id="db-history-retention" disabled>
                            <option value="30" data-label="30 days">Keep last 30 days of chat history</option>
                            <option value="30" data-label="1 month">Keep 1 month of chat history</option>
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

// 渲染数据库信息
function renderDatabaseInfo() {
    const infoSection = document.getElementById('database-info-section');
    const btnClean = document.getElementById('btn-clean-db');
    const btnCleanupChat = document.getElementById('btn-cleanup-chat');
    const selRetention = document.getElementById('db-history-retention');

    const data = cachedDatabaseData;
    if (!data || !data.dbReady) {
        if (infoSection) {
            infoSection.innerHTML = `
                <div class="database-empty">
                    <div class="database-empty-icon">🗄️</div>
                    <div class="database-empty-text">Database is not opened</div>
                    <div class="database-empty-subtext">Please open a solution first to use database features</div>
                </div>
            `;
        }
        if (btnClean) btnClean.disabled = true;
        if (btnCleanupChat) btnCleanupChat.disabled = true;
        if (selRetention) selRetention.disabled = true;
        return;
    }

    if (btnClean) btnClean.disabled = false;
    if (btnCleanupChat) btnCleanupChat.disabled = false;
    if (selRetention) selRetention.disabled = false;

    if (infoSection) {
        infoSection.innerHTML = `
            <div class="database-info-card">
                <div class="database-info-item">
                    <span class="database-info-label">Database Location</span>
                    <div class="database-path-row">
                        <span class="database-path">${escHtml(data.dbFolderPath || '')}</span>
                        <button class="database-path-btn" onclick="onOpenDatabaseFolder()" title="Open folder">📂</button>
                    </div>
                </div>
            </div>
        `;
    }
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
