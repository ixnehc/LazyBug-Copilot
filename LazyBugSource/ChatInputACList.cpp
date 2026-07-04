#include "stdh.h"
#include "ChatInputACList.h"
#include "ChatInput.h"
#include <algorithm>
#include <cctype>

#include "stringparser/stringparser.h"
#include "nlohmann/json.hpp"


// 构造函数
CChatInputACList::CChatInputACList()
    : _pChatInput(nullptr)
    , _isVisible(false)
    , _isInitialized(false)
    , _selectedIndex(-1)
    , _maxVisibleItems(12)
    , _listWidth(300)
    , _anchorX(0)
    , _anchorY(0)
{
}

// 析构函数
CChatInputACList::~CChatInputACList()
{
    Destroy();
}


// 初始化（关联到ChatInput）
bool CChatInputACList::Initialize(CChatInput* pChatInput)
{
    if (pChatInput == nullptr)
        return false;
    
    _pChatInput = pChatInput;
    
    // 创建原生窗口
    if (!_acWindow.CreateACWindow(pChatInput->GetParent(),this))
        return false;

    // 设置回调
    _acWindow.SetItemSelectedCallback([this](const ChatInputACItem& item) {
        if (_itemSelectedCallback)
            _itemSelectedCallback(item);
    });
    
    _isInitialized = true;
    
    return true;
}

// 销毁
void CChatInputACList::Destroy()
{
    Hide();
    
    if (_acWindow.m_hWnd)
        _acWindow.DestroyWindow();
    _pChatInput = nullptr;
    _isInitialized = false;
    ClearItems();
}

// 显示自动补全列表
void CChatInputACList::Show(const std::string& query, int x, int y)
{
    if (!_IsReady())
        return;
    
    _currentQuery = query;
    
    // 设置锚点坐标（ChatInput窗口内的局部坐标）
    if (x >= 0 && y >= 0)
    {
        _anchorX = x;
        _anchorY = y;
    }
    
    _isVisible = true;
    _selectedIndex = 0;
    
    // 请求候选项
    _RequestItems(query);
    
    _FilterItems();
    _UpdateWindowPosition();
}

// 隐藏自动补全列表
void CChatInputACList::Hide()
{
    if (!_IsReady() || !_isVisible)
        return;
    
    _isVisible = false;
    _selectedIndex = -1;
    _currentQuery.clear();
    
    // 同步JavaScript状态
    if (_pChatInput)
    {
		if (_pChatInput->GetSafeHwnd())
		{
			nlohmann::json jsonMsg;
			jsonMsg["action"] = "hideAutoComplete";
			std::wstring msg = utf8_to_widechar(jsonMsg.dump());
			if (_pChatInput->GetCoreWebView2())
				_pChatInput->GetCoreWebView2()->PostWebMessageAsJson(msg.c_str());
		}
    }
    
    // 隐藏原生窗口
    _acWindow.HideWindow();
}

// 设置候选项列表
void CChatInputACList::SetItems(const std::vector<ChatInputACItem>& items)
{
	_sortedItems = items;
	_selectedIndex = 0;

    if (_isVisible)
    {
        _FilterItems();
        _UpdateWindowPosition(); // 立即更新窗口位置和大小
        
        // 重置滚动位置
        _acWindow.ResetScroll();
        
        // 通知JavaScript更新候选项数据
        if (_pChatInput)
        {
            nlohmann::json jsonMsg;
            jsonMsg["action"] = "updateAutoComplete";

            nlohmann::json jsonItems = nlohmann::json::array();
            for (const auto& item : _sortedItems)
            {
                nlohmann::json jsonItem;
                jsonItem["text"] = item.text;
                jsonItem["description"] = item.description;
                jsonItem["fullPath"] = item.fullPath;
                jsonItem["icon"] = "📄";
                jsonItems.push_back(jsonItem);
            }
            jsonMsg["items"] = jsonItems;
            jsonMsg["selectedIndex"] = _selectedIndex;

            std::wstring msg = utf8_to_widechar(jsonMsg.dump());
            _pChatInput->GetCoreWebView2()->PostWebMessageAsJson(msg.c_str());
        }
    }
}

// 清空候选项
void CChatInputACList::ClearItems()
{
    _sortedItems.clear();
    _selectedIndex = -1;
    
    if (_isVisible)
    {
        _UpdateWindowPosition();
    }
}

// 更新查询字符串
void CChatInputACList::UpdateQuery(const std::string& query)
{
    _currentQuery = query;
    
    // 请求新的候选项
    _RequestItems(query);
    
    _FilterItems();
    _UpdateWindowPosition();
}


