#include "stdh.h"
#include "RepairWnd.h"
#include <fstream>
#include <algorithm>
#include "timer/wuid.h"
#include <nlohmann/json.hpp>

#include "Utils.h"

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
	if (_webView != nullptr)
	{
		if (_navigationCompletedToken.value != 0)
		{
			_webView->remove_NavigationCompleted(_navigationCompletedToken);
			_navigationCompletedToken.value = 0;
		}
		if (_webMessageReceivedToken.value != 0)
		{
			_webView->remove_WebMessageReceived(_webMessageReceivedToken);
			_webMessageReceivedToken.value = 0;
		}
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
			MY_TRACE(L"Failed to initialize WebView2 environment: 0x%08lx\n", hr);
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
	HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(nullptr, Utils::GetWebViewUserFolder(), nullptr,
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

#ifdef DISABLE_WEBVIEW_CONTEXTMENU
					// 禁用WebView2默认右键菜单
					ICoreWebView2Settings* settings = nullptr;
					if (SUCCEEDED(_webView->get_Settings(&settings)) && settings)
					{
						settings->put_AreDefaultContextMenusEnabled(FALSE);
						settings->Release();
					}
#endif

					// 先添加WebMessage事件处理
					HRESULT hr = _webView->add_WebMessageReceived(
						Microsoft::WRL::Callback<ICoreWebView2WebMessageReceivedEventHandler>(
							[this](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
						LPWSTR messageRaw;
						args->get_WebMessageAsJson(&messageRaw);

						if (messageRaw)
						{
							std::wstring message(messageRaw);
							CoTaskMemFree(messageRaw);

							MY_TRACE(L"Received WebMessage: %s\n", message.c_str());

							// 解析消息
							try {
								// 简单解析JSON消息 - 现在使用正常的JSON格式
								if (message.find(L"\"action\":\"contentSizeReady\"") != std::wstring::npos)
								{
									MY_TRACE(L"Processing contentSizeReady message\n");
									// 提取data部分 - 修正解析逻辑
									size_t dataPos = message.find(L"\"data\":");
									if (dataPos != std::wstring::npos)
									{
										dataPos += 7; // 跳过"data":
										// 跳过可能的空格
										while (dataPos < message.length() && (message[dataPos] == L' ' || message[dataPos] == L'\t'))
											dataPos++;

										if (dataPos < message.length() && message[dataPos] == L'"')
										{
											// data是字符串类型
											dataPos++; // 跳过开始引号
											size_t dataEnd = message.find(L'"', dataPos);
											if (dataEnd != std::wstring::npos)
											{
												std::wstring sizeData = message.substr(dataPos, dataEnd - dataPos);
												MY_TRACE(L"Extracted size data: %s\n", sizeData.c_str());
												_HandleContentSizeReady(sizeData);
											}
										}
										else
										{
											// data可能是对象类型，直接提取到消息结尾
											size_t dataEnd = message.find_last_of(L'}');
											if (dataEnd != std::wstring::npos)
											{
												std::wstring sizeData = message.substr(dataPos, dataEnd - dataPos);
												MY_TRACE(L"Extracted size data (object): %s\n", sizeData.c_str());
												_HandleContentSizeReady(sizeData);
											}
										}
									}
								}
								else if (message.find(L"\"action\":\"pageReady\"") != std::wstring::npos)
								{
									MY_TRACE(L"Page ready message received\n");
									// 页面准备就绪
									if (!_isCodeUIInitialized)
									{
										InitializeCodeUI();
									}
								}
								else if (message.find(L"\"action\":\"log\"") != std::wstring::npos)
								{
									// 处理来自HTML的日志消息
									size_t dataPos = message.find(L"\"data\":");
									if (dataPos != std::wstring::npos)
									{
										dataPos += 7; // 跳过 "data":
										// 跳过可能的空格
										while (dataPos < message.length() && (message[dataPos] == L' ' || message[dataPos] == L'\t'))
											dataPos++;
										
										if (dataPos < message.length() && message[dataPos] == L'"')
										{
											// data是字符串类型
											dataPos++; // 跳过开始引号
											size_t dataEnd = message.find(L'"', dataPos);
											if (dataEnd != std::wstring::npos)
											{
												std::wstring logMsg = message.substr(dataPos, dataEnd - dataPos);
												OutputDebugStringW((L"[HTML] " + logMsg + L"\n").c_str());
											}
										}
									}
								}
								else if (message.find(L"\"action\":\"scriptResult\"") != std::wstring::npos)
								{
									// 这是脚本执行结果的回调，这里不处理
								}
								else
								{
									MY_TRACE(L"Unknown message action: %s\n", message.c_str());
								}
							}
							catch (...)
							{
								MY_TRACE(L"Error parsing WebMessage\n");
							}
						}
						else
						{
							MY_TRACE(L"Received empty WebMessage\n");
						}

						return S_OK;
					}).Get(),
						&_webMessageReceivedToken);

					if (FAILED(hr))
					{
						MY_TRACE(L"Failed to add WebMessageReceived event handler: 0x%08lx\n", hr);
					}
					else
					{
						MY_TRACE(L"WebMessageReceived event handler added successfully\n");
					}

					// 导航完成事件
					_webView->add_NavigationCompleted(
						Microsoft::WRL::Callback<ICoreWebView2NavigationCompletedEventHandler>(
							[this](ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
						BOOL success = FALSE;
						args->get_IsSuccess(&success);
						MY_TRACE(L"Navigation completed, success: %d\n", success);

						if (success)
						{
							MY_TRACE(L"Navigation successful, waiting for pageReady message\n");
							// 导航成功，等待页面发送pageReady消息
							// 不在这里直接初始化UI，而是等待JS端的确认
						}
						else
						{
							MY_TRACE(L"Navigation failed\n");
						}

						if (_navigationCompletedCallback)
						{
							_navigationCompletedCallback(success == TRUE);
						}

						return S_OK;
					}).Get(),
						&_navigationCompletedToken);

					extern const char* GetCurModuleFolderPath_utf8();
					std::string htmlPath = GetCurModuleFolderPath_utf8();
					htmlPath += "\\RepairWnd.html";
					Navigate(utf8_to_widechar(htmlPath.c_str()));

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

	MY_TRACE(L"Initializing Code UI\n");
	_isCodeUIInitialized = true;

	// 发送初始化完成消息到WebView
	_PostWebMessage(L"initializeComplete", L"");

	// 如果有待显示的内容，现在可以显示了
	if (_isPendingShow)
	{
		MY_TRACE(L"Processing pending show request\n");
		// 重新触发Show流程
		_PostWebMessage(L"requestSizeAndShow", L"");
	}
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
void CRepairWnd::Show(const CodeComparingChars& comparingChars, const RECT& focusRect)
{
	// 先隐藏窗口
	ShowWindow(SW_HIDE);

	// 保存focusRect用于后续位置计算
	_pendingFocusRect = focusRect;
	_isPendingShow = true;

	// 检查是否已经准备就绪
	if (!_IsReady())
	{
		MY_TRACE(L"WebView not ready, waiting...\n");
		return;
	}

	MY_TRACE(L"Setting content and requesting size\n");

	// 设置内容
	SetContent(comparingChars);

	// 等待内容设置完成后再调整大小和位置
	_PostWebMessage(L"requestSizeAndShow", L"");
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

	// 使用PostWebMessageAsString而不是PostWebMessageAsJson
	// 因为我们已经构建了JSON字符串，不需要额外的JSON编码
	_webView->PostWebMessageAsString(jsonMessage.c_str());
}

// 构建代码内容JSON
std::wstring CRepairWnd::_BuildCodeContentJson(const CodeComparingChars& content)
{
	std::wstring json = L"{";

	// 构建代码内容字符串，只包含NewCode和Both的字符
	std::wstring filteredContent;
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

	// 直接转义宽字符串（不需要UTF-8转换）
	std::wstring escapedContent = EscapeJsonString(filteredContent);

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

// 隐藏窗口
void CRepairWnd::Hide()
{
	// 隐藏窗口
	ShowWindow(SW_HIDE);
	
	// 中断正在进行的Show()过程
	_isPendingShow = false;
	
	// 清空待显示的内容
	Clear();
	
	MY_TRACE(L"RepairWnd hidden and pending show cancelled\n");
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

	ShowWindow(SW_SHOW);
}

// 处理内容尺寸就绪消息
void CRepairWnd::_HandleContentSizeReady(const std::wstring& sizeData)
{
	if (!_isPendingShow)
		return;

	MY_TRACE(L"_HandleContentSizeReady called with data: %s\n", sizeData.c_str());

	try {
		// 解析尺寸数据 - 现在sizeData是JSON对象格式
		size_t widthPos = sizeData.find(L"\"width\":");
		size_t heightPos = sizeData.find(L"\"height\":");

		if (widthPos != std::wstring::npos && heightPos != std::wstring::npos) {
			// 提取width值
			widthPos += 8; // 跳过"width":
			// 跳过可能的空格
			while (widthPos < sizeData.length() && (sizeData[widthPos] == L' ' || sizeData[widthPos] == L'\t'))
				widthPos++;
			
			size_t widthEnd = sizeData.find(L',', widthPos);
			if (widthEnd == std::wstring::npos) widthEnd = sizeData.find(L'}', widthPos);

			// 提取height值
			heightPos += 9; // 跳过"height":
			// 跳过可能的空格
			while (heightPos < sizeData.length() && (sizeData[heightPos] == L' ' || sizeData[heightPos] == L'\t'))
				heightPos++;
			
			size_t heightEnd = sizeData.find(L'}', heightPos);
			if (heightEnd == std::wstring::npos) heightEnd = sizeData.length();

			if (widthEnd != std::wstring::npos && heightEnd != std::wstring::npos) {
				std::wstring widthStr = sizeData.substr(widthPos, widthEnd - widthPos);
				std::wstring heightStr = sizeData.substr(heightPos, heightEnd - heightPos);
				
				MY_TRACE(L"Extracted width string: '%s', height string: '%s'\n", widthStr.c_str(), heightStr.c_str());
				
				int contentWidth = _wtoi(widthStr.c_str());
				int contentHeight = _wtoi(heightStr.c_str());

				MY_TRACE(L"Parsed dimensions: width=%d, height=%d\n", contentWidth, contentHeight);

				if (contentWidth > 0 && contentHeight > 0) {
		// 添加最小边距
		const int MARGIN = 0;
		int newWidth = contentWidth + MARGIN * 2;
		int newHeight = contentHeight + MARGIN * 2;

					// 限制最大和最小尺寸
					const int MIN_WIDTH = 100;
					const int MIN_HEIGHT = 25;  // 一行文字的高度（14px字体 + 行高1.4 ≈ 20px + 边距）
					const int MAX_WIDTH = 800;
					const int MAX_HEIGHT = 600;

					newWidth = max(MIN_WIDTH, min(MAX_WIDTH, newWidth));
					newHeight = max(MIN_HEIGHT, min(MAX_HEIGHT, newHeight));

					MY_TRACE(L"Final window dimensions: width=%d, height=%d\n", newWidth, newHeight);

					// 调整窗口大小
					SetWindowPos(nullptr, 0, 0, newWidth, newHeight,
						SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

					// 完成显示流程
					_CompleteShow();
				}
				else {
					MY_TRACE(L"Invalid dimensions, using default size\n");
					// 使用默认尺寸
					SetWindowPos(nullptr, 0, 0, 400, 200,
						SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
					_CompleteShow();
				}
			}
			else {
				MY_TRACE(L"Failed to find width/height end positions\n");
			}
		}
		else {
			MY_TRACE(L"Failed to find width/height in JSON data\n");
		}
	}
	catch (...) {
		MY_TRACE(L"Exception in _HandleContentSizeReady, using default size\n");
		// 解析失败，使用默认尺寸完成显示
		SetWindowPos(nullptr, 0, 0, 400, 200,
			SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
		_CompleteShow();
	}
}


