#include "stdh.h"
#include "ChatSkillsTree.h"
#include <fstream>
#include <algorithm>
#include <sstream>
#include "timer/wuid.h"
#include <nlohmann/json.hpp>
#include "Utils.h"
#include "Utils_Skill.h"

// 外部函数声明
extern std::string widechar_to_utf8(const wchar_t* str);
extern std::wstring utf8_to_widechar(const std::string& utf8_str);
extern std::wstring utf8_to_widechar(const char* utf8_str);
extern std::wstring local_to_widechar(const char* str);
extern const char* GetCurModuleFolderPath_utf8();
extern const char* GetOpenedDBFolderPath_utf8();

//////////////////////////////////////////////////////////////////////////
// CChatSkillsTree

BEGIN_MESSAGE_MAP(CChatSkillsTree, CWnd)
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ACTIVATE()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

CChatSkillsTree::CChatSkillsTree()
	: _isWebViewCreated(false)
	, _isUIInitialized(false)
	, _callbackId(0)
	, _webViewEnvironment(nullptr)
	, _webView(nullptr)
	, _controller(nullptr)
	, _windowWidth(540)
	, _windowHeight(400)
	, _currentProcessId(0)
{
	_currentProcessId = GetCurrentProcessId();
}

CChatSkillsTree::~CChatSkillsTree()
{
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

	SAFE_RELEASE(_webView);
	SAFE_RELEASE(_controller);
	SAFE_RELEASE(_webViewEnvironment);
}

BOOL CChatSkillsTree::CreateSkillsTreeWindow(CWnd* pParent)
{
	// 注册窗口类
	static CString className = AfxRegisterWndClass(
		CS_HREDRAW | CS_VREDRAW | CS_DROPSHADOW,
		::LoadCursor(NULL, IDC_ARROW),
		NULL, // 设置背景画刷为NULL，防止系统擦除背景
		NULL);

	// 创建弹出式窗口
	BOOL result = CreateEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
		className, _T("SkillsTree"),
		WS_POPUP | WS_BORDER | WS_CLIPCHILDREN,
		0, 0, _windowWidth, _windowHeight, pParent->GetSafeHwnd(), NULL);

	// 创建Skill Tip窗口
	if (result)
	{
		_skillTip.CreateSkillTipWindow(this);
	}

	return result;
}

HRESULT CChatSkillsTree::InitializeWebView()
{
	extern const wchar_t* GetWebViewUserFolder();
	HRESULT hr = CreateCoreWebView2EnvironmentWithOptions(nullptr, Utils::GetWebViewUserFolder(), nullptr,
		Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
			[this](HRESULT result, ICoreWebView2Environment* environment) -> HRESULT {
		if (SUCCEEDED(result))
		{
			_webViewEnvironment = environment;
			if (_webViewEnvironment) _webViewEnvironment->AddRef();

			return _webViewEnvironment->CreateCoreWebView2Controller(GetSafeHwnd(),
				Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
					[this](HRESULT result, ICoreWebView2Controller* controller) -> HRESULT {
				if (SUCCEEDED(result))
				{
					_controller = controller;
					if (_controller) _controller->AddRef();

					_controller->put_IsVisible(TRUE);

					_controller->get_CoreWebView2(&_webView);

					RECT bounds;
					GetClientRect(&bounds);
					_controller->put_Bounds(bounds);

#ifdef DISABLE_WEBVIEW_CONTEXTMENU
					ICoreWebView2Settings* settings = nullptr;
					if (SUCCEEDED(_webView->get_Settings(&settings)) && settings)
					{
						settings->put_AreDefaultContextMenusEnabled(FALSE);
						settings->Release();
					}
#endif

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

						if (success && !_isUIInitialized)
						{
							InitializeUI();
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

						std::wstring msgStr(message);
						_HandleWebMessage(msgStr);

						if (_webMessageReceivedCallback)
						{
							_webMessageReceivedCallback(message);
						}
						CoTaskMemFree(message);
						return S_OK;
					}).Get(),
						&_webMessageReceivedToken);

					std::string htmlPath = GetCurModuleFolderPath_utf8();
					htmlPath += "\\ChatSkillsTree.html";
					Navigate(utf8_to_widechar(htmlPath.c_str()));

					_isWebViewCreated = true;
				}
				return S_OK;
			}).Get());
		}
		return S_OK;
	}).Get());

	return hr;
}

