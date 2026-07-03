#include "stdh.h"

#include "LspClient.h"
#include <Windows.h>
#include <sstream>
#include <algorithm>
#include "nlohmann/json.hpp"

#include "stringparser/stringparser.h"
//#include "CppSymbol.h"

using json = nlohmann::ordered_json;

// 定义缓冲区大小
#define BUFFER_SIZE 8192

// "variable","variable","parameter","function","method","function","property","variable","class","interface","enum","enumMember","type","type","unknown","namespace","typeParameter","concept","type","macro","modifier","operator","bracket","label","comment"
// 初始化静态成员（语义标记类型和修饰器的字符串表）
const std::vector<std::string> CLspClient::m_tokenTypes = {
    "namespace", "type", "class", "enum", "interface", "struct", "typeParameter", "parameter",
    "variable", "property", "enumMember", "event", "function", "method", "macro", "keyword",
    "comment", "string", "number", "operator","bracket", "unknown"
};

const std::vector<std::string> CLspClient::m_tokenModifiers = {
    "declaration", "definition", "readonly", "static", "deprecated", "abstract", "async",
    "modification", "documentation", "defaultLibrary"
};


//////////////////////////////////////////////////////////////////////////
//LspSymbolInformation

// LspSymbolInformation的ToString方法实现
void LspSymbolInformation::ToString(std::string& s)
{
	std::stringstream ss;
	ss << name << "\n";
	ss << ((int)kind) << "\n";
	ss << range.start.line << " " << range.start.character << "\n";
	ss << range.end.line << " " << range.end.character << "\n";
	ss << containerName << "\n";
	ss << uri;
	s = ss.str();
}

// LspSymbolInformation的FromString方法实现
void LspSymbolInformation::FromString(std::string& s)
{
	std::stringstream ss(s);
	std::string line;

	// 读取name
	if (std::getline(ss, line))
	{
		name = line;
	}

	// 读取kind
	if (std::getline(ss, line))
	{
		kind = (LspSymbolKind)std::stoi(line);
	}

	// 读取range.start
	if (std::getline(ss, line))
	{
		std::stringstream ss2(line);
		ss2 >> range.start.line >> range.start.character;
	}

	// 读取range.end
	if (std::getline(ss, line))
	{
		std::stringstream ss2(line);
		ss2 >> range.end.line >> range.end.character;
	}

	// 读取containerName
	if (std::getline(ss, line))
	{
		containerName = line;
	}

	// 读取uri
	if (std::getline(ss, line))
	{
		uri = line;
	}
}

//////////////////////////////////////////////////////////////////////////
//CLspClient

// 构造函数
CLspClient::CLspClient()
    : m_requestId(0)
    , m_isRunning(false)
    , m_isInitialized(false)
    , m_hProcess(nullptr)
    , m_hPipeRead(nullptr)
    , m_hPipeWrite(nullptr)
{
}

// 析构函数
CLspClient::~CLspClient()
{
    Shutdown();
}

bool CLspClient::IsValid()
{
    return m_isInitialized;
}

