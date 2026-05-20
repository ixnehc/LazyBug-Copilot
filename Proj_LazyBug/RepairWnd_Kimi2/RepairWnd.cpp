#include "stdh.h"
#include "RepairWnd.h"
#include <fstream>
#include <algorithm>
#include "timer/wuid.h"
#include <nlohmann/json.hpp>
#include <afxwin.h>  // 添加MFC支持

// 外部函数声明
extern std::string widechar_to_utf8(const wchar_t* str);
extern std::wstring utf8_to_widechar(const std::string& utf8_str);
extern const wchar_t* GetWebViewUserFolder();
extern const char* GetCurModuleFolderPath();
extern std::wstring local_to_widechar(const char* str);


//////////////////////////////////////////////////////////////////////////
// CRepairWnd

BEGIN_MESSAGE_MAP(CRepairWnd, CWnd)
    ON_WM_SIZE()
    ON_WM_CREATE()
    ON_WM_DESTROY()
END_MESSAGE_MAP()

// 构造函数
CRepairWnd::CRepairWnd()
    : _isWebViewCreated(false)
    , _isContentLoaded(false)
    , _webViewEnvironment(nullptr)
    , _webView(nullptr)
    , _controller(nullptr)
{
}

// 析构函数
CRepairWnd::~CRepairWnd()
{
    // 确保COM对象在析构时正确释放
    if (_webView != nullptr && _navigationCompletedToken.value != 0)
    {
        _webView->remove_NavigationCompleted(_navigationCompletedToken);
        _navigationCompletedToken.value = 0;
    }

    if (_webView != nullptr && _webMessageReceivedToken.value != 0)
    {
        _webView->remove_WebMessageReceived(_webMessageReceivedToken);
        _webMessageReceivedToken.value = 0;
    }

    // 释放COM对象
    SAFE_RELEASE(_webView);
    SAFE_RELEASE(_controller);
    SAFE_RELEASE(_webViewEnvironment);
}

// 创建WebView2控件
BOOL CRepairWnd::Create(const RECT& rect, CWnd* pParentWnd, UINT nID)
{
    // 注册窗口类
    static CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW,
        ::LoadCursor(NULL, IDC_ARROW),
        (HBRUSH)::GetStockObject(WHITE_BRUSH),
        ::LoadIcon(NULL, IDI_APPLICATION));

    // 创建窗口
    BOOL result = CWnd::CreateEx(0, className, _T("Repair Window"),
        WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_BORDER, rect, pParentWnd, nID);

    if (result)
    {
        // 初始化WebView2环境
        HRESULT hr = InitializeWebView();
        if (FAILED(hr))
        {
            TRACE(_T("Failed to initialize WebView2 environment: 0x%08lx\n"), hr);
            return FALSE;
        }
    }

    return result;
}

// 初始化WebView2环境
HRESULT CRepairWnd::InitializeWebView()
{
    // 创建WebView2环境
    HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(nullptr, GetWebViewUserFolder(), nullptr,
        Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [this](HRESULT result, ICoreWebView2Environment* environment) -> HRESULT {
                if (SUCCEEDED(result))
                {
                    // 保存环境
                    _webViewEnvironment = environment;
                    if (_webViewEnvironment) _webViewEnvironment->AddRef();

                    // 创建WebView2控制器
                    return _webViewEnvironment->CreateCoreWebView2Controller(GetSafeHwnd(),
                        Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                            [this](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
                                if (SUCCEEDED(result))
                                {
                                    // 保存控制器
                                    _controller = controller;
                                    if (_controller) _controller->AddRef();

                                    _controller->put_IsVisible(TRUE);

                                    // 获取WebView
                                    _controller->get_CoreWebView2(&_webView);

                                    // 设置边界
                                    RECT bounds;
                                    GetClientRect(&bounds);
                                    _controller->put_Bounds(bounds);

                                    // 导航完成事件
                                    _webView->add_NavigationCompleted(
                                        Microsoft::WRL::Callback<ICoreWebView2NavigationCompletedEventHandler>(
                                            [this](ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
                                                BOOL success = FALSE;
                                                args->get_IsSuccess(&success);
                                                if (success && !_isContentLoaded)
                                                {
                                                    _isContentLoaded = true;
                                                    // 发送当前内容到WebView
                                                    _SendContentToWebView();
                                                }
                                                return S_OK;
                                            }).Get(),
                                        &_navigationCompletedToken);

    // Web消息接收事件
    _webView->add_WebMessageReceived(
        Microsoft::WRL::Callback<ICoreWebView2WebMessageReceivedEventHandler>(
            [this](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
                LPWSTR message;
                args->get_WebMessageAsJson(&message);
                if (message)
                {
                    _HandleWebMessage(message);
                    CoTaskMemFree(message);
                }
                return S_OK;
            }).Get(),
        &_webMessageReceivedToken);

                                    // 导航到HTML文件
                                    std::string htmlPath = GetCurModuleFolderPath();
                                    htmlPath += "\\RepairWnd.html";
                                    Navigate(local_to_widechar(htmlPath.c_str()));

                                    // 标记WebView创建完成
                                    _isWebViewCreated = true;
                                }
                                return S_OK;
                            }).Get());
                }
                return S_OK;
            }).Get());

    return hr;
}

