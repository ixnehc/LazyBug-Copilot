#include "stdh.h"
#include "TreeSitterLanguageCSS.h"

// TreeSitter C接口
#include "treesitter_api.h"

// TreeSitter语言库声明
extern "C" {
	const TSLanguage* tree_sitter_css();
}

TreeSitterSymbol_Begin

//////////////////////////////////////////////////////////////////////////
// CLanguageSupportCss

CLanguageSupportCss::CLanguageSupportCss()
{
	_extensions = { "css" };
}

CLanguageSupportCss::~CLanguageSupportCss()
{
}

const TSLanguage* CLanguageSupportCss::GetTSLanguage() const
{
	return tree_sitter_css();
}

bool CLanguageSupportCss::IsSymbolDefinition(TSNode node) const
{
	// 粗筛：CollectSymbols 覆写中做精判
	const char* nodeType = ts_node_type(node);
	return strcmp(nodeType, "rule_set") == 0 ||
	       strcmp(nodeType, "declaration") == 0 ||
	       _IsAtRule(nodeType);
}

std::string CLanguageSupportCss::GetNodeName(TSNode node, const std::string& sourceCode) const
{
	const char* nodeType = ts_node_type(node);

	if (strcmp(nodeType, "rule_set") == 0)
	{
		// 单值语义：取第一个 selector 文本
		TSNode selectorsNode = _GetChildByType(node, "selectors");
		if (ts_node_is_null(selectorsNode))
			return "";
		uint32_t count = ts_node_named_child_count(selectorsNode);
		if (count == 0)
			return "";
		TSNode firstSel = ts_node_named_child(selectorsNode, 0);
		return _GetNodeText(firstSel, sourceCode);
	}

	if (strcmp(nodeType, "keyframes_statement") == 0)
	{
		TSNode nameNode = _GetChildByType(node, "keyframes_name");
		if (!ts_node_is_null(nameNode))
			return _GetNodeText(nameNode, sourceCode);
	}

	if (_IsAtRule(nodeType))
		return _GetNodeText(node, sourceCode);

	if (strcmp(nodeType, "declaration") == 0)
	{
		TSNode propNode = _GetChildByType(node, "property_name");
		if (ts_node_is_null(propNode))
			return "";
		std::string prop = _GetNodeText(propNode, sourceCode);
		if (prop.length() >= 2 && prop[0] == '-' && prop[1] == '-')
			return prop;
	}

	return "";
}

std::string CLanguageSupportCss::GetNodeDisplayName(TSNode node, const std::string& sourceCode) const
{
	return GetNodeName(node, sourceCode);
}

bool CLanguageSupportCss::GetNameNodeRange(TSNode node, TSPoint& startPoint, TSPoint& endPoint) const
{
	const char* nodeType = ts_node_type(node);

	if (strcmp(nodeType, "rule_set") == 0)
	{
		TSNode selectorsNode = _GetChildByType(node, "selectors");
		if (ts_node_is_null(selectorsNode))
			return false;
		uint32_t count = ts_node_named_child_count(selectorsNode);
		if (count == 0)
			return false;
		TSNode firstSel = ts_node_named_child(selectorsNode, 0);
		startPoint = ts_node_start_point(firstSel);
		endPoint = ts_node_end_point(firstSel);
		return true;
	}

	if (strcmp(nodeType, "keyframes_statement") == 0)
	{
		TSNode nameNode = _GetChildByType(node, "keyframes_name");
		if (!ts_node_is_null(nameNode))
		{
			startPoint = ts_node_start_point(nameNode);
			endPoint = ts_node_end_point(nameNode);
			return true;
		}
	}

	if (strcmp(nodeType, "declaration") == 0)
	{
		TSNode propNode = _GetChildByType(node, "property_name");
		if (!ts_node_is_null(propNode))
		{
			startPoint = ts_node_start_point(propNode);
			endPoint = ts_node_end_point(propNode);
			return true;
		}
	}

	startPoint = ts_node_start_point(node);
	endPoint = ts_node_end_point(node);
	return true;
}

const std::vector<std::string>& CLanguageSupportCss::GetFileExtensions() const
{
	return _extensions;
}

SymbolKind CLanguageSupportCss::GetSymbolKind(TSNode node) const
{
	const char* nodeType = ts_node_type(node);

	if (strcmp(nodeType, "rule_set") == 0)
		return SymbolKind::Css_Selector;

	if (_IsAtRule(nodeType))
		return SymbolKind::Css_AtRule;

	if (strcmp(nodeType, "declaration") == 0)
		return SymbolKind::Css_Variable;

	return SymbolKind::Invalid;
}

LineRange CLanguageSupportCss::GetNodeLineRange(TSNode node, const std::string& sourceCode) const
{
	LineRange range;
	TSPoint sp = ts_node_start_point(node);
	TSPoint ep = ts_node_end_point(node);
	range.start = sp.row;
	range.end = ep.row;
	return range;
}