// 初始化并连接LSP服务器，支持指定compile_commands.json路径和缓存目录
bool CLspClient::Initialize(const std::string& workspacePath, const std::string& dbFolder,const std::string&clangdExePath)
{
    if (m_isInitialized)
    {
        return true;
    }

    m_workspacePath = workspacePath;
    m_dbFolder = dbFolder;

    m_version = 1;

    // 创建管道用于与clangd进程通信
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = nullptr;

    HANDLE hChildStdInRead, hChildStdInWrite;
    HANDLE hChildStdOutRead, hChildStdOutWrite;

    // 创建标准输入管道
    if (!CreatePipe(&hChildStdInRead, &hChildStdInWrite, &sa, 0))
    {
        return false;
    }

    // 创建标准输出管道
    if (!CreatePipe(&hChildStdOutRead, &hChildStdOutWrite, &sa, 0))
    {
        CloseHandle(hChildStdInRead);
        CloseHandle(hChildStdInWrite);
        return false;
    }

    // 确保子进程不继承管道写入句柄
    if (!SetHandleInformation(hChildStdInWrite, HANDLE_FLAG_INHERIT, 0))
    {
        CloseHandle(hChildStdInRead);
        CloseHandle(hChildStdInWrite);
        CloseHandle(hChildStdOutRead);
        CloseHandle(hChildStdOutWrite);
        return false;
    }

    // 确保子进程不继承管道读取句柄
    if (!SetHandleInformation(hChildStdOutRead, HANDLE_FLAG_INHERIT, 0))
    {
        CloseHandle(hChildStdInRead);
        CloseHandle(hChildStdInWrite);
        CloseHandle(hChildStdOutRead);
        CloseHandle(hChildStdOutWrite);
        return false;
    }

    // 准备启动进程
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdError = hChildStdOutWrite;
    si.hStdOutput = hChildStdOutWrite;
    si.hStdInput = hChildStdInRead;
    si.dwFlags |= STARTF_USESTDHANDLES;

    ZeroMemory(&pi, sizeof(pi));

    // 获取当前进程路径
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    
    // 提取目录部分
    std::string exeDir = exePath;
    size_t lastBackslash = exeDir.find_last_of('\\');
    if (lastBackslash != std::string::npos)
    {
        exeDir = exeDir.substr(0, lastBackslash + 1);
    }
    
    // 构建clangd.exe完整路径
    std::string clangdPath = exeDir + "clangd.exe";
    if (!clangdExePath.empty())
        clangdPath = clangdExePath;
    
    // 构建命令行
    std::string commandLine = "\"" + clangdPath + "\"";
     commandLine += " --background-index --clang-tidy";
    
    if (!dbFolder.empty())
    {
        commandLine += " --compile-commands-dir=\"" + dbFolder + "\\_clangd\"";
    }

    if (!CreateProcessA(
        NULL,                   // 应用程序名称
        (LPSTR)commandLine.c_str(), // 命令行
        NULL,                   // 进程安全属性
        NULL,                   // 线程安全属性
        TRUE,                   // 是否继承句柄
        CREATE_NO_WINDOW,       // 创建标志
        NULL,                   // 环境块
        m_workspacePath.c_str(),// 当前目录
        &si,                    // 启动信息
        &pi))                   // 进程信息
    {
        CloseHandle(hChildStdInRead);
        CloseHandle(hChildStdInWrite);
        CloseHandle(hChildStdOutRead);
        CloseHandle(hChildStdOutWrite);
        return false;
    }


    // 存储进程句柄
    m_hProcess = pi.hProcess;
    CloseHandle(pi.hThread);

    // 存储通信管道
    CloseHandle(hChildStdInRead);  // 子进程使用这个，我们不需要
    CloseHandle(hChildStdOutWrite); // 子进程使用这个，我们不需要

    m_hPipeWrite = hChildStdInWrite;
    m_hPipeRead = hChildStdOutRead;

    // 启动读取线程
    m_isRunning = true;
    m_readThread = std::make_unique<std::thread>(&CLspClient::ReadThread, this);

    // 发送初始化请求
    std::string initializeParams = R"({
        "processId": )" + std::to_string(GetCurrentProcessId()) + R"(,
        "rootUri": ")" + FilePathToUri(m_workspacePath) + R"(",
        "capabilities": {
            "textDocument": {
                "synchronization": {
                    "didSave": true,
                    "willSave": true
                },
                "completion": {
                    "contextSupport": true,
                    "snippetSupport": true
                },
                "definition": true,
                "references": true,
                "documentSymbol": true,
                "semanticTokens": {
                    "requests": {
                        "full": true
                    },
                    "tokenTypes": [
                    ],
                    "tokenModifiers": [
                    ]
                }
            },
            "workspace": {
                "symbol": true
            }
        },
        "initializationOptions": {
          "clangd": {
            "limitResults": 500
          }
        },
        "trace": "off"
    })";

    SendRequest("initialize", initializeParams, [this](const std::string& response)
    {
        json j = json::parse(response);
        if (j.contains("result") && j["result"].is_object())
        {
            const auto& result = j["result"];
            if (result.contains("capabilities") && result["capabilities"].is_object())
            {
                const auto& capabilities = result["capabilities"];
                if (capabilities.contains("semanticTokensProvider") && capabilities["semanticTokensProvider"].is_object())
                {
                    const auto& semanticTokens = capabilities["semanticTokensProvider"];
                    if (semanticTokens.contains("legend") && semanticTokens["legend"].is_object())
                    {
                        const auto& legend = semanticTokens["legend"];
                        
                        // 处理 tokenTypes
                        if (legend.contains("tokenTypes") && legend["tokenTypes"].is_array())
                        {
                            std::vector<std::string> serverTypes = legend["tokenTypes"].get<std::vector<std::string>>();
                            std::lock_guard<std::mutex> lock(m_mutex); 
                            m_tokenTypeMap.clear();
                            for (int i = 0; i < serverTypes.size(); ++i)
                            {
                                // 在客户端定义的类型中查找服务器返回的类型字符串
                                for (int clientIdx = 0; clientIdx < m_tokenTypes.size(); ++clientIdx)
                                {
                                    if (m_tokenTypes[clientIdx] == serverTypes[i])
                                    {
                                        m_tokenTypeMap[i] = static_cast<LspTokenType>(clientIdx);
                                        break;
                                    }
                                }
                                // 如果未找到，可以考虑添加一个默认映射或日志记录
                                if (m_tokenTypeMap.find(i) == m_tokenTypeMap.end())
                                {
                                    m_tokenTypeMap[i] = LspTokenType::Unknown; // 映射到未知类型
                                }
                            }
                        }
                        
                        // 处理 tokenModifiers
                        if (legend.contains("tokenModifiers") && legend["tokenModifiers"].is_array())
                        {
                            std::vector<std::string> serverModifiers = legend["tokenModifiers"].get<std::vector<std::string>>();
                            std::lock_guard<std::mutex> lock(m_mutex); 
                            m_tokenModifierMap.clear();
                            for (int i = 0; i < serverModifiers.size(); ++i)
                            {
                                // 在客户端定义的修饰符中查找服务器返回的修饰符字符串
                                for (int clientIdx = 0; clientIdx < m_tokenModifiers.size(); ++clientIdx)
                                {
                                    if (m_tokenModifiers[clientIdx] == serverModifiers[i])
                                    {
                                        m_tokenModifierMap[i] = static_cast<LspTokenModifier>(clientIdx);
                                        break;
                                    }
                                }
                                // 如果未找到，可以忽略或记录日志，因为修饰符通常是可选的
                            }
                        }
                    }
                }
            }
        }

        // 初始化完成后发送initialized通知
        SendNotification("initialized", "{}");
    });

	m_isInitialized = true;

    return true;
}

