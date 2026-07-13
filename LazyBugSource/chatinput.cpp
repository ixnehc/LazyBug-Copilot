#include "stdh.h"
#include "ChatInput.h"
#include "stringparser/stringparser.h"
#include "timer/wuid.h"
#include "LlmLib.h"
#include "nlohmann/json.hpp"

#include "CppSymbolDefines.h"

#include "Utils.h"
#include "utils_image.h"
#include "Utils_Skill.h"

#include <algorithm>
#include <shellapi.h>
#include <atlimage.h>


BEGIN_MESSAGE_MAP(CChatInput, CWnd)
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

// 构造函数
CChatInput::CChatInput()
	: _isWebViewCreated(false)
	, _isInputInitialized(false)
	, _callbackId(0)
	, _webViewEnvironment(nullptr)
	, _webView(nullptr)
	, _controller(nullptr)
	, _autoCompleteEnabled(true)
	, _majorChatApiChangedCallback(nullptr)
	, _lastTimeOwnFocus(0)
	, _lastTimeForeground(0)
	, _wasForeground(false)
	, _requestGainFocus(false)
	, _compressIntensityChangedCallback(nullptr)
{
}

// 析构函数
CChatInput::~CChatInput()
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

	if (_controller != nullptr && _acceleratorKeyPressedToken.value != 0)
	{
		_controller->remove_AcceleratorKeyPressed(_acceleratorKeyPressedToken);
		_acceleratorKeyPressedToken.value = 0;
	}

	// 释放COM对象
	SAFE_RELEASE(_webView);
	SAFE_RELEASE(_controller);
	SAFE_RELEASE(_webViewEnvironment);
}

// 创建WebView2控件
BOOL CChatInput::Create(const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	// 注册窗口类
	static CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW,
		::LoadCursor(NULL, IDC_ARROW),
		(HBRUSH)::GetStockObject(WHITE_BRUSH),
		::LoadIcon(NULL, IDI_APPLICATION));

	// 创建窗口
	BOOL result = CWnd::CreateEx(0, className, _T("ChatInput Control"),
		WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_BORDER, rect, pParentWnd, nID);

	if (result)
	{
		// 初始化WebView2环境
		HRESULT hr = InitializeWebView();
		if (FAILED(hr))
		{
			return FALSE;
		}

		// 初始化自动补全列表
		_autoCompleteList.Initialize(this);
		_autoCompleteList.SetItemSelectedCallback([this](const ChatInputACItem& item) {
			OnAutoCompleteItemSelected(item);
		});
		_autoCompleteList.SetListCancelledCallback([this]() {
			OnAutoCompleteCancelled();
		});

		// 初始化标签菜单窗口
		_tagMenuWindow.CreateTagMenuWindow(this);
		_tagMenuWindow.SetTagVisibilityChangedCallback([this](const std::wstring& tagId, bool visible) {
			SetTagVisible(tagId, visible);
		});

		// 初始化LLM菜单窗口
		_llmMenuWindow.CreateLlmMenuWindow(this);
		_llmMenuWindow.SetLlmApiSelectedCallback([this](const std::wstring& apiName) {
			SetCurrentMajorChatApi(apiName);
		});

		// 初始化图片 Tag 预览提示窗口
		_imageTipWindow.CreateTipWindow(this);
	}

	return result;
}