void CChatSkillsTree::Navigate(const std::wstring& url)
{
	if (_webView != nullptr)
	{
		_webView->Navigate(url.c_str());
	}
}

void CChatSkillsTree::ExecuteScript(const std::wstring& script, std::function<void(const std::wstring&)> callback)
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

void CChatSkillsTree::SetNavigationCompletedCallback(SkillsTreeNavigationCompletedCallback callback)
{
	_navigationCompletedCallback = callback;
}

void CChatSkillsTree::SetWebMessageReceivedCallback(SkillsTreeMessageReceivedCallback callback)
{
	_webMessageReceivedCallback = callback;
}

void CChatSkillsTree::ResizeWebView()
{
	if (_controller != nullptr)
	{
		RECT bounds;
		GetClientRect(&bounds);
		_controller->put_Bounds(bounds);
	}
}

void CChatSkillsTree::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	ResizeWebView();
}

int CChatSkillsTree::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!_isWebViewCreated)
	{
		HRESULT hr = InitializeWebView();
		if (FAILED(hr))
		{
			TRACE(_T("Failed to initialize WebView2 environment: 0x%08lx\n"), hr);
			return 0;
		}
	}

	return 0;
}

void CChatSkillsTree::OnDestroy()
{
	CWnd::OnDestroy();

	if (_controller != nullptr)
	{
		_controller->Close();
	}
}

void CChatSkillsTree::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CWnd::OnActivate(nState, pWndOther, bMinimized);

	// 当窗口失去激活状态时，隐藏窗口
	if (nState == WA_INACTIVE)
	{
		HideWindow();
	}
}

void CChatSkillsTree::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// 按ESC键关闭窗口
	if (nChar == VK_ESCAPE)
	{
		HideWindow();
		return;
	}

	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

//====================== 弹出窗口显示/隐藏 ======================

void CChatSkillsTree::ShowWindow(const RECT& btnRect)
{
	// 如果WebView未创建，先创建
	if (!_isWebViewCreated)
	{
		HRESULT hr = InitializeWebView();
		if (FAILED(hr))
		{
			TRACE(_T("Failed to initialize WebView2 environment: 0x%08lx\n"), hr);
			return;
		}
	}

	// 刷新Skills树数据
	if (_isUIInitialized)
	{
		SendSkillTreeData();
	}

	// 计算窗口位置和大小
	CRect windowRect = CalculateWindowRect(btnRect);

	// 设置窗口位置和大小，显示并激活窗口
	SetWindowPos(&CWnd::wndTopMost, windowRect.left, windowRect.top,
		windowRect.Width(), windowRect.Height(),
		SWP_SHOWWINDOW);

	// 调整WebView大小
	ResizeWebView();
}

void CChatSkillsTree::HideWindow()
{
	// 隐藏tip窗口
	_skillTip.HideTip();

	if (IsWindowVisible())
	{
		CWnd::ShowWindow(SW_HIDE);
	}
}

CRect CChatSkillsTree::CalculateWindowRect(const RECT& btnRect)
{
	// 使用固定尺寸
	int width = _windowWidth;
	int height = _windowHeight;

	// 获取屏幕工作区
	RECT workArea;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &workArea, 0);

	// 计算窗口位置：默认显示在按钮上方
	int x = btnRect.left;
	int y = btnRect.top - height - 4;

	// 如果上方空间不够，显示在按钮下方
	if (y < workArea.top)
	{
		y = btnRect.bottom + 4;
	}

	// 调整X位置，确保窗口不超出屏幕右侧
	if (x + width > workArea.right)
	{
		x = workArea.right - width;
	}
	if (x < workArea.left)
	{
		x = workArea.left;
	}

	// 调整Y位置，确保窗口不超出屏幕底部
	if (y + height > workArea.bottom)
	{
		y = workArea.bottom - height;
	}
	if (y < workArea.top)
	{
		y = workArea.top;
	}

	return CRect(x, y, x + width, y + height);
}

void CChatSkillsTree::CheckForegroundWindow()
{
	// 获取前台窗口
	HWND foregroundWnd = ::GetForegroundWindow();
	if (foregroundWnd == NULL)
		return;

	// 获取前台窗口的进程ID
	DWORD foregroundProcessId = 0;
	GetWindowThreadProcessId(foregroundWnd, &foregroundProcessId);

	// 如果前台窗口不属于当前进程，隐藏窗口
	if (foregroundProcessId != _currentProcessId)
	{
		HideWindow();
	}
}