// 关闭LSP服务器连接
void CLspClient::Shutdown()
{
    if (!m_isInitialized)
    {
        return;
    }

    // 发送关闭请求
    if (m_isRunning)
    {
        SendRequest("shutdown", "{}", [](const std::string&) {});
        SendNotification("exit", "{}");
    }

    m_isRunning = false;

    // 等待线程结束
    if (m_readThread && m_readThread->joinable())
    {
        m_readThread->join();
    }

    // 关闭句柄
    if (m_hPipeWrite != nullptr)
    {
        CloseHandle(m_hPipeWrite);
        m_hPipeWrite = nullptr;
    }

    if (m_hPipeRead != nullptr)
    {
        CloseHandle(m_hPipeRead);
        m_hPipeRead = nullptr;
    }

    if (m_hProcess != nullptr)
    {
        // 尝试正常关闭进程
        WaitForSingleObject(m_hProcess, 1000);
        
        // 如果进程仍在运行，则强制终止
        DWORD exitCode;
        if (GetExitCodeProcess(m_hProcess, &exitCode) && exitCode == STILL_ACTIVE)
        {
            TerminateProcess(m_hProcess, 0);
        }
        
        CloseHandle(m_hProcess);
        m_hProcess = nullptr;
    }

    m_isInitialized = false;
}

// 更新文档内容
void CLspClient::UpdateDocument(const std::string& filePath, AbsTick ft, const std::string& content)
{
    if (!m_isInitialized)
    {
        return;
    }

    if (m_documentTimes.find(filePath) == m_documentTimes.end())
    {
		json params;
		params["textDocument"]["uri"] = FilePathToUri(filePath);
		params["textDocument"]["languageId"] = "cpp";
		params["textDocument"]["version"] = m_version++;
		params["textDocument"]["text"] = content;

		SendNotification("textDocument/didOpen", params.dump());
    }
    else
    {
        json params;
        params["textDocument"]["uri"] = FilePathToUri(filePath);
        params["textDocument"]["version"] = m_version++;

//         json contentChanges;
//         contentChanges["text"] = content;
        params["contentChanges"] = json::array({ {{"text", content}} });

        SendNotification("textDocument/didChange", params.dump());
    }

	m_documentTimes[filePath] = ft;
}

// 关闭文档
void CLspClient::CloseDocument(const std::string& filePath)
{
    if (!m_isInitialized)
    {
        return;
    }

    json params;
    params["textDocument"]["uri"] = FilePathToUri(filePath);

    SendNotification("textDocument/didClose", params.dump());

    auto it = m_documentTimes.find(filePath);
    if (it != m_documentTimes.end())
        m_documentTimes.erase(it);
}

// 请求代码补全
void CLspClient::RequestCompletion(const std::string& filePath, int line, int character)
{
    if (!m_isInitialized)
        return;

    json params;
    params["textDocument"]["uri"] = FilePathToUri(filePath);
    params["position"]["line"] = line;
    params["position"]["character"] = character;
    params["context"]["triggerKind"] = 1;

    AbsTick curTime = GetAbsTick();

    // 使用原始回调将响应传递给补全解析器
    LspResponseCallback rawCallback = [this, filePath, line, character, curTime](const std::string& response)
    {
        DWORD ver = m_completionResult.ver;
        LspCompletionResult result;
        this->ParseCompletionResponse(response, result.items);
        result.filePath = filePath;
        result.line = line;
        result.column = character;
        result.startTime = curTime;

        std::lock_guard<std::mutex> lock(m_mutex);
        m_completionResult = std::move(result);
        m_completionResult.ver = ++ver;
    };

    SendRequest("textDocument/completion", params.dump(), rawCallback);
}

