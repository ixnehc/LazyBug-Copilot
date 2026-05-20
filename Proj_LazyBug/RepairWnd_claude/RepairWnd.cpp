#include "stdh.h"
#include "RepairWnd.h"
#include <fstream>
#include <algorithm>
#include "timer/wuid.h"
#include <nlohmann/json.hpp>

// 外部函数声明
extern std::string widechar_to_utf8(const wchar_t* str);
extern std::wstring utf8_to_widechar(const std::string& utf8_str);

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
    , _isCodeUIInitialized(false)
    , _isPendingShow(false)
    , _callbackId(0)
    , _webViewEnvironment(nullptr)
    , _webView(nullptr)
    , _controller(nullptr)
{
    memset(&_pendingFocusRect, 0, sizeof(_pendingFocusRect));
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
    extern const wchar_t* GetWebViewUserFolder();
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
                                                if (_navigationCompletedCallback)
                                                {
                                                    _navigationCompletedCallback(success == TRUE);
                                                }
                                                
                                                // 如果导航成功且还没初始化代码界面，则初始化
                                                if (success && !_isCodeUIInitialized)
                                                {
                                                    InitializeCodeUI();
                                                }

                                                return S_OK;
                                            }).Get(),
                                        &_navigationCompletedToken);

                                    // 添加WebMessage事件处理
                                    _webView->add_WebMessageReceived(
                                        Microsoft::WRL::Callback<ICoreWebView2WebMessageReceivedEventHandler>(
                                            [this](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
                                                LPWSTR messageRaw;
                                                args->get_WebMessageAsJson(&messageRaw);
                                                
                                                if (messageRaw)
                                                {
                                                    std::wstring message(messageRaw);
                                                    CoTaskMemFree(messageRaw);
                                                    
                                                    // 解析消息
                                                    try {
                                                        // 简单解析JSON消息
                                                        if (message.find(L"\"action\":\"contentSizeReady\"") != std::wstring::npos)
                                                        {
                                                            // 提取data部分
                                                            size_t dataPos = message.find(L"\"data\":");
                                                            if (dataPos != std::wstring::npos)
                                                            {
                                                                dataPos += 7; // 跳过"data":
                                                                size_t dataStart = message.find(L'"', dataPos);
                                                                if (dataStart != std::wstring::npos)
                                                                {
                                                                    dataStart++; // 跳过引号
                                                                    size_t dataEnd = message.find(L'"', dataStart);
                                                                    if (dataEnd != std::wstring::npos)
                                                                    {
                                                                        std::wstring sizeData = message.substr(dataStart, dataEnd - dataStart);
                                                                        _HandleContentSizeReady(sizeData);
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                    catch (...)
                                                    {
                                                        // 忽略解析错误
                                                    }
                                                }
                                                
                                                return S_OK;
                                            }).Get(),
                                        nullptr);

                                    extern const char* GetCurModuleFolderPath();
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

// 导航到HTML字符串
void CRepairWnd::NavigateToString(const std::wstring& htmlContent)
{
    if (_webView != nullptr)
    {
        _webView->NavigateToString(htmlContent.c_str());
    }
}

// 重新加载当前页面
void CRepairWnd::Reload()
{
    if (_webView != nullptr)
    {
        _webView->Reload();
    }
}

// 执行JavaScript脚本
void CRepairWnd::ExecuteScript(const std::wstring& script, std::function<void(const std::wstring&)> callback)
{
    if (_webView != nullptr)
    {
        int callbackId = -1;
        if (callback)
        {
            callbackId = _callbackId++;
            _scriptCallbacks[callbackId] = callback;
        }

        _webView->ExecuteScript(script.c_str(),
            Microsoft::WRL::Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
                [this, callbackId](HRESULT errorCode, LPCWSTR resultObjectAsJson) -> HRESULT {
                    if (callbackId >= 0 && _scriptCallbacks.find(callbackId) != _scriptCallbacks.end())
                    {
                        _scriptCallbacks[callbackId](resultObjectAsJson ? resultObjectAsJson : L"");
                        _scriptCallbacks.erase(callbackId);
                    }
                    return S_OK;
                }).Get());
    }
}

// 设置导航完成回调
void CRepairWnd::SetNavigationCompletedCallback(RepairWndNavigationCompletedCallback callback)
{
    _navigationCompletedCallback = callback;
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

//====================== 代码显示功能相关实现 ======================

// 初始化代码显示界面
void CRepairWnd::InitializeCodeUI()
{
    if (_isCodeUIInitialized)
        return;
    
    _isCodeUIInitialized = true;
    
    // 发送初始化完成消息到WebView
    _PostWebMessage(L"initializeComplete", L"");
}

// 设置代码内容
void CRepairWnd::SetContent(const CodeComparingChars& content)
{
    if (!_IsReady())
        return;
    
    // 构建代码内容JSON并发送到WebView
    std::wstring codeJson = _BuildCodeContentJson(content);
    _PostWebMessage(L"setCodeContent", codeJson);
}

// 清空内容
void CRepairWnd::Clear()
{
    if (!_IsReady())
        return;
    
    _PostWebMessage(L"clearContent", L"");
}


// 显示差异内容
void CRepairWnd::Show(const char* diffStr, const RECT& focusRect)
{
    if (!diffStr || strlen(diffStr) == 0)
        return;
    
    // 解析diffStr内容，生成CodeComparingChars
    CodeComparingChars comparingChars;
    _ParseDiffString(diffStr, comparingChars);
    
    if (comparingChars.content.empty())
        return;
    
    // 先隐藏窗口
    ShowWindow(SW_HIDE);
    
    // 设置内容
    SetContent(comparingChars);
    
    // 等待内容设置完成后再调整大小和位置
    _PostWebMessage(L"requestSizeAndShow", L"");
    
    // 保存focusRect用于后续位置计算
    _pendingFocusRect = focusRect;
    _isPendingShow = true;
}

//====================== 私有辅助方法实现 ======================

// 检查WebView和CodeUI是否已初始化
bool CRepairWnd::_IsReady() const
{
    return _isWebViewCreated && _isCodeUIInitialized;
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
    pos = 0;
    while ((pos = result.find(L"\t", pos)) != std::wstring::npos)
    {
        result.replace(pos, 1, L"\\t");
        pos += 2;
    }
    return result;
}

// 构建代码内容JSON
std::wstring CRepairWnd::_BuildCodeContentJson(const CodeComparingChars& content)
{
    std::wstring json = L"{";
    
    // 构建代码内容字符串，只包含NewCode和Both的字符
    std::string filteredContent;
    std::vector<bool> isNewCode;
    
    for (size_t i = 0; i < content.content.size() && i < content.charTypes.size(); ++i)
    {
        if (content.charTypes[i] == CodeComparingChars::NewCode || 
            content.charTypes[i] == CodeComparingChars::Both)
        {
            filteredContent += content.content[i];
            isNewCode.push_back(content.charTypes[i] == CodeComparingChars::NewCode);
        }
    }
    
    // 转换为宽字符串并转义
    std::wstring wideContent = utf8_to_widechar(filteredContent);
    std::wstring escapedContent = _EscapeJsonString(wideContent);
    
    json += L"\"content\":\"" + escapedContent + L"\",";
    
    // 构建字符类型数组
    json += L"\"charTypes\":[";
    for (size_t i = 0; i < isNewCode.size(); ++i)
    {
        if (i > 0) json += L",";
        json += isNewCode[i] ? L"true" : L"false";
    }
    json += L"]";
    
    json += L"}";
    return json;
}

//====================== 私有辅助方法实现 ======================

// 解析差异字符串
void CRepairWnd::_ParseDiffString(const char* diffStr, CodeComparingChars& comparingChars)
{
    comparingChars.content.clear();
    comparingChars.charTypes.clear();
    
    if (!diffStr || strlen(diffStr) == 0)
        return;
    
    std::string diffString(diffStr);
    
    // 查找分隔符
    const std::string oldMarker = "###old lines###";
    const std::string newMarker = "###new lines###";
    
    size_t oldPos = diffString.find(oldMarker);
    size_t newPos = diffString.find(newMarker);
    
    if (oldPos == std::string::npos || newPos == std::string::npos)
    {
        // 如果没有找到标记，将整个内容作为新代码
        comparingChars.content = diffString;
        comparingChars.charTypes.assign(diffString.size(), CodeComparingChars::NewCode);
        return;
    }
    
    // 提取旧代码和新代码
    std::string oldCode, newCode;
    
    if (oldPos < newPos)
    {
        // ###old lines### 在前
        size_t oldStart = oldPos + oldMarker.length();
        size_t oldEnd = newPos;
        oldCode = diffString.substr(oldStart, oldEnd - oldStart);
        
        size_t newStart = newPos + newMarker.length();
        newCode = diffString.substr(newStart);
    }
    else
    {
        // ###new lines### 在前
        size_t newStart = newPos + newMarker.length();
        size_t newEnd = oldPos;
        newCode = diffString.substr(newStart, newEnd - newStart);
        
        size_t oldStart = oldPos + oldMarker.length();
        oldCode = diffString.substr(oldStart);
    }
    
    // 去除首尾空白
    auto trim = [](std::string& str) {
        size_t start = str.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) {
            str.clear();
            return;
        }
        size_t end = str.find_last_not_of(" \t\r\n");
        str = str.substr(start, end - start + 1);
    };
    
    trim(oldCode);
    trim(newCode);
    
    // 使用CodeDiff进行字符级比较
    MakeCodeComparing_Chars(oldCode, newCode, comparingChars);
}

// 计算窗口位置（避免遮挡focusRect）
void CRepairWnd::_CalculateWindowPosition(const RECT& focusRect, int windowWidth, int windowHeight, int& x, int& y)
{
    // 获取屏幕工作区域
    RECT workArea;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);
    
    const int MARGIN = 10; // 与focusRect的间距
    
    // 优先尝试在focusRect右侧显示
    x = focusRect.right + MARGIN;
    y = focusRect.top;
    
    // 检查是否超出屏幕右边界
    if (x + windowWidth > workArea.right)
    {
        // 尝试在focusRect左侧显示
        x = focusRect.left - windowWidth - MARGIN;
        
        // 如果左侧也放不下，则在focusRect下方显示
        if (x < workArea.left)
        {
            x = focusRect.left;
            y = focusRect.bottom + MARGIN;
            
            // 如果下方也放不下，则在focusRect上方显示
            if (y + windowHeight > workArea.bottom)
            {
                y = focusRect.top - windowHeight - MARGIN;
                
                // 如果上方也放不下，则强制在屏幕内显示
                if (y < workArea.top)
                {
                    y = workArea.top;
                }
            }
        }
    }
    
    // 确保窗口在屏幕范围内
    if (x < workArea.left) x = workArea.left;
    if (y < workArea.top) y = workArea.top;
    if (x + windowWidth > workArea.right) x = workArea.right - windowWidth;
    if (y + windowHeight > workArea.bottom) y = workArea.bottom - windowHeight;
}

// 完成显示流程
void CRepairWnd::_CompleteShow()
{
    if (!_isPendingShow)
        return;
    
    _isPendingShow = false;
    
    // 获取当前窗口大小
    RECT windowRect;
    GetWindowRect(&windowRect);
    int windowWidth = windowRect.right - windowRect.left;
    int windowHeight = windowRect.bottom - windowRect.top;
    
    // 计算窗口位置
    int x, y;
    _CalculateWindowPosition(_pendingFocusRect, windowWidth, windowHeight, x, y);
    
    // 设置窗口位置并显示为TopMost
    SetWindowPos(&wndTopMost, x, y, 0, 0, 
        SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOACTIVATE);
}

// 处理内容尺寸就绪消息
void CRepairWnd::_HandleContentSizeReady(const std::wstring& sizeData)
{
    if (!_isPendingShow)
        return;
    
    try {
        // 解析尺寸数据
        size_t widthPos = sizeData.find(L"\"width\":");
        size_t heightPos = sizeData.find(L"\"height\":");
        
        if (widthPos != std::wstring::npos && heightPos != std::wstring::npos) {
            // 提取width值
            widthPos += 8; // 跳过"width":
            size_t widthEnd = sizeData.find(L',', widthPos);
            if (widthEnd == std::wstring::npos) widthEnd = sizeData.find(L'}', widthPos);
            
            // 提取height值
            heightPos += 9; // 跳过"height":
            size_t heightEnd = sizeData.find(L'}', heightPos);
            if (heightEnd == std::wstring::npos) heightEnd = sizeData.length();
            
            if (widthEnd != std::wstring::npos && heightEnd != std::wstring::npos) {
                int contentWidth = _wtoi(sizeData.substr(widthPos, widthEnd - widthPos).c_str());
                int contentHeight = _wtoi(sizeData.substr(heightPos, heightEnd - heightPos).c_str());
                
                if (contentWidth > 0 && contentHeight > 0) {
                    // 添加一些边距
                    const int MARGIN = 16;
                    int newWidth = contentWidth + MARGIN * 2;
                    int newHeight = contentHeight + MARGIN * 2;
                    
                    // 限制最大和最小尺寸
                    const int MIN_WIDTH = 200;
                    const int MIN_HEIGHT = 100;
                    const int MAX_WIDTH = 800;
                    const int MAX_HEIGHT = 600;
                    
                    newWidth = max(MIN_WIDTH, min(MAX_WIDTH, newWidth));
                    newHeight = max(MIN_HEIGHT, min(MAX_HEIGHT, newHeight));
                    
                    // 调整窗口大小
                    SetWindowPos(nullptr, 0, 0, newWidth, newHeight, 
                        SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
                    
                    // 完成显示流程
                    _CompleteShow();
                }
            }
        }
    }
    catch (...) {
        // 解析失败，使用默认尺寸完成显示
        _CompleteShow();
    }
}


