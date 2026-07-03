#include "stdh.h"
#include "TreeSitterLanguageCSharp.h"
#include "Utils.h"
#include "stringparser/stringparser.h"

// TreeSitter C接口
#include "treesitter_api.h"

// TreeSitter语言库声明
extern "C" {
	const TSLanguage* tree_sitter_c_sharp();
}

TreeSitterSymbol_Begin

//////////////////////////////////////////////////////////////////////////
// CLanguageSupportCSharp

CLanguageSupportCSharp::CLanguageSupportCSharp()
{
	_extensions = { "cs" };
}

CLanguageSupportCSharp::~CLanguageSupportCSharp()
{
}

const TSLanguage* CLanguageSupportCSharp::GetTSLanguage() const
{
	return tree_sitter_c_sharp();
}


bool CLanguageSupportCSharp::IsSymbolDefinition(TSNode node) const
{
	if (IsClassDeclaration(node)) return true;
	if (IsStructDeclaration(node)) return true;
	if (IsInterfaceDeclaration(node)) return true;
	if (IsEnumDeclaration(node)) return true;
	if (IsMethodDeclaration(node)) return true;
	if (IsPropertyDeclaration(node)) return true;
	if (IsFieldDeclaration(node)) return true;
	if (IsNamespaceDeclaration(node)) return true;
	if (IsDelegateDeclaration(node)) return true;
	if (IsRecordDeclaration(node)) return true;
	if (IsEventDeclaration(node)) return true;

	return false;
}

std::string CLanguageSupportCSharp::GetNodeName(TSNode node, const std::string& sourceCode) const
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

std::string CLanguageSupportCSharp::GetNodeDisplayName(TSNode node, const std::string& sourceCode) const
{
	return GetNodeName(node, sourceCode);
}

bool CLanguageSupportCSharp::GetNameNodeRange(TSNode node, TSPoint& startPoint, TSPoint& endPoint) const
{
	// 尝试通过字段名 name 获取
	TSNode nameNode = ts_node_child_by_field_name(node, "name", 4);
	if (!ts_node_is_null(nameNode))
	{
		startPoint = ts_node_start_point(nameNode);
		endPoint = ts_node_end_point(nameNode);
		return true;
	}
	
	nameNode = ts_node_child_by_field_name(node, "declarator", 10);
	if (!ts_node_is_null(nameNode))
	{
		TSNode subName = ts_node_child_by_field_name(nameNode, "name", 4);
		if (!ts_node_is_null(subName))
		{
			startPoint = ts_node_start_point(subName);
			endPoint = ts_node_end_point(subName);
			return true;
		}
		startPoint = ts_node_start_point(nameNode);
		endPoint = ts_node_end_point(nameNode);
		return true;
	}

	// 首先，如果是包含 variable_declarator 的节点，先深入进去查找（避免把 type 当成 name）
	uint32_t childCount = ts_node_child_count(node);
	for (uint32_t i = 0; i < childCount; i++)
	{
		TSNode child = ts_node_child(node, i);
		const char* childType = ts_node_type(child);
		if (strcmp(childType, "variable_declaration") == 0 ||
			strcmp(childType, "variable_declarator") == 0 ||
			strcmp(childType, "event_declaration") == 0 ||
			strcmp(childType, "event_field_declaration") == 0)
		{
			if (GetNameNodeRange(child, startPoint, endPoint))
			{
				return true;
			}
		}
	}

	// 兜底遍历，寻找 identifier, type_identifier, name, qualified_name 等
	for (uint32_t i = 0; i < childCount; i++)
	{
		TSNode child = ts_node_child(node, i);
		const char* childType = ts_node_type(child);
		
		if (strcmp(childType, "identifier") == 0 ||
			strcmp(childType, "name") == 0 ||
			strcmp(childType, "type_identifier") == 0 ||
			strcmp(childType, "qualified_name") == 0)
		{
			// 如果这个节点是 type 字段，则跳过
			if (strcmp(childType, "qualified_name") == 0 || strcmp(childType, "type_identifier") == 0)
			{
				// 有些时候是 namespace，所以如果是 namespace declaration 的孩子，可能没有 type field
				// 所以我们只在它确实是我们想要的 name 的时候才返回。这里通过前面的优先遍历 declarations 已经可以避开大部分坑了
			}
			
			startPoint = ts_node_start_point(child);
			endPoint = ts_node_end_point(child);
			return true;
		}
	}
	
	return false;
}

const std::vector<std::string>& CLanguageSupportCSharp::GetFileExtensions() const
{
	return _extensions;
}