void CChatSkillsTree::Update()
{
	if (!IsWindowVisible())
		return;

	CheckForegroundWindow();
}

//====================== UI初始化 ======================

void CChatSkillsTree::InitializeUI()
{
	if (_isUIInitialized)
		return;

	_isUIInitialized = true;

	// 发送Skills树数据
	SendSkillTreeData();

	_PostWebMessage(L"initializeComplete", L"");
}

void CChatSkillsTree::SendSkillTreeData()
{
	if (!_IsReady())
		return;

	std::wstring treeJson = _BuildSkillTreeJson();
	_PostWebMessage(L"setSkillTree", treeJson);
}

//====================== 内部辅助方法 ======================

bool CChatSkillsTree::_IsReady() const
{
	return _isWebViewCreated && _isUIInitialized;
}

void CChatSkillsTree::_PostWebMessage(const std::wstring& action, const std::wstring& data)
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

std::wstring CChatSkillsTree::_EscapeJsonString(const std::wstring& str)
{
	std::wstring result = str;
	std::wstring::size_type pos = 0;
	// 先转义反斜杠
	while ((pos = result.find(L"\\", pos)) != std::wstring::npos)
	{
		result.replace(pos, 1, L"\\\\");
		pos += 2;
	}
	// 再转义双引号
	pos = 0;
	while ((pos = result.find(L"\"", pos)) != std::wstring::npos)
	{
		result.replace(pos, 1, L"\\\"");
		pos += 2;
	}
	return result;
}

