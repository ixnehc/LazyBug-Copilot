#pragma once

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <deque>
#include <memory>
#include <functional>
#include <thread>
#include <mutex>

// LSP消息类型定义
enum class LspMessageType
{
    Request,
    Response,
    Notification,
    Error
};

// LSP请求参数基类
struct LspParams
{
    virtual ~LspParams() = default;
};

// LSP响应结果基类
struct LspResult
{
    virtual ~LspResult() = default;
};

// 位置定义
struct LspPosition
{
    int line;
    int character;
    bool IsEmpty()
    {
        return line < 0 || character < 0;
    }
    
    LspPosition() : line(-1), character(-1) {}
    LspPosition(int l, int c) : line(l), character(c) {}
};

// 范围定义
struct LspRange
{
	bool IsEmpty()
	{
		return start.IsEmpty()|| end.IsEmpty();
	}

    void Reset()
    {
        start = end = LspPosition();
    }

    LspPosition start;
    LspPosition end;
};

// 文本文档标识
struct LspTextDocumentIdentifier
{
    std::string uri;
};

// 文本文档位置
struct LspTextDocumentPositionParams : public LspParams
{
    LspTextDocumentIdentifier textDocument;
    LspPosition position;
};

enum class LspSymbolKind 
{
	None = 0,
	File = 1,
	Module = 2,
	Namespace = 3,
	Package = 4,
	Class = 5,
	Method = 6,
	Property = 7,
	Field = 8,
	Constructor = 9,
	Enum = 10,
	Interface = 11,
	Function = 12,
	Variable = 13,
	Constant = 14,
	String = 15,
	Number = 16,
	Boolean = 17,
	Array = 18,
	Object = 19,
	Key = 20,
	Null = 21,
	EnumMember = 22,
	Struct = 23,
	Event = 24,
	Operator = 25,
	TypeParameter = 26,
};

// 符号信息
struct LspSymbolInformation
{
    LspSymbolInformation()
    {
        kind = LspSymbolKind::None;
    }
    void ToString(std::string& s);
    void FromString(std::string& s);

    std::string name;
    LspSymbolKind kind;
    LspRange range;
    std::string containerName;
    std::string uri;
};

struct LspSymbols
{
    std::string key;
    std::vector< LspSymbolInformation> buf;
};

// 代码补全项
struct LspCompletionItem
{
    std::string label;
    std::string detail;
    std::string documentation;
    LspSymbolKind kind;
	std::string filterText;
    std::string insertText;
    std::string sortText;
};

// 补全结果
struct LspCompletionResult : public LspResult
{
    LspCompletionResult()
    {
        ver = 0;
    }
    std::string filePath;
    int line;
    int column;
    AbsTick startTime;
    std::vector<LspCompletionItem> items;

    DWORD ver;
};

// 跳转位置
struct LspLocation
{
    std::string uri;
    LspRange range;
};

// 定义查找结果
struct LspDefinitionResult : public LspResult
{
    std::vector<LspLocation> locations;
};

// 引用查找结果
struct LspReferencesParams : public LspTextDocumentPositionParams
{
    bool includeDeclaration;
};

// 引用查找结果
struct LspReferencesResult : public LspResult
{
    std::vector<LspLocation> locations;
};

// 符号查找参数
struct LspWorkspaceSymbolParams : public LspParams
{
    std::string query;
};

// 符号查找结果
struct LspWorkspaceSymbolResult : public LspResult
{
    std::vector<LspSymbolInformation> symbols;
};

// 诊断信息
struct LspDiagnostic
{
    LspRange range;
    int severity;
    std::string code;
    std::string source;
    std::string message;
};

// 文档诊断结果
struct LspPublishDiagnosticsParams
{
    std::string uri;
    std::vector<LspDiagnostic> diagnostics;
};

// 语义标记类型
enum class LspTokenType
{
    Namespace,
    Type,
    Class,
    Enum,
    Interface,
    Struct,
    TypeParameter,
    Parameter,
    Variable,
    Property,
    EnumMember,
    Event,
    Function,
    Method,
    Macro,
    Keyword,
    Comment,
    String,
    Number,
    Operator,
    Bracket,
    Unknown
};

// 语义标记修饰器
enum class LspTokenModifier
{
    Declaration,
    Definition,
    Readonly,
    Static,
    Deprecated,
    Abstract,
    Async,
    Modification,
    Documentation,
    DefaultLibrary
};

// 语义标记
struct LspSemanticToken
{
    // 行偏移
    unsigned int deltaLine = 0;
    // 字符偏移
    unsigned int deltaStart = 0;
    // 标记长度
    unsigned int length = 0;
    // 标记类型（索引）
    unsigned int tokenType = 0;
    // 标记修饰器（位掩码）
    unsigned int tokenModifiers = 0;
    
    // 方便使用的实际位置
    int line = 0;       // 行号
    int character = 0;  // 字符位置
    LspTokenType type = LspTokenType::Unknown;
};

struct LspSemanticTokens
{
    std::string filePath;
    AbsTick fileTime;
    std::vector< LspSemanticToken> buf;
};

// 语义标记结果
struct LspSemanticTokensResult : public LspResult
{
    std::vector<LspSemanticToken> tokens;
    std::string resultId;
};

// 文档符号结果
struct LspDocumentSymbols
{
    std::string filePath;
    AbsTick fileTime;
    std::vector<LspSymbolInformation> symbols;
};

