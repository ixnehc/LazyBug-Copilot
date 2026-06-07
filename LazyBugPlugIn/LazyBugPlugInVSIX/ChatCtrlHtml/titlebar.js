// ====== 标题栏控制模块 ======

// 常量定义
const DEFAULT_CHAT_TITLE = '[ Untitled Chat ]';

// DOM 元素引用
let webviewTitlebar, webviewTitle, webviewSettingsButton, webviewFavoriteListButton;

/**
 * 初始化标题栏模块
 */
function initTitlebar() {
    webviewTitlebar = document.getElementById('webview-titlebar');
    webviewTitle = document.getElementById('webview-title');
    webviewSettingsButton = document.getElementById('webview-settings-button');
    webviewFavoriteListButton = document.getElementById('webview-favorite-list-button');
    
    setupTitlebarEvents();
}

/**
 * 设置标题栏事件监听
 */
function setupTitlebarEvents() {
    // 标题栏点击事件
    webviewTitlebar.onclick = (e) => {
        e.stopPropagation();
        // 发送消息请求更新菜单内容并显示
        window.chrome.webview.postMessage({
            action: 'titlebarClicked'
        });
    };

    // 设置按钮点击事件
    webviewSettingsButton.onclick = (e) => {
        e.stopPropagation();
        window.chrome.webview.postMessage({
            action: 'settingsButtonClicked'
        });
    };

    // Favorite列表按钮点击事件
    webviewFavoriteListButton.onclick = (e) => {
        e.stopPropagation();
        window.chrome.webview.postMessage({
            action: 'favoriteListButtonClicked'
        });
    };
}

/**
 * 设置 WebView 标题
 * @param {string} title - 标题文本
 */
function setWebViewTitle(title) {
    webviewTitle.textContent = title || DEFAULT_CHAT_TITLE;
}

/**
 * 隐藏标题栏菜单（占位函数，实际菜单逻辑在 C++ 端）
 */
function hideTitlebarMenu() {
    // 实际菜单显示/隐藏逻辑在 C++ 端处理
    // 这里可以添加额外的清理逻辑
}