// 初始化WebView2环境
HRESULT CChatInput::InitializeWebView()
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

					// 设置WebView通信处理
					// 导航完成事件
					_webView->add_NavigationCompleted(
						Microsoft::WRL::Callback<ICoreWebView2NavigationCompletedEventHandler>(
							[this](ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
						BOOL success = FALSE;
						args->get_IsSuccess(&success);

						// 如果导航成功且还没初始化输入界面，则初始化
						if (success && !_isInputInitialized)
						{
							InitializeInputUI();
						}

						return S_OK;
					}).Get(),
						&_navigationCompletedToken);

					// Web消息接收事件
					_webView->add_WebMessageReceived(
						Microsoft::WRL::Callback<ICoreWebView2WebMessageReceivedEventHandler>(
							[this](ICoreWebView2* sender, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
						LPWSTR messageJson;
						HRESULT hr = args->get_WebMessageAsJson(&messageJson);
						if (FAILED(hr)) {
							CoTaskMemFree(messageJson);
							return S_OK;
						}

						std::string utf8Message = widechar_to_utf8(messageJson);
						CoTaskMemFree(messageJson);

						try
						{
							nlohmann::json jsonMsg = nlohmann::json::parse(utf8Message);
							if (!jsonMsg.contains("action") || !jsonMsg["action"].is_string())
							{
								return S_OK;
							}

							std::string action = jsonMsg["action"];

							if (action == "send")
							{
								if (_sendCallback)
								{
									std::wstring contentJson = L"[]";
									if (jsonMsg.contains("content"))
									{
										contentJson = utf8_to_widechar(jsonMsg["content"].dump());
									}
									std::wstring plainText;
									if (jsonMsg.contains("plainText") && jsonMsg["plainText"].is_string())
									{
										plainText = utf8_to_widechar(jsonMsg["plainText"].get<std::string>());
									}
									_sendCallback(contentJson, plainText);
								}
							}
							else if (action == "hideTag")
							{
								if (jsonMsg.contains("tagId") && jsonMsg["tagId"].is_string())
								{
									std::wstring tagId = utf8_to_widechar(jsonMsg["tagId"].get<std::string>());
									HideTag(tagId);
									if (_tagRemovedCallback)
									{
										_tagRemovedCallback(tagId);
									}
								}
							}
							else if (action == "toolButton")
							{
								if (jsonMsg.contains("buttonId") && jsonMsg.contains("buttonAction") &&
									jsonMsg["buttonId"].is_string() && jsonMsg["buttonAction"].is_string())
								{
									std::wstring buttonId = utf8_to_widechar(jsonMsg["buttonId"].get<std::string>());
									std::wstring buttonAction = utf8_to_widechar(jsonMsg["buttonAction"].get<std::string>());
									if (_toolButtonClickedCallback)
									{
										_toolButtonClickedCallback(buttonId, buttonAction);
									}
								}
							}
							else if (action == "contentChanged")
							{
								if (jsonMsg.contains("content") && _contentChangedCallback)
								{
									// content 是 JSON 数组对象，需要转换为字符串
									std::string contentJsonStr = jsonMsg["content"].dump();
									std::wstring content = utf8_to_widechar(contentJsonStr);

									int caretPos = -1;
									if (jsonMsg.contains("caretPos") && jsonMsg["caretPos"].is_number())
									{
										caretPos = jsonMsg["caretPos"].get<int>();
									}

									bool isComposing = false;
									if (jsonMsg.contains("isComposing") && jsonMsg["isComposing"].is_boolean())
									{
										isComposing = jsonMsg["isComposing"].get<bool>();
									}

									_contentChangedCallback(content, caretPos, isComposing);
								}
							}
							else if (action == "initialized")
							{
								// Initialization message, can set a flag here if needed.
							}
							else if (action == "autoCompleteShow" || action == "autoCompleteUpdate")
							{
								if (jsonMsg.contains("query") && jsonMsg["query"].is_string())
								{
									std::wstring query = utf8_to_widechar(jsonMsg["query"].get<std::string>());
									_autoCompletePrefix = query;

									int x = -1, y = -1;
									if (jsonMsg.contains("position") && jsonMsg["position"].is_object())
									{
										auto pos = jsonMsg["position"];
										if (pos.contains("x") && pos["x"].is_number() && pos.contains("y") && pos["y"].is_number())
										{
											x = pos["x"].get<int>();
											y = pos["y"].get<int>();
										}
									}

									if (_autoCompleteEnabled)
									{
										if (action == "autoCompleteShow")
										{
											_autoCompleteList.Show(widechar_to_utf8(query.c_str()), x, y);
										}
										else // autoCompleteUpdate
										{
											_autoCompleteList.UpdateQuery(widechar_to_utf8(query.c_str()));
											if (x >= 0 && y >= 0)
											{
												_autoCompleteList.SetAnchorPos(x, y);
											}
										}
									}
								}
							}
							else if (action == "autoCompleteHide")
							{
								_autoCompleteList.Hide();
							}
							else if (action == "autoCompleteSelect")
							{
								if (jsonMsg.contains("index") && jsonMsg["index"].is_number())
								{
									int index = jsonMsg["index"].get<int>();
									_autoCompleteList.SetSelectedIndex(index);
								}
							}
							else if (action == "autoCompleteConfirm")
							{
								_autoCompleteList.ConfirmSelection();
							}
							else if (action == "toggleTagVisibility")
							{
								if (jsonMsg.contains("tagId") && jsonMsg["tagId"].is_string())
								{
									std::wstring tagId = utf8_to_widechar(jsonMsg["tagId"].get<std::string>());
									if (ChatInputTag* tag = _FindTag(tagId))
									{
										tag->visible = !tag->visible;
										_SendTagsUpdateMessage();
									}
								}
							}
							else if (action == "showTagMenu")
							{
								if (jsonMsg.contains("position") && jsonMsg["position"].is_object())
								{
									auto pos = jsonMsg["position"];
									if (pos.contains("x") && pos["x"].is_number() && pos.contains("y") && pos["y"].is_number())
									{
										int x = pos["x"].get<int>();
										int y = pos["y"].get<int>();

										CRect chatInputRect;
										GetWindowRect(&chatInputRect);
										int screenX = chatInputRect.left + x;
										int screenY = chatInputRect.top + y;
										_tagMenuWindow.ShowWindow(_tags, screenX, screenY);
									}
								}
							}
							else if (action == "pageNavigation")
							{
								if (jsonMsg.contains("direction") && jsonMsg["direction"].is_string() && _pageNavigationCallback)
								{
									std::string direction = jsonMsg["direction"];
									std::wstring contentJson = L"[]";
									if (jsonMsg.contains("content"))
									{
										contentJson = utf8_to_widechar(jsonMsg["content"].dump());
									}
									_pageNavigationCallback(direction == "up", contentJson);
								}
							}
							else if (action == "stopButtonClicked")
							{
								if (_stopButtonClickedCallback)
								{
									_stopButtonClickedCallback();
								}
							}
							else if (action == "majorChatApiSelected")
							{
								if (jsonMsg.contains("apiName") && jsonMsg["apiName"].is_string())
								{
									std::wstring apiName = utf8_to_widechar(jsonMsg["apiName"].get<std::string>());
									SetCurrentMajorChatApi(apiName);
								}
							}
							else if (action == "showLlmMenu")
							{
								if (jsonMsg.contains("position") && jsonMsg["position"].is_object())
								{
									auto pos = jsonMsg["position"];
									if (pos.contains("x") && pos["x"].is_number() && pos.contains("y") && pos["y"].is_number())
									{
										int x = pos["x"].get<int>();
										int y = pos["y"].get<int>();

										CRect chatInputRect;
										GetWindowRect(&chatInputRect);
										int screenX = chatInputRect.left + x;
										int screenY = chatInputRect.top + y;
										ShowLlmMenu(screenX, screenY);
									}
								}
							}
							else if (action == "tagClicked")
							{
								if (jsonMsg.contains("tagId") && jsonMsg["tagId"].is_string())
								{
									std::wstring tagId = utf8_to_widechar(jsonMsg["tagId"].get<std::string>());
									if (_tagClickedCallback)
									{
										_tagClickedCallback(tagId);
									}
								}
							}
							else if (action == "imageTagHoverResult")
							{
								// 处理 JS 返回的 image tag 悬停状态
								std::wstring filePath;
								bool hasHover = false;
								int x = 0, y = 0;

								if (jsonMsg.contains("filePath") && !jsonMsg["filePath"].is_null() && jsonMsg["filePath"].is_string())
								{
									filePath = utf8_to_widechar(jsonMsg["filePath"].get<std::string>());
									hasHover = !filePath.empty();

									if (jsonMsg.contains("position") && jsonMsg["position"].is_object())
									{
										auto pos = jsonMsg["position"];
										if (pos.contains("x") && pos["x"].is_number())
											x = pos["x"].get<int>();
										if (pos.contains("y") && pos["y"].is_number())
											y = pos["y"].get<int>();
									}
								}

								if (hasHover)
								{
									// 鼠标在某个 image tag 上
									if (_currentHoveredImageFilePath != filePath)
									{
										// 切换到新的 image tag
										_currentHoveredImageFilePath = filePath;
										if (!filePath.empty())
										{
											// 转换为屏幕坐标
											CRect chatInputRect;
											GetWindowRect(&chatInputRect);
											int screenX = chatInputRect.left + x;
											int screenY = chatInputRect.top + y;
											_imageTipWindow.ShowImage(filePath, screenX, screenY, chatInputRect);
										}
									}
								}
								else
								{
									// 鼠标不在任何 image tag 上
									if (!_currentHoveredImageFilePath.empty())
									{
										// 从有悬停变为无悬停，隐藏预览
										_currentHoveredImageFilePath.clear();
										_imageTipWindow.HideWindow();
									}
								}
							}
							else if (action == "escape")
							{
								if (_escapeCallback)
								{
									_escapeCallback();
								}
							}
							else if (action == "tab")
							{
								if (_tabCallback)
								{
									_tabCallback();
								}
							}
							else if (action == "filePasted")
							{
								if (_filePastedCallback)
								{
									std::wstring fileType = L"files";
									if (jsonMsg.contains("fileType") && jsonMsg["fileType"].is_string())
									{
										fileType = utf8_to_widechar(jsonMsg["fileType"].get<std::string>());
									}
									_filePastedCallback(fileType);
								}
							}
							else if (action == "skillButtonClicked")
							{
								if (_skillButtonClickedCallback)
								{
									RECT btnRect = { 0, 0, 0, 0 };
									if (jsonMsg.contains("rect") && jsonMsg["rect"].is_object())
									{
										auto& r = jsonMsg["rect"];
										if (r.contains("left") && r["left"].is_number())     btnRect.left = r["left"].get<int>();
										if (r.contains("top") && r["top"].is_number())       btnRect.top = r["top"].get<int>();
										if (r.contains("right") && r["right"].is_number())   btnRect.right = r["right"].get<int>();
										if (r.contains("bottom") && r["bottom"].is_number()) btnRect.bottom = r["bottom"].get<int>();

										// 转换为屏幕坐标
										CRect chatInputRect;
										GetWindowRect(&chatInputRect);
										btnRect.left += chatInputRect.left;
										btnRect.top += chatInputRect.top;
										btnRect.right += chatInputRect.left;
										btnRect.bottom += chatInputRect.top;
									}
								_skillButtonClickedCallback(btnRect);
								}
							}
							else if (action == "mcpButtonClicked")
							{
								if (_mcpButtonClickedCallback)
								{
									RECT btnRect = { 0, 0, 0, 0 };
									if (jsonMsg.contains("rect") && jsonMsg["rect"].is_object())
									{
										auto& r = jsonMsg["rect"];
										if (r.contains("left") && r["left"].is_number())     btnRect.left = r["left"].get<int>();
										if (r.contains("top") && r["top"].is_number())       btnRect.top = r["top"].get<int>();
										if (r.contains("right") && r["right"].is_number())   btnRect.right = r["right"].get<int>();
										if (r.contains("bottom") && r["bottom"].is_number()) btnRect.bottom = r["bottom"].get<int>();

										// 转换为屏幕坐标
										CRect chatInputRect;
										GetWindowRect(&chatInputRect);
										btnRect.left += chatInputRect.left;
										btnRect.top += chatInputRect.top;
										btnRect.right += chatInputRect.left;
										btnRect.bottom += chatInputRect.top;
									}
									_mcpButtonClickedCallback(btnRect);
								}
							}
							else if (action == "compressIntensityChanged")
							{
								if (jsonMsg.contains("intensity") && jsonMsg["intensity"].is_number())
								{
									int intensity = jsonMsg["intensity"].get<int>();
									if (_compressIntensityChangedCallback)
									{
										_compressIntensityChangedCallback(intensity);
									}
								}
							}
							else if (action == "inputHintToggleClicked")
							{
								if (jsonMsg.contains("enabled") && jsonMsg["enabled"].is_boolean())
								{
									bool enabled = jsonMsg["enabled"].get<bool>();
									if (_inputHintToggleCallback)
									{
										_inputHintToggleCallback(enabled);
									}
								}
							}
							else if (action == "openLogFile")
							{
								if (jsonMsg.contains("path") && jsonMsg["path"].is_string())
								{
									std::wstring path = utf8_to_widechar(jsonMsg["path"].get<std::string>());
									// 使用系统默认程序打开日志文件
									ShellExecuteW(NULL, L"open", path.c_str(), NULL, NULL, SW_SHOWNORMAL);
								}
							}
						}
						catch (const nlohmann::json::parse_error&)
						{
							// Optional: Log the parsing error
						}

						return S_OK;
					}).Get(),
						&_webMessageReceivedToken);

					// 加速器键按下事件（用于拦截 Ctrl+V 等快捷键）
					_controller->add_AcceleratorKeyPressed(
						Microsoft::WRL::Callback<ICoreWebView2AcceleratorKeyPressedEventHandler>(
							[this](ICoreWebView2Controller* sender, ICoreWebView2AcceleratorKeyPressedEventArgs* args) -> HRESULT {
						COREWEBVIEW2_KEY_EVENT_KIND keyEventKind;
						args->get_KeyEventKind(&keyEventKind);

						UINT virtualKey;
						args->get_VirtualKey(&virtualKey);

						// 只处理按键按下事件
						if (keyEventKind == COREWEBVIEW2_KEY_EVENT_KIND_KEY_DOWN ||
							keyEventKind == COREWEBVIEW2_KEY_EVENT_KIND_SYSTEM_KEY_DOWN)
						{
							// 检查是否是 Ctrl+V (粘贴)
							if (virtualKey == 'V' && (GetKeyState(VK_CONTROL) & 0x8000))
							{
								// 调用 HandlePaste 处理粘贴
								// 如果 HandlePaste 返回 true，表示已处理（文件/图片粘贴）
								// 如果返回 false，表示未处理，让 WebView2 执行默认粘贴行为
								bool handled = HandlePaste();
								if (handled)
								{
									// 标记为已处理，阻止WebView2默认处理
									args->put_Handled(TRUE);
								}

								return S_OK;
							}
						}

						return S_OK;
					}).Get(),
						&_acceleratorKeyPressedToken);

					// 构建 ChatInput.html 的完整路径
					extern const char* GetCurModuleFolderPath_utf8();
					std::string htmlPath = GetCurModuleFolderPath_utf8();
					//									htmlPath += "\\ChatInput.html";
					htmlPath += "\\ChatInputHtml\\index.html";
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
void CChatInput::Navigate(const std::wstring& url)
{
	if (_webView != nullptr)
	{
		_webView->Navigate(url.c_str());
	}
}

