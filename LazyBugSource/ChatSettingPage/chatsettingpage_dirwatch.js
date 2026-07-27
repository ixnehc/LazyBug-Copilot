// ===== DirWatch Tab - JavaScript =====

// DirWatch 数据缓存
let cachedDirWatchData = null;

// 展开的目录路径集合（默认全部折叠）
let expandedDirWatchPaths = new Set();

// 辅助：发送消息到 C++
function postDirWatchMsg(obj) {
    if (window.chrome && window.chrome.webview)
        window.chrome.webview.postMessage(obj);
}

// 请求 DirWatch 数据
function requestDirWatchData() {
    postDirWatchMsg({ action: 'requestDirWatchData' });
}

// 处理 setDirWatchData 消息（由 HTML 的 message listener 调用）
function setDirWatchData(data) {
    // DB 未打开时清空展开状态
    if (!data || !data.dbReady) {
        expandedDirWatchPaths.clear();
    }

    // 处理 justChanged 标记：新增或修改路径的 entry 自动展开
    if (data && data.entries) {
        data.entries.forEach(entry => {
            if (entry.justChanged && entry.path) {
                expandedDirWatchPaths.add(entry.path);
            }
        });
    }

    cachedDirWatchData = data;
    renderDirWatchEntries();
}

// 创建 DirWatch Tab 内容
function createDirWatchTabContent(contentDiv) {
    contentDiv.innerHTML = `
        <div class="dirwatch-container" id="dirwatch-container">
            <div class="dirwatch-toolbar" id="dirwatch-toolbar" style="display:flex;justify-content:center;align-items:center;">
                <div class="add-provider-button" onclick="onAddDirWatchEntry()" title="Add new folder">+</div>
            </div>
            <div class="dirwatch-entries" id="dirwatch-entries">
            </div>
        </div>
    `;
}

// 渲染所有条目
function renderDirWatchEntries() {
    const toolbar = document.getElementById('dirwatch-toolbar');
    const container = document.getElementById('dirwatch-entries');
    if (!container) return;

    const data = cachedDirWatchData;
    if (!data || !data.dbReady) {
        if (toolbar) toolbar.style.display = 'none';
        container.innerHTML = `
            <div class="dirwatch-empty">
                <div class="dirwatch-empty-icon">📁</div>
                <div class="dirwatch-empty-text">Database is not opened</div>
                <div class="dirwatch-empty-subtext">Please open a database first to use Folder Watch</div>
            </div>
        `;
        return;
    }

    if (toolbar) toolbar.style.display = '';

    const entries = data.entries || [];

    if (entries.length === 0) {
        container.innerHTML = `
            <div class="dirwatch-empty">
                <div class="dirwatch-empty-icon">📁</div>
                <div class="dirwatch-empty-text">No folders watched</div>
                <div class="dirwatch-empty-subtext">Click the button above to add a folder to watch</div>
            </div>
        `;
        return;
    }

    // 清理已不存在的路径
    const currentPaths = new Set(entries.map(e => e.path || ''));
    for (const p of expandedDirWatchPaths) {
        if (!currentPaths.has(p))
            expandedDirWatchPaths.delete(p);
    }

    let html = '';
    entries.forEach(entry => {
        html += buildEntryCard(entry);
    });
    container.innerHTML = html;

    // 恢复展开状态：默认全部折叠，仅展开 expandedDirWatchPaths 中的路径
    const newCards = container.querySelectorAll('.dirwatch-card');
    newCards.forEach(card => {
        const p = card.getAttribute('data-path');
        if (!expandedDirWatchPaths.has(p)) {
            const body = card.querySelector('.dirwatch-card-body');
            if (body) body.style.display = 'none';
        }
    });
}

