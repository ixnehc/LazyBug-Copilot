#pragma once
#include <vector>
#include <unordered_map>
#include <string>

#include "treesitter_api.h"
#include "stringparser/stringparser.h"

// Tree-sitter的语言实现通常是C函数, 需要用 extern "C" 来声明
// 这里我们假设有一个用于C++的tree-sitter grammar函数
extern "C" TSLanguage* tree_sitter_cpp();

// CTreeSitterFiles类
// 管理多个源文件的Tree-sitter解析树, 并提供查询功能
//
// 这个类维护一个文件路径到其对应语法树的映射。
// 它能够根据文件后缀自动选择合适的语言进行解析,
// 支持文件的增量更新, 以及查询特定代码范围对应的最小语法节点。
class CTreeSitterFiles
{
public:
    // 构造函数
    // 初始化解析器, 并注册默认支持的语言(例如C++)。
    CTreeSitterFiles();

    // 析构函数
    // 释放所有已分配的资源, 包括解析器、所有文件的语法树和语言对象。
    ~CTreeSitterFiles();

public:

	// 字符范围结构体，使用[start,end)表示法
	struct SCharRange
	{
		int nStartLine;    // 起始行号 (包含，从0开始计数)
		int nStartColumn;  // 起始列号 (包含，从0开始计数)
		int nEndLine;      // 结束行号 (不包含，从0开始计数)
		int nEndColumn;    // 结束列号 (不包含，从0开始计数)

		SCharRange() : nStartLine(-1), nStartColumn(-1), nEndLine(-1), nEndColumn(-1) {}
		SCharRange(int startLine, int startColumn, int endLine, int endColumn) 
			: nStartLine(startLine), nStartColumn(startColumn), nEndLine(endLine), nEndColumn(endColumn) {}

		// 检查范围是否有效
		bool IsValid() const { 
			return nStartLine >= 0 && nStartColumn >= 0 && 
				   nEndLine >= 0 && nEndColumn >= 0 &&
				   (nEndLine > nStartLine || (nEndLine == nStartLine && nEndColumn >= nStartColumn)); 
		}
	};

	struct AstNode
	{
		SCharRange charRange;
		TSNode tsNode;           // Tree-sitter节点信息，用于快速定位
		std::string nodeType;    // 节点类型名称
		std::string filePath;    // 所属文件路径，用于验证TSNode的有效性
		std::string debugContent;//调试用
		
		AstNode() 
		{
			// 初始化TSNode为null节点
			memset(&tsNode, 0, sizeof(TSNode));
		}
		
		// 检查TSNode是否仍然有效（即对应的TSTree没有被更新）
		// 需要配合CTreeSitterFiles的方法来验证
		bool IsNodeValid() const 
		{
			return !ts_node_is_null(tsNode);
		}
	};


    // 初始化函数
    // 初始化解析器并注册默认语言
    // @return bool 如果初始化成功返回true, 否则返回false
    bool Init();

    // 清理函数
    // 释放所有资源并重置状态
    void Clear();


    // 更新文件内容，如果文件不存在则添加新文件。
    // 此函数会利用Tree-sitter的增量解析功能来高效更新语法树。
    // @param sFilePath 文件的完整路径, 将被用作唯一标识符。
    // @param sContent 文件的文本内容。
    // @return bool 如果成功解析并更新文件, 返回true; 如果找不到对应的语言解析器, 返回false。
    bool UpdateFile(const std::string& sFilePath, const std::string& sContent);

    // 获取包含指定字符范围的最小语法节点。
    // @param sFilePath 要查询的文件的路径。
    // @param charRange 查询的字符范围。
    // @return AstNode 返回包含指定范围的最小语法节点。
    //         如果文件不存在或未找到合适的节点, 返回无效的AstNode。
    // @warning 返回的AstNode中的TSNode在文件更新后会失效，使用前应检查IsNodeValid()
	AstNode GetSmallestNodeIncludingRange(const std::string& sFilePath, const SCharRange &charRange);

    // 验证AstNode中的TSNode是否仍然有效
    // @param astNode 要验证的AstNode
    // @return bool 如果TSNode仍然有效返回true，否则返回false
    bool IsAstNodeValid(const AstNode& astNode) const;

