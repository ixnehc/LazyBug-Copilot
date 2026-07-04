#include "stdh.h"
#include "TreeSitterLanguageCpp.h"
#include "Utils.h"
#include "stringparser/stringparser.h"

// TreeSitter C接口
#include "treesitter_api.h"

// TreeSitter语言库声明
extern "C" {
	const TSLanguage* tree_sitter_cpp();
	// 注意：tree_sitter_c() 可能未链接，我们使用tree_sitter_cpp()来处理C文件
	// const TSLanguage* tree_sitter_c();
}

TreeSitterSymbol_Begin

//////////////////////////////////////////////////////////////////////////
// CLanguageSupportCpp

CLanguageSupportCpp::CLanguageSupportCpp()
{
	// 初始化文件扩展名列表
	_extensions = { "cpp", "cc", "cxx", "hpp", "hh", "hxx", "h", "c" }; // 添加了.c扩展名
}

CLanguageSupportCpp::~CLanguageSupportCpp()
{
}

const TSLanguage* CLanguageSupportCpp::GetTSLanguage() const
{
	return tree_sitter_cpp();
}

bool CLanguageSupportCpp::IsSymbolDefinition(TSNode node) const
{
	const char* nodeType = ts_node_type(node);
	
	// 检查是否是类定义
	if (IsClassSpecifier(node))
		return true;
	
	// 检查是否是结构体定义
	if (IsStructSpecifier(node))
		return true;
	
	// 检查是否是枚举定义
	if (IsEnumSpecifier(node))
		return true;
	
	// 检查是否是函数定义
	if (IsFunctionDefinition(node))
		return true;
	
	// 检查是否是命名空间定义
	if (IsNamespaceDefinition(node))
		return true;
	
	// 检查是否是类声明
	if (IsClassDeclaration(node))
		return true;
	
	// 检查是否是模板声明
	if (IsTemplateDeclaration(node))
		return true;
	
	// 检查是否是变量声明
	if (IsDeclaration(node))
	{
		// 获取父节点，确保不是在函数参数或其他上下文中
		TSNode parent = ts_node_parent(node);
		if (!ts_node_is_null(parent))
		{
			const char* parentType = ts_node_type(parent);
			// 如果父节点是函数定义或参数列表，则不是全局变量
			if (strcmp(parentType, "function_definition") == 0 ||
				strcmp(parentType, "parameter_list") == 0 ||
				strcmp(parentType, "lambda_expression") == 0)
				return false;
		}
		return true;
	}
	
	return false;
}

std::string CLanguageSupportCpp::GetNodeName(TSNode node, const std::string& sourceCode) const
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

std::string CLanguageSupportCpp::GetNodeDisplayName(TSNode node, const std::string& sourceCode) const
{
	// 基本实现，返回名称即可
	// 后续可以添加类型信息等
	return GetNodeName(node, sourceCode);
}

bool CLanguageSupportCpp::GetNameNodeRange(TSNode node, TSPoint& startPoint, TSPoint& endPoint) const
{
	// 尝试通过字段名 declarator 获取
	TSNode nameNode = ts_node_child_by_field_name(node, "declarator", 10);
	if (!ts_node_is_null(nameNode))
	{
		// C++ 中的 declarator 可能是 function_declarator, pointer_declarator, reference_declarator 等
		// 内部通常还有 declarator (如果嵌套)，或者是 identifier
		TSNode subName = ts_node_child_by_field_name(nameNode, "declarator", 10);
		if (!ts_node_is_null(subName))
		{
			startPoint = ts_node_start_point(subName);
			endPoint = ts_node_end_point(subName);
			return true;
		}

		uint32_t declaratorChildCount = ts_node_child_count(nameNode);
		for (uint32_t j = 0; j < declaratorChildCount; j++)
		{
			TSNode declaratorChild = ts_node_child(nameNode, j);
			const char* declaratorChildType = ts_node_type(declaratorChild);
			
			if (strcmp(declaratorChildType, "identifier") == 0 ||
				strcmp(declaratorChildType, "qualified_identifier") == 0 ||
				strcmp(declaratorChildType, "destructor_name") == 0 ||
				strcmp(declaratorChildType, "operator_name") == 0)
			{
				startPoint = ts_node_start_point(declaratorChild);
				endPoint = ts_node_end_point(declaratorChild);
				return true;
			}
		}

		startPoint = ts_node_start_point(nameNode);
		endPoint = ts_node_end_point(nameNode);
		return true;
	}

	// 尝试通过字段名 name 获取
	nameNode = ts_node_child_by_field_name(node, "name", 4);
	if (!ts_node_is_null(nameNode))
	{
		startPoint = ts_node_start_point(nameNode);
		endPoint = ts_node_end_point(nameNode);
		return true;
	}

	uint32_t childCount = ts_node_child_count(node);

	// 如果上面没找到，优先深入到各种 declarator 节点，内部才有真实的 name
	for (uint32_t i = 0; i < childCount; i++)
	{
		TSNode child = ts_node_child(node, i);
		const char* childType = ts_node_type(child);
		if (strcmp(childType, "function_declarator") == 0 ||
			strcmp(childType, "declarator") == 0 ||
			strcmp(childType, "init_declarator") == 0 ||
			strcmp(childType, "struct_specifier") == 0 ||
			strcmp(childType, "class_specifier") == 0 ||
			strcmp(childType, "enum_specifier") == 0 ||
			strcmp(childType, "pointer_declarator") == 0 ||
			strcmp(childType, "reference_declarator") == 0 ||
			strcmp(childType, "array_declarator") == 0)
		{
			if (GetNameNodeRange(child, startPoint, endPoint))
			{
				return true;
			}
		}
	}

	// 兜底遍历，寻找类型标识符、标识符等（排在后面，以防止把 type 抓错当成 name）
	for (uint32_t i = 0; i < childCount; i++)
	{
		TSNode child = ts_node_child(node, i);
		const char* childType = ts_node_type(child);
		
		if (strcmp(childType, "identifier") == 0 ||
			strcmp(childType, "type_identifier") == 0 ||
			strcmp(childType, "qualified_identifier") == 0 ||
			strcmp(childType, "namespace_identifier") == 0 ||
			strcmp(childType, "destructor_name") == 0 ||
			strcmp(childType, "operator_name") == 0)
		{
			startPoint = ts_node_start_point(child);
			endPoint = ts_node_end_point(child);
			return true;
		}
	}
	
	return false;
}

