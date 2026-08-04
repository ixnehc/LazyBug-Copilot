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
                    <div class="database-actions-title">Operations</div>
                    <div class="database-actions-buttons">
                        <button class="database-action-btn" onclick="onOpenDatabaseFolder()" id="btn-open-db-folder" disabled>
                            <span class="database-btn-icon">📂</span>
                            <span class="database-btn-text">Open Database Folder</span>
                        </button>
                        <button class="database-action-btn database-action-btn-danger" onclick="onClearDatabase()" id="btn-clear-db" disabled>
                            <span class="database-btn-icon">🗑️</span>
                            <span class="database-btn-text">Clear Database</span>
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
    const btnOpen = document.getElementById('btn-open-db-folder');
    const btnClear = document.getElementById('btn-clear-db');

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
        if (btnOpen) btnOpen.disabled = true;
        if (btnClear) btnClear.disabled = true;
        return;
    }

    if (btnOpen) btnOpen.disabled = false;
    if (btnClear) btnClear.disabled = false;

    if (infoSection) {
        infoSection.innerHTML = `
            <div class="database-info-card">
                <div class="database-info-item">
                    <span class="database-info-label">Database Location</span>
                    <span class="database-info-value database-path">${escHtml(data.dbFolderPath || '')}</span>
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

// 清空数据库
function onClearDatabase() {
    showDatabaseConfirmDialog(
        'Clear Database',
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