// 重新加载当前页面
void CChatInput::Reload()
{
	if (_webView != nullptr)
	{
		_webView->Reload();
	}
}

// 执行JavaScript脚本
void CChatInput::ExecuteScript(const std::wstring& script, std::function<void(const std::wstring&)> callback)
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

void CChatInput::Update()
{
	_autoCompleteList.Update();
	_tagMenuWindow.Update();
	_llmMenuWindow.Update();
	_UpdateGainFocus();

	_imageTipWindow.Update();
}


// 设置回调函数
void CChatInput::SetSendCallback(InputSendCallback callback)
{
	_sendCallback = callback;
}

void CChatInput::SetToolButtonClickedCallback(InputToolButtonClickedCallback callback)
{
	_toolButtonClickedCallback = callback;
}

void CChatInput::SetContentChangedCallback(InputContentChangedCallback callback)
{
	_contentChangedCallback = callback;
}

void CChatInput::SetTagRemovedCallback(InputTagRemovedCallback callback)
{
	_tagRemovedCallback = callback;
}

void CChatInput::SetAutoCompleteRequestCallback(InputAutoCompleteRequestCallback callback)
{
	_autoCompleteRequestCallback = callback;
}

void CChatInput::SetPageNavigationCallback(InputPageNavigationCallback callback)
{
	_pageNavigationCallback = callback;
}

void CChatInput::SetStopButtonClickedCallback(InputStopButtonClickedCallback callback)
{
	_stopButtonClickedCallback = callback;
}

void CChatInput::SetMajorChatApiChangedCallback(InputMajorChatApiChangedCallback callback)
{
	_majorChatApiChangedCallback = callback;
}

void CChatInput::SetTagClickedCallback(InputTagClickedCallback callback)
{
	_tagClickedCallback = callback;
}

void CChatInput::SetEscapeCallback(InputEscapeCallback callback)
{
	_escapeCallback = callback;
}

void CChatInput::SetReadyCallback(InputReadyCallback callback)
{
	_readyCallback = callback;
}

void CChatInput::SetFilePastedCallback(InputFilePastedCallback callback)
{
	_filePastedCallback = callback;
}

void CChatInput::SetSkillButtonClickedCallback(InputSkillButtonClickedCallback callback)
{
	_skillButtonClickedCallback = callback;
}

void CChatInput::SetMcpButtonClickedCallback(InputMcpButtonClickedCallback callback)
{
	_mcpButtonClickedCallback = callback;
}

void CChatInput::SetCompressIntensityChangedCallback(InputCompressIntensityChangedCallback callback)
{
	_compressIntensityChangedCallback = callback;
}

void CChatInput::SetInputHintToggleCallback(InputHintToggleCallback callback)
{
	_inputHintToggleCallback = callback;
}

