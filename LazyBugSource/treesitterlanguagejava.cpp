#include "stdh.h"
#include "TreeSitterLanguageJava.h"

// TreeSitter C接口
#include "treesitter_api.h"

// TreeSitter语言库声明
extern "C" {
	const TSLanguage* tree_sitter_java();
}

TreeSitterSymbol_Begin

//////////////////////////////////////////////////////////////////////////
// CLanguageSupportJava

CLanguageSupportJava::CLanguageSupportJava()
{
	_extensions = { "java" };
}

CLanguageSupportJava::~CLanguageSupportJava()
{
}

const TSLanguage* CLanguageSupportJava::GetTSLanguage() const
{
	return tree_sitter_java();
}

bool CLanguageSupportJava::IsSymbolDefinition(TSNode node) const
{
	if (IsTypeContainer(node)) return true;
	if (IsMethodDeclaration(node)) return true;
	if (IsFieldDeclaration(node)) return true;
	if (IsEnumConstant(node)) return true;
	if (IsPackageDeclaration(node)) return true;

	return false;
}

std::string CLanguageSupportJava::GetTextByPointRange(const TSPoint& start, const TSPoint& end, const std::string& sourceCode) const
{
	size_t lineStart = 0;
	size_t currentLine = 0;
	size_t pos = sourceCode.find('\n');

	while (currentLine < start.row && pos != std::string::npos)
	{
		lineStart = pos + 1;
		pos = sourceCode.find('\n', lineStart);
		currentLine++;
	}

	if (currentLine == start.row)
	{
		size_t nameStart = lineStart + start.column;
		size_t nameEnd = lineStart + end.column;

		if (nameStart < sourceCode.length() && nameEnd <= sourceCode.length() && nameEnd >= nameStart)
		{
			return sourceCode.substr(nameStart, nameEnd - nameStart);
		}
	}

	return "";
}

std::string CLanguageSupportJava::GetNodeName(TSNode node, const std::string& sourceCode) const
{
	TSPoint start, end;
	if (GetNameNodeRange(node, start, end))
	{
		return GetTextByPointRange(start, end, sourceCode);
	}

	return "";
}

std::string CLanguageSupportJava::GetNodeDisplayName(TSNode node, const std::string& sourceCode) const
{
	return GetNodeName(node, sourceCode);
}

bool CLanguageSupportJava::GetNameNodeRange(TSNode node, TSPoint& startPoint, TSPoint& endPoint) const
{
	// 大部分声明（class/interface/enum/record/annotation/method/constructor/enum_constant）
	// 都带有 name 字段
	TSNode nameNode = ts_node_child_by_field_name(node, "name", 4);
	if (!ts_node_is_null(nameNode))
	{
		startPoint = ts_node_start_point(nameNode);
		endPoint = ts_node_end_point(nameNode);
		return true;
	}

	// field_declaration：取其 variable_declarator 的 name
	uint32_t childCount = ts_node_child_count(node);
	for (uint32_t i = 0; i < childCount; i++)
	{
		TSNode child = ts_node_child(node, i);
		const char* childType = ts_node_type(child);
		if (strcmp(childType, "variable_declarator") == 0)
		{
			TSNode subName = ts_node_child_by_field_name(child, "name", 4);
			if (!ts_node_is_null(subName))
			{
				startPoint = ts_node_start_point(subName);
				endPoint = ts_node_end_point(subName);
				return true;
			}
		}
	}

	// 兜底：package_declaration 等，找 identifier / scoped_identifier
	for (uint32_t i = 0; i < childCount; i++)
	{
		TSNode child = ts_node_child(node, i);
		const char* childType = ts_node_type(child);

		if (strcmp(childType, "identifier") == 0 ||
			strcmp(childType, "scoped_identifier") == 0)
		{
			startPoint = ts_node_start_point(child);
			endPoint = ts_node_end_point(child);
			return true;
		}
	}

	return false;
}

const std::vector<std::string>& CLanguageSupportJava::GetFileExtensions() const
{
	return _extensions;
}

SymbolKind CLanguageSupportJava::GetSymbolKind(TSNode node) const
{
	if (IsClassDeclaration(node))
		return SymbolKind::Class;

	if (IsInterfaceDeclaration(node))
		return SymbolKind::Interface;

	if (IsEnumDeclaration(node))
		return SymbolKind::Enum;

	if (IsRecordDeclaration(node))
		return SymbolKind::Java_Record;

	if (IsAnnotationTypeDeclaration(node))
		return SymbolKind::Java_Annotation;

	if (IsMethodDeclaration(node))
	{
		const char* nodeType = ts_node_type(node);
		if (strcmp(nodeType, "constructor_declaration") == 0)
			return SymbolKind::Constructor;
		return SymbolKind::Method;
	}

	if (IsEnumConstant(node))
		return SymbolKind::EnumConstant;

	if (IsFieldDeclaration(node))
		return SymbolKind::Field;

	if (IsPackageDeclaration(node))
		return SymbolKind::Java_Package;

	return SymbolKind::Invalid;
}

LineRange CLanguageSupportJava::GetNodeLineRange(TSNode node, const std::string& sourceCode) const
{
	LineRange range;
	int startLine, endLine;
	GetNodeRange(node, startLine, endLine, sourceCode);
	range.start = startLine;
	range.end = endLine;
	return range;
}