void CChatSkillsTree::_HandleWebMessage(const std::wstring& message)
{
	std::string utf8Message = widechar_to_utf8(message.c_str());

	try {
		nlohmann::json jsonMsg = nlohmann::json::parse(utf8Message);

		if (!jsonMsg.contains("action") || !jsonMsg["action"].is_string())
		{
			return;
		}

		std::string action = jsonMsg["action"];

		if (action == "nodeDoubleClicked")
		{
			// 处理节点双击事件（打开skill）
			if (jsonMsg.contains("data") && jsonMsg["data"].is_object())
			{
				auto& data = jsonMsg["data"];
				if (data.contains("fullPath") && data["fullPath"].is_string())
				{
					std::string skillPath = data["fullPath"];
					// 回调通知选中的Skill
					if (_skillSelectedCallback)
					{
						_skillSelectedCallback(utf8_to_widechar(skillPath));
					}
					// 隐藏窗口
					HideWindow();
				}
			}
		}
		else if (action == "renameSkill")
		{
			// 处理Skill重命名事件
			if (jsonMsg.contains("data") && jsonMsg["data"].is_object())
			{
				auto& data = jsonMsg["data"];
				if (data.contains("oldName") && data["oldName"].is_string() &&
					data.contains("newName") && data["newName"].is_string() &&
					data.contains("fullPath") && data["fullPath"].is_string())
				{
					std::string oldName = data["oldName"];
					std::string newName = data["newName"];
					std::string fullPath = data["fullPath"];

			if (!oldName.empty() && !newName.empty() && !fullPath.empty() && oldName != newName)
				{
					bool isLeaf = data.contains("isLeaf") && data["isLeaf"].is_boolean() ? data["isLeaf"].get<bool>() : true;
					_RenameSkill(utf8_to_widechar(oldName), utf8_to_widechar(newName), utf8_to_widechar(fullPath), isLeaf);
				}
				}
			}
		}
		else if (action == "skillChecked")
		{
			// 处理Skill checkbox选中事件
			if (jsonMsg.contains("data") && jsonMsg["data"].is_object())
			{
				auto& data = jsonMsg["data"];
				if (data.contains("fullPath") && data["fullPath"].is_string() &&
					data.contains("checked") && data["checked"].is_boolean())
				{
					std::string skillPath = data["fullPath"];
					bool checked = data["checked"].get<bool>();

					// 调用回调通知启用状态变化
					if (_skillEnableChangedCallback)
					{
						_skillEnableChangedCallback(utf8_to_widechar(skillPath), checked);
					}
				}
			}
		}
		else if (action == "escapePressed")
		{
			// 处理ESC键按下，关闭窗口
			HideWindow();
		}
		else if (action == "openFolder")
		{
			// 处理打开目录
			if (jsonMsg.contains("data") && jsonMsg["data"].is_object())
			{
				auto& data = jsonMsg["data"];
				if (data.contains("fullPath") && data["fullPath"].is_string())
				{
					std::string folderPath = data["fullPath"];

					Utils::EnsureFolder(folderPath.c_str());
					// 使用ShellExecuteW打开目录（路径需要转换为宽字符以支持UTF-8）
					std::wstring wFolderPath = utf8_to_widechar(folderPath);
					ShellExecuteW(NULL, L"open", wFolderPath.c_str(), NULL, NULL, SW_SHOWNORMAL);
				}
			}
		}
		else if (action == "newFolder")
		{
			// 处理新建目录
			if (jsonMsg.contains("data") && jsonMsg["data"].is_object())
			{
				auto& data = jsonMsg["data"];
				if (data.contains("fullPath") && data["fullPath"].is_string() &&
					data.contains("newFolderName") && data["newFolderName"].is_string())
				{
					std::string parentPath = data["fullPath"];
					std::string newFolderName = data["newFolderName"];

					if (!parentPath.empty() && !newFolderName.empty())
					{
						_CreateNewFolder(utf8_to_widechar(parentPath), utf8_to_widechar(newFolderName));
					}
				}
			}
		}
		else if (action == "newSkill")
		{
			// 处理新建Skill
			if (jsonMsg.contains("data") && jsonMsg["data"].is_object())
			{
				auto& data = jsonMsg["data"];
				if (data.contains("fullPath") && data["fullPath"].is_string() &&
					data.contains("newSkillName") && data["newSkillName"].is_string())
				{
					std::string parentPath = data["fullPath"];
					std::string newSkillName = data["newSkillName"];

					if (!parentPath.empty() && !newSkillName.empty())
					{
						_CreateNewSkill(utf8_to_widechar(parentPath), utf8_to_widechar(newSkillName));
					}
				}
			}
		}
		else if (action == "showSkillTip")
		{
			// 显示Skill Tip
			if (jsonMsg.contains("data") && jsonMsg["data"].is_object())
			{
				auto& data = jsonMsg["data"];
				if (data.contains("fullPath") && data["fullPath"].is_string() &&
					data.contains("anchorRect") && data["anchorRect"].is_object())
				{
					std::string skillPath = data["fullPath"];
					auto& anchorRect = data["anchorRect"];
					
					// 构建SKILL.md文件路径
					std::wstring skillMdPath = utf8_to_widechar(skillPath) + L"\\SKILL.md";
					
					// 获取锚点矩形（相对于屏幕坐标）
					RECT rect;
					rect.left = anchorRect.value("left", 0);
					rect.top = anchorRect.value("top", 0);
					rect.right = anchorRect.value("right", 0);
					rect.bottom = anchorRect.value("bottom", 0);
					
					// 将ClientRect转换为ScreenRect
					// 由于HTML中的getBoundingClientRect返回的是相对于viewport的坐标
					// 需要转换为屏幕坐标
					POINT pt = { rect.left, rect.top };
					ClientToScreen(&pt);
					rect.left = pt.x;
					rect.top = pt.y;
					
					pt.x = rect.right;
					pt.y = rect.bottom;
					ClientToScreen(&pt);
					rect.right = pt.x;
					rect.bottom = pt.y;
					
					_skillTip.ShowTip(rect, skillMdPath);
				}
			}
		}
		else if (action == "hideSkillTip")
		{
			// 隐藏Skill Tip
			_skillTip.HideTip();
		}
	}
	catch (const std::exception& e)
	{
		TRACE(_T("Failed to parse WebView message: %s\n"), utf8_to_widechar(e.what()).c_str());
	}
}

// ======================== 构建Skills树JSON ========================

static std::wstring _ToWide(const std::string& s)
{
	return utf8_to_widechar(s);
}

static std::string _SkillTypeToString(CLlmSkills::Skill::Type tp)
{
	switch (tp)
	{
	case CLlmSkills::Skill::Type::BuiltIn: return "BuiltIn";
	case CLlmSkills::Skill::Type::Global:  return "Global";
	case CLlmSkills::Skill::Type::Project: return "Project";
	default: return "Unknown";
	}
}

// 树节点结构（用于内部构建）
struct SkillTreeNode
{
	std::string name;           // 节点显示名称
	std::string fullPath;       // 完整路径（仅叶子节点有效）
	bool isLeaf;                // 是否是叶子节点（Skill所在目录）
	bool enable;                // 是否启用（仅叶子节点有效）
	std::map<std::string, SkillTreeNode> children;  // 子节点（key=名称）
};