void CChatInput::SetTabCallback(InputTabCallback callback)
{
	_tabCallback = callback;
}

void CChatInput::SetHintVisible(bool visible)
{
	if (_webView)
	{
		std::wstring script = L"window.__hintVisible = " + std::wstring(visible ? L"true;" : L"false;");
		_webView->ExecuteScript(script.c_str(), nullptr);
	}
}

void CChatInput::SetInputHintToggleButtonState(bool enabled)
{
	if (!_IsReady())
		return;

	std::wstring jsonMessage = L"{\"action\":\"setInputHintToggleButtonState\",\"enabled\":";
	jsonMessage += (enabled ? L"true" : L"false");
	jsonMessage += L"}";
	_PostWebMessageAsJson(jsonMessage);
}

// 调整WebView大小
void CChatInput::ResizeWebView()
{
	if (_controller != nullptr)
	{
		RECT bounds;
		GetClientRect(&bounds);
		_controller->put_Bounds(bounds);
	}
}

// 消息处理：大小变化
void CChatInput::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	ResizeWebView();
}

// 消息处理：创建
int CChatInput::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

// 消息处理：销毁
void CChatInput::OnDestroy()
{
	CWnd::OnDestroy();

	// 关闭WebView
	if (_controller != nullptr)
	{
		_controller->Close();
	}
}

//====================== 粘贴处理相关实现 ======================

// 处理粘贴的文件路径（公共逻辑）
void CChatInput::_ProcessPastedFilePath(const wchar_t* filePath)
{
	// 检查是否是文件（不是目录）
	DWORD attrs = GetFileAttributesW(filePath);
	if (attrs == INVALID_FILE_ATTRIBUTES || (attrs & FILE_ATTRIBUTE_DIRECTORY))
		return;

	// 转换为UTF-8
	std::string utf8Path = widechar_to_utf8(filePath);
	std::string actualPath = Utils::GetActualFilePath(utf8Path.c_str());
	std::wstring fullPathW = utf8_to_widechar(actualPath);

	// 获取文件名
	std::string fileName = GetFileName(actualPath.c_str());
	if(!Utils::MakeSkillTagName(actualPath.c_str(), fileName))
		fileName= GetFileName(actualPath.c_str());

	std::wstring fileNameW = utf8_to_widechar(fileName);

	// 判断是否为图片文件
	bool isImage = Utils::IsImageFile(actualPath.c_str());
	std::wstring tagType = isImage ? L"image" : L"file";

	bool canAddInline = false;
	bool canAttach = false;
	bool showAttach = false;
	if (!isImage)
	{
		canAddInline = true;
		canAttach = true;
		if (Utils::CheckFileBinary(actualPath.c_str()))
			canAttach = false;
		else
			showAttach = true;
	}
	else
		canAddInline = true;

	// 使用 InsertInlineTag 插入文件标签
	if (canAddInline)
	{
		InsertInlineTag(fileNameW, tagType, fullPathW);
	}

	// 添加文件标签到标签栏
	if (canAttach)
		AddFilePathTag(filePath, showAttach);
}

//====================== 输入功能相关实现 ======================

// 初始化输入界面
void CChatInput::InitializeInputUI()
{
	if (_isInputInitialized)
		return;

	_isInputInitialized = true;
	_tags.clear();
	_toolButtons.clear();

	// 初始化MajorChat API菜单
	UpdateMajorChatApiMenu();

	// 恢复 Context Level 流光状态
	if (_contextLevelFlowing)
		ExecuteScript(L"startCompressFlowing()");

	// 调用ready回调

	if (_readyCallback)
	{
		_readyCallback();
	}
}

// 获取输入内容（包含标签信息的完整内容）
void CChatInput::GetInputContent(std::function<void(const std::wstring&)> callback)
{
	if (!_IsReady() || !callback)
		return;

	ExecuteScript(L"getInputContent();", callback);
}

// 获取纯文本内容
void CChatInput::GetInputPlainText(std::function<void(const std::wstring&)> callback)
{
	if (!_IsReady() || !callback)
		return;

	ExecuteScript(L"getInputPlainText();", callback);
}

// 设置输入内容（支持包含标签信息的完整内容）
void CChatInput::SetInputContent_(const std::wstring& content, int caretTokenPos)
{
	if (!_IsReady())
		return;

	// 检查内容是否为空
	if (content.empty())
	{
		// 空内容，清空编辑器
		std::wstring jsonMessage = L"{\"action\":\"clearContent\"}";
		_PostWebMessageAsJson(jsonMessage);
		return;
	}

	// 将content作为字符串传递，让JavaScript端解析
	// 先转义JSON字符串，确保它能安全地嵌入到JSON消息中
	std::wstring safeContent = EscapeJsonString(content);
	std::wstring jsonMessage;
	if (caretTokenPos >= 0)
	{
		jsonMessage = L"{\"action\":\"setContent\",\"content\":\"" + safeContent + L"\",\"caretPos\":" + std::to_wstring(caretTokenPos) + L"}";
	}
	else
	{
		jsonMessage = L"{\"action\":\"setContent\",\"content\":\"" + safeContent + L"\"}";
	}

	_PostWebMessageAsJson(jsonMessage);
}

// 清空输入
void CChatInput::ClearInput_()
{
	if (!_IsReady())
		return;

	std::wstring jsonMessage = L"{\"action\":\"clearContent\"}";
	_PostWebMessageAsJson(jsonMessage);
}

// 插入文本到光标位置
void CChatInput::InsertText(const std::wstring& text)
{
	if (!_IsReady())
		return;

	std::wstring safeText = EscapeJsonString(text);
	std::wstring jsonMessage = L"{\"action\":\"insertText\",\"text\":\"" + safeText + L"\"}";
	_PostWebMessageAsJson(jsonMessage);
}

// 插入内联标签到光标位置
void CChatInput::InsertInlineTag(const std::wstring& text, const std::wstring& type,
	const std::wstring& data)
{
	if (!_IsReady())
		return;

	std::wstring tagJson = _BuildTagJson(text, type, data);
	std::wstring jsonMessage = L"{\"action\":\"insertInlineTag\",\"tag\":" + tagJson + L"}";
	_PostWebMessageAsJson(jsonMessage);
}

// 获取选中文本
void CChatInput::GetSelectedText(std::function<void(const std::wstring&)> callback)
{
	if (!_IsReady() || !callback)
		return;

	ExecuteScript(L"getSelectedText();", callback);
}

// 设置删除标记 token 索引列表
void CChatInput::SetDeletionMarks(const std::vector<int>& deletionIndices)
{
	if (!_IsReady())
		return;

	// 构建 JSON 数组
	std::wstring indicesJson = L"[";
	for (size_t i = 0; i < deletionIndices.size(); i++)
	{
		if (i > 0)
			indicesJson += L",";
		indicesJson += std::to_wstring(deletionIndices[i]);
	}
	indicesJson += L"]";

	std::wstring jsonMessage = L"{\"action\":\"setDeletionMarks\",\"indices\":" + indicesJson + L"}";
	_PostWebMessageAsJson(jsonMessage);
}

// 清除删除标记
void CChatInput::ClearDeletionMarks()
{
	if (!_IsReady())
		return;

	std::wstring jsonMessage = L"{\"action\":\"clearDeletionMarks\"}";
	_PostWebMessageAsJson(jsonMessage);
}