// 导航到指定URL
void CRepairWnd::Navigate(const std::wstring& url)
{
    if (_webView != nullptr)
    {
        _webView->Navigate(url.c_str());
    }
}

// 调整WebView大小
void CRepairWnd::ResizeWebView()
{
    if (_controller != nullptr)
    {
        RECT bounds;
        GetClientRect(&bounds);
        _controller->put_Bounds(bounds);
    }
}

// 消息处理：大小变化
void CRepairWnd::OnSize(UINT nType, int cx, int cy)
{
    CWnd::OnSize(nType, cx, cy);
    ResizeWebView();
}

// 消息处理：创建
int CRepairWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    return 0;
}

// 消息处理：销毁
void CRepairWnd::OnDestroy()
{
    CWnd::OnDestroy();

    // 关闭WebView
    if (_controller != nullptr)
    {
        _controller->Close();
    }
}

// 处理来自WebView的消息
void CRepairWnd::_HandleWebMessage(const wchar_t* message)
{
    try
    {
        std::wstring msg(message);
        
        // 解析JSON消息
        if (msg.find(L"contentReady") != std::wstring::npos)
        {
            // 提取尺寸信息
            size_t widthPos = msg.find(L"\"width\":");
            size_t heightPos = msg.find(L"\"height\":");
            
            if (widthPos != std::wstring::npos && heightPos != std::wstring::npos)
            {
                widthPos += 8; // 跳过"width":
                heightPos += 9; // 跳过"height":
                
                size_t widthEnd = msg.find(L",", widthPos);
                size_t heightEnd = msg.find(L"}", heightPos);
                
                if (widthEnd != std::wstring::npos && heightEnd != std::wstring::npos)
                {
                    std::wstring widthStr = msg.substr(widthPos, widthEnd - widthPos);
                    std::wstring heightStr = msg.substr(heightPos, heightEnd - heightPos);
                    
                    int width = std::stoi(widthStr);
                    int height = std::stoi(heightStr);
                    
                    // 调整窗口大小和位置
                    _RepositionAndShow(width, height);
                }
            }
        }
    }
    catch (const std::exception& e)
    {
        TRACE(_T("Error handling web message: %s\n"), e.what());
    }
}