void CLspClient::GetCompletionResponse(LspCompletionResult& result)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    result = m_completionResult;
}

DWORD CLspClient::GetCompletionResponseVer()
{
    return m_completionResult.ver;
}


// 请求定义位置
void CLspClient::RequestDefinition(const std::string& filePath, int line, int character)
{
    if (!m_isInitialized)
    {
        return;
    }

    json params;
    params["textDocument"]["uri"] = FilePathToUri(filePath);
    params["position"]["line"] = line;
    params["position"]["character"] = character;

    AbsTick curTime = GetAbsTick();

	// 使用原始回调将响应传递给语义标记解析器
	LspResponseCallback rawCallback = [this,filePath,line,character,curTime](const std::string& response)
	{
        LspGotoDefinationResult result;
		this->ParseDefinitionResponse(response, result.locations);
		result.startfilePath= filePath;
        result.startline = line;
        result.startcolumn = character;
		result.startTime = curTime;

		std::lock_guard<std::mutex> lock(m_mutex);

        m_gotoDefinationResults.push_back(std::move(result));
	};


    SendRequest("textDocument/declaration", params.dump(), rawCallback);
}

// 解析定义响应
void CLspClient::ParseDefinitionResponse(const std::string& response, std::vector<LspLocation>& locations)
{
	json j = json::parse(response);

	if (j.contains("result"))
	{
		const json& result = j["result"];

		// 处理单个位置
		if (result.is_object() && result.contains("uri"))
		{
			LspLocation location;
			location.uri = result.value("uri", "");

			if (result.contains("range"))
			{
				if (result["range"].contains("start"))
				{
					location.range.start.line = result["range"]["start"].value("line", 0);
					location.range.start.character = result["range"]["start"].value("character", 0);
				}

				if (result["range"].contains("end"))
				{
					location.range.end.line = result["range"]["end"].value("line", 0);
					location.range.end.character = result["range"]["end"].value("character", 0);
				}
			}

			locations.push_back(location);
		}
		// 处理位置数组
		else if (result.is_array())
		{
			for (const auto& locJson : result)
			{
				LspLocation location;
				location.uri = locJson.value("uri", "");

				if (locJson.contains("range"))
				{
					if (locJson["range"].contains("start"))
					{
						location.range.start.line = locJson["range"]["start"].value("line", 0);
						location.range.start.character = locJson["range"]["start"].value("character", 0);
					}

					if (locJson["range"].contains("end"))
					{
						location.range.end.line = locJson["range"]["end"].value("line", 0);
						location.range.end.character = locJson["range"]["end"].value("character", 0);
					}
				}

				locations.push_back(location);
			}
		}
	}
}

bool CLspClient::FlushAndFetchDefinitionResponse(const std::string& filePath, int line, int column, LspGotoDefinationResult& result)
{
	std::lock_guard<std::mutex> lock(m_mutex);

	bool ret = false;
	int c = 0;
	for (int i = 0;i < m_gotoDefinationResults.size();i++)
	{
        LspGotoDefinationResult& r= m_gotoDefinationResults[i];
		if (r.startfilePath == filePath)
		{
            if ((r.startline==line)&&(r.startcolumn=column))
			{
				result = r;
				ret = true;
				continue;
			}
		}
        m_gotoDefinationResults[c] = m_gotoDefinationResults[i];
		c++;
	}
    m_gotoDefinationResults.resize(c);

	return ret;
}



// 请求引用位置
void CLspClient::RequestReferences(const std::string& filePath, int line, int character, bool includeDeclaration, LspResponseCallback callback)
{
    if (!m_isInitialized)
    {
        callback("{}");
        return;
    }

    json params;
    params["textDocument"]["uri"] = FilePathToUri(filePath);
    params["position"]["line"] = line;
    params["position"]["character"] = character;
    params["context"]["includeDeclaration"] = includeDeclaration;

    SendRequest("textDocument/references", params.dump(), callback);
}

// 请求工作区符号
void CLspClient::RequestWorkspaceSymbols(const std::string& query)
{
    if (!m_isInitialized)
    {
        return;
    }

    json params;
    params["query"] = query;

	LspResponseCallback rawCallback = [this, query](const std::string& response)
	{
        std::vector<LspSymbolInformation> result;
		this->ParseWorkspaceSymbolResponse(response, result);

		std::lock_guard<std::mutex> lock(m_mutex);

        m_symbols.buf = std::move(result);
        m_symbols.key = query;
	};

    SendRequest("workspace/symbol", params.dump(), rawCallback);
}

