#include "stdh.h"
#include "ChatMcpsTree.h"
#include <fstream>
#include <algorithm>
#include <sstream>
#include <nlohmann/json.hpp>
#include "Utils.h"
#include "Utils_Mcp.h"

// 外部函数声明
extern std::string widechar_to_utf8(const wchar_t* str);
extern std::wstring utf8_to_widechar(const std::string& utf8_str);
extern std::wstring utf8_to_widechar(const char* utf8_str);
extern const char* GetCurModuleFolderPath_utf8();

//////////////////////////////////////////////////////////////////////////
// CChatMcpsTree

BEGIN_MESSAGE_MAP(CChatMcpsTree, CWnd)
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ACTIVATE()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

CChatMcpsTree::CChatMcpsTree()
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

CChatMcpsTree::~CChatMcpsTree()
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

BOOL CChatMcpsTree::CreateMcpsTreeWindow(CWnd* pParent)
{
	// 注册窗口类
	static CString className = AfxRegisterWndClass(
		CS_HREDRAW | CS_VREDRAW | CS_DROPSHADOW,
		::LoadCursor(NULL, IDC_ARROW),
		NULL,
		NULL);

	// 创建弹出式窗口
	BOOL result = CreateEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
		className, _T("McpsTree"),
		WS_POPUP | WS_BORDER | WS_CLIPCHILDREN,
		0, 0, _windowWidth, _windowHeight, pParent->GetSafeHwnd(), NULL);

	// 创建MCP Tip子窗口
	if (result)
	{
		_mcpTip.CreateMcpTipWindow(this);
	}

	return result;
}

HRESULT CChatMcpsTree::InitializeWebView()
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
					htmlPath += "\\ChatMcpsTree.html";
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

void CChatMcpsTree::Navigate(const std::wstring& url)
{
	if (_webView != nullptr)
	{
		_webView->Navigate(url.c_str());
	}
}

void CChatMcpsTree::ExecuteScript(const std::wstring& script, std::function<void(const std::wstring&)> callback)
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

void CChatMcpsTree::SetNavigationCompletedCallback(McpsTreeNavigationCompletedCallback callback)
{
	_navigationCompletedCallback = callback;
}

void CChatMcpsTree::SetWebMessageReceivedCallback(McpsTreeMessageReceivedCallback callback)
{
	_webMessageReceivedCallback = callback;
}

void CChatMcpsTree::ResizeWebView()
{
	if (_controller != nullptr)
	{
		RECT bounds;
		GetClientRect(&bounds);
		_controller->put_Bounds(bounds);
	}
}

void CChatMcpsTree::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	ResizeWebView();
}

int CChatMcpsTree::OnCreate(LPCREATESTRUCT lpCreateStruct)
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

void CChatMcpsTree::OnDestroy()
{
	CWnd::OnDestroy();

	if (_controller != nullptr)
	{
		_controller->Close();
	}
}

void CChatMcpsTree::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
	CWnd::OnActivate(nState, pWndOther, bMinimized);

	// 当窗口失去激活状态时，隐藏窗口
	if (nState == WA_INACTIVE)
	{
		HideWindow();
	}
}

void CChatMcpsTree::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
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

void CChatMcpsTree::ShowWindow(const RECT& btnRect)
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

	// 刷新Mcps树数据和权限状态
	if (_isUIInitialized)
	{
		SendMcpTreeData();
		SendEnableModify();
	}

	// 计算窗口位置和大小
	CRect windowRect = CalculateWindowRect(btnRect);

	// 设置窗口位置和大小，显示并激活窗口
	SetWindowPos(&CWnd::wndTopMost, windowRect.left, windowRect.top,
		windowRect.Width(), windowRect.Height(),
		SWP_SHOWWINDOW);

	// 调整WebView大小
	ResizeWebView();

//	EnableModify(false);
}

void CChatMcpsTree::HideWindow()
{
	_mcpTip.HideTip();
	if (IsWindowVisible())
	{
		CWnd::ShowWindow(SW_HIDE);
	}
}

