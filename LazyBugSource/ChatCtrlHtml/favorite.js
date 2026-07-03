// ====== Favorite 按钮模块 ======

let favoriteBtn;
let favoriteCornerMark;
let favoriteHotArea;
let isFavorite = false;

const HOT_AREA_SIZE = 40;  // 左下角热区大小（像素）

/**
 * 初始化 Favorite 按钮
 */
function initFavoriteButton() {
    favoriteBtn = document.getElementById('favorite-btn');
    favoriteCornerMark = document.getElementById('favorite-corner-mark');
    favoriteHotArea = document.getElementById('favorite-hot-area');
    if (!favoriteBtn || !favoriteCornerMark || !favoriteHotArea) return;

    // 按钮点击事件
    favoriteBtn.addEventListener('click', () => {
        toggleFavorite();
    });

    // 角落标记点击事件
    favoriteCornerMark.addEventListener('click', () => {
        toggleFavorite();
    });

    // 热区点击事件
    favoriteHotArea.addEventListener('click', () => {
        toggleFavorite();
    });

    // 鼠标移动监听（检测是否在热区）
    document.addEventListener('mousemove', handleMouseMove);

    // 初始状态
    updateButtonState();
}

/**
 * 设置 favorite 状态（由 C++ 调用）
 * @param {boolean} favorited - 是否已 favorite
 */
function setFavoriteStatus(favorited) {
    isFavorite = favorited;
    updateButtonState();
}

/**
 * 处理鼠标移动 - 检测是否在左下角热区
 */
function handleMouseMove(e) {
    const isInHotArea = (e.clientX < HOT_AREA_SIZE && e.clientY > window.innerHeight - HOT_AREA_SIZE);

    if (isInHotArea) {
        favoriteBtn.classList.add('visible');
        favoriteCornerMark.classList.add('visible');
        favoriteHotArea.classList.add('active');
    } else {
        favoriteBtn.classList.remove('visible');
        favoriteCornerMark.classList.remove('visible');
        favoriteHotArea.classList.remove('active');
    }
}

/**
 * 更新按钮状态（实心/空心）和角落标记
 */
function updateButtonState() {
    if (isFavorite) {
        favoriteBtn.classList.add('favorited');
        favoriteCornerMark.classList.add('favorited');
        favoriteHotArea.classList.add('active');
    } else {
        favoriteBtn.classList.remove('favorited');
        favoriteCornerMark.classList.remove('favorited');
        favoriteHotArea.classList.remove('active');
    }
}

/**
 * 切换 favorite 状态
 */
function toggleFavorite() {
    isFavorite = !isFavorite;
    updateButtonState();

    // 发送消息到 C++
    if (window.chrome && window.chrome.webview) {
        window.chrome.webview.postMessage({
            action: 'toggleFavorite',
            isFavorite: isFavorite
        });
    }
}

// 导出函数供 main.js 调用
window.initFavoriteButton = initFavoriteButton;
window.setFavoriteStatus = setFavoriteStatus;
