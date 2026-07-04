#include "stdh.h"
#include "TreeSitterLanguageHTML.h"

// TreeSitter C接口
#include "treesitter_api.h"

// TreeSitter语言库声明
extern "C" {
	const TSLanguage* tree_sitter_html();
}

TreeSitterSymbol_Begin

//////////////////////////////////////////////////////////////////////////
// CLanguageSupportHtml

CLanguageSupportHtml::CLanguageSupportHtml()
{
	_extensions = { "html", "htm" };
}

CLanguageSupportHtml::~CLanguageSupportHtml()
{
}

const TSLanguage* CLanguageSupportHtml::GetTSLanguage() const
{
	return tree_sitter_html();
}

bool CLanguageSupportHtml::IsSymbolDefinition(TSNode node) const
{
	// HTML 通过 CollectSymbols 覆写处理多 symbol，此方法仅做粗筛。
	// 无法在无 sourceCode 的情况下判断 attribute_name 是否为 id/class，
	// 故对所有 attribute 节点返回 true，实际过滤在 CollectSymbols 中进行。
	const char* nodeType = ts_node_type(node);
	return strcmp(nodeType, "attribute") == 0;
}

std::string CLanguageSupportHtml::GetNodeName(TSNode node, const std::string& sourceCode) const
{
	const char* nodeType = ts_node_type(node);
	if (strcmp(nodeType, "attribute") != 0)
		return "";

	std::string attrName = _GetAttributeNameText(node, sourceCode);
	if (attrName != "id" && attrName != "class")
		return "";

	// 单值语义：class 返回整串（多值拆分由 CollectSymbols 处理）
	TSNode valueNode = _GetAttributeValueNode(node);
	if (ts_node_is_null(valueNode))
		return "";

	uint32_t startByte = ts_node_start_byte(valueNode);
	uint32_t endByte = ts_node_end_byte(valueNode);
	if (startByte < endByte && endByte <= sourceCode.length())
		return sourceCode.substr(startByte, endByte - startByte);

	return "";
}

std::string CLanguageSupportHtml::GetNodeDisplayName(TSNode node, const std::string& sourceCode) const
{
	std::string name = GetNodeName(node, sourceCode);
	if (name.empty())
		return name;

	std::string attrName = _GetAttributeNameText(node, sourceCode);
	if (attrName == "id")
		return "#" + name;
	if (attrName == "class")
		return "." + name;
	return name;
}

bool CLanguageSupportHtml::GetNameNodeRange(TSNode node, TSPoint& startPoint, TSPoint& endPoint) const
{
	TSNode valueNode = _GetAttributeValueNode(node);
	if (ts_node_is_null(valueNode))
		return false;

	startPoint = ts_node_start_point(valueNode);
	endPoint = ts_node_end_point(valueNode);
	return true;
}

const std::vector<std::string>& CLanguageSupportHtml::GetFileExtensions() const
{
	return _extensions;
}

SymbolKind CLanguageSupportHtml::GetSymbolKind(TSNode node) const
{
	// 无 sourceCode 参数，无法区分 id/class，返回 Html_Class 作为兜底。
	// 此方法仅在默认 CollectSymbols 路径中被调用，HTML 已覆写 CollectSymbols。
	const char* nodeType = ts_node_type(node);
	if (strcmp(nodeType, "attribute") == 0)
		return SymbolKind::Html_Class;
	return SymbolKind::Invalid;
}

LineRange CLanguageSupportHtml::GetNodeLineRange(TSNode node, const std::string& sourceCode) const
{
	return _GetElementLineRange(node, sourceCode);
}

