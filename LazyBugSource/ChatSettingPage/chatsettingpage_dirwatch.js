// ===== DirWatch Tab - JavaScript =====

// DirWatch 数据缓存
let cachedDirWatchData = null;

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
    cachedDirWatchData = data;
    renderDirWatchEntries();
}

// 创建 DirWatch Tab 内容
function createDirWatchTabContent(contentDiv) {
    contentDiv.innerHTML = `
        <div class="dirwatch-container" id="dirwatch-container">
            <div class="dirwatch-toolbar">
                <button class="dirwatch-add-btn" onclick="onAddDirWatchEntry()">Add Directory</button>
            </div>
            <div class="dirwatch-entries" id="dirwatch-entries">
                <div class="dirwatch-empty">
                    <div class="dirwatch-empty-icon">📁</div>
                    <div class="dirwatch-empty-text">No directories watched</div>
                    <div class="dirwatch-empty-subtext">Click the button above to add a directory to watch</div>
                </div>
            </div>
        </div>
    `;
}

// 渲染所有条目
function renderDirWatchEntries() {
    const container = document.getElementById('dirwatch-entries');
    if (!container) return;

    const entries = cachedDirWatchData || [];

    if (entries.length === 0) {
        container.innerHTML = `
            <div class="dirwatch-empty">
                <div class="dirwatch-empty-icon">📁</div>
                <div class="dirwatch-empty-text">No directories watched</div>
                <div class="dirwatch-empty-subtext">Click the button above to add a directory to watch</div>
            </div>
        `;
        return;
    }

    let html = '';
    entries.forEach(entry => {
        html += buildEntryCard(entry);
    });
    container.innerHTML = html;
}

// 构建单个条目卡片 HTML
function buildEntryCard(entry) {
    const path = entry.path || '';
    const recursive = entry.recursive || false;
    const scanStatus = entry.scanStatus || 'idle';
    const extButtons = entry.extButtons || [];

    // 统计信息
    let totalExts = extButtons.length;
    let totalFiles = 0;
    extButtons.forEach(b => { totalFiles += b.count; });

    let statusHtml = '';
    if (scanStatus === 'scanning') {
        statusHtml = `<span class="dirwatch-card-status scanning">Scanning... found ${totalExts} extension(s), ${totalFiles} file(s)</span>`;
    } else if (scanStatus === 'done') {
        statusHtml = `<span class="dirwatch-card-status done">Done (${totalExts} extension(s), ${totalFiles} file(s))</span>`;
    } else {
        statusHtml = `<span class="dirwatch-card-status">Waiting...</span>`;
    }

    // 扩展名按钮
    let buttonsHtml = '';
    extButtons.forEach(btn => {
        const cls = btn.selected ? 'dirwatch-ext-btn selected' : 'dirwatch-ext-btn';
        buttonsHtml += `
            <button class="${cls}" onclick="onToggleExt('${escHtml(path)}', '${escHtml(btn.ext)}')" title="${btn.selected ? 'Click to deselect' : 'Click to select'}">
                ${escHtml(btn.ext)}<span class="ext-count">${btn.count}</span>
            </button>`;
    });

    return `
        <div class="dirwatch-card">
            <div class="dirwatch-card-header">
                <span class="dirwatch-card-path">${escHtml(path)}</span>
                <div class="dirwatch-card-actions">
                    <button class="dirwatch-action-btn" onclick="onEditDirWatchEntry('${escHtml(path)}')">Edit</button>
                    <button class="dirwatch-action-btn delete-btn" onclick="onDeleteDirWatchEntry('${escHtml(path)}')">Delete</button>
                    <label class="dirwatch-recursive-label">
                        <input type="checkbox" ${recursive ? 'checked' : ''} onchange="onToggleRecursive('${escHtml(path)}', this.checked)">
                        Recursive
                    </label>
                </div>
            </div>
            ${statusHtml}
            <div class="dirwatch-ext-buttons">
                ${buttonsHtml || '<span style="font-size:0.78em;color:#666;">(no files)</span>'}
            </div>
        </div>
    `;
}

// HTML 转义
function escHtml(str) {
    if (!str) return '';
    return str.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;').replace(/'/g, '&#39;');
}

// ===== 事件处理 =====

// 新增目录
function onAddDirWatchEntry() {
    postDirWatchMsg({ action: 'pickDirWatchFolder' });
}

// 修改目录
function onEditDirWatchEntry(path) {
    postDirWatchMsg({ action: 'pickDirWatchFolder', oldPath: path });
}

// 删除目录
function onDeleteDirWatchEntry(path) {
    if (!confirm('Are you sure you want to delete this directory?\n\n' + path))
        return;
    postDirWatchMsg({ action: 'deleteDirWatchEntry', path: path });
}

// Toggle 扩展名
function onToggleExt(path, ext) {
    postDirWatchMsg({ action: 'toggleDirWatchExtension', path: path, ext: ext });
}

// Toggle 递归
function onToggleRecursive(path, checked) {
    postDirWatchMsg({ action: 'updateDirWatchRecursive', path: path, recursive: checked });
}
