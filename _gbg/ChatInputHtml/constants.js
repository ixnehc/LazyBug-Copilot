// 全局变量和状态
const AppState = {
    isInitialized: false,
    currentTags: [],
    currentToolButtons: [],
    zwsp: '\u200B',
    isPasting: false  // paste 期间屏蔽 ensureTagIntegrity，防止其 DOM 操作污染 undo 栈
};



// 自动补全状态
const AutoCompleteState = {
    isVisible: false,
    items: [],
    selectedIndex: -1,
    query: '',
    position: { x: 0, y: 0 }
};

// 导出到全局
window.AppState = AppState;
window.AutoCompleteState = AutoCompleteState;