std::string CLanguageSupportJava::MakeSymbol(
	TSNode node,
	const std::string& nodeName,
	const TSPoint& nameStart,
	const TSPoint& nameEnd,
	SymbolKind kind,
	const std::string& currentPrefix,
	const std::string& sourceCode,
	std::vector<RawSymbolDefine>& outSymbols) const
{
	RawSymbolDefine symbol;

	if (!currentPrefix.empty() && !nodeName.empty())
		symbol.name = currentPrefix + "." + nodeName;
	else
		symbol.name = nodeName;

	symbol.showName = nodeName;
	symbol.kind = kind;
	symbol.language = GetLanguage();
	symbol.lineRange = GetNodeLineRange(node, sourceCode);

	symbol.lineLoc.line = nameStart.row;
	symbol.lineLoc.startColumn = nameStart.column;
	symbol.lineLoc.endColumn = nameEnd.column;

	if (symbol.lineRange.start > symbol.lineLoc.line)
		symbol.lineRange.start = symbol.lineLoc.line;

	if (!symbol.name.empty())
		outSymbols.push_back(symbol);

	return symbol.name;
}

void CLanguageSupportJava::CollectSymbols(
	TSNode node,
	const std::string& sourceCode,
	std::vector<RawSymbolDefine>& outSymbols,
	std::string& currentPrefix) const
{
	if (!IsSymbolDefinition(node))
		return;

	// package：单独成符号，不参与嵌套前缀
	if (IsPackageDeclaration(node))
	{
		TSPoint nameStart, nameEnd;
		if (GetNameNodeRange(node, nameStart, nameEnd))
		{
			std::string nodeName = GetTextByPointRange(nameStart, nameEnd, sourceCode);
			std::string emptyPrefix;
			MakeSymbol(node, nodeName, nameStart, nameEnd, SymbolKind::Java_Package, emptyPrefix, sourceCode, outSymbols);
		}
		return;
	}

	// field_declaration：可能一行声明多个变量，逐个产出 symbol
	if (IsFieldDeclaration(node))
	{
		uint32_t childCount = ts_node_child_count(node);
		for (uint32_t i = 0; i < childCount; i++)
		{
			TSNode child = ts_node_child(node, i);
			if (strcmp(ts_node_type(child), "variable_declarator") != 0)
				continue;

			TSNode subName = ts_node_child_by_field_name(child, "name", 4);
			if (ts_node_is_null(subName))
				continue;

			TSPoint nameStart = ts_node_start_point(subName);
			TSPoint nameEnd = ts_node_end_point(subName);
			std::string nodeName = GetTextByPointRange(nameStart, nameEnd, sourceCode);

			// 字段不改变 currentPrefix
			MakeSymbol(node, nodeName, nameStart, nameEnd, SymbolKind::Field, currentPrefix, sourceCode, outSymbols);
		}
		return;
	}

	// 其余：类型容器、方法/构造器、枚举常量
	TSPoint nameStart, nameEnd;
	if (!GetNameNodeRange(node, nameStart, nameEnd))
		return;

	std::string nodeName = GetTextByPointRange(nameStart, nameEnd, sourceCode);
	SymbolKind kind = GetSymbolKind(node);

	std::string fullName = MakeSymbol(node, nodeName, nameStart, nameEnd, kind, currentPrefix, sourceCode, outSymbols);

	// 仅类型容器（class/interface/enum/record/annotation）会作为后续子节点的命名前缀
	if (IsTypeContainer(node) && !fullName.empty())
		currentPrefix = fullName;
}

//////////////////////////////////////////////////////////////////////////
// 辅助函数实现

bool CLanguageSupportJava::IsClassDeclaration(TSNode node) const
{
	return strcmp(ts_node_type(node), "class_declaration") == 0;
}

bool CLanguageSupportJava::IsInterfaceDeclaration(TSNode node) const
{
	return strcmp(ts_node_type(node), "interface_declaration") == 0;
}

bool CLanguageSupportJava::IsEnumDeclaration(TSNode node) const
{
	return strcmp(ts_node_type(node), "enum_declaration") == 0;
}

bool CLanguageSupportJava::IsRecordDeclaration(TSNode node) const
{
	return strcmp(ts_node_type(node), "record_declaration") == 0;
}

bool CLanguageSupportJava::IsAnnotationTypeDeclaration(TSNode node) const
{
	return strcmp(ts_node_type(node), "annotation_type_declaration") == 0;
}

bool CLanguageSupportJava::IsMethodDeclaration(TSNode node) const
{
	const char* nodeType = ts_node_type(node);
	return strcmp(nodeType, "method_declaration") == 0 ||
		   strcmp(nodeType, "constructor_declaration") == 0;
}

bool CLanguageSupportJava::IsFieldDeclaration(TSNode node) const
{
	return strcmp(ts_node_type(node), "field_declaration") == 0;
}

bool CLanguageSupportJava::IsEnumConstant(TSNode node) const
{
	return strcmp(ts_node_type(node), "enum_constant") == 0;
}

bool CLanguageSupportJava::IsPackageDeclaration(TSNode node) const
{
	return strcmp(ts_node_type(node), "package_declaration") == 0;
}

bool CLanguageSupportJava::IsTypeContainer(TSNode node) const
{
	return IsClassDeclaration(node) ||
		   IsInterfaceDeclaration(node) ||
		   IsEnumDeclaration(node) ||
		   IsRecordDeclaration(node) ||
		   IsAnnotationTypeDeclaration(node);
}

void CLanguageSupportJava::GetNodeRange(TSNode node, int& startLine, int& endLine, const std::string& sourceCode) const
{
	TSPoint start = ts_node_start_point(node);
	TSPoint end = ts_node_end_point(node);

	startLine = start.row;  // TreeSitter 行号从 0 开始；与 lineLoc.line(0基) 保持一致
	endLine = end.row;
}

TreeSitterSymbol_End

