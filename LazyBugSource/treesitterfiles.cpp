#include "stdh.h"
#include "TreeSitterFiles.h"
#include <algorithm>
#include <cstring>

// Tree-sitter C++ language function
extern "C" TSLanguage* tree_sitter_cpp();

CTreeSitterFiles::CTreeSitterFiles()
    : _parser(nullptr)
{
}

CTreeSitterFiles::~CTreeSitterFiles()
{
    Clear();
}

bool CTreeSitterFiles::Init()
{
    // 创建解析器
    _parser = ts_parser_new();
    if (!_parser)
    {
        return false;
    }

    // 注册默认支持的语言(C++)
    _RegisterLanguageInternal(tree_sitter_cpp(), {"cpp", "cxx", "cc", "c", "h", "hpp", "hxx"});

    return true;
}

void CTreeSitterFiles::Clear()
{
    // 释放所有文件的语法树
    for (auto& pair : _files)
    {
        if (pair.second.pTree)
        {
            ts_tree_delete(pair.second.pTree);
            pair.second.pTree = nullptr;
        }
    }
    _files.clear();

    // 释放解析器
    if (_parser)
    {
        ts_parser_delete(_parser);
        _parser = nullptr;
    }

    // 清理语言映射(注意：我们不释放TSLanguage*，因为它们是静态的)
    _languages.clear();
    _registeredLanguages.clear();
}

bool CTreeSitterFiles::UpdateFile(const std::string& sFilePath, const std::string& sContent)
{
    if (!_parser)
    {
        return false;
    }

    // 获取对应的语言解析器
    TSLanguage* language = _GetLanguageForFile(sFilePath);
    if (!language)
    {
        return false;
    }

    // 设置解析器语言
    if (!ts_parser_set_language(_parser, language))
    {
        return false;
    }

    // 检查文件是否已存在
    auto it = _files.find(sFilePath);
    TSTree* oldTree = nullptr;
    if (it != _files.end())
    {
        oldTree = it->second.pTree;
    }

    // 解析文件内容（使用增量解析如果有旧树）
    TSTree* newTree = ts_parser_parse_string_encoding(_parser, nullptr, sContent.c_str(), (uint32_t)sContent.length(), TSInputEncodingUTF8);
    if (!newTree)
    {
        return false;
    }

    // 释放旧的语法树
    if (oldTree)
    {
        ts_tree_delete(oldTree);
    }

    // 存储或更新文件信息
    SFileInfo& fileInfo = _files[sFilePath];
    fileInfo.content = sContent;
    fileInfo.pTree = newTree;
    fileInfo.version++;

    return true;
}

CTreeSitterFiles::AstNode CTreeSitterFiles::GetSmallestNodeIncludingRange(const std::string& sFilePath, const SCharRange &charRange)
{
    AstNode result;
    
    if (!_parser || !charRange.IsValid())
    {
        return result;
    }

    // 检查文件是否存在
    auto it = _files.find(sFilePath);
    if (it == _files.end() || !it->second.pTree)
    {
        return result;
    }

    const std::string& content = it->second.content;
    TSTree* tree = it->second.pTree;

    // 创建TSPoint结构
    TSPoint startPoint;
    startPoint.row = (uint32_t)charRange.nStartLine;
    startPoint.column = (uint32_t)charRange.nStartColumn;
    
    TSPoint endPoint;
    endPoint.row = (uint32_t)charRange.nEndLine;
    endPoint.column = (uint32_t)charRange.nEndColumn;

    // 获取根节点
    TSNode rootNode = ts_tree_root_node(tree);

    // 查找包含指定范围的最小节点
    TSNode targetNode = ts_node_descendant_for_point_range(rootNode, startPoint, endPoint);

    // 如果没有找到合适的节点，使用根节点
    if (ts_node_is_null(targetNode))
    {
        targetNode = rootNode;
    }

    // 直接转换为AstNode（不考虑注释节点）
    result = _ConvertTSNodeToAstNode(targetNode, content);
    result.filePath = sFilePath;

    return result;
}

bool CTreeSitterFiles::IsAstNodeValid(const AstNode& astNode) const
{
    // 检查TSNode是否为null
    if (ts_node_is_null(astNode.tsNode))
    {
        return false;
    }

    // 检查文件是否还存在
    auto it = _files.find(astNode.filePath);
    if (it == _files.end())
    {
        return false;
    }

    // 检查TSNode是否来自当前的TSTree
    // 通过比较TSNode的tree指针和当前文件的tree指针
    if (astNode.tsNode.tree != it->second.pTree)
    {
        return false;
    }

    return true;
}