CRect CChatMcpsTree::CalculateWindowRect(const RECT& btnRect)
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

void CChatMcpsTree::CheckForegroundWindow()
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

void CChatMcpsTree::Update()
{
	if (!IsWindowVisible())
		return;

	CheckForegroundWindow();
	_mcpTip.Update();
}

//====================== 修改权限控制 ======================

void CChatMcpsTree::EnableModify(bool enable)
{
	_enableModify = enable;
	// 如果UI已初始化，立即发送状态更新
	if (_isUIInitialized)
	{
		SendEnableModify();
	}
}

void CChatMcpsTree::SendEnableModify()
{
	if (!_IsReady())
		return;

	_PostWebMessage(L"setEnableModify", _enableModify ? L"true" : L"false");
}

//====================== UI初始化 ======================

void CChatMcpsTree::InitializeUI()
{
	if (_isUIInitialized)
		return;

	_isUIInitialized = true;

	// 发送Mcps树数据
	SendMcpTreeData();

	// 发送修改权限状态
	SendEnableModify();

	_PostWebMessage(L"initializeComplete", L"");
}

void CChatMcpsTree::SendMcpTreeData()
{
	if (!_IsReady())
		return;

	std::wstring treeJson = _BuildMcpTreeJson();
	_PostWebMessage(L"setMcpTree", treeJson);
}

//====================== 内部辅助方法 ======================

bool CChatMcpsTree::_IsReady() const
{
	return _isWebViewCreated && _isUIInitialized;
}

void CChatMcpsTree::_PostWebMessage(const std::wstring& action, const std::wstring& data)
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

std::wstring CChatMcpsTree::_EscapeJsonString(const std::wstring& str)
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

// ======================== 构建Mcps树JSON ========================

static std::string _McpTypeToString(CLlmMcps::Mcp::Type tp)
{
	switch (tp)
	{
	case CLlmMcps::Mcp::Type::Global:   return "Global";
	case CLlmMcps::Mcp::Type::Project:  return "Project";
	default: return "Unknown";
	}
}




// MCP树节点（用于内部构建，类似SkillTreeNode）
struct McpTreeNode
{
	std::string name;
	std::string fullPath;       // 完整路径
	bool isLeaf;                // true=MCP节点（包含tools），false=目录
	bool enable;                // MCP是否启用（仅叶子有效）
	std::string description;    // MCP描述（仅叶子有效）
	bool toolsLoaded;           // tools是否已加载（仅叶子有效）
	std::string lastError;      // 最后错误（仅叶子有效）
	std::string type;           // "Global"/"Project"（仅叶子有效）
	WUID uid = 0;              // 唯一标识（仅叶子有效）
	std::map<std::string, McpTreeNode> children;
	// tools单独存储，在叶子节点转JSON时使用
	std::vector<CLlmMcps::Mcp::Tool> tools;
	std::unordered_set<std::string> disabledTools;
};