void CLanguageSupportCss::CollectSymbols(
	TSNode node,
	const std::string& sourceCode,
	std::vector<RawSymbolDefine>& outSymbols,
	std::string& currentPrefix) const
{
	const char* nodeType = ts_node_type(node);

	//////////////////////////////////////////////////////////////////////////
	// rule_set → 按逗号分组，每个 selector 产出一条 Css_Selector symbol
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(nodeType, "rule_set") == 0)
	{
		TSNode selectorsNode = _GetChildByType(node, "selectors");
		if (ts_node_is_null(selectorsNode))
			return;

		LineRange ruleRange = GetNodeLineRange(node, sourceCode);

		uint32_t count = ts_node_named_child_count(selectorsNode);
		for (uint32_t i = 0; i < count; i++)
		{
			TSNode selNode = ts_node_named_child(selectorsNode, i);
			std::string selText = _GetNodeText(selNode, sourceCode);
			if (selText.empty())
				continue;

			RawSymbolDefine symbol;
			symbol.name = selText;
			symbol.showName = selText;
			symbol.kind = SymbolKind::Css_Selector;
			symbol.language = Language::Css;

			TSPoint sp = ts_node_start_point(selNode);
			TSPoint ep = ts_node_end_point(selNode);
			symbol.lineLoc.line = sp.row;
			symbol.lineLoc.startColumn = sp.column;
			symbol.lineLoc.endColumn = ep.column;

			symbol.lineRange = ruleRange;
			if (symbol.lineRange.start > symbol.lineLoc.line)
				symbol.lineRange.start = symbol.lineLoc.line;

			outSymbols.push_back(symbol);
		}
		return;
	}

	//////////////////////////////////////////////////////////////////////////
	// at-rule → 产出一条 Css_AtRule symbol
	//////////////////////////////////////////////////////////////////////////
	if (_IsAtRule(nodeType))
	{
		std::string name;

		// keyframes 优先取名字
		if (strcmp(nodeType, "keyframes_statement") == 0)
		{
			TSNode nameNode = _GetChildByType(node, "keyframes_name");
			if (!ts_node_is_null(nameNode))
				name = _GetNodeText(nameNode, sourceCode);
		}

		// 其他 at-rule 用整行文本
		if (name.empty())
			name = _GetNodeText(node, sourceCode);

		if (name.empty())
			return;

		RawSymbolDefine symbol;
		symbol.name = name;
		symbol.showName = name;
		symbol.kind = SymbolKind::Css_AtRule;
		symbol.language = Language::Css;

		TSPoint sp = ts_node_start_point(node);
		symbol.lineLoc.line = sp.row;
		symbol.lineLoc.startColumn = sp.column;
		symbol.lineLoc.endColumn = sp.column + (WORD)name.length();

		symbol.lineRange = GetNodeLineRange(node, sourceCode);
		if (symbol.lineRange.start > symbol.lineLoc.line)
			symbol.lineRange.start = symbol.lineLoc.line;

		outSymbols.push_back(symbol);
		return;
	}

	//////////////////////////////////////////////////////////////////////////
	// declaration → 仅收集 -- 开头的自定义属性（CSS 变量）
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(nodeType, "declaration") == 0)
	{
		TSNode propNode = _GetChildByType(node, "property_name");
		if (ts_node_is_null(propNode))
			return;

		std::string propName = _GetNodeText(propNode, sourceCode);
		if (propName.length() < 2 || propName[0] != '-' || propName[1] != '-')
			return;

		RawSymbolDefine symbol;
		symbol.name = propName;
		symbol.showName = propName;
		symbol.kind = SymbolKind::Css_Variable;
		symbol.language = Language::Css;

		TSPoint sp = ts_node_start_point(propNode);
		TSPoint ep = ts_node_end_point(propNode);
		symbol.lineLoc.line = sp.row;
		symbol.lineLoc.startColumn = sp.column;
		symbol.lineLoc.endColumn = ep.column;

		symbol.lineRange = GetNodeLineRange(node, sourceCode);
		if (symbol.lineRange.start > symbol.lineLoc.line)
			symbol.lineRange.start = symbol.lineLoc.line;

		outSymbols.push_back(symbol);
		return;
	}

	// CSS 无层级命名需求，不修改 currentPrefix
}

//////////////////////////////////////////////////////////////////////////
// 私有辅助方法

std::string CLanguageSupportCss::_GetNodeText(TSNode node, const std::string& sourceCode) const
{
	uint32_t startByte = ts_node_start_byte(node);
	uint32_t endByte = ts_node_end_byte(node);
	if (startByte < endByte && endByte <= sourceCode.length())
		return sourceCode.substr(startByte, endByte - startByte);
	return "";
}

TSNode CLanguageSupportCss::_GetChildByType(TSNode node, const char* type) const
{
	TSNode nullNode = {};
	uint32_t count = ts_node_named_child_count(node);
	for (uint32_t i = 0; i < count; i++)
	{
		TSNode child = ts_node_named_child(node, i);
		if (strcmp(ts_node_type(child), type) == 0)
			return child;
	}
	return nullNode;
}

bool CLanguageSupportCss::_IsAtRule(const char* nodeType)
{
	return strcmp(nodeType, "media_statement") == 0 ||
	       strcmp(nodeType, "keyframes_statement") == 0 ||
	       strcmp(nodeType, "font_face_statement") == 0 ||
	       strcmp(nodeType, "supports_statement") == 0 ||
	       strcmp(nodeType, "import_statement") == 0 ||
	       strcmp(nodeType, "charset_statement") == 0 ||
	       strcmp(nodeType, "page_statement") == 0 ||
	       strcmp(nodeType, "namespace_statement") == 0;
}

TreeSitterSymbol_End