SymbolKind CLanguageSupportCSharp::GetSymbolKind(TSNode node) const
{
	if (IsClassDeclaration(node))
		return SymbolKind::Class;
	
	if (IsRecordDeclaration(node))
		return SymbolKind::CSharp_Record;

	if (IsStructDeclaration(node))
		return SymbolKind::Struct;
	
	if (IsInterfaceDeclaration(node))
		return SymbolKind::Interface;
	
	if (IsEnumDeclaration(node))
		return SymbolKind::Enum;
	
	if (IsMethodDeclaration(node))
	{
		const char* nodeType = ts_node_type(node);
		if (strcmp(nodeType, "constructor_declaration") == 0)
			return SymbolKind::Constructor;
		if (strcmp(nodeType, "destructor_declaration") == 0)
			return SymbolKind::Destructor;
		return SymbolKind::Method;
	}
	
	if (IsPropertyDeclaration(node))
		return SymbolKind::Property;
		
	if (IsEventDeclaration(node))
		return SymbolKind::CSharp_Event;

	if (IsFieldDeclaration(node))
		return SymbolKind::Field;
	
	if (IsNamespaceDeclaration(node))
		return SymbolKind::Namespace;
		
	if (IsDelegateDeclaration(node))
		return SymbolKind::CSharp_Delegate;

	return SymbolKind::Invalid;
}

LineRange CLanguageSupportCSharp::GetNodeLineRange(TSNode node, const std::string& sourceCode) const
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

bool CLanguageSupportCSharp::IsClassDeclaration(TSNode node) const
{
	const char* nodeType = ts_node_type(node);
	return strcmp(nodeType, "class_declaration") == 0;
}

bool CLanguageSupportCSharp::IsStructDeclaration(TSNode node) const
{
	const char* nodeType = ts_node_type(node);
	return strcmp(nodeType, "struct_declaration") == 0 || strcmp(nodeType, "record_struct_declaration") == 0;
}

bool CLanguageSupportCSharp::IsInterfaceDeclaration(TSNode node) const
{
	const char* nodeType = ts_node_type(node);
	return strcmp(nodeType, "interface_declaration") == 0;
}

bool CLanguageSupportCSharp::IsEnumDeclaration(TSNode node) const
{
	const char* nodeType = ts_node_type(node);
	return strcmp(nodeType, "enum_declaration") == 0;
}

bool CLanguageSupportCSharp::IsMethodDeclaration(TSNode node) const
{
	const char* nodeType = ts_node_type(node);
	return strcmp(nodeType, "method_declaration") == 0 ||
		   strcmp(nodeType, "constructor_declaration") == 0 ||
		   strcmp(nodeType, "destructor_declaration") == 0 ||
		   strcmp(nodeType, "local_function_statement") == 0;
}

bool CLanguageSupportCSharp::IsPropertyDeclaration(TSNode node) const
{
	const char* nodeType = ts_node_type(node);
	return strcmp(nodeType, "property_declaration") == 0 ||
		   strcmp(nodeType, "indexer_declaration") == 0;
}

bool CLanguageSupportCSharp::IsFieldDeclaration(TSNode node) const
{
	const char* nodeType = ts_node_type(node);
	return strcmp(nodeType, "field_declaration") == 0;
}

bool CLanguageSupportCSharp::IsNamespaceDeclaration(TSNode node) const
{
	const char* nodeType = ts_node_type(node);
	return strcmp(nodeType, "namespace_declaration") == 0 ||
		   strcmp(nodeType, "file_scoped_namespace_declaration") == 0;
}

bool CLanguageSupportCSharp::IsDelegateDeclaration(TSNode node) const
{
	const char* nodeType = ts_node_type(node);
	return strcmp(nodeType, "delegate_declaration") == 0;
}

bool CLanguageSupportCSharp::IsRecordDeclaration(TSNode node) const
{
	const char* nodeType = ts_node_type(node);
	return strcmp(nodeType, "record_declaration") == 0;
}

bool CLanguageSupportCSharp::IsEventDeclaration(TSNode node) const
{
	const char* nodeType = ts_node_type(node);
	return strcmp(nodeType, "event_declaration") == 0 ||
		   strcmp(nodeType, "event_field_declaration") == 0;
}

void CLanguageSupportCSharp::GetNodeRange(TSNode node, int& startLine, int& endLine, const std::string& sourceCode) const
{
	TSPoint start = ts_node_start_point(node);
	TSPoint end = ts_node_end_point(node);
	
	startLine = start.row;  // TreeSitter 行号从 0 开始；与 lineLoc.line(0基) 保持一致
	endLine = end.row;
}

TreeSitterSymbol_End