// 显示 ghost text 提示
void CChatInput::ShowGhostSuggestion(const std::wstring& text, int tokenIndex)
{
	if (!_IsReady())
		return;

	std::wstring safeText = EscapeJsonString(text);
	std::wstring jsonMessage = L"{\"action\":\"showGhostSuggestion\",\"text\":\"" + safeText +
		L"\",\"tokenIndex\":" + std::to_wstring(tokenIndex) + L"}";
	_PostWebMessageAsJson(jsonMessage);
}

// 清除 ghost text 提示
void CChatInput::ClearGhostSuggestion()
{
	if (!_IsReady())
		return;

	std::wstring jsonMessage = L"{\"action\":\"clearGhostSuggestion\"}";
	_PostWebMessageAsJson(jsonMessage);
}


//====================== 标签相关实现 ======================

// 添加标签
std::wstring CChatInput::AddTag(const std::wstring& text, const std::wstring& type,
	const std::wstring& path, const std::wstring& color, bool removable, bool visible)
{
	if (!_IsReady())
		return L"";

	for (auto& tag : _tags)
	{
		if (tag.path == path)
		{
			if (tag.visible != visible)
			{
				tag.visible = visible;
				_SendTagsUpdateMessage();
			}
			return L"";
		}
	}


	std::wstring tagId = _GenTagId();

	ChatInputTag tag;
	tag.id = tagId;
	tag.text = text;
	tag.type = type;
	tag.path = path;
	tag.color = color;
	tag.removable = removable;
	tag.visible = visible;

	_tags.push_back(tag);
	_SendTagsUpdateMessage();

	return tagId;
}

void CChatInput::_AddFilePathTag(const char* fullPath, bool isVisible)
{
	if (!Utils::IsFileExist(fullPath))
		return;

	std::string casePath = Utils::GetActualFilePath(fullPath);
	std::wstring fullPathW = utf8_to_widechar(casePath);

	std::string fileName;
	if (!Utils::MakeSkillTagName(casePath.c_str(), fileName))
		fileName = GetFileName(casePath.c_str());

	// 判断是否为图片文件
	bool isImage = Utils::IsImageFile(fullPath);

	// 设置标签类型（颜色由 CSS 根据类型自动控制）
	const wchar_t* tagType = isImage ? L"image" : L"file";

	AddTag(utf8_to_widechar(fileName.c_str()), tagType, fullPathW, L"", true, isVisible);
}

void CChatInput::AddFilePathTag(const wchar_t* fullPath, bool isVisible)
{
	if (!fullPath || !*fullPath)
		return;

	// 将宽字符路径转换为UTF-8字符串
	std::string utf8Path = widechar_to_utf8(fullPath);

	// 调用现有的窄字符版本
	_AddFilePathTag(utf8Path.c_str(), isVisible);
}

bool CChatInput::HandlePaste()
{
	if (!_IsReady())
		return false;

	// 打开剪贴板
	if (!OpenClipboard())
		return false;

	// 检查是否有文件列表
	BOOL hasFiles = IsClipboardFormatAvailable(CF_HDROP);

	if (hasFiles)
	{
		// 获取文件列表数据
		HANDLE hData = GetClipboardData(CF_HDROP);
		if (hData != NULL)
		{
			HDROP hDrop = (HDROP)hData;

			// 获取文件数量
			UINT fileCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);

			// 遍历所有文件
			for (UINT i = 0; i < fileCount; i++)
			{
				// 获取文件路径长度
				UINT pathLength = DragQueryFileW(hDrop, i, NULL, 0);
				if (pathLength > 0)
				{
					// 分配缓冲区并获取文件路径
					wchar_t* filePath = new wchar_t[pathLength + 1];
					if (DragQueryFileW(hDrop, i, filePath, pathLength + 1) > 0)
					{
						_ProcessPastedFilePath(filePath);
					}
					delete[] filePath;
				}
			}
		}
		// 关闭剪贴板
		CloseClipboard();
		return true;
	}
	else
	{
		// 检查剪贴板是否有文本格式（可能是文件路径）
		BOOL hasUnicodeText = IsClipboardFormatAvailable(CF_UNICODETEXT);
		BOOL hasAnsiText = IsClipboardFormatAvailable(CF_TEXT);
		bool handledAsFilePath = false;

		if (hasUnicodeText || hasAnsiText)
		{
			// 获取文本数据
			HANDLE hData = GetClipboardData(hasUnicodeText ? CF_UNICODETEXT : CF_TEXT);
			if (hData != NULL)
			{
				char* pData = (char*)GlobalLock(hData);
				if (pData != NULL)
				{
					std::wstring textW;
					if (hasUnicodeText)
					{
						textW = (wchar_t*)pData;
					}
					else
					{
						// 将 ANSI 文本转换为宽字符
						int len = MultiByteToWideChar(CP_ACP, 0, pData, -1, NULL, 0);
						if (len > 0)
						{
							wchar_t* buf = new wchar_t[len];
							MultiByteToWideChar(CP_ACP, 0, pData, -1, buf, len);
							textW = buf;
							delete[] buf;
						}
					}
					GlobalUnlock(hData);

					// 去除前后空白字符
					size_t start = textW.find_first_not_of(L" \t\r\n");
					size_t end = textW.find_last_not_of(L" \t\r\n");
					if (start != std::wstring::npos && end != std::wstring::npos)
					{
						std::wstring trimmedPath = textW.substr(start, end - start + 1);

						// 检查是否是有效的文件路径（文件存在且不是目录）
						DWORD attrs = GetFileAttributesW(trimmedPath.c_str());
						if (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY))
						{
							_ProcessPastedFilePath(trimmedPath.c_str());
							handledAsFilePath = true;
						}
					}
				}
			}
		}

		// 关闭剪贴板，因为 Utils::GenTempImageFromClipboard 会自己处理剪贴板
		CloseClipboard();

		// 如果不是文件路径，则尝试处理为图片
		if (!handledAsFilePath)
		{
			// 使用 Utils::GenTempImageFromClipboard 从剪贴板生成临时图片
			std::string imagePath = Utils::GenTempImageFromClipboard();
			if (!imagePath.empty())
			{
				// 转换为宽字符路径（GenTempImageFromClipboard 返回本地编码字符串）
				std::wstring filePathStr = utf8_to_widechar(imagePath.c_str());

				// 获取文件名
				std::string fileName = GetFileName(imagePath.c_str());
				std::wstring fileNameW = utf8_to_widechar(fileName.c_str());

				// 使用 InsertInlineTag 插入图片标签
				InsertInlineTag(fileNameW, L"image", filePathStr);

				// 				AddFilePathTag(filePathStr.c_str(), true);
			}
			else
			{
				// 不是图片，返回 false 让 WebView2 执行默认粘贴行为
				return false;
			}
		}

		// 已经关闭剪贴板了，直接返回
		return true;
	}

	// 关闭剪贴板
	CloseClipboard();
}


// 移除标签（真正删除）
void CChatInput::RemoveTag(const std::wstring& tagId)
{
	auto it = std::find_if(_tags.begin(), _tags.end(),
		[&tagId](const ChatInputTag& tag) { return tag.id == tagId; });

	if (it != _tags.end())
	{
		_tags.erase(it);
		_SendTagsUpdateMessage();
	}
}