const std::vector<std::string>& CLanguageSupportCpp::GetFileExtensions() const
{
	return _extensions;
}

SymbolKind CLanguageSupportCpp::GetSymbolKind(TSNode node) const
{
	if (IsClassSpecifier(node))
		return SymbolKind::Class;
	
	if (IsStructSpecifier(node))
		return SymbolKind::Struct;
	
	if (IsEnumSpecifier(node))
		return SymbolKind::Enum;
	
	if (IsFunctionDefinition(node))
	{
		// 判断是否是成员函数
		TSNode parent = ts_node_parent(node);
		if (!ts_node_is_null(parent))
		{
			const char* parentType = ts_node_type(parent);
			if (strcmp(parentType, "class_specifier") == 0 ||
				strcmp(parentType, "struct_specifier") == 0)
				return SymbolKind::Method;
		}
		return SymbolKind::Function;
	}
	
	if (IsNamespaceDefinition(node))
		return SymbolKind::Namespace;
	
	if (IsDeclaration(node))
		return SymbolKind::Variable;
	
	if (IsClassDeclaration(node))
		return SymbolKind::Class;
	
	if (IsTemplateDeclaration(node))
		return SymbolKind::Template;
	
	return SymbolKind::Invalid;
}

LineRange CLanguageSupportCpp::GetNodeLineRange(TSNode node, const std::string& sourceCode) const
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

bool CLanguageSupportCpp::IsClassSpecifier(TSNode node) const
{
	const char* nodeType = ts_node_type(node);
	return strcmp(nodeType, "class_specifier") == 0;
}

bool CLanguageSupportCpp::IsStructSpecifier(TSNode node) const
{
	const char* nodeType = ts_node_type(node);
	return strcmp(nodeType, "struct_specifier") == 0;
}

bool CLanguageSupportCpp::IsEnumSpecifier(TSNode node) const
{
	const char* nodeType = ts_node_type(node);
	return strcmp(nodeType, "enum_specifier") == 0;
}

bool CLanguageSupportCpp::IsFunctionDefinition(TSNode node) const
{
	const char* nodeType = ts_node_type(node);
	return strcmp(nodeType, "function_definition") == 0;
}

bool CLanguageSupportCpp::IsDeclaration(TSNode node) const
{
	const char* nodeType = ts_node_type(node);
	
	// 声明类型
	if (strcmp(nodeType, "declaration") == 0)
		return true;
	
	// 字段声明
	if (strcmp(nodeType, "field_declaration") == 0)
		return true;
	
	return false;
}

bool CLanguageSupportCpp::IsNamespaceDefinition(TSNode node) const
{
	const char* nodeType = ts_node_type(node);
	return strcmp(nodeType, "namespace_definition") == 0;
}

bool CLanguageSupportCpp::IsTemplateDeclaration(TSNode node) const
{
	const char* nodeType = ts_node_type(node);
	return strcmp(nodeType, "template_declaration") == 0;
}

bool CLanguageSupportCpp::IsClassDeclaration(TSNode node) const
{
	// 在C++中，类声明通常在class_specifier中
	// 这里可以根据需要进一步细化
	return false;
}


void CLanguageSupportCpp::GetNodeRange(TSNode node, int& startLine, int& endLine, const std::string& sourceCode) const
{
	TSPoint start = ts_node_start_point(node);
	TSPoint end = ts_node_end_point(node);
	
	startLine = start.row;  // TreeSitter的行号从0开始
	endLine = end.row;
}


TreeSitterSymbol_End