struct LspGotoDefinationResult
{
    LspGotoDefinationResult()
    {
        Zero();
    }
    void Zero()
    {
		startline = -1;
		startcolumn = -1;
		startTime = 0;
    }
    void Reset()
    {
        Zero();
        startfilePath = "";
        locations.clear();
    }
    bool IsEmpty()
    {
        return locations.size() <= 0;
    }
	std::string startfilePath;
    int startline=-1;
    int startcolumn=-1;
    AbsTick startTime=0;

    std::vector<LspLocation> locations;
};

// LSP回调函数类型定义
using LspResponseCallback = std::function<void(const std::string&)>;
using LspDiagnosticsCallback = std::function<void(const LspPublishDiagnosticsParams&)>;
using LspSemanticTokensCallback = std::function<void(const std::vector<LspSemanticToken>&)>;

// LSP客户端类
class CLspClient
{
public:
    CLspClient();
    ~CLspClient();

    bool IsValid();
   
    // 初始化并连接LSP服务器，支持指定compile_commands.json路径
    bool Initialize(const std::string& workspacePath, const std::string& dbFolder, const std::string& clangdExePath="");
    
    // 关闭连接
    void Shutdown();
    
    // 更新文档内容
    void UpdateDocument(const std::string& filePath, AbsTick ft, const std::string& content);
    
    // 关闭文档
    void CloseDocument(const std::string& filePath);

    AbsTick GetDocumentTime(const std::string& filePath);
    
    // 获取代码补全
    void RequestCompletion(const std::string& filePath, int line, int character);
	void ParseCompletionResponse(const std::string& response, std::vector<LspCompletionItem> &items);
	void GetCompletionResponse(LspCompletionResult& result);
    DWORD GetCompletionResponseVer();
	LspCompletionResult m_completionResult;

    // 获取定义位置
    void RequestDefinition(const std::string& filePath, int line, int character);
	void ParseDefinitionResponse(const std::string& response, std::vector<LspLocation>& locations);
	bool FlushAndFetchDefinitionResponse(const std::string& filePath, int line,int column,LspGotoDefinationResult &result);
	std::vector < LspGotoDefinationResult> m_gotoDefinationResults;

    // 获取引用位置
    void RequestReferences(const std::string& filePath, int line, int character, bool includeDeclaration, LspResponseCallback callback);
    
    // 获取工作区符号
    void RequestWorkspaceSymbols(const std::string& query);
	void ParseWorkspaceSymbolResponse(const std::string& response, std::vector<LspSymbolInformation>&symbols);
    void GetWorkspaceSymbols(LspSymbols& symbols);
    void GetWorkspaceSymbolsKey(std::string& key);
    LspSymbols m_symbols;

    // 获取文档符号
    void RequestDocumentSymbol(const std::string& filePath);
    void ParseDocumentSymbolResponse(const std::string& response, std::vector<LspSymbolInformation>& symbols);
    bool FlushAndFetchDocumentSymbols(const std::string& filePath, AbsTick fileTime, LspDocumentSymbols& symbols);
    std::vector<LspDocumentSymbols> m_documentSymbols;

    // 获取语义标记（用于语法高亮）
    void RequestSemanticTokens(const std::string& filePath);
	void ParseSemanticTokensResponse(const std::string& response, std::vector<LspSemanticToken>& tokens);
    bool FlushAndFetchSemanticTokens(const std::string& filePath,AbsTick fileTime,LspSemanticTokens& tokens);
	std::vector<LspSemanticTokens> m_documentSemanticTokens;

    // 设置诊断回调
    void SetDiagnosticsCallback(LspDiagnosticsCallback callback);

    // 解析引用响应
    void ParseReferencesResponse(const std::string& response, std::vector<LspLocation>&locations);
    
   
    // 获取语义标记类型的字符串表示
    static std::string GetTokenTypeString(LspTokenType type);
    
    // 获取语义标记修饰器的字符串表示
    static std::string GetTokenModifierString(LspTokenModifier modifier);

private:
    
    // 发送请求
    void SendRequest(const std::string& method, const std::string& params, LspResponseCallback callback);
    
    // 发送通知
    void SendNotification(const std::string& method, const std::string& params);
    
    // 处理LSP服务器的响应
    void HandleServerResponse(const std::string& response);
    
    // LSP读取线程
    void ReadThread();

private:
    std::string m_workspacePath;
    std::string m_serverPath;
    std::string m_dbFolder; 
    int m_requestId;
    bool m_isRunning;
    bool m_isInitialized;

    int m_version;
    
    // LSP服务器进程相关
    void* m_hProcess;
    void* m_hPipeRead;
    void* m_hPipeWrite;

   
    // 响应回调
    std::unordered_map<int, LspResponseCallback> m_responseCallbacks;
    
    // 诊断回调
    LspDiagnosticsCallback m_diagnosticsCallback;
    
    // 语义标记回调映射
    std::map<int, LspSemanticTokensCallback> m_semanticTokensCallbacks;

	std::unordered_map<std::string, AbsTick> m_documentTimes;

   
    // 线程相关
    std::unique_ptr<std::thread> m_readThread;
    std::mutex m_mutex;
    
    // 从服务器获取的实际标记类型和修饰符图例的映射表
    std::unordered_map<int, LspTokenType> m_tokenTypeMap;       // Server Index -> Client Enum
    std::unordered_map<int, LspTokenModifier> m_tokenModifierMap; // Server Index -> Client Enum

    // 标记类型字符串表 (客户端定义的)
    static const std::vector<std::string> m_tokenTypes;
    
    // 标记修饰器字符串表
    static const std::vector<std::string> m_tokenModifiers;

};