// 隐藏标签（设置为不可见）
void CChatInput::HideTag(const std::wstring& tagId)
{
	SetTagVisible(tagId, false);
}

// 清空所有标签
void CChatInput::ClearTags()
{
	_tags.clear();
	_SendTagsUpdateMessage();
}

// 检查标签是否存在
bool CChatInput::HasTag(const std::wstring& tagId) const
{
	return _FindTag(tagId) != nullptr;
}

// 设置标签可见性
void CChatInput::SetTagVisible(const std::wstring& tagId, bool visible)
{
	ChatInputTag* tag = _FindTag(tagId);
	if (tag != nullptr && tag->visible != visible)
	{
		tag->visible = visible;
		_SendTagsUpdateMessage();
	}
}

std::vector<ChatInputTag> CChatInput::GetVisibleFileTags() const
{
	std::vector<ChatInputTag> tags;
	for (const auto& tag : _tags)
	{
		if (tag.visible)
		{
			if ((tag.type == L"file") || (tag.type == L"image"))
				tags.push_back(tag);
		}
	}
	return tags;
}

//====================== 工具栏相关实现 ======================

// 添加工具按钮
std::wstring CChatInput::AddToolButton(const std::wstring& text, const std::wstring& icon,
	const std::wstring& action, const std::wstring& tooltip)
{
	if (!_IsReady())
		return L"";

	std::wstring buttonId = _GenButtonId();

	ChatInputToolButton button;
	button.id = buttonId;
	button.text = text;
	button.icon = icon;
	button.action = action;
	button.tooltip = tooltip;
	button.enabled = true;

	_toolButtons.push_back(button);
	_SendToolButtonsUpdateMessage();

	return buttonId;
}

// 移除工具按钮
void CChatInput::RemoveToolButton(const std::wstring& buttonId)
{
	auto it = std::find_if(_toolButtons.begin(), _toolButtons.end(),
		[&buttonId](const ChatInputToolButton& button) { return button.id == buttonId; });

	if (it != _toolButtons.end())
	{
		_toolButtons.erase(it);
		_SendToolButtonsUpdateMessage();
	}
}

// 设置按钮启用状态
void CChatInput::SetToolButtonEnabled(const std::wstring& buttonId, bool enabled)
{
	ChatInputToolButton* button = _FindToolButton(buttonId);
	if (button != nullptr)
	{
		button->enabled = enabled;
		_SendToolButtonsUpdateMessage();
	}
}

// 清空所有工具按钮
void CChatInput::ClearToolButtons()
{
	_toolButtons.clear();
	_SendToolButtonsUpdateMessage();
}

//====================== 发送按钮相关实现 ======================

// 设置发送按钮启用状态
void CChatInput::SetSendButtonEnabled(bool enabled)
{
	if (!_IsReady())
		return;

	std::wstring jsonMessage = L"{\"action\":\"setSendButtonEnabled\",\"enabled\":";
	jsonMessage += (enabled ? L"true" : L"false");
	jsonMessage += L"}";
	_PostWebMessageAsJson(jsonMessage);
}

// 设置发送按钮文字
void CChatInput::SetSendButtonText(const std::wstring& text)
{
	if (!_IsReady())
		return;

	std::wstring safeText = EscapeJsonString(text);
	std::wstring jsonMessage = L"{\"action\":\"setSendButtonText\",\"text\":\"" + safeText + L"\"}";
	_PostWebMessageAsJson(jsonMessage);
}

//====================== Stop按钮功能实现 ======================

// 显示stop按钮
void CChatInput::ShowStopButton()
{
	if (!_IsReady())
		return;

	std::wstring jsonMessage = L"{\"action\":\"showStopButton\"}";
	_PostWebMessageAsJson(jsonMessage);
}

// 隐藏stop按钮
void CChatInput::HideStopButton()
{
	if (!_IsReady())
		return;

	std::wstring jsonMessage = L"{\"action\":\"hideStopButton\"}";
	_PostWebMessageAsJson(jsonMessage);
}

//====================== 压缩结果提示实现 ======================

// 显示压缩结果提示
void CChatInput::ShowCompressSummarizeTip(bool success, const std::wstring& message, const std::wstring& logPath)
{
	if (!_IsReady())
		return;

	std::wstring safeMessage = EscapeJsonString(message);
	std::wstring safeLogPath = EscapeJsonString(logPath);
	
	std::wstring jsonMessage = L"{\"action\":\"showCompressSummarizeTip\",\"success\":";
	jsonMessage += success ? L"true" : L"false";
	jsonMessage += L",\"message\":\"" + safeMessage + L"\"";
	if (!logPath.empty())
	{
		jsonMessage += L",\"logPath\":\"" + safeLogPath + L"\"";
	}
	jsonMessage += L"}";
	
	_PostWebMessageAsJson(jsonMessage);
}

// 隐藏压缩结果提示
void CChatInput::HideCompressSummarizeTip()
{
	if (!_IsReady())
		return;

	std::wstring jsonMessage = L"{\"action\":\"hideCompressSummarizeTip\"}";
	_PostWebMessageAsJson(jsonMessage);
}

//====================== 外观设置实现 ======================

// 设置占位符文字
void CChatInput::SetPlaceholder(const std::wstring& placeholder)
{
	if (!_IsReady())
		return;

	std::wstring safePlaceholder = EscapeJsonString(placeholder);
	std::wstring jsonMessage = L"{\"action\":\"setPlaceholder\",\"placeholder\":\"" + safePlaceholder + L"\"}";
	_PostWebMessageAsJson(jsonMessage);
}

bool CChatInput::SetCompressIntensity(int intensity, const std::wstring& tooltip)
{
	if (!_IsReady())
		return false;

	// 确保强度值在有效范围内
	if (intensity < 0 || intensity > 4)
		return false;

	std::wstring escapedTooltip = tooltip;
	// 转义换行符
	size_t pos = 0;
	while ((pos = escapedTooltip.find(L'\n', pos)) != std::wstring::npos)
	{
		escapedTooltip.replace(pos, 1, L"\\n");
		pos += 2;
	}
	// 转义双引号
	pos = 0;
	while ((pos = escapedTooltip.find(L'"', pos)) != std::wstring::npos)
	{
		escapedTooltip.replace(pos, 1, L"\\\"");
		pos += 2;
	}

	std::wstring script = L"setCompressIntensity(" + std::to_wstring(intensity) + L", \"" + escapedTooltip + L"\")";
	ExecuteScript(script);

	return true;
}

// 设置压缩后大小显示 (如 "18K", "1.21M", "0B" 等)
bool CChatInput::SetCompressedSize(const std::wstring& sizeText, const std::wstring& tooltip)
{
	if (!_IsReady())
		return false;

	std::wstring escapedSize = EscapeJsonString(sizeText);
	std::wstring escapedTooltip = EscapeJsonString(tooltip);
	std::wstring script = L"setCompressedSize(\"" + escapedSize + L"\", \"" + escapedTooltip + L"\")";
	ExecuteScript(script);

	return true;
}

// 开始 Context Level 按钮的流光效果
void CChatInput::StartContextLevelFlowing()
{
	if (_contextLevelFlowing)
		return;

	_contextLevelFlowing = true;

	if (!_IsReady())
		return;

	ExecuteScript(L"startCompressFlowing()");
}