// 构建单个条目卡片 HTML
function buildEntryCard(entry) {
    const path = entry.path || '';
    const scanStatus = entry.scanStatus || 'idle';
    const extButtons = entry.extButtons || [];
    const enabled = entry.enabled !== false;  // 默认为 true

    // 统计信息
    let totalExts = extButtons.length;
    let totalFiles = 0;
    extButtons.forEach(b => { totalFiles += b.count; });

    // 选中的扩展名 pill（如果 disabled，显示为灰色）
    let selectedHtml = '';
    extButtons.filter(b => b.selected).forEach(btn => {
        selectedHtml += `<span class="dirwatch-selected-pill">${escHtml(btn.ext)}</span>`;
    });
    if (!selectedHtml) {
        selectedHtml = '<span class="dirwatch-selected-pill none">none</span>';
    }

    // 展开区域：统计 + 所有扩展名按钮
    let statsHtml = '';
    if (scanStatus === 'scanning') {
        statsHtml = `<div class="dirwatch-stats scanning">Scanning... found ${totalExts} extension(s), ${totalFiles} file(s)</div>`;
    } else if (scanStatus === 'done') {
        statsHtml = `<div class="dirwatch-stats done">${totalExts} extension(s), ${totalFiles} file(s)</div>`;
    } else if (totalExts > 0) {
        statsHtml = `<div class="dirwatch-stats">${totalExts} extension(s), ${totalFiles} file(s)</div>`;
    } else {
        statsHtml = `<div class="dirwatch-stats">Waiting...</div>`;
    }

    let buttonsHtml = '';
    extButtons.forEach(btn => {
        const cls = btn.selected ? 'dirwatch-ext-btn selected' : 'dirwatch-ext-btn';
        buttonsHtml += `
            <button class="${cls}" data-ext="${escHtml(btn.ext)}" onclick="onToggleExt(this)" title="${btn.selected ? 'Click to deselect' : 'Click to select'}">
                ${escHtml(btn.ext)}<span class="ext-count">${btn.count}</span>
            </button>`;
    });

    // enable checkbox
    const enableToggleHtml = `
        <input type="checkbox" class="dirwatch-enable-checkbox" ${enabled ? 'checked' : ''} 
               onchange="onToggleEnabled(this)" 
               title="${enabled ? 'Disable this folder' : 'Enable this folder'}"
               onclick="event.stopPropagation()">
    `;

    // SVG 图标（与 API 页面按钮风格一致）
    const rescanSvg = `<svg viewBox="0 0 24 24" stroke-linecap="round" stroke-linejoin="round"><path d="M21.5 2v6h-6M2.5 22v-6h6M2 11.5a10 10 0 0 1 18.8-4.3M22 12.5a10 10 0 0 1-18.8 4.3"/></svg>`;
    const folderSvg = `<svg viewBox="0 0 24 24" stroke-linecap="round" stroke-linejoin="round"><path d="M22 19a2 2 0 0 1-2 2H4a2 2 0 0 1-2-2V5a2 2 0 0 1 2-2h5l2 3h9a2 2 0 0 1 2 2z"/></svg>`;

    return `
        <div class="dirwatch-card" data-path="${escHtml(path)}">
            <div class="dirwatch-card-title" onclick="toggleDirWatchCard(this)" title="Click to expand/collapse">
                <div class="dirwatch-title-row">
                    ${enableToggleHtml}
                    <span class="dirwatch-card-path">${escHtml(path)}</span>
                    <div class="dirwatch-card-actions" onclick="event.stopPropagation()">
                        <button class="dirwatch-action-btn" onclick="onRescanDirWatchEntry(this)" title="Rescan folder">${rescanSvg}</button>
                        <button class="dirwatch-action-btn" onclick="onEditDirWatchEntry(this)" title="Change folder">${folderSvg}</button>
                        <div class="delete-provider-btn" onclick="onDeleteDirWatchEntry(this)" title="Remove folder"></div>
                    </div>
                </div>
                <div class="dirwatch-selected-row">
                    <span class="dirwatch-selected-summary">${selectedHtml}</span>
                </div>
            </div>
            <div class="dirwatch-card-body">
                ${statsHtml}
                <div class="dirwatch-ext-buttons">
                    ${buttonsHtml || '<span class="dirwatch-no-files">(no files)</span>'}
                </div>
            </div>
        </div>
    `;
}