// 递归将树节点转换为JSON对象，rootType从根节点向下传播
static nlohmann::json _McpNodeToJson(const McpTreeNode& node, const std::string& rootType = "")
{
	nlohmann::json j;
	j["name"] = node.name;
	j["isLeaf"] = node.isLeaf;

	if (!node.fullPath.empty())
		j["fullPath"] = node.fullPath;

	// 所有非根节点都携带type（从根节点继承）
	if (!rootType.empty())
		j["type"] = rootType;

	if (node.isLeaf)
	{
		j["enable"] = node.enable;
		j["description"] = node.description;
		j["toolsLoaded"] = node.toolsLoaded;
		j["lastError"] = node.lastError;
		j["type"] = node.type;
		j["uid"] = std::to_string(node.uid);

		// 添加tools子节点
		j["children"] = nlohmann::json::array();
		for (const auto& tool : node.tools)
		{
			nlohmann::json toolNode;
			toolNode["name"] = tool.name;
			toolNode["isLeaf"] = true;
			toolNode["fullPath"] = node.fullPath;
			toolNode["enable"] = node.disabledTools.find(tool.name) == node.disabledTools.end();
			toolNode["description"] = tool.description;
			toolNode["mcpName"] = node.name;
			toolNode["mcpType"] = node.type;
			toolNode["uid"] = std::to_string(node.uid);
			j["children"].push_back(toolNode);
		}
	}
	else if (!node.children.empty())
	{
		j["children"] = nlohmann::json::array();

		// 收集子节点，排序：目录在前，MCP在后
		std::vector<const McpTreeNode*> childrenVec;
		for (const auto& child : node.children)
			childrenVec.push_back(&child.second);

		std::sort(childrenVec.begin(), childrenVec.end(), [](const McpTreeNode* a, const McpTreeNode* b) {
			if (a->isLeaf != b->isLeaf)
				return !a->isLeaf; // 目录在前
			return a->name < b->name;
		});

		for (const auto* child : childrenVec)
			j["children"].push_back(_McpNodeToJson(*child, rootType));
	}

	return j;
}

// 将路径按'\\'拆分
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

std::wstring CChatMcpsTree::_BuildMcpTreeJson()
{
	// 构建类型根路径映射
	std::map<std::string, std::string> typeToRootPath;
	typeToRootPath["Global"] = Utils::GetGlobalMcpsFolder();
	typeToRootPath["Project"] = Utils::GetProjectMcpsFolder();

	// 两个根节点
	McpTreeNode rootGlobal;
	rootGlobal.name = "Global";
	rootGlobal.isLeaf = false;
	rootGlobal.fullPath = typeToRootPath["Global"];

	McpTreeNode rootProject;
	rootProject.name = "Project";
	rootProject.isLeaf = false;
	rootProject.fullPath = typeToRootPath["Project"];

	std::map<std::string, McpTreeNode*> typeToRoot;
	typeToRoot["Global"] = &rootGlobal;
	typeToRoot["Project"] = &rootProject;

	// 遍历 g_llmMcps 中的所有 Mcp
	for (const auto& mcp : g_llmMcps._mcps)
	{
		std::string typeStr = _McpTypeToString(mcp.tp);

		auto rootIt = typeToRoot.find(typeStr);
		if (rootIt == typeToRoot.end())
			continue;

		auto pathIt = typeToRootPath.find(typeStr);
		if (pathIt == typeToRootPath.end() || pathIt->second.empty())
			continue;

		// 将 folderPath 转换为相对于类型根目录的路径
		std::string relativePath;
		const std::string& rootPath = pathIt->second;
		if (mcp.folderPath.size() > rootPath.size() &&
			_strnicmp(mcp.folderPath.c_str(), rootPath.c_str(), rootPath.size()) == 0)
		{
			relativePath = mcp.folderPath.substr(rootPath.size());
			while (!relativePath.empty() && (relativePath[0] == '\\' || relativePath[0] == '/'))
				relativePath = relativePath.substr(1);
		}
		else
		{
			size_t lastSlash = mcp.folderPath.rfind('\\');
			if (lastSlash != std::string::npos)
				relativePath = mcp.folderPath.substr(lastSlash + 1);
			else
				relativePath = mcp.folderPath;
		}

		if (relativePath.empty())
			continue;

		std::vector<std::string> parts = _SplitPath(relativePath);
		if (parts.empty())
			continue;

		// 从根节点开始，沿着路径创建/查找节点
		McpTreeNode* current = rootIt->second;
		std::string currentPath = rootPath;
		for (size_t i = 0; i < parts.size(); i++)
		{
			currentPath += "\\" + parts[i];

			auto childIt = current->children.find(parts[i]);
			if (childIt == current->children.end())
			{
				McpTreeNode newNode;
				newNode.name = parts[i];
				newNode.fullPath = currentPath;

				if (i == parts.size() - 1)
				{
					if (!mcp.name.empty())
					{
					// MCP节点（叶子）
						newNode.isLeaf = true;
						newNode.enable = mcp.enable;
						newNode.description = mcp.description;
						newNode.toolsLoaded = mcp.toolsLoaded;
						newNode.lastError = mcp.lastError;
						newNode.type = typeStr;
						newNode.uid = mcp.uid;
						newNode.tools = mcp.tools;
						newNode.disabledTools = mcp.disabledTools;
					}
					else
					{
						// name为空的空目录
						newNode.isLeaf = false;
					}
				}
				else
				{
					newNode.isLeaf = false;
				}

				current->children[parts[i]] = newNode;
				current = &current->children[parts[i]];
			}
			else
			{
				current = &childIt->second;
				if (i == parts.size() - 1 && !mcp.name.empty())
				{
					current->isLeaf = true;
					current->fullPath = mcp.folderPath;
					current->enable = mcp.enable;
					current->description = mcp.description;
					current->toolsLoaded = mcp.toolsLoaded;
					current->lastError = mcp.lastError;
					current->type = typeStr;
					current->uid = mcp.uid;
					current->tools = mcp.tools;
					current->disabledTools = mcp.disabledTools;
				}
			}
		}
	}

	// 构建顶层JSON数组
	nlohmann::json jsonArr = nlohmann::json::array();
	jsonArr.push_back(_McpNodeToJson(rootGlobal, "Global"));
	jsonArr.push_back(_McpNodeToJson(rootProject, "Project"));

	std::string utf8Json = jsonArr.dump();
	return utf8_to_widechar(utf8Json);
}