// 请求文档符号
void CLspClient::RequestDocumentSymbol(const std::string& filePath)
{
    if (!m_isInitialized)
    {
        return;
    }

    json params;
    params["textDocument"]["uri"] = FilePathToUri(filePath);

    AbsTick fileTime = 0;
    if (TRUE)
    {
        auto it = m_documentTimes.find(filePath);
        if (it == m_documentTimes.end())
            return;
        fileTime = (*it).second;
    }

    LspResponseCallback rawCallback = [this, filePath, fileTime](const std::string& response)
    {
        LspDocumentSymbols result;
        this->ParseDocumentSymbolResponse(response, result.symbols);
        result.filePath = filePath;
        result.fileTime = fileTime;

        std::lock_guard<std::mutex> lock(m_mutex);
        m_documentSymbols.push_back(std::move(result));
    };

    SendRequest("textDocument/documentSymbol", params.dump(), rawCallback);
}

// 解析文档符号响应
void CLspClient::ParseDocumentSymbolResponse(const std::string& response, std::vector<LspSymbolInformation>& symbols)
{
    symbols.clear();
    json j = json::parse(response);

    if (j.contains("result") && j["result"].is_array())
    {
        // 处理DocumentSymbol格式（层次结构）
        std::function<void(const json&, const std::string&, const std::string&)> processSymbol;
        
        processSymbol = [&symbols, &processSymbol](const json& item, const std::string& containerName, const std::string& uri) {
            // 检查这是DocumentSymbol还是SymbolInformation
            if (item.contains("range") && item.contains("name"))
            {
                LspSymbolInformation symbol;
                symbol.name = item.value("name", "");
                symbol.kind = (LspSymbolKind)item.value("kind", 0);
                symbol.containerName = containerName;
                symbol.uri = uri;
                
                // 解析range
                if (item.contains("range"))
                {
                    if (item["range"].contains("start"))
                    {
                        symbol.range.start.line = item["range"]["start"].value("line", 0);
                        symbol.range.start.character = item["range"]["start"].value("character", 0);
                    }
                    
                    if (item["range"].contains("end"))
                    {
                        symbol.range.end.line = item["range"]["end"].value("line", 0);
                        symbol.range.end.character = item["range"]["end"].value("character", 0);
                    }
                }
                
                symbols.push_back(symbol);
                
                // 处理子符号
                if (item.contains("children") && item["children"].is_array())
                {
                    for (const auto& child : item["children"])
                    {
                        processSymbol(child, symbol.name, uri);
                    }
                }
            }
            // 直接的SymbolInformation格式
            else if (item.contains("name") && item.contains("location"))
            {
                LspSymbolInformation symbol;
                symbol.name = item.value("name", "");
                symbol.kind = (LspSymbolKind)item.value("kind", 0);
                symbol.containerName = item.value("containerName", containerName);
                
                if (item.contains("location"))
                {
                    symbol.uri = item["location"].value("uri", uri);
                    
                    if (item["location"].contains("range"))
                    {
                        if (item["location"]["range"].contains("start"))
                        {
                            symbol.range.start.line = item["location"]["range"]["start"].value("line", 0);
                            symbol.range.start.character = item["location"]["range"]["start"].value("character", 0);
                        }
                        
                        if (item["location"]["range"].contains("end"))
                        {
                            symbol.range.end.line = item["location"]["range"]["end"].value("line", 0);
                            symbol.range.end.character = item["location"]["range"]["end"].value("character", 0);
                        }
                    }
                }
                
                symbols.push_back(symbol);
            }
        };
        
        // 处理每个顶级符号
        for (const auto& item : j["result"])
        {
            std::string uri = ""; // 在SymbolInformation格式中，uri在location字段内
            if (item.contains("location") && item["location"].contains("uri"))
            {
                uri = item["location"].value("uri", "");
            }
            
            processSymbol(item, "", uri);
        }
    }
}

// 获取文档符号
bool CLspClient::FlushAndFetchDocumentSymbols(const std::string& filePath, AbsTick fileTime, LspDocumentSymbols& symbols)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    bool ret = false;
    int c = 0;
    for (int i = 0; i < m_documentSymbols.size(); i++)
    {
        if (m_documentSymbols[i].filePath == filePath)
        {
            if (m_documentSymbols[i].fileTime < fileTime)
                continue;
            if (m_documentSymbols[i].fileTime == fileTime)
            {
                symbols = std::move(m_documentSymbols[i]);
                ret = true;
                continue;
            }
        }
        m_documentSymbols[c] = std::move(m_documentSymbols[i]);
        c++;
    }
    m_documentSymbols.resize(c);

    return ret;
}