// 展开/收起卡片
function toggleDirWatchCard(titleEl) {
    const card = titleEl.closest('.dirwatch-card');
    const body = card.querySelector('.dirwatch-card-body');
    const path = card.getAttribute('data-path');
    if (body.style.display === 'none') {
        body.style.display = '';
        expandedDirWatchPaths.add(path);
    } else {
        body.style.display = 'none';
        expandedDirWatchPaths.delete(path);
    }
}

// HTML 转义
function escHtml(str) {
    if (!str) return '';
    return str.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;').replace(/'/g, '&#39;');
}

// 从元素向上查找所属的 dirwatch-card，读取 data-path 属性
function getCardPath(el) {
    const card = el.closest('.dirwatch-card');
    return card ? card.getAttribute('data-path') : '';
}

// ===== 事件处理 =====

// 新增目录
function onAddDirWatchEntry() {
    postDirWatchMsg({ action: 'pickDirWatchFolder' });
}

// 修改目录
function onEditDirWatchEntry(el) {
    const path = getCardPath(el);
    postDirWatchMsg({ action: 'pickDirWatchFolder', oldPath: path });
}

// 重新扫描目录
function onRescanDirWatchEntry(el) {
    const path = getCardPath(el);
    // 展开卡片
    expandedDirWatchPaths.add(path);
    const card = el.closest('.dirwatch-card');
    const body = card.querySelector('.dirwatch-card-body');
    if (body) body.style.display = '';
    postDirWatchMsg({ action: 'rescanDirWatchEntry', path: path });
}
 
// 删除目录
function onDeleteDirWatchEntry(el) {
    const path = getCardPath(el);
    showConfirmDialog(
        'Are you sure you want to remove this folder?',
        path,
        function() {
            postDirWatchMsg({ action: 'deleteDirWatchEntry', path: path });
        }
    );
}

// 深色自定义确认对话框
function showConfirmDialog(title, message, onConfirm) {
    // 移除可能已存在的旧对话框
    const old = document.querySelector('.custom-confirm-overlay');
    if (old) old.remove();

    const overlay = document.createElement('div');
    overlay.className = 'custom-confirm-overlay';

    // 点击遮罩关闭
    overlay.addEventListener('click', function(e) {
        if (e.target === overlay) overlay.remove();
    });

    const dialog = document.createElement('div');
    dialog.className = 'custom-confirm-dialog';

    dialog.innerHTML = `
        <div class="custom-confirm-title">${escHtml(title)}</div>
        <div class="custom-confirm-message">${escHtml(message)}</div>
        <div class="custom-confirm-actions">
            <button class="custom-confirm-cancel">Cancel</button>
            <button class="custom-confirm-ok">Remove</button>
        </div>
    `;

    overlay.appendChild(dialog);
    document.body.appendChild(overlay);

    // 按钮事件
    dialog.querySelector('.custom-confirm-cancel').addEventListener('click', function() {
        overlay.remove();
    });
    dialog.querySelector('.custom-confirm-ok').addEventListener('click', function() {
        overlay.remove();
        if (onConfirm) onConfirm();
    });

    // ESC 关闭
    function onKey(e) {
        if (e.key === 'Escape') {
            overlay.remove();
            document.removeEventListener('keydown', onKey);
        }
    }
    document.addEventListener('keydown', onKey);
}

// Toggle 扩展名
function onToggleExt(el) {
    const path = getCardPath(el);
    const ext = el.getAttribute('data-ext');
    postDirWatchMsg({ action: 'toggleDirWatchExtension', path: path, ext: ext });
}

// Toggle enabled 状态
function onToggleEnabled(el) {
    const path = getCardPath(el);
    const enabled = el.checked;
    postDirWatchMsg({ action: 'toggleDirWatchEnabled', path: path, enabled: enabled });
}