// 停止 Context Level 按钮的流光效果
void CChatInput::StopContextLevelFlowing()
{
	if (!_contextLevelFlowing)
		return;

	_contextLevelFlowing = false;

	if (!_IsReady())
		return;

	ExecuteScript(L"stopCompressFlowing()");
}



//====================== 私有辅助方法实现 ======================

// 生成唯一标签ID
std::wstring CChatInput::_GenTagId()
{
	WUID wuid = GenWUID();
	return L"tag_" + std::to_wstring(wuid);
}

// 生成唯一按钮ID
std::wstring CChatInput::_GenButtonId()
{
	WUID wuid = GenWUID();
	return L"btn_" + std::to_wstring(wuid);
}


// 检查WebView和Input是否已初始化
bool CChatInput::_IsReady() const
{
	return _isWebViewCreated && _isInputInitialized;
}

// 查找标签
ChatInputTag* CChatInput::_FindTag(const std::wstring& tagId)
{
	auto it = std::find_if(_tags.begin(), _tags.end(),
		[&tagId](const ChatInputTag& tag) { return tag.id == tagId; });

	return (it != _tags.end()) ? &(*it) : nullptr;
}

const ChatInputTag* CChatInput::_FindTag(const std::wstring& tagId) const
{
	auto it = std::find_if(_tags.begin(), _tags.end(),
		[&tagId](const ChatInputTag& tag) { return tag.id == tagId; });

	return (it != _tags.end()) ? &(*it) : nullptr;
}

// 查找工具按钮
ChatInputToolButton* CChatInput::_FindToolButton(const std::wstring& buttonId)
{
	auto it = std::find_if(_toolButtons.begin(), _toolButtons.end(),
		[&buttonId](const ChatInputToolButton& button) { return button.id == buttonId; });

	return (it != _toolButtons.end()) ? &(*it) : nullptr;
}

// 发送标签更新消息
void CChatInput::_SendTagsUpdateMessage()
{
	if (!_IsReady())
		return;

	std::wstring tagsJson = L"[";
	for (size_t i = 0; i < _tags.size(); ++i)
	{
		if (i > 0) tagsJson += L",";

		const ChatInputTag& tag = _tags[i];
		tagsJson += L"{";
		tagsJson += L"\"id\":\"" + EscapeJsonString(tag.id) + L"\",";
		tagsJson += L"\"text\":\"" + EscapeJsonString(tag.text) + L"\",";
		tagsJson += L"\"type\":\"" + EscapeJsonString(tag.type) + L"\",";
		tagsJson += L"\"data\":\"" + EscapeJsonString(tag.path) + L"\",";
		tagsJson += L"\"color\":\"" + EscapeJsonString(tag.color) + L"\",";
		tagsJson += L"\"removable\":";
		tagsJson += (tag.removable ? L"true" : L"false");
		tagsJson += L",\"visible\":";
		tagsJson += (tag.visible ? L"true" : L"false");

		std::wstring imgSrc = _BuildImgSrc(tag.type, tag.path);
		if (!imgSrc.empty())
		{
			tagsJson += L",\"imgSrc\":\"" + EscapeJsonString(imgSrc) + L"\"";
		}

		tagsJson += L"}";
	}
	tagsJson += L"]";

	std::wstring jsonMessage = L"{\"action\":\"updateTags\",\"tags\":" + tagsJson + L"}";
	_PostWebMessageAsJson(jsonMessage);
}

// 发送工具按钮更新消息
void CChatInput::_SendToolButtonsUpdateMessage()
{
	if (!_IsReady())
		return;

	std::wstring buttonsJson = L"[";
	for (size_t i = 0; i < _toolButtons.size(); ++i)
	{
		if (i > 0) buttonsJson += L",";

		const ChatInputToolButton& button = _toolButtons[i];
		buttonsJson += L"{";
		buttonsJson += L"\"id\":\"" + EscapeJsonString(button.id) + L"\",";
		buttonsJson += L"\"text\":\"" + EscapeJsonString(button.text) + L"\",";
		buttonsJson += L"\"icon\":\"" + EscapeJsonString(button.icon) + L"\",";
		buttonsJson += L"\"action\":\"" + EscapeJsonString(button.action) + L"\",";
		buttonsJson += L"\"tooltip\":\"" + EscapeJsonString(button.tooltip) + L"\",";
		buttonsJson += L"\"enabled\":";
		buttonsJson += (button.enabled ? L"true" : L"false");
		buttonsJson += L"}";
	}
	buttonsJson += L"]";

	std::wstring jsonMessage = L"{\"action\":\"updateToolButtons\",\"buttons\":" + buttonsJson + L"}";
	_PostWebMessageAsJson(jsonMessage);
}

// 发送消息到WebView
void CChatInput::_PostWebMessageAsJson(const std::wstring& message)
{
	if (_webView != nullptr)
	{
		_webView->PostWebMessageAsJson(message.c_str());
	}
}

// 生成图片src的辅助方法（用于image类型标签）
std::wstring CChatInput::_BuildImgSrc(const std::wstring& type, const std::wstring& data)
{
	if (type != L"image" || data.empty())
		return L"";

	std::string localPath = widechar_to_utf8(data.c_str());
	std::string base64Content;
	if (Utils::LoadImageThumbnailIntoBase64(localPath.c_str(), 256, 64, base64Content) && !base64Content.empty())
	{
		// 根据文件扩展名确定 MIME 类型
		std::string suffix = GetFileSuffix(localPath);
		StringLower(suffix);
		std::string mimeType = "image/png";
		if (suffix == "jpg" || suffix == "jpeg") mimeType = "image/jpeg";
		// 		else if (suffix == "webp")               mimeType = "image/webp";
		// 		else if (suffix == "gif")                mimeType = "image/gif";

		return utf8_to_widechar(
			("data:" + mimeType + ";base64," + base64Content).c_str());
	}
	return L"";
}

// 生成标签JSON的辅助方法
std::wstring CChatInput::_BuildTagJson(const std::wstring& text, const std::wstring& type,
	const std::wstring& data)
{
	std::wstring safeText = EscapeJsonString(text);
	std::wstring safeType = EscapeJsonString(type);
	std::wstring safeData = EscapeJsonString(data);

	std::wstring tagJson = L"{";
	tagJson += L"\"text\":\"" + safeText + L"\",";
	tagJson += L"\"type\":\"" + safeType + L"\",";
	tagJson += L"\"data\":\"" + safeData + L"\"";

	// image 类型：额外生成 imgSrc 字段
	std::wstring imgSrc = _BuildImgSrc(type, data);
	if (!imgSrc.empty())
	{
		tagJson += L",\"imgSrc\":\"" + EscapeJsonString(imgSrc) + L"\"";
	}

	tagJson += L"}";
	return tagJson;
}

//====================== 自动补全相关实现 ======================

//====================== Agent API 相关实现 ======================


// 获取当前的Agent API
std::wstring CChatInput::GetCurrentMajorChatApi() const
{
	extern CLlmLib g_llmLib;
	std::string currentApi = g_llmLib.GetMajorChatApi();
	return utf8_to_widechar(currentApi.c_str());
}