// 设置诊断回调
void CLspClient::SetDiagnosticsCallback(LspDiagnosticsCallback callback)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_diagnosticsCallback = callback;
}

// 解析工作区符号响应
void CLspClient::ParseWorkspaceSymbolResponse(const std::string& response, std::vector<LspSymbolInformation>&symbols)
{
    
    json j = json::parse(response);
        
    if (j.contains("result") && j["result"].is_array())
    {
        for (const auto& item : j["result"])
        {
            LspSymbolInformation symbol;
            symbol.name = item.value("name", "");
            symbol.kind = (LspSymbolKind)item.value("kind", 0);
                
            if (item.contains("location"))
            {
                symbol.uri = item["location"].value("uri", "");
                    
                if (item["location"].contains("range"))
                {
                    if (item["location"]["range"].contains("start"))
                    {
                        symbol.range.start.line = item["location"]["range"]["start"].value("line", 0);
                        symbol.range.start.character = item["location"]["range"]["start"].value("character", 0);
                    }
                        
                    if (item["location"]["range"].contains("end"))
                    {
                        symbol.range.end.line = item["location"]["range"]["end"].value("line", 0);
                        symbol.range.end.character = item["location"]["range"]["end"].value("character", 0);
                    }
                }
            }
                
            symbol.containerName = item.value("containerName", "");
            symbols.push_back(symbol);
        }
    }
}

void CLspClient::GetWorkspaceSymbols(LspSymbols& symbols)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	symbols = m_symbols;
}

void CLspClient::GetWorkspaceSymbolsKey(std::string& key)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	key = m_symbols.key;
}


// 解析补全响应
void CLspClient::ParseCompletionResponse(const std::string& response, std::vector<LspCompletionItem>& items)
{
    json j = json::parse(response);
        
    if (j.contains("result"))
    {
        // 处理直接返回项目数组的情况
        const json& result = j["result"];
        json itemsArray;
            
        if (result.is_array())
        {
            itemsArray = result;
        }
        // 处理返回 {isIncomplete, items} 对象的情况
        else if (result.is_object() && result.contains("items") && result["items"].is_array())
        {
            itemsArray = result["items"];
        }
            
        for (const auto& item : itemsArray)
        {
            LspCompletionItem completionItem;
            completionItem.label = item.value("label", "");
            completionItem.detail = item.value("detail", "");
                
            // 处理documentation可能是字符串或对象的情况
            if (item.contains("documentation"))
            {
                if (item["documentation"].is_string())
                {
                    completionItem.documentation = item["documentation"].get<std::string>();
                }
                else if (item["documentation"].is_object() && item["documentation"].contains("value"))
                {
                    completionItem.documentation = item["documentation"]["value"].get<std::string>();
                }
            }
                
            completionItem.kind = (LspSymbolKind)item.value("kind", 0);
			completionItem.filterText = item.value("filterText", "");
            completionItem.insertText = item.value("insertText", completionItem.filterText);
            completionItem.sortText = item.value("sortText", "");
                
            items.push_back(completionItem);
        }
    }
    
}


// 解析引用响应
void CLspClient::ParseReferencesResponse(const std::string& response, std::vector<LspLocation>&result)
{
    return ParseDefinitionResponse(response,result); // 引用和定义响应格式相同
}

// 发送LSP请求
void CLspClient::SendRequest(const std::string& method, const std::string& params, LspResponseCallback callback)
{
    if (!m_isRunning || m_hPipeWrite == nullptr)
    {
        return;
    }

    // 获取新的请求ID
    int id = m_requestId++;
    
    // 构建LSP请求
    std::string request = "{\"jsonrpc\":\"2.0\",\"id\":" + std::to_string(id) + ",\"method\":\"" + method + "\",\"params\":" + params + "}";
    
    // 添加Content-Length头
    std::string message = "Content-Length: " + std::to_string(request.length()) + "\r\n\r\n" + request;
    
    // 发送请求
    DWORD bytesWritten;
    if (!WriteFile(m_hPipeWrite, message.c_str(), static_cast<DWORD>(message.length()), &bytesWritten, NULL))
    {
        return;
    }
    
    // 保存回调
    std::lock_guard<std::mutex> lock(m_mutex);
    m_responseCallbacks[id] = callback;
}

// 发送LSP通知
void CLspClient::SendNotification(const std::string& method, const std::string& params)
{
    if (!m_isRunning || m_hPipeWrite == nullptr)
    {
        return;
    }
    
    // 构建LSP通知
    std::string notification = "{\"jsonrpc\":\"2.0\",\"method\":\"" + method + "\",\"params\":" + params + "}";
    
    // 添加Content-Length头
    std::string message = "Content-Length: " + std::to_string(notification.length()) + "\r\n\r\n" + notification;
    
    // 发送通知
    DWORD bytesWritten;
    WriteFile(m_hPipeWrite, message.c_str(), static_cast<DWORD>(message.length()), &bytesWritten, NULL);
}