    // 获取节点的字节范围
    // @param astNode 要查询的AstNode
    // @param startByte 输出参数，节点的起始字节偏移
    // @param endByte 输出参数，节点的结束字节偏移
    // @return bool 如果成功获取字节范围返回true，否则返回false
    bool GetByteRange(const AstNode& astNode, uint32_t& startByte, uint32_t& endByte) const;

	// 获取字符范围对应的字节范围
	// @param filePath 文件路径
	// @param range 字符范围
	// @param startByte 输出参数，起始字节偏移
	// @param endByte 输出参数，结束字节偏移
	// @return bool 如果成功获取字节范围返回true，否则返回false
	bool GetByteRange(const std::string& filePath, const SCharRange& range, uint32_t& startByte, uint32_t& endByte) const;

    // 获取节点的字节数量
    // @param astNode 要查询的AstNode
    // @param byteCount 输出参数，节点的字节数量
    // @return bool 如果成功获取字节数量返回true，否则返回false
    bool GetByteCount(const AstNode& astNode, uint32_t& byteCount) const;

    // 获取兄弟节点
    // @param node 当前AstNode
    // @param isPrev true表示获取前一个兄弟节点，false表示获取下一个兄弟节点
    // @param siblingNode 输出的兄弟节点，如果未找到则返回无效节点
    // @return bool 找到返回true，否则false
    bool GetSiblingNode(const AstNode &node, bool isPrev, AstNode &siblingNode) const;

    // 扩展范围
    // @param sFilePath 文件路径
    // @param range 要扩展的字符范围
    // @param maxByte 扩展后的最大字节数限制
    // @param expandedRange 输出的扩展后范围
    // @return bool 成功扩展返回true，否则false
    bool ExpandRange(const std::string& sFilePath, const SCharRange& range, uint32_t maxByte, SCharRange& expandedRange);

private:
    // 存储每个文件的相关信息。
    struct SFileInfo
    {
        std::string content;      // 文件内容
        TSTree* pTree = nullptr;  // 解析后的语法树
        uint64_t version = 0;     // 文件版本号，每次更新时递增，用于验证TSNode有效性
    };

    // 根据文件路径获取对应的语言解析器。
    // 如果该语言尚未注册，会自动注册支持的语言。
    // @param sFilePath 文件路径。
    // @return TSLanguage* 对应的语言解析器，如果不支持该文件类型则返回nullptr。
    TSLanguage* _GetLanguageForFile(const std::string& sFilePath);

    // 注册一种语言及其对应的多个文件后缀。
    // @param pLanguage 指向Tree-sitter语言对象的指针。
    // @param extensions 该语言支持的文件后缀列表。
    void _RegisterLanguageInternal(TSLanguage* pLanguage, const std::vector<std::string>& extensions);

    // 将TSNode转换为AstNode结构。
    // @param tsNode Tree-sitter节点。
    // @param sContent 文件内容，用于计算行号。
    // @return AstNode 包含行范围和节点信息的结构体。
    AstNode _ConvertTSNodeToAstNode(const TSNode& tsNode, const std::string& sContent) const;

    // 查找包含前置注释的节点。
    // 从给定节点开始向前查找紧邻的注释节点，返回包含这些注释的最早节点。
    // @param targetNode 目标节点。
    // @param content 文件内容，用于检查注释和节点之间的空白字符。
    // @return TSNode 包含前置注释的节点，如果没有找到注释则返回原节点。
    TSNode _FindNodeWithPrecedingComments(const TSNode& targetNode, const std::string& content) const;

private:
    TSParser* _parser;  // 可复用的Tree-sitter解析器

    // 存储每个已解析文件的信息, key是文件路径
    std::unordered_map<std::string, SFileInfo> _files;

    // 存储文件后缀到语言的映射, key是文件后缀(如 ".cpp")
    std::unordered_map<std::string, TSLanguage*> _languages;
    
    // 存储已注册的语言，避免重复注册，key是语言名称(如 "cpp", "python")
    std::unordered_map<std::string, TSLanguage*> _registeredLanguages;
};