// 递归将树节点转换为JSON对象
static nlohmann::json _NodeToJson(const SkillTreeNode& node)
{
	nlohmann::json j;
	j["name"] = node.name;
	j["isLeaf"] = node.isLeaf;

	// 所有节点都输出fullPath（用于打开目录功能）
	if (!node.fullPath.empty())
	{
		j["fullPath"] = node.fullPath;
	}

	if (node.isLeaf)
	{
		j["enable"] = node.enable;
	}

	if (!node.children.empty())
	{
		j["children"] = nlohmann::json::array();
		
		// 收集所有子节点到vector以便排序
		std::vector<const SkillTreeNode*> childrenVec;
		for (const auto& child : node.children)
		{
			childrenVec.push_back(&child.second);
		}
		
		// 排序：folder（非叶子）排在前面，叶子排在后面，同类之间按名称字母顺序
		std::sort(childrenVec.begin(), childrenVec.end(), [](const SkillTreeNode* a, const SkillTreeNode* b) {
			if (a->isLeaf != b->isLeaf)
				return !a->isLeaf; // folder（非叶子）排在前面
			return a->name < b->name; // 字母顺序
		});
		
		for (const auto* child : childrenVec)
		{
			j["children"].push_back(_NodeToJson(*child));
		}
	}

	return j;
}

// 将相对路径按'\\'拆分为路径段
static std::vector<std::string> _SplitPath(const std::string& path)
{
	std::vector<std::string> parts;
	std::istringstream ss(path);
	std::string part;
	while (std::getline(ss, part, '\\'))
	{
		if (!part.empty())
			parts.push_back(part);
	}
	return parts;
}

