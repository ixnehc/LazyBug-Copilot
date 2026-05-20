#pragma once

#include "ChatInputACWindow.h"


class CChatInputACList
{
public:
    // 构造函数和析构函数
    CChatInputACList();
    virtual ~CChatInputACList();

    bool Initialize(CChatInput* pChatInput);
    void Destroy();
	void Update();

    // ===== 显示控制 =====
    
    // 显示自动补全列表
    void Show(const std::string& query = "", int x = -1, int y = -1);
    void Hide();
    bool IsVisible() const { return _isVisible; }
    
    // 获取关联的ChatInput指针
    CChatInput* GetChatInput() const { return _pChatInput; }

    // ===== 数据管理 =====
    
    // 候选项列表
    void SetItems(const std::vector<ChatInputACItem>& items);
    void ClearItems();

    // ===== 查询和过滤 =====
    void UpdateQuery(const std::string& query);
    const std::string& GetQuery() const { return _currentQuery; }

    // ===== 选择操作 =====
    void ConfirmSelection();
    void CancelSelection();
    void SetSelectedIndex(int index);
    int GetSelectedIndex() const { return _selectedIndex; }

    // ===== 位置设置 =====
    
    void SetAnchorPos(int webViewX, int webViewY);// 设置列表位置（从WebView坐标转换）

    // ===== 回调设置 =====
    void SetItemSelectedCallback(ACItemSelectedCallback callback);// 设置项目选中回调
    void SetListCancelledCallback(ACListCancelledCallback callback);// 设置列表取消回调

    // ===== 配置选项 =====
    void SetMaxVisibleItems(int maxItems) { _maxVisibleItems = maxItems; }// 设置最大显示项数
    void SetListWidth(int width) { _listWidth = width; }// 设置列表宽度

private:
    // 关联的ChatInput
    CChatInput* _pChatInput;
    
    // 状态标志
    bool _isVisible;
    bool _isInitialized;
    
    // 候选项数据
    std::vector<ChatInputACItem> _sortedItems;
    int _selectedIndex; // 选中项在过滤列表中的索引
    
    // 查询相关
    std::string _currentQuery;
    
    // 锚点坐标（ChatInput窗口内的局部坐标）
    int _anchorX;
    int _anchorY;

    // 配置选项
    int _maxVisibleItems;
    int _listWidth;
    
    // 回调函数
    ACItemSelectedCallback _itemSelectedCallback;
    ACListCancelledCallback _listCancelledCallback;
    
    // 原生窗口
    CChatInputACWindow _acWindow;

	//query相关
	void _RequestItems(const std::string& query);
	CChatInputACListBuilder _builder;
    
    // 过滤候选项
    void _FilterItems();
    
    // 计算匹配分数
    int _CalculateMatchScore(const std::string& text, const std::string& query);
    
    // 更新窗口位置和大小
    void _UpdateWindowPosition();
    
    // 检查是否已初始化
    bool _IsReady() const { return _isInitialized && _pChatInput != nullptr; }
}; 