bool CTreeSitterFiles::GetByteRange(const AstNode& astNode, uint32_t& startByte, uint32_t& endByte) const
{
    // 首先验证AstNode是否有效
    if (!IsAstNodeValid(astNode))
    {
        return false;
    }

    // 获取节点的字节范围
    startByte = ts_node_start_byte(astNode.tsNode);
    endByte = ts_node_end_byte(astNode.tsNode);

    return true;
}

bool CTreeSitterFiles::GetByteCount(const AstNode& astNode, uint32_t& byteCount) const
{
    uint32_t startByte, endByte;
    if (!GetByteRange(astNode, startByte, endByte))
    {
        return false;
    }
    
    if (endByte >= startByte)
    {
        byteCount = endByte - startByte;
    }
    else
    {
        byteCount = 0;
    }
    return true;
}

// 内部辅助函数：从给定字节偏移开始，扫描到指定行列位置
// 返回新的字节偏移
static uint32_t ScanToPosition(const std::string& content, uint32_t startByte, int targetLine, int targetColumn, int& currentLine, int& currentColumn)
{
    size_t i = startByte;
    
    while (i < content.length())
    {
        if (currentLine == targetLine && currentColumn == targetColumn)
        {
            break;
        }
        
        if (content[i] == '\n')
        {
            currentLine++;
            currentColumn = 0;
            i++;
        }
        else if (content[i] == '\r')
        {
            if (i + 1 < content.length() && content[i + 1] == '\n')
            {
                i += 2; // 跳过 \r\n
            }
            else
            {
                i++; // 只有 \r
            }
            currentLine++;
            currentColumn = 0;
        }
        else
        {
            // 处理UTF-8字符
            int charBytes = 1;
            unsigned char byte = (unsigned char)content[i];
            if (byte >= 0x80)
            {
                if ((byte & 0xE0) == 0xC0) charBytes = 2;      // 110xxxxx
                else if ((byte & 0xF0) == 0xE0) charBytes = 3; // 1110xxxx
                else if ((byte & 0xF8) == 0xF0) charBytes = 4; // 11110xxx
            }
            
            i += charBytes;
            
            // 只在目标行内才增加列号
            if (currentLine == targetLine)
            {
                currentColumn++;
                if (currentColumn == targetColumn)
                {
                    break;
                }
            }
        }
    }
    
    return (uint32_t)i;
}

bool CTreeSitterFiles::GetByteRange(const std::string& filePath, const SCharRange& range, uint32_t& startByte, uint32_t& endByte) const
{
    if (!range.IsValid())
    {
        return false;
    }

    // 检查文件是否存在
    auto it = _files.find(filePath);
    if (it == _files.end())
    {
        return false;
    }

    const std::string& content = it->second.content;
    
    int currentLine = 0;
    int currentColumn = 0;
    
    // 扫描到起始位置
    startByte = ScanToPosition(content, 0, range.nStartLine, range.nStartColumn, currentLine, currentColumn);
    
    // 从起始位置继续扫描到结束位置
    endByte = ScanToPosition(content, startByte, range.nEndLine, range.nEndColumn, currentLine, currentColumn);
    
    return true;
}

bool CTreeSitterFiles::GetSiblingNode(const AstNode &node, bool isPrev, AstNode &siblingNode) const
{
    siblingNode = AstNode();

    // 验证节点有效性
    if (!IsAstNodeValid(node))
    {
        return false;
    }

    TSNode siblingTsNode;
    if (isPrev)
        siblingTsNode = ts_node_prev_named_sibling(node.tsNode);
    else
        siblingTsNode = ts_node_next_named_sibling(node.tsNode);

    if (ts_node_is_null(siblingTsNode))
    {
        return false;
    }

    // 找到文件内容
    auto it = _files.find(node.filePath);
    if (it == _files.end())
    {
        return false;
    }

    siblingNode = _ConvertTSNodeToAstNode(siblingTsNode, it->second.content);
    siblingNode.filePath = node.filePath;
    return true;
}


TSLanguage* CTreeSitterFiles::_GetLanguageForFile(const std::string& sFilePath)
{
    // 获取文件后缀
    std::string suffix = GetFileSuffix(sFilePath);
    if (suffix.empty())
    {
        return nullptr;
    }

    // 移除后缀前的点号
    if (suffix[0] == '.')
    {
        suffix = suffix.substr(1);
    }

    // 转换为小写
    std::transform(suffix.begin(), suffix.end(), suffix.begin(), ::tolower);

    // 查找对应的语言
    auto it = _languages.find(suffix);
    if (it != _languages.end())
    {
        return it->second;
    }

    // 如果没有找到，尝试注册新的语言
    // 目前只支持C++，未来可以在这里添加其他语言的支持
    if (suffix == "cpp" || suffix == "cxx" || suffix == "cc" || 
        suffix == "c" || suffix == "h" || suffix == "hpp" || suffix == "hxx")
    {
        // C++语言应该已经在Init中注册了，如果没有找到说明有问题
        return nullptr;
    }

    return nullptr;
}