void CChatMcpsTree::_HandleWebMessage(const std::wstring& message)
{
	std::string utf8Message = widechar_to_utf8(message.c_str());

	try {
		nlohmann::json jsonMsg = nlohmann::json::parse(utf8Message);

		if (!jsonMsg.contains("action") || !jsonMsg["action"].is_string())
		{
			return;
		}

		std::string action = jsonMsg["action"];

		// 修改操作：检查是否允许修改
		bool isModifyAction = (action == "mcpChecked" || action == "mcpToolChecked" ||
		                       action == "newFolder" || action == "newMcp" || action == "renameMcp");
		if (isModifyAction && !_enableModify)
		{
			// 只读模式，忽略修改操作
			return;
		}

		if (action == "mcpChecked")
		{
			// 直接处理MCP启用/禁用，调用Utils::EnableMcps修改setting文件
			if (jsonMsg.contains("data") && jsonMsg["data"].is_object())
			{
				auto& data = jsonMsg["data"];
				if (!data.contains("checked") || !data["checked"].is_boolean())
					return;
				bool checked = data["checked"].get<bool>();

				std::vector<WUID> uids;

				// 支持uids数组（根/目录节点批量操作）
				if (data.contains("uids") && data["uids"].is_array())
				{
					for (const auto& uidVal : data["uids"])
					{
						if (uidVal.is_string())
							uids.push_back(std::stoull(uidVal.get<std::string>()));
					}
				}
				// 兼容单个uid（单个MCP节点操作）
				else if (data.contains("uid") && data["uid"].is_string())
				{
					uids.push_back(std::stoull(data["uid"].get<std::string>()));
				}

				if (!uids.empty())
				{
					Utils::EnableMcps(checked, uids);
				}
			}
		}
		else if (action == "mcpToolChecked")
		{
			// 处理MCP Tool checkbox选中事件，直接修改setting文件
			if (jsonMsg.contains("data") && jsonMsg["data"].is_object())
			{
				auto& data = jsonMsg["data"];
				if (data.contains("uid") && data["uid"].is_string() &&
					data.contains("checked") && data["checked"].is_boolean())
				{
					std::string uidStr = data["uid"];
					bool checked = data["checked"].get<bool>();
					WUID mcpUid = std::stoull(uidStr);

					std::vector<std::string> toolNames;
					if (data.contains("toolNames") && data["toolNames"].is_array())
					{
						for (auto& t : data["toolNames"])
						{
							if (t.is_string())
								toolNames.push_back(t.get<std::string>());
						}
					}
					else if (data.contains("toolName") && data["toolName"].is_string())
					{
						toolNames.push_back(data["toolName"].get<std::string>());
					}

					if (!toolNames.empty())
					{
						// checked=true 表示启用工具（从disabledTools中移除）
						// checked=false 表示禁用工具（添加到disabledTools）
						Utils::EnableMcpTools(checked, mcpUid, toolNames);
					}
				}
			}
		}
		else if (action == "openMcpJson")
		{
			// 处理打开MCP.json文件（未enable的MCP双击）
			if (jsonMsg.contains("data") && jsonMsg["data"].is_object())
			{
				auto& data = jsonMsg["data"];
				if (data.contains("fullPath") && data["fullPath"].is_string())
				{
					std::string folderPath = data["fullPath"];
					// 构建MCP.json完整路径
					std::wstring wFolderPath = utf8_to_widechar(folderPath);
					std::wstring jsonPath = wFolderPath + L"\\MCP.json";
					
					// 检查文件是否存在
					if (GetFileAttributesW(jsonPath.c_str()) != INVALID_FILE_ATTRIBUTES)
					{
						// 回调通知打开MCP.json文件
						if (_mcpJsonOpenCallback)
						{
							_mcpJsonOpenCallback(jsonPath);
						}
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
		else if (action == "newMcp")
		{
			// 处理新建MCP
			if (jsonMsg.contains("data") && jsonMsg["data"].is_object())
			{
				auto& data = jsonMsg["data"];
				if (data.contains("fullPath") && data["fullPath"].is_string() &&
					data.contains("newMcpName") && data["newMcpName"].is_string() &&
					data.contains("mcpType") && data["mcpType"].is_string())
				{
					std::string parentPath = data["fullPath"];
					std::string newMcpName = data["newMcpName"];
					std::string mcpType = data["mcpType"];

					if (!parentPath.empty() && !newMcpName.empty())
					{
						_CreateNewMcp(utf8_to_widechar(parentPath), utf8_to_widechar(newMcpName), utf8_to_widechar(mcpType));
					}
				}
			}
		}
		else if (action == "renameMcp")
		{
			// 处理MCP重命名
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
						_RenameMcp(utf8_to_widechar(oldName), utf8_to_widechar(newName), utf8_to_widechar(fullPath));
					}
				}
			}
		}
		else if (action == "showMcpTip")
		{
			// 显示MCP Tool Tip
			if (jsonMsg.contains("data") && jsonMsg["data"].is_object())
			{
				auto& data = jsonMsg["data"];
				if (data.contains("uid") && data["uid"].is_string() &&
					data.contains("toolName") && data["toolName"].is_string() &&
					data.contains("anchorRect") && data["anchorRect"].is_object())
				{
					WUID uid = std::stoull(data["uid"].get<std::string>());
					std::string toolName = data["toolName"];
					auto& anchorRect = data["anchorRect"];

					const CLlmMcps::Mcp::Tool* tool = g_llmMcps.FindTool(uid, toolName.c_str());
					if (tool)
					{
						std::string md;
						Utils::MakeMcpToolDescription(*tool, md);

						RECT rect;
						rect.left = anchorRect.value("left", 0);
						rect.top = anchorRect.value("top", 0);
						rect.right = anchorRect.value("right", 0);
						rect.bottom = anchorRect.value("bottom", 0);

						POINT pt = { rect.left, rect.top };
						ClientToScreen(&pt);
						rect.left = pt.x;
						rect.top = pt.y;

						pt.x = rect.right;
						pt.y = rect.bottom;
						ClientToScreen(&pt);
						rect.right = pt.x;
						rect.bottom = pt.y;

						_mcpTip.ShowTip(rect, md);
					}
				}
			}
		}
		else if (action == "hideMcpTip")
		{
			_mcpTip.HideTip();
		}
	}
	catch (const std::exception& e)
	{
		TRACE(_T("Failed to parse WebView message: %s\n"), utf8_to_widechar(e.what()).c_str());
	}
}

// ======================== 新建目录 ========================

void CChatMcpsTree::_CreateNewFolder(const std::wstring& parentPath, const std::wstring& folderName)
{
	if (parentPath.empty() || folderName.empty())
		return;

	std::wstring newFolderPath = parentPath + L"\\" + folderName;

	// 检查目录是否已存在
	DWORD attribs = GetFileAttributesW(newFolderPath.c_str());
	if (attribs != INVALID_FILE_ATTRIBUTES && (attribs & FILE_ATTRIBUTE_DIRECTORY))
	{
		TRACE(_T("Folder already exists: %s\n"), newFolderPath.c_str());
		return;
	}

	// 创建目录
	if (!CreateDirectoryW(newFolderPath.c_str(), NULL))
	{
		DWORD error = GetLastError();
		TRACE(_T("Failed to create folder '%s', error: %d\n"), newFolderPath.c_str(), error);
		return;
	}

	// CMcpUpdater会检测到目录变化并自动重新加载
}

// ======================== 新建MCP ========================

void CChatMcpsTree::_CreateNewMcp(const std::wstring& parentPath, const std::wstring& mcpName, const std::wstring& mcpType)
{
	if (parentPath.empty() || mcpName.empty())
		return;

	std::wstring newMcpPath = parentPath + L"\\" + mcpName;

	// 检查目录是否已存在
	DWORD attribs = GetFileAttributesW(newMcpPath.c_str());
	if (attribs != INVALID_FILE_ATTRIBUTES && (attribs & FILE_ATTRIBUTE_DIRECTORY))
	{
		TRACE(_T("MCP folder already exists: %s\n"), newMcpPath.c_str());
		return;
	}

	// 创建目录
	if (!CreateDirectoryW(newMcpPath.c_str(), NULL))
	{
		DWORD error = GetLastError();
		TRACE(_T("Failed to create MCP folder '%s', error: %d\n"), newMcpPath.c_str(), error);
		return;
	}

	// 创建MCP.json文件
	std::wstring mcpJsonPath = newMcpPath + L"\\MCP.json";

	std::string mcpNameUtf8 = widechar_to_utf8(mcpName.c_str());
	std::string mcpContent = "{\n";
	mcpContent += "  \"description\": \"A new MCP server\",\n";
	mcpContent += "  \"command\": \"\",\n";
	mcpContent += "  \"args\": []\n";
	mcpContent += "}";

	std::ofstream outFile(mcpJsonPath, std::ios::binary);
	if (outFile.is_open())
	{
		outFile.write(mcpContent.c_str(), mcpContent.length());
		outFile.close();
	}
	else
	{
		TRACE(_T("Failed to create MCP.json for new mcp\n"));
	}

	// CMcpUpdater会检测到目录变化并自动重新加载
}

// ======================== 重命名MCP ========================

void CChatMcpsTree::_RenameMcp(const std::wstring& oldName, const std::wstring& newName, const std::wstring& fullPath)
{
	if (oldName.empty() || newName.empty() || fullPath.empty())
		return;

	// 构建父路径
	std::wstring parentPath;
	size_t lastSlash = fullPath.find_last_of(L'\\');
	if (lastSlash != std::wstring::npos)
		parentPath = fullPath.substr(0, lastSlash);
	else
		return;

	if (parentPath.empty())
		return;

	std::wstring newFullPath = parentPath + L"\\" + newName;

	// 重命名目录
	if (!MoveFileW(fullPath.c_str(), newFullPath.c_str()))
	{
		DWORD error = GetLastError();
		TRACE(_T("Failed to rename MCP folder from '%s' to '%s', error: %d\n"),
			fullPath.c_str(), newFullPath.c_str(), error);
		return;
	}

	// CMcpUpdater会检测到目录变化并自动重新加载
}