// LSP读取线程
void CLspClient::ReadThread()
{
    if (m_hPipeRead == nullptr)
    {
        return;
    }
    
    std::string buffer;
    buffer.reserve(BUFFER_SIZE);
    
    char readBuffer[BUFFER_SIZE];
    DWORD bytesRead;
    
    while (m_isRunning)
    {
        // 读取服务器响应
        if (!ReadFile(m_hPipeRead, readBuffer, BUFFER_SIZE, &bytesRead, NULL))
        {
            if (GetLastError() == ERROR_BROKEN_PIPE)
            {
                // 管道已关闭
                break;
            }
            
            // 其他错误
            Sleep(10);
            continue;
        }
        
        if (bytesRead == 0)
        {
            Sleep(10);
            continue;
        }
        
        // 添加到缓冲区
        buffer.append(readBuffer, bytesRead);
        
        // 处理完整消息
        while (!buffer.empty())
        {
            // 查找消息头边界
            size_t headerEnd = buffer.find("\r\n\r\n");
            if (headerEnd == std::string::npos)
            {
                break;
            }
            
            // 解析Content-Length
            size_t contentLengthPos = buffer.find("Content-Length: ");
            if (contentLengthPos == std::string::npos)
            {
                buffer.erase(0, headerEnd + 4);
                continue;
            }
            
            size_t lengthStartPos = contentLengthPos + 16; // "Content-Length: " 的长度
            size_t lengthEndPos = buffer.find("\r\n", lengthStartPos);
            if (lengthEndPos == std::string::npos)
            {
                buffer.erase(0, headerEnd + 4);
                continue;
            }
            
            int contentLength = std::stoi(buffer.substr(lengthStartPos, lengthEndPos - lengthStartPos));
            
            // 检查是否有完整的消息体
            size_t contentStart = headerEnd + 4;
            if (buffer.length() < contentStart + contentLength)
            {
                break;
            }
            
            // 提取消息体
            std::string content = buffer.substr(contentStart, contentLength);
            
            // 处理消息
            HandleServerResponse(content);
            
            // 移除已处理的消息
            buffer.erase(0, contentStart + contentLength);
        }
    }
}

// 处理LSP服务器的响应
void CLspClient::HandleServerResponse(const std::string& response)
{
    try
    {
        json j = json::parse(response);
        
        // 检查是否是响应
        if (j.contains("id") && !j["id"].is_null())
        {
            int id = j["id"].get<int>();
            
            bool isCallback = false;
            LspResponseCallback callback;
            if (true)
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                auto it = m_responseCallbacks.find(id);
                if (it != m_responseCallbacks.end())
                {
                    callback = it->second;
                    isCallback = true;
                    // 移除已使用的回调
                    m_responseCallbacks.erase(it);
                }
            }

			if (isCallback)
    			callback(response);

        }
        // 检查是否是通知
        else if (j.contains("method"))
        {
            std::string method = j["method"].get<std::string>();
            
            // 处理诊断通知
            if (method == "textDocument/publishDiagnostics" && j.contains("params"))
            {
                std::lock_guard<std::mutex> lock(m_mutex);
                if (m_diagnosticsCallback)
                {
                    LspPublishDiagnosticsParams params;
                    
                    // 解析uri
                    params.uri = j["params"].value("uri", "");
                    
                    // 解析diagnostics
                    if (j["params"].contains("diagnostics") && j["params"]["diagnostics"].is_array())
                    {
                        for (const auto& diag : j["params"]["diagnostics"])
                        {
                            LspDiagnostic diagnostic;
                            
                            // 解析range
                            if (diag.contains("range"))
                            {
                                if (diag["range"].contains("start"))
                                {
                                    diagnostic.range.start.line = diag["range"]["start"].value("line", 0);
                                    diagnostic.range.start.character = diag["range"]["start"].value("character", 0);
                                }
                                
                                if (diag["range"].contains("end"))
                                {
                                    diagnostic.range.end.line = diag["range"]["end"].value("line", 0);
                                    diagnostic.range.end.character = diag["range"]["end"].value("character", 0);
                                }
                            }
                            
                            diagnostic.severity = diag.value("severity", 1);
                            diagnostic.code = diag.value("code", "");
                            diagnostic.message = diag.value("message", "");
                            diagnostic.source = diag.value("source", "clangd");
                            
                            params.diagnostics.push_back(diagnostic);
                        }
                    }
                    
                    m_diagnosticsCallback(params);
                }
            }
        }
    }
    catch (const std::exception&)
    {
        // 解析错误
    }
}