void CTreeSitterFiles::_RegisterLanguageInternal(TSLanguage* pLanguage, const std::vector<std::string>& extensions)
{
    if (!pLanguage)
    {
        return;
    }

    // 获取语言名称(这里简化处理，使用第一个扩展名作为语言名称)
    std::string languageName = extensions.empty() ? "unknown" : extensions[0];

    // 检查是否已经注册过这个语言
    if (_registeredLanguages.find(languageName) != _registeredLanguages.end())
    {
        return;
    }

    // 注册语言
    _registeredLanguages[languageName] = pLanguage;

    // 为所有扩展名建立映射
    for (const std::string& ext : extensions)
    {
        _languages[ext] = pLanguage;
    }
}

CTreeSitterFiles::AstNode CTreeSitterFiles::_ConvertTSNodeToAstNode(const TSNode& tsNode, const std::string& sContent) const
{
    AstNode astNode;
    
    if (ts_node_is_null(tsNode))
    {
        return astNode;
    }

    // 复制TSNode
    astNode.tsNode = tsNode;

    // 获取节点类型
    const char* nodeTypeStr = ts_node_type(tsNode);
    if (nodeTypeStr)
    {
        astNode.nodeType = nodeTypeStr;
    }

    // 获取位置信息
    TSPoint startPoint = ts_node_start_point(tsNode);
    TSPoint endPoint = ts_node_end_point(tsNode);

    astNode.charRange.nStartLine = (int)startPoint.row;
    astNode.charRange.nStartColumn = (int)startPoint.column;
    astNode.charRange.nEndLine = (int)endPoint.row;
    astNode.charRange.nEndColumn = (int)endPoint.column;

	uint32_t startByte = ts_node_start_byte(tsNode);
	uint32_t endByte = ts_node_end_byte(tsNode);

	astNode.debugContent = sContent.substr(startByte, endByte - startByte);

    return astNode;
}

TSNode CTreeSitterFiles::_FindNodeWithPrecedingComments(const TSNode& targetNode, const std::string& content) const
{
    if (ts_node_is_null(targetNode))
    {
        return targetNode;
    }

    TSNode currentNode = targetNode;
    TSNode resultNode = targetNode;

    // 获取目标节点的起始位置
    TSPoint targetStartPoint = ts_node_start_point(targetNode);

    // 向前查找兄弟节点中的注释
    TSNode parent = ts_node_parent(currentNode);
    if (!ts_node_is_null(parent))
    {
        uint32_t childCount = ts_node_child_count(parent);
        
        // 找到当前节点在父节点中的索引
        int currentIndex = -1;
        for (uint32_t i = 0; i < childCount; ++i)
        {
            TSNode child = ts_node_child(parent, i);
            if (ts_node_eq(child, currentNode))
            {
                currentIndex = (int)i;
                break;
            }
        }

        // 向前查找注释节点
        if (currentIndex > 0)
        {
            for (int i = currentIndex - 1; i >= 0; --i)
            {
                TSNode prevSibling = ts_node_child(parent, (uint32_t)i);
                const char* nodeType = ts_node_type(prevSibling);
                
                // 检查是否是注释节点
                if (nodeType && (strcmp(nodeType, "comment") == 0 || 
                                strcmp(nodeType, "line_comment") == 0 ||
                                strcmp(nodeType, "block_comment") == 0))
                {
                    // 检查注释和目标节点之间是否只有空白字符
                    TSPoint commentEndPoint = ts_node_end_point(prevSibling);
                    
                    // 简化检查：如果注释和目标节点在相邻行或同一行，认为是紧邻的
                    if (targetStartPoint.row <= commentEndPoint.row + 1)
                    {
                        resultNode = prevSibling;
                        continue; // 继续向前查找更多注释
                    }
                    else
                    {
                        break; // 如果行距太远，停止查找
                    }
                }
                else
                {
                    break; // 如果遇到非注释节点，停止查找
                }
            }
        }
    }

    return resultNode;
}