void CChatSkillsTree::_RenameSkill(const std::wstring& oldName, const std::wstring& newName, const std::wstring& fullPath, bool isLeaf)
{
	if (oldName.empty() || newName.empty() || fullPath.empty())
		return;

	// 1. 构建路径
	std::wstring parentPath;
	std::wstring oldFolderName;
	
	size_t lastSlash = fullPath.find_last_of(L'\\');
	if (lastSlash != std::wstring::npos)
	{
		parentPath = fullPath.substr(0, lastSlash);
		oldFolderName = fullPath.substr(lastSlash + 1);
	}
	else
	{
		parentPath = L"";
		oldFolderName = fullPath;
	}

	if (parentPath.empty())
		return;

	// 新目录路径
	std::wstring newFullPath = parentPath + L"\\" + newName;

	std::string newContent;
	bool needUpdateSkillMd = false;

	// 如果是叶子节点（skill目录），需要先修改 SKILL.md 文件中的 name 字段
	if (isLeaf)
	{
		std::wstring skillMdPath = fullPath + L"\\SKILL.md";
		
		// 读取文件内容
		std::ifstream inFile(skillMdPath, std::ios::binary);
		if (inFile.is_open())
		{
			std::string content((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
			inFile.close();

			// 将内容转换为宽字符以便处理
			std::wstring wContent = utf8_to_widechar(content);

			// 查找并替换 name 字段
			std::wstring namePattern = L"name:";
			size_t namePos = wContent.find(namePattern);
			if (namePos != std::wstring::npos)
			{
				// 找到 name: 后面的值的位置
				size_t valueStart = namePos + namePattern.length();
				// 跳过空白字符
				while (valueStart < wContent.length() && (wContent[valueStart] == L' ' || wContent[valueStart] == L'\t'))
					valueStart++;

				// 找到行尾或下一个换行符
				size_t valueEnd = valueStart;
				while (valueEnd < wContent.length() && wContent[valueEnd] != L'\n' && wContent[valueEnd] != L'\r')
					valueEnd++;

				// 替换 name 的值
				if (valueStart < valueEnd)
				{
					wContent.replace(valueStart, valueEnd - valueStart, newName);
					needUpdateSkillMd = true;
				}
			}

			// 将修改后的内容转回UTF-8
			newContent = widechar_to_utf8(wContent.c_str());
		}
	}

	// 2. 重命名目录
	BOOL moveResult = MoveFileW(fullPath.c_str(), newFullPath.c_str());
	if (!moveResult)
	{
		DWORD error = GetLastError();
		TRACE(_T("Failed to rename skill folder from '%s' to '%s', error: %d\n"),
			fullPath.c_str(), newFullPath.c_str(), error);
		return;
	}

	// 3. 如果是叶子节点，写入修改后的 SKILL.md 内容
	if (isLeaf && needUpdateSkillMd)
	{
		std::wstring newSkillMdPath = newFullPath + L"\\SKILL.md";
		std::ofstream outFile(newSkillMdPath, std::ios::binary);
		if (outFile.is_open())
		{
			outFile.write(newContent.c_str(), newContent.length());
			outFile.close();
		}
		else
		{
			TRACE(_T("Failed to write modified SKILL.md after rename\n"));
		}
	}

	// 4. 重新加载skills并刷新UI
	Utils::LoadLlmSkills(g_llmSkills, GetOpenedDBFolderPath_utf8());
	SendSkillTreeData();
}

void CChatSkillsTree::_CreateNewFolder(const std::wstring& parentPath, const std::wstring& folderName)
{
	if (parentPath.empty() || folderName.empty())
		return;

	// 构建新目录的完整路径
	std::wstring newFolderPath = parentPath + L"\\" + folderName;

	// 检查目录是否已存在（不应该发生，因为JS端已经处理了唯一性）
	DWORD attribs = GetFileAttributesW(newFolderPath.c_str());
	if (attribs != INVALID_FILE_ATTRIBUTES && (attribs & FILE_ATTRIBUTE_DIRECTORY))
	{
		// 目录已存在，不执行任何操作
		TRACE(_T("Folder already exists: %s\n"), newFolderPath.c_str());
		return;
	}

	// 创建目录
	BOOL result = CreateDirectoryW(newFolderPath.c_str(), NULL);
	if (!result)
	{
		DWORD error = GetLastError();
		TRACE(_T("Failed to create folder '%s', error: %d\n"), newFolderPath.c_str(), error);
		return;
	}

	// 刷新UI
	Utils::LoadLlmSkills(g_llmSkills, GetOpenedDBFolderPath_utf8());
	SendSkillTreeData();
}

void CChatSkillsTree::_CreateNewSkill(const std::wstring& parentPath, const std::wstring& skillName)
{
	if (parentPath.empty() || skillName.empty())
		return;

	// 构建新Skill目录的完整路径
	std::wstring newSkillPath = parentPath + L"\\" + skillName;

	// 检查目录是否已存在（不应该发生，因为JS端已经处理了唯一性）
	DWORD attribs = GetFileAttributesW(newSkillPath.c_str());
	if (attribs != INVALID_FILE_ATTRIBUTES && (attribs & FILE_ATTRIBUTE_DIRECTORY))
	{
		// 目录已存在，不执行任何操作
		TRACE(_T("Skill folder already exists: %s\n"), newSkillPath.c_str());
		return;
	}

	// 创建目录
	BOOL result = CreateDirectoryW(newSkillPath.c_str(), NULL);
	if (!result)
	{
		DWORD error = GetLastError();
		TRACE(_T("Failed to create skill folder '%s', error: %d\n"), newSkillPath.c_str(), error);
		return;
	}

	// 创建SKILL.md文件
	std::wstring skillMdPath = newSkillPath + L"\\SKILL.md";
	
	// 构建SKILL.md内容
	std::string skillNameUtf8 = widechar_to_utf8(skillName.c_str());
	std::string skillContent = "---\n";
	skillContent += "name: " + skillNameUtf8 + "\n";
	skillContent += "description: A new skill\n";
	skillContent += "---\n\n";
	skillContent += "# " + skillNameUtf8 + "\n\n";
	skillContent += "Add your skill instructions here.\n";

	// 写入文件
	std::ofstream outFile(skillMdPath, std::ios::binary);
	if (outFile.is_open())
	{
		outFile.write(skillContent.c_str(), skillContent.length());
		outFile.close();
	}
	else
	{
		TRACE(_T("Failed to create SKILL.md for new skill\n"));
	}

	// 刷新UI
	Utils::LoadLlmSkills(g_llmSkills, GetOpenedDBFolderPath_utf8());
	SendSkillTreeData();
}

std::wstring CChatSkillsTree::_BuildSkillTreeJson()
{
	// 获取三种类型的根目录
	auto folders = Utils::GetSkillsFolder(GetOpenedDBFolderPath_utf8());

	// 构建类型根路径映射
	std::map<std::string, std::string> typeToRootPath;
	for (const auto& folder : folders)
	{
		typeToRootPath[_SkillTypeToString(folder.second)] = folder.first;
	}

	// 三个根节点（初始化时设置fullPath）
	SkillTreeNode rootBuiltIn;
	rootBuiltIn.name = "BuiltIn";
	rootBuiltIn.isLeaf = false;
	rootBuiltIn.fullPath = typeToRootPath["BuiltIn"];

	SkillTreeNode rootGlobal;
	rootGlobal.name = "Global";
	rootGlobal.isLeaf = false;
	rootGlobal.fullPath = typeToRootPath["Global"];

	SkillTreeNode rootProject;
	rootProject.name = "Project";
	rootProject.isLeaf = false;
	rootProject.fullPath = typeToRootPath["Project"];

	// 构建类型名到根节点指针的映射
	std::map<std::string, SkillTreeNode*> typeToRoot;
	typeToRoot["BuiltIn"] = &rootBuiltIn;
	typeToRoot["Global"] = &rootGlobal;
	typeToRoot["Project"] = &rootProject;

	// 遍历 g_llmSkills 中的所有 Skill
	for (const auto& skill : g_llmSkills._skills)
	{
		std::string typeStr = _SkillTypeToString(skill.tp);

		auto rootIt = typeToRoot.find(typeStr);
		if (rootIt == typeToRoot.end())
			continue;

		auto pathIt = typeToRootPath.find(typeStr);
		if (pathIt == typeToRootPath.end())
			continue;

		// 将 folderPath 转换为相对于类型根目录的路径
		std::string relativePath;
		const std::string& rootPath = pathIt->second;
		if (skill.folderPath.size() > rootPath.size() &&
			_strnicmp(skill.folderPath.c_str(), rootPath.c_str(), rootPath.size()) == 0)
		{
			// 跳过根路径前缀，去掉开头的 '\\'
			relativePath = skill.folderPath.substr(rootPath.size());
			while (!relativePath.empty() && (relativePath[0] == '\\' || relativePath[0] == '/'))
				relativePath = relativePath.substr(1);
		}
		else
		{
			// fallback: 使用 folderPath 的最后一段作为叶子节点名
			size_t lastSlash = skill.folderPath.rfind('\\');
			if (lastSlash != std::string::npos)
				relativePath = skill.folderPath.substr(lastSlash + 1);
			else
				relativePath = skill.folderPath;
		}

		if (relativePath.empty())
			continue;

		// 按路径拆分
		std::vector<std::string> parts = _SplitPath(relativePath);
		if (parts.empty())
			continue;

		// 从根节点开始，沿着路径创建/查找节点
		SkillTreeNode* current = rootIt->second;
		std::string currentPath = pathIt->second; // 当前节点的完整路径
		for (size_t i = 0; i < parts.size(); i++)
		{
			// 更新当前路径
			if (i > 0)
				currentPath += "\\" + parts[i];
			else
				currentPath += "\\" + parts[i];

			auto childIt = current->children.find(parts[i]);
			if (childIt == current->children.end())
			{
			// 创建新节点
				SkillTreeNode newNode;
				newNode.name = parts[i];
				// 最后一段且 name 不为空才是叶子节点（skill 目录）
				// 如果 name 为空，说明是空目录，显示为目录节点
				newNode.isLeaf = (i == parts.size() - 1) && !skill.name.empty();
				newNode.fullPath = currentPath;  // 设置完整路径
				if (newNode.isLeaf)
				{
					newNode.enable = skill.enable;
				}
				current->children[parts[i]] = newNode;
				current = &current->children[parts[i]];
			}
			else
			{
				current = &childIt->second;
				// 如果已经存在且当前parts[i]是路径的最后一段
				if (i == parts.size() - 1)
				{
					// 只有 name 不为空时才标记为叶子节点
					if (!skill.name.empty())
					{
						current->isLeaf = true;
						current->fullPath = skill.folderPath;
						current->enable = skill.enable;
					}
				}
			}
		}
	}

	// 构建顶层JSON数组
	nlohmann::json jsonArr = nlohmann::json::array();
	jsonArr.push_back(_NodeToJson(rootBuiltIn));
	jsonArr.push_back(_NodeToJson(rootGlobal));
	jsonArr.push_back(_NodeToJson(rootProject));

	// 转换为UTF-8字符串，再转为wstring
	std::string utf8Json = jsonArr.dump();
	return utf8_to_widechar(utf8Json);
}