// 获取语义标记（用于语法高亮）
void CLspClient::RequestSemanticTokens(const std::string& filePath)
{
    if (!m_isInitialized)
    {
        return;
    }

    json params;
    params["textDocument"]["uri"] = FilePathToUri(filePath);
    
    int requestId = m_requestId++;
    
	AbsTick fileTime;
    if (TRUE)
    {
        auto it = m_documentTimes.find(filePath);
        if (it == m_documentTimes.end())
            return;
        fileTime = (*it).second;
    } 
    
    // 使用原始回调将响应传递给语义标记解析器
    LspResponseCallback rawCallback = [this, filePath,fileTime,requestId](const std::string& response) 
    {
        LspSemanticTokens result;
        this->ParseSemanticTokensResponse(response,result.buf);
        result.filePath = filePath;
        result.fileTime = fileTime;
        
        std::lock_guard<std::mutex> lock(m_mutex);

        m_documentSemanticTokens.resize(m_documentSemanticTokens.size() + 1);
        m_documentSemanticTokens[m_documentSemanticTokens.size() - 1] = std::move(result);
    };
    
    SendRequest("textDocument/semanticTokens/full", params.dump(), rawCallback);
}

bool CLspClient::FlushAndFetchSemanticTokens(const std::string& filePath, AbsTick fileTime, LspSemanticTokens& tokens)
{
	std::lock_guard<std::mutex> lock(m_mutex);

    bool ret = false;
    int c = 0;
    for (int i = 0;i < m_documentSemanticTokens.size();i++)
    {
        if (m_documentSemanticTokens[i].filePath == filePath)
        {
            if (m_documentSemanticTokens[i].fileTime < fileTime)
                continue;
            if (m_documentSemanticTokens[i].fileTime == fileTime)
            {
                tokens = std::move(m_documentSemanticTokens[i]);
                ret = true;
                continue;
            }
        }
        m_documentSemanticTokens[c] = std::move(m_documentSemanticTokens[i]);
        c++;
    }
    m_documentSemanticTokens.resize(c);

    return ret;
}


// 解析语义标记响应
void CLspClient::ParseSemanticTokensResponse(const std::string& response, std::vector<LspSemanticToken> &tokens)
{
    tokens.clear();
    
    json j = json::parse(response);
        
    if (j.contains("result") && j["result"].is_object() && j["result"].contains("data"))
    {
        const json& data = j["result"]["data"];
        if (!data.is_array())
        {
            return;
        }
            
        // 语义标记数据是一个平面数组，每5个元素一组：deltaLine, deltaStart, length, tokenType, tokenModifiers
        std::vector<unsigned int> tokenData;
        for (const auto& item : data)
        {
            tokenData.push_back(item.get<unsigned int>());
        }
            
        // 处理数据
        int currentLine = 0;
        int currentStart = 0;
            
        for (size_t i = 0; i < tokenData.size(); i += 5)
        {
            if (i + 4 >= tokenData.size())
            {
                break;
            }
                
            LspSemanticToken token;
            token.deltaLine = tokenData[i];
            token.deltaStart = tokenData[i + 1];
            token.length = tokenData[i + 2];
            token.tokenType = tokenData[i + 3];
            token.tokenModifiers = tokenData[i + 4];
                
            // 计算实际位置
            if (token.deltaLine > 0)
            {
                currentLine += token.deltaLine;
                currentStart = token.deltaStart;
            }
            else
            {
                currentStart += token.deltaStart;
            }
                
            token.line = currentLine;
            token.character = currentStart;

			token.type = (LspTokenType)m_tokenTypeMap[(int)token.tokenType];

            tokens.push_back(token);
        }
            
        // 如果有resultId，可以存储起来用于增量更新
        if (j["result"].contains("resultId"))
        {
            // 此处可以保存resultId用于后续增量更新
            // std::string resultId = j["result"]["resultId"].get<std::string>();
        }
    }
}

// 获取语义标记类型的字符串表示
std::string CLspClient::GetTokenTypeString(LspTokenType type)
{
    size_t index = static_cast<size_t>(type);
    if (index < m_tokenTypes.size())
    {
        return m_tokenTypes[index];
    }
    return m_tokenTypes.back(); // 返回"unknown"
}


// 获取语义标记修饰器的字符串表示
std::string CLspClient::GetTokenModifierString(LspTokenModifier modifier)
{
    size_t index = static_cast<size_t>(modifier);
    if (index < m_tokenModifiers.size())
    {
        return m_tokenModifiers[index];
    }
    return "";
}

AbsTick CLspClient::GetDocumentTime(const std::string& filePath)
{
    auto it = m_documentTimes.find(filePath);
    if (it == m_documentTimes.end())
        return 0;
    return (*it).second;
}
