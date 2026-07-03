#include "stdh.h"
#include "TreeSitterLanguagePython.h"

// TreeSitter C接口
#include "treesitter_api.h"

// TreeSitter语言库声明
extern "C" {
	const TSLanguage* tree_sitter_python();
}

TreeSitterSymbol_Begin

//////////////////////////////////////////////////////////////////////////
// CLanguageSupportPython

CLanguageSupportPython::CLanguageSupportPython()
{
	_extensions = { "py", "pyw" };
}

CLanguageSupportPython::~CLanguageSupportPython()
{
}

const TSLanguage* CLanguageSupportPython::GetTSLanguage() const
{
	return tree_sitter_python();
}

bool CLanguageSupportPython::IsSymbolDefinition(TSNode node) const
{
	if (IsClassDefinition(node)) return true;
	if (IsFunctionDefinition(node)) return true;
	return false;
}

std::string CLanguageSupportPython::GetNodeName(TSNode node, const std::string& sourceCode) const
{
	TSPoint start, end;
	if (GetNameNodeRange(node, start, end))
	{
		size_t startIndex = sourceCode.find('\n');
		size_t currentLine = 0;
		size_t lineStart = 0;

		while (currentLine < start.row && startIndex != std::string::npos)
		{
			lineStart = startIndex + 1;
			startIndex = sourceCode.find('\n', lineStart);
			currentLine++;
		}

		if (currentLine == start.row)
		{
			size_t nameStart = lineStart + start.column;
			size_t nameEnd = lineStart + end.column;

			if (nameStart < sourceCode.length() && nameEnd <= sourceCode.length())
			{
				return sourceCode.substr(nameStart, nameEnd - nameStart);
			}
		}
	}

	return "";
}

std::string CLanguageSupportPython::GetNodeDisplayName(TSNode node, const std::string& sourceCode) const
{
	return GetNodeName(node, sourceCode);
}

bool CLanguageSupportPython::GetNameNodeRange(TSNode node, TSPoint& startPoint, TSPoint& endPoint) const
{
	// Python 的 class_definition 和 function_definition 都有 name 字段
	TSNode nameNode = ts_node_child_by_field_name(node, "name", 4);
	if (!ts_node_is_null(nameNode))
	{
		startPoint = ts_node_start_point(nameNode);
		endPoint = ts_node_end_point(nameNode);
		return true;
	}

	return false;
}

const std::vector<std::string>& CLanguageSupportPython::GetFileExtensions() const
{
	return _extensions;
}

SymbolKind CLanguageSupportPython::GetSymbolKind(TSNode node) const
{
	if (IsClassDefinition(node))
		return SymbolKind::Class;

	if (IsFunctionDefinition(node))
		return SymbolKind::Function;

	return SymbolKind::Invalid;
}

LineRange CLanguageSupportPython::GetNodeLineRange(TSNode node, const std::string& sourceCode) const
{
	LineRange range;
	int startLine, endLine;
	GetNodeRange(node, startLine, endLine, sourceCode);
	range.start = startLine;
	range.end = endLine;
	return range;
}

//////////////////////////////////////////////////////////////////////////
// 辅助函数实现

bool CLanguageSupportPython::IsClassDefinition(TSNode node) const
{
	return strcmp(ts_node_type(node), "class_definition") == 0;
}

bool CLanguageSupportPython::IsFunctionDefinition(TSNode node) const
{
	return strcmp(ts_node_type(node), "function_definition") == 0;
}

void CLanguageSupportPython::GetNodeRange(TSNode node, int& startLine, int& endLine, const std::string& sourceCode) const
{
	TSPoint start = ts_node_start_point(node);
	TSPoint end = ts_node_end_point(node);

	startLine = start.row;  // TreeSitter 行号从 0 开始；与 lineLoc.line(0基) 保持一致
	endLine = end.row;
}

TreeSitterSymbol_End