// 重新定位并显示窗口
void CRepairWnd::_RepositionAndShow(int contentWidth, int contentHeight)
{
    if (!::IsWindow(GetSafeHwnd()))
        return;

    // 获取父窗口信息
    CWnd* pParent = GetParent();
    if (!pParent || !::IsWindow(pParent->GetSafeHwnd()))
        return;

    CRect parentRect;
    pParent->GetClientRect(&parentRect);
    pParent->ClientToScreen(&parentRect);

    // 添加边距和滚动条空间
    const int padding = 40;
    const int scrollbar = 20;
    int windowWidth = contentWidth + padding + scrollbar;
    int windowHeight = contentHeight + padding;

    // 限制窗口大小
    windowWidth = max(300, min(800, windowWidth));
    windowHeight = max(200, min(600, windowHeight));

    // 计算最佳位置
    CRect bestRect;
    _CalculateBestPosition(_pendingFocusRect, parentRect, windowWidth, windowHeight, bestRect);

    // 设置窗口位置和大小
    SetWindowPos(nullptr, bestRect.left, bestRect.top, bestRect.Width(), bestRect.Height(),
        SWP_NOZORDER | SWP_NOACTIVATE);

    // 更新WebView大小
    ResizeWebView();

    // 显示窗口为top most
    ShowWindow(SW_SHOW);
    SetWindowPos(&CWnd::wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
    SetForegroundWindow();
}

//====================== 内容处理相关实现 ======================

// 设置内容
void CRepairWnd::SetContent(const CodeComparingChars& content)
{
    _currentContent = content;
    if (_IsReady())
    {
        _SendContentToWebView();
    }
}


// 清空内容
void CRepairWnd::Clear()
{
    _currentContent.content.clear();
    _currentContent.charTypes.clear();
    if (_IsReady())
    {
        _SendContentToWebView();
    }
}

// 检查WebView是否已初始化
bool CRepairWnd::_IsReady() const
{
    return _isWebViewCreated && _isContentLoaded;
}

// 发送内容到WebView
void CRepairWnd::_SendContentToWebView()
{
    if (!_IsReady())
        return;

    std::wstring contentJson = _BuildContentJson();
    _PostWebMessage(L"setContent", contentJson);
}

// 构建内容JSON
std::wstring CRepairWnd::_BuildContentJson()
{
    if (_currentContent.content.empty())
    {
        return L"{\"content\":\"\",\"highlights\":[]}";
    }

    std::wstring json = L"{";
    
    // 转义内容
    std::wstring contentW = utf8_to_widechar(_currentContent.content);
    std::wstring escapedContent = _EscapeJsonString(contentW);
    json += L"\"content\":\"" + escapedContent + L"\",";
    
    // 构建高亮信息
    json += L"\"highlights\":[";
    
    bool first = true;
    int startPos = -1;
    
    for (size_t i = 0; i < _currentContent.charTypes.size(); ++i)
    {
        CodeComparingChars::CharType type = _currentContent.charTypes[i];
        
        // 只处理NewCode和Both类型
        if (type == CodeComparingChars::NewCode || type == CodeComparingChars::Both)
        {
            if (startPos == -1)
            {
                startPos = (int)i;
            }
            
            // 检查是否需要结束当前高亮段
            if (i + 1 >= _currentContent.charTypes.size() || 
                _currentContent.charTypes[i + 1] != type)
            {
                if (!first) json += L",";
                first = false;
                
                json += L"{";
                json += L"\"start\":" + std::to_wstring(startPos) + L",";
                json += L"\"end\":" + std::to_wstring(i + 1) + L",";
                json += L"\"type\":\"" + std::wstring(type == CodeComparingChars::NewCode ? L"new" : L"both") + L"\"";
                json += L"}";
                
                startPos = -1;
            }
        }
    }
    
    json += L"]";
    json += L"}";
    
    return json;
}

// JSON转义
std::wstring CRepairWnd::_EscapeJsonString(const std::wstring& str)
{
    std::wstring result = str;
    // 简单的转义实现
    std::wstring::size_type pos = 0;
    while ((pos = result.find(L"\"", pos)) != std::wstring::npos)
    {
        result.replace(pos, 1, L"\\\"");
        pos += 2;
    }
    pos = 0;
    while ((pos = result.find(L"\\", pos)) != std::wstring::npos)
    {
        result.replace(pos, 1, L"\\\\");
        pos += 2;
    }
    pos = 0;
    while ((pos = result.find(L"\n", pos)) != std::wstring::npos)
    {
        result.replace(pos, 1, L"\\n");
        pos += 2;
    }
    pos = 0;
    while ((pos = result.find(L"\r", pos)) != std::wstring::npos)
    {
        result.replace(pos, 1, L"\\r");
        pos += 2;
    }
    return result;
}

// 内部消息发送
void CRepairWnd::_PostWebMessage(const std::wstring& action, const std::wstring& data)
{
    if (_webView == nullptr)
        return;
        
    std::wstring jsonMessage = L"{\"action\":\"" + action + L"\"";
    if (!data.empty())
    {
        jsonMessage += L",\"data\":" + data;
    }
    jsonMessage += L"}";
    
    _webView->PostWebMessageAsJson(jsonMessage.c_str());
}

void CRepairWnd::Update()
{
	// 可以在这里添加更新逻辑
}

// 显示窗口并设置内容
void CRepairWnd::Show(const char* diffStr, const RECT& focusRect)
{
	if (!diffStr || !*diffStr)
	{
		Clear();
		ShowWindow(SW_HIDE);
		return;
	}

	// 保存focusRect用于后续定位
	_pendingFocusRect = focusRect;

	// 隐藏窗口直到准备就绪
	ShowWindow(SW_HIDE);

	// 解析diffStr内容
	std::string oldCode, newCode;
	_ParseDiffString(diffStr, oldCode, newCode);

	// 生成CodeComparingChars
	CodeComparingChars comparingContent;
	MakeCodeComparing_Chars(oldCode, newCode, comparingContent);

	// 设置内容，HTML会在内容就绪后发送contentReady消息
	SetContent(comparingContent);
}

// 解析diffStr内容
void CRepairWnd::_ParseDiffString(const char* diffStr, std::string& oldCode, std::string& newCode)
{
	oldCode.clear();
	newCode.clear();

	if (!diffStr || !*diffStr)
		return;

	std::string content(diffStr);
	const std::string oldMarker = "###old lines###";
	const std::string newMarker = "###new lines###";

	// 查找标记位置
	size_t oldStart = content.find(oldMarker);
	size_t newStart = content.find(newMarker);

	if (oldStart == std::string::npos || newStart == std::string::npos)
	{
		// 如果没有标记，将整个内容作为新代码
		newCode = content;
		return;
	}

	// 提取旧代码
	oldStart += oldMarker.length();
	while (oldStart < content.length() && (content[oldStart] == '\n' || content[oldStart] == '\r'))
		oldStart++;

	// 提取新代码
	newStart += newMarker.length();
	while (newStart < content.length() && (content[newStart] == '\n' || content[newStart] == '\r'))
		newStart++;

	// 提取内容
	oldCode = content.substr(oldStart, newStart - oldStart - newMarker.length() - 1);
	newCode = content.substr(newStart);

	// 移除末尾的空白行
	while (!oldCode.empty() && (oldCode.back() == '\n' || oldCode.back() == '\r'))
		oldCode.pop_back();
	while (!newCode.empty() && (newCode.back() == '\n' || newCode.back() == '\r'))
		newCode.pop_back();
}


// 计算最佳位置，避免遮挡focusRect
void CRepairWnd::_CalculateBestPosition(const RECT& focusRect, const RECT& parentRect,
	int windowWidth, int windowHeight, CRect& bestRect)
{
	// 默认位置：focusRect的右侧
	int x = focusRect.right + 10;
	int y = focusRect.top;

	// 检查是否会超出右边界
	if (x + windowWidth > parentRect.right)
	{
		// 尝试放在左侧
		x = focusRect.left - windowWidth - 10;
		if (x < parentRect.left)
		{
			// 两侧都放不下，放在下方
			x = focusRect.left;
			y = focusRect.bottom + 10;
			
			// 检查是否会超出下边界
			if (y + windowHeight > parentRect.bottom)
			{
				// 放在上方
				y = focusRect.top - windowHeight - 10;
				if (y < parentRect.top)
				{
					// 所有方向都放不下，居中显示
					x = parentRect.left + (parentRect.right - parentRect.left - windowWidth) / 2;
					y = parentRect.top + (parentRect.bottom - parentRect.top - windowHeight) / 2;
				}
			}
		}
	}

	// 检查是否会超出下边界
	if (y + windowHeight > parentRect.bottom)
	{
		y = parentRect.bottom - windowHeight - 10;
		if (y < parentRect.top)
			y = parentRect.top + 10;
	}

	// 确保在父窗口范围内
	x = max(parentRect.left + 10, min(x, parentRect.right - windowWidth - 10));
	y = max(parentRect.top + 10, min(y, parentRect.bottom - windowHeight - 10));

	bestRect.SetRect(x, y, x + windowWidth, y + windowHeight);
}


// 执行JavaScript脚本
void CRepairWnd::ExecuteScript(const std::wstring& script, std::function<void(const std::wstring&)> callback)
{
    if (_webView != nullptr)
    {
        _webView->ExecuteScript(script.c_str(),
            Microsoft::WRL::Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
                [callback](HRESULT errorCode, LPCWSTR resultObjectAsJson) -> HRESULT {
                    if (callback && SUCCEEDED(errorCode))
                    {
                        callback(resultObjectAsJson ? resultObjectAsJson : L"");
                    }
                    return S_OK;
                }).Get());
    }
}