// 确认当前选中项
void CChatInputACList::ConfirmSelection()
{
    if (!_isVisible || _sortedItems.empty() || _selectedIndex < 0 ||
        _selectedIndex >= static_cast<int>(_sortedItems.size()))
        return;
    
    // 获取选中的项目
    int realIndex = _selectedIndex;
    if (realIndex >= 0 && realIndex < static_cast<int>(_sortedItems.size()))
    {
        const ChatInputACItem& selectedItem = _sortedItems[realIndex];
        
        // 隐藏列表
        Hide();
        
        // 调用回调
        if (_itemSelectedCallback)
        {
            _itemSelectedCallback(selectedItem);
        }
    }
}

// 取消选择
void CChatInputACList::CancelSelection()
{
    if (!_isVisible)
        return;
    
    Hide();
    
    if (_listCancelledCallback)
    {
        _listCancelledCallback();
    }
}

// 设置选中项索引
void CChatInputACList::SetSelectedIndex(int index)
{
    if (!_isVisible || _sortedItems.empty())
        return;
    
    if (index >= 0 && index < static_cast<int>(_sortedItems.size()))
    {
        _selectedIndex = index;
        _acWindow.SetSelectedIndex(index);
        
        // 通知JavaScript更新选中索引
        if (_pChatInput)
        {
            std::wstring script = L"autoCompleteData.selectedIndex = " + std::to_wstring(index) + L";";
            _pChatInput->ExecuteScript(script);
        }
    }
}

// 设置锚点位置（从WebView坐标转换）- 保持兼容性
void CChatInputACList::SetAnchorPos(int webViewX, int webViewY)
{
    _anchorX = webViewX;
    _anchorY = webViewY;
    
    if (_isVisible)
    {
        _UpdateWindowPosition();
    }
}

// 设置回调函数
void CChatInputACList::SetItemSelectedCallback(ACItemSelectedCallback callback)
{
    _itemSelectedCallback = callback;
}

void CChatInputACList::SetListCancelledCallback(ACListCancelledCallback callback)
{
    _listCancelledCallback = callback;
}

// 过滤候选项
void CChatInputACList::_FilterItems()
{
    
    // 调整选中索引
    if (_sortedItems.empty())
    {
        _selectedIndex = -1;
    }
    else if (_selectedIndex < 0 || _selectedIndex >= static_cast<int>(_sortedItems.size()))
    {
        _selectedIndex = 0;
    }
}


// 更新窗口位置和大小
void CChatInputACList::_UpdateWindowPosition()
{
    if (!_IsReady())
        return;
    
    if (_sortedItems.empty())
    {
        _acWindow.HideWindow();
        return;
    }
    
    // 计算窗口大小
    int visibleItems = min(static_cast<int>(_sortedItems.size()), _maxVisibleItems);
    int windowHeight = visibleItems * _acWindow.GetItemHeight() + 4; // 4px边框
    CSize windowSize(_listWidth, windowHeight);
    
    // 获取ChatInput窗口的屏幕坐标
    if (!_pChatInput || !_pChatInput->GetSafeHwnd())
        return;
    
    CRect chatInputRect;
    _pChatInput->GetWindowRect(&chatInputRect);
    
    // 计算屏幕坐标
    int screenX = chatInputRect.left + _anchorX;
    int screenY = chatInputRect.top + _anchorY - windowSize.cy; // 窗口下沿对齐锚点Y
    
    // 获取屏幕工作区
    RECT workArea;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
    
    // 检查右边界是否超出屏幕
    if (screenX + windowSize.cx > workArea.right)
    {
        screenX = workArea.right - windowSize.cx;
    }
    
    // 确保不超出左边界
    if (screenX < workArea.left)
    {
        screenX = workArea.left;
    }
    
    // 检查是否超出屏幕顶部
    if (screenY < workArea.top)
    {
        // 显示在锚点下方
        screenY = chatInputRect.top + _anchorY + 20; // 锚点下方20像素
    }
    
    // 确保不超出屏幕底部
    if (screenY + windowSize.cy > workArea.bottom)
    {
        screenY = workArea.bottom - windowSize.cy;
    }
    
    // 显示原生窗口
    _acWindow.ShowWindow(_sortedItems, screenX, screenY, _selectedIndex);
}



// ChatInputACItem userItem1;
// userItem1.id = "user_001";
// userItem1.text = "OnAutoCompleteRequestItems";
// userItem1.value = "zhangsan";
// userItem1.description = "auto completion";
// userItem1.icon = "👤";
// userItem1.type = "user";
// userItem1.data = "{\"userId\":\"001\",\"department\":\"产品部\"}";
// filteredItems.push_back(userItem1);

extern CLspClient* GetLspClient();
void CChatInputACList::_RequestItems(const std::string& query)
{
	CChatInputACListBuilder::Context ctx;
	_builder.Query(query,ctx);
}

void CChatInputACList::Update()
{
	std::string query;
	std::vector<ChatInputACItem> sortedItems;
	if (_builder.Fetch(query, sortedItems))
		SetItems(sortedItems);

	_acWindow.Update();
}