bool CTreeSitterFiles::ExpandRange(const std::string& sFilePath, const SCharRange& curRange, uint32_t maxByte, SCharRange& expandedRange)
{
    expandedRange = curRange;

    if (!_parser || !curRange.IsValid())
    {
        return false;
    }

    // 检查文件是否存在
    auto it = _files.find(sFilePath);
    if (it == _files.end() || !it->second.pTree)
    {
        return false;
    }

    // 获取包含当前范围的最小节点
    AstNode currentNode = GetSmallestNodeIncludingRange(sFilePath, curRange);
    if (!IsAstNodeValid(currentNode))
    {
        return false;
    }

    // 检查当前节点的字节数是否已经超出限制
    uint32_t currentByteCount;
    if (!GetByteCount(currentNode, currentByteCount))
    {
        return false;
    }

	expandedRange = currentNode.charRange;
	if (currentByteCount >= maxByte)
    {
		SCharRange bestRange;

        // 当前节点已经超出限制,尝试缩小
		uint32_t byteStart, byteEnd;
		if (GetByteRange(currentNode, byteStart, byteEnd))
		{
			uint32_t maxByteEnd = byteStart + maxByte;
			uint32_t bestByteEnd = byteStart;

			int testEndLine = curRange.nStartLine + maxByte / 20;
			if (testEndLine > expandedRange.nEndLine)
				testEndLine = expandedRange.nEndLine;
			for (int i = curRange.nStartLine + 1;i < testEndLine;i++)
			{
				SCharRange testRange;
				testRange.nStartLine = testRange.nEndLine = i;
				testRange.nStartColumn = testRange.nEndColumn = 0;
				AstNode testNode = GetSmallestNodeIncludingRange(sFilePath, testRange);
				
				uint32_t testByteStart, testByteEnd;

				if (!GetByteRange(testNode, testByteStart, testByteEnd))
					break;

				if (testByteEnd < maxByteEnd)
				{
					if (testByteEnd > bestByteEnd)
					{
						bestByteEnd = testByteEnd;
						bestRange = testRange;
					}
				}
			}
		}

		if (bestRange.IsValid())
		{
			expandedRange.nEndLine = bestRange.nEndLine;
			expandedRange.nEndColumn = bestRange.nEndColumn;
		}

        return true;
    }

	// 尝试扩展到父节点（包括更高层的父节点）
	{
		AstNode bestNode = currentNode;
		uint32_t bestByteCount = currentByteCount;

		while (bestByteCount < maxByte)
		{
			TSNode parentTsNode = ts_node_parent(bestNode.tsNode);
			if (ts_node_is_null(parentTsNode))
			{
				break; // 已经到达根节点
			}

			AstNode parentNode = _ConvertTSNodeToAstNode(parentTsNode, it->second.content);
			parentNode.filePath = sFilePath;

			uint32_t parentByteCount;
			if (GetByteCount(parentNode, parentByteCount) && parentByteCount < maxByte)
			{
				bestNode = parentNode;
				bestByteCount = parentByteCount;
			}
			else
			{
				break; // 父节点超出限制，停止向上扩展
			}
		}

		currentNode = bestNode;
		currentByteCount = bestByteCount;
		expandedRange = currentNode.charRange;
	}

    // 如果父节点扩展失败后还有剩余空间，尝试交错扩展兄弟节点
    if (currentByteCount < maxByte)
    {
        AstNode workingNode[2];
        workingNode[0] = currentNode; // 向前工作节点
        workingNode[1] = currentNode; // 向后工作节点
        bool expanded[2] = {false, false};

        // 0 表示向前，1 表示向后
        for (int dir = 0; currentByteCount < maxByte && (!expanded[0] || !expanded[1]); dir = 1 - dir)
        {
            if (expanded[dir])
            {
                continue; // 这个方向已经不能扩展了
            }

            AstNode sibling;
            if (!GetSiblingNode(workingNode[dir], dir == 0, sibling))
            {
                expanded[dir] = true; // 找不到兄弟节点
                continue;
            }

            // sibling 必须有效
            uint32_t sByteCount;
            if (!GetByteCount(sibling, sByteCount) || sByteCount == 0)
            {
                expanded[dir] = true;
                continue;
            }

			//超出上限
			if (sByteCount + currentByteCount > maxByte)
			{
				expanded[dir] = true;
				continue;
			}

			currentByteCount += sByteCount;

            // 更新expandedRange
            if (dir == 0)
            {
                expandedRange.nStartLine = sibling.charRange.nStartLine;
                expandedRange.nStartColumn = sibling.charRange.nStartColumn;
            }
            else
            {
                expandedRange.nEndLine = sibling.charRange.nEndLine;
                expandedRange.nEndColumn = sibling.charRange.nEndColumn;
            }

            workingNode[dir] = sibling;
        }
    }

    return true;
}