void CLanguageSupportHtml::CollectSymbols(
	TSNode node,
	const std::string& sourceCode,
	std::vector<RawSymbolDefine>& outSymbols,
	std::string& currentPrefix) const
{
	const char* nodeType = ts_node_type(node);
	if (strcmp(nodeType, "attribute") != 0)
		return;

	std::string attrName = _GetAttributeNameText(node, sourceCode);
	if (attrName != "id" && attrName != "class")
		return;

	TSNode valueNode = _GetAttributeValueNode(node);
	if (ts_node_is_null(valueNode))
		return;

	uint32_t valueStartByte = ts_node_start_byte(valueNode);
	uint32_t valueEndByte = ts_node_end_byte(valueNode);
	if (valueStartByte >= valueEndByte || valueEndByte > sourceCode.length())
		return;

	std::string valueText = sourceCode.substr(valueStartByte, valueEndByte - valueStartByte);
	TSPoint valueStartPoint = ts_node_start_point(valueNode);
	LineRange elemRange = _GetElementLineRange(node, sourceCode);

	if (attrName == "id")
	{
		if (valueText.empty())
			return;

		RawSymbolDefine symbol;
		symbol.name = valueText;
		symbol.showName = "#" + valueText;
		symbol.kind = SymbolKind::Html_Id;
		symbol.language = Language::Html;
		symbol.lineLoc.line = valueStartPoint.row;
		symbol.lineLoc.startColumn = valueStartPoint.column;
		symbol.lineLoc.endColumn = valueStartPoint.column + (WORD)valueText.length();
		symbol.lineRange = elemRange;

		if (symbol.lineRange.start > symbol.lineLoc.line)
			symbol.lineRange.start = symbol.lineLoc.line;

		outSymbols.push_back(symbol);
	}
	else // class
	{
		// 按空白拆分，逐个产出 symbol
		uint32_t pos = 0;
		while (pos < valueText.length())
		{
			// 跳过空白
			while (pos < valueText.length() && isspace((unsigned char)valueText[pos]))
				pos++;
			if (pos >= valueText.length())
				break;

			uint32_t tokenStart = pos;
			while (pos < valueText.length() && !isspace((unsigned char)valueText[pos]))
				pos++;
			uint32_t tokenEnd = pos;

			std::string token = valueText.substr(tokenStart, tokenEnd - tokenStart);

			RawSymbolDefine symbol;
			symbol.name = token;
			symbol.showName = "." + token;
			symbol.kind = SymbolKind::Html_Class;
			symbol.language = Language::Html;
			symbol.lineLoc.line = valueStartPoint.row;
			symbol.lineLoc.startColumn = valueStartPoint.column + (WORD)tokenStart;
			symbol.lineLoc.endColumn = valueStartPoint.column + (WORD)tokenEnd;
			symbol.lineRange = elemRange;

			if (symbol.lineRange.start > symbol.lineLoc.line)
				symbol.lineRange.start = symbol.lineLoc.line;

			outSymbols.push_back(symbol);
		}
	}

	// HTML 无层级命名需求，不修改 currentPrefix
}

//////////////////////////////////////////////////////////////////////////
// 私有辅助方法

std::string CLanguageSupportHtml::_GetAttributeNameText(TSNode node, const std::string& sourceCode) const
{
	uint32_t childCount = ts_node_child_count(node);
	for (uint32_t i = 0; i < childCount; i++)
	{
		TSNode child = ts_node_child(node, i);
		if (strcmp(ts_node_type(child), "attribute_name") == 0)
		{
			uint32_t startByte = ts_node_start_byte(child);
			uint32_t endByte = ts_node_end_byte(child);
			if (startByte < endByte && endByte <= sourceCode.length())
				return sourceCode.substr(startByte, endByte - startByte);
			return "";
		}
	}
	return "";
}

TSNode CLanguageSupportHtml::_GetAttributeValueNode(TSNode node) const
{
	TSNode nullNode = {};
	uint32_t childCount = ts_node_child_count(node);
	for (uint32_t i = 0; i < childCount; i++)
	{
		TSNode child = ts_node_child(node, i);
		const char* childType = ts_node_type(child);

		// quoted_attribute_value → 内含 attribute_value
		if (strcmp(childType, "quoted_attribute_value") == 0)
		{
			uint32_t innerCount = ts_node_child_count(child);
			for (uint32_t j = 0; j < innerCount; j++)
			{
				TSNode inner = ts_node_child(child, j);
				if (strcmp(ts_node_type(inner), "attribute_value") == 0)
					return inner;
			}
			return nullNode;
		}

		// 直接 attribute_value（无引号）
		if (strcmp(childType, "attribute_value") == 0)
			return child;
	}
	return nullNode;
}

LineRange CLanguageSupportHtml::_GetElementLineRange(TSNode node, const std::string& sourceCode) const
{
	LineRange range;
	range.Zero();

	// 默认取节点自身范围
	TSPoint startPoint = ts_node_start_point(node);
	TSPoint endPoint = ts_node_end_point(node);
	range.start = startPoint.row;
	range.end = endPoint.row;

	// 向上查找 element / script_element / style_element
	TSNode cur = ts_node_parent(node);
	while (!ts_node_is_null(cur))
	{
		const char* curType = ts_node_type(cur);
		if (strcmp(curType, "element") == 0 ||
			strcmp(curType, "script_element") == 0 ||
			strcmp(curType, "style_element") == 0)
		{
			TSPoint s = ts_node_start_point(cur);
			TSPoint e = ts_node_end_point(cur);
			range.start = s.row;
			range.end = e.row;
			break;
		}
		cur = ts_node_parent(cur);
	}

	return range;
}

TreeSitterSymbol_End