// 设置当前的Agent API
void CChatInput::SetCurrentMajorChatApi(const std::wstring& apiName)
{
	extern CLlmLib g_llmLib;
	std::string apiNameStr = widechar_to_utf8(apiName.c_str());

	// 设置新的Agent API
	g_llmLib.SetMajorChatApi(apiNameStr);

	// 更新菜单显示
	UpdateMajorChatApiMenu();

	// 触发回调
	if (_majorChatApiChangedCallback)
	{
		_majorChatApiChangedCallback(apiName);
	}
}

// 更新API菜单显示
void CChatInput::UpdateMajorChatApiMenu()
{
	if (!_IsReady())
		return;

	std::wstring currentApi = GetCurrentMajorChatApi();
	std::wstring jsonMessage = L"{\"action\":\"updateMajorChatApiMenu\",\"current\":\"" +
		EscapeJsonString(currentApi) + L"\"}";
	_PostWebMessageAsJson(jsonMessage);
}

// 显示LLM菜单
void CChatInput::ShowLlmMenu(int x, int y)
{
	// 构建API列表
	std::vector<ChatLlmApiItem> apiItems;
	std::wstring currentApi = GetCurrentMajorChatApi();

	extern CLlmLib g_llmLib;
	const auto& apis = g_llmLib.GetApis();

	// 收集所有可用的Agent API
	std::vector<const LlmApi*> availableApis;
	for (const auto& api : apis)
	{
		if (api.role == LlmApiRole::Agent && api.enable)
		{
			if (g_llmLib.IsApiAvailable(api.name))
				availableApis.push_back(&api);
		}
	}

	// 按名称字母排序（忽略大小写）
	std::sort(availableApis.begin(), availableApis.end(),
		[](const LlmApi* a, const LlmApi* b) {
			return _stricmp(a->name.c_str(), b->name.c_str()) < 0;
		});

	// 构建列表
	for (size_t i = 0; i < availableApis.size(); i++)
	{
		const LlmApi* api = availableApis[i];
		const LlmApiProvider* provider = g_llmLib.GetProvider(api->providerTypeName);
		bool isAvailable = (provider && provider->IsAvailable());

		ChatLlmApiItem item;
		item.name = utf8_to_widechar(api->name.c_str());
		item.available = isAvailable;
		item.selected = (item.name == currentApi);

		apiItems.push_back(item);
	}

	// 显示菜单
	_llmMenuWindow.ShowWindow(apiItems, x, y);
}

// 隐藏LLM菜单
void CChatInput::HideLlmMenu()
{
	_llmMenuWindow.HideWindow();
}

//====================== 自动补全相关实现 ======================

// 自动补全项目被选中的处理
void CChatInput::OnAutoCompleteItemSelected(const ChatInputACItem& item)
{
	if (!_IsReady())
		return;

	std::string text = item.text;
	if (item.type == "symbol")
	{
		text = Utils::RestoreSymbolName(text);

		if ((item.kind == (int)SymbolKind::Function) ||
			(item.kind == (int)SymbolKind::Method))
			text += "(..)";
	}

	std::string fullPath = Utils::GetActualFilePath(item.fullPath.c_str());

	// 构建要插入的标签
	std::wstring tagText = utf8_to_widechar(text.c_str());
	std::wstring tagType = item.type.empty() ? L"autocomplete" : utf8_to_widechar(item.type.c_str());
	std::wstring tagData = utf8_to_widechar(fullPath);

	// 使用公用函数构建标签JSON
	std::wstring tagJson = _BuildTagJson(tagText, tagType, tagData);

	std::wstring jsonMessage = L"{\"action\":\"replaceAutoCompleteWithTag\",\"prefix\":\"" +
		EscapeJsonString(_autoCompletePrefix) + L"\",\"tag\":" + tagJson + L"}";

	_PostWebMessageAsJson(jsonMessage);

	_autoCompletePrefix.clear();

	std::string basePath = fullPath;
	RemoveFileSuffix(basePath);

	bool isTagVisible = (!(item.type == "symbol"));

	_AddFilePathTag(fullPath.c_str(), isTagVisible);

	// 	std::string path = basePath + ".cpp";
	// 	_AddFilePathTag(path.c_str(), isTagVisible);
	// 	path = basePath + ".h";
	// 	_AddFilePathTag(path.c_str(), isTagVisible);
	// 	path = basePath + ".c";
	// 	_AddFilePathTag(path.c_str(), isTagVisible);
	// 	path = basePath + ".hpp";
	// 	_AddFilePathTag(path.c_str(), isTagVisible);
	// 	path = basePath + ".inl";
	// 	_AddFilePathTag(path.c_str(), isTagVisible);
}

// 自动补全被取消的处理
void CChatInput::OnAutoCompleteCancelled()
{
	// 清空前缀
	_autoCompletePrefix.clear();
}

void CChatInput::_UpdateGainFocus()
{

	if (_requestGainFocus)
	{
		_requestGainFocus = false;
		OccupyFocus();
		return;
	}

	HWND hMainWnd = ::GetAncestor(m_hWnd, GA_ROOT);
	if (!hMainWnd) return;

	HWND hForeWnd = ::GetForegroundWindow();
	bool isForeground = (hMainWnd == hForeWnd) || ::IsChild(hMainWnd, hForeWnd);

	HWND hFocusWnd = ::GetFocus();
	bool hasFocus = (hFocusWnd == m_hWnd || (hFocusWnd && ::IsChild(m_hWnd, hFocusWnd)));

	if (hasFocus)
	{
		_lastTimeOwnFocus = ::GetTickCount();
	}

	if (isForeground)
	{
		// 刚切换到前台
		if (!_wasForeground)
		{
			// 如果失去焦点和失去前台的时间很接近，则认为是由于切到后台导致的，此时应夺回焦点
			if (abs((long)(_lastTimeOwnFocus - _lastTimeForeground)) < 200)
			{
				OccupyFocus();
			}
		}
		_lastTimeForeground = ::GetTickCount();
	}

	_wasForeground = isForeground;
}

void CChatInput::OccupyFocus()
{
	if (_controller)
	{
		// 先让 CChatInput 窗口本身获得焦点（MoveFocus 需要宿主窗口有焦点才能正确工作）
		if (GetSafeHwnd() && ::GetFocus() != m_hWnd)
		{
			SetFocus();
		}
		// 使用ICoreWebView2Controller::MoveFocus，这是更可靠的、官方推荐的将焦点移入WebView的方式
		_controller->MoveFocus(COREWEBVIEW2_MOVE_FOCUS_REASON_PROGRAMMATIC);
		FocusEditor();
	}
}

void CChatInput::FocusEditor()
{
	if (!_IsReady())
		return;

	ExecuteScript(L"focusEditor();");
}

void CChatInput::WaitTillWebViewReady()
{
	// 如果WebView已经创建且输入界面已初始化，直接返回
	if (_isWebViewCreated && _isInputInitialized)
		return;

	// 等待WebView创建完成
	while (!_isWebViewCreated)
	{
		// 让出CPU时间片，避免忙等待
		Sleep(10);

		// 处理消息队列，确保WebView创建过程能够继续
		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	// 等待输入界面初始化完成
	while (!_isInputInitialized)
	{
		// 让出CPU时间片，避免忙等待
		Sleep(10);

		// 处理消息队列，确保导航和初始化过程能够继续
		MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}
