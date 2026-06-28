#include "stdh.h"
#include "TreeSitterLanguageTypeScript.h"
#include "Utils.h"
#include "stringparser/stringparser.h"

// TreeSitter C接口
#include "treesitter_api.h"

// TreeSitter语言库声明
extern "C" {
	const TSLanguage* tree_sitter_typescript();
}

TreeSitterSymbol_Begin

//////////////////////////////////////////////////////////////////////////
// CLanguageSupportTypeScript

CLanguageSupportTypeScript::CLanguageSupportTypeScript()
{
	_extensions = { "ts", "tsx" };
}

CLanguageSupportTypeScript::~CLanguageSupportTypeScript()
{
}

const TSLanguage* CLanguageSupportTypeScript::GetTSLanguage() const
{
	return tree_sitter_typescript();
}

bool CLanguageSupportTypeScript::IsSymbolDefinition(TSNode node) const
{
	// === JS/TS 共有 ===
	if (IsFunctionDeclaration(node)) return true;
	if (IsGeneratorFunctionDeclaration(node)) return true;
	if (IsClassDeclaration(node)) return true;
	if (IsMethodDefinition(node)) return true;
	if (IsFieldDefinition(node)) return true;
	if (IsArrowFunction(node))
		return !_IsValueOfVariableDeclarator(node);
	if (IsImportStatement(node)) return true;
	if (IsExportStatement(node))
		return _IsStandaloneExport(node);
	if (IsVariableDeclarator(node))
		return _IsTopLevelOrClassLevel(node);

	// === TS 专属 ===
	if (IsInterfaceDeclaration(node)) return true;
	if (IsTypeAliasDeclaration(node)) return true;
	if (IsEnumDeclaration(node)) return true;
	if (IsAbstractClassDeclaration(node)) return true;
	if (IsModuleDeclaration(node)) return true;
	if (IsAbstractMethodDefinition(node)) return true;

	return false;
}

std::string CLanguageSupportTypeScript::GetNodeName(TSNode node, const std::string& sourceCode) const
{
	TSNode nameNode = _GetNameNode(node);
	if (ts_node_is_null(nameNode))
		return "";

	uint32_t startByte = ts_node_start_byte(nameNode);
	uint32_t endByte = ts_node_end_byte(nameNode);
	if (startByte <= endByte && endByte <= sourceCode.length())
		return sourceCode.substr(startByte, endByte - startByte);

	return "";
}

std::string CLanguageSupportTypeScript::GetNodeDisplayName(TSNode node, const std::string& sourceCode) const
{
	return GetNodeName(node, sourceCode);
}

bool CLanguageSupportTypeScript::GetNameNodeRange(TSNode node, TSPoint& startPoint, TSPoint& endPoint) const
{
	TSNode nameNode = _GetNameNode(node);
	if (ts_node_is_null(nameNode))
		return false;

	startPoint = ts_node_start_point(nameNode);
	endPoint = ts_node_end_point(nameNode);
	return true;
}

TSNode CLanguageSupportTypeScript::_GetNameNode(TSNode node) const
{
	TSNode nullNode = {};

	// 大多数声明都有 "name" 字段
	TSNode nameNode = ts_node_child_by_field_name(node, "name", 4);
	if (!ts_node_is_null(nameNode))
		return nameNode;

	// field_definition 使用 "property" 字段
	if (IsFieldDefinition(node))
	{
		TSNode propNode = ts_node_child_by_field_name(node, "property", 8);
		if (!ts_node_is_null(propNode))
			return propNode;
		uint32_t fieldChildCount = ts_node_child_count(node);
		for (uint32_t i = 0; i < fieldChildCount; i++)
		{
			TSNode child = ts_node_child(node, i);
			const char* childType = ts_node_type(child);
			if (strcmp(childType, "property_identifier") == 0 ||
				strcmp(childType, "private_property_identifier") == 0)
				return child;
		}
		return nullNode;
	}

	// arrow_function 没有 name 字段，从父节点 variable_declarator 获取
	if (IsArrowFunction(node))
	{
		TSNode parent = ts_node_parent(node);
		if (!ts_node_is_null(parent))
		{
			const char* parentType = ts_node_type(parent);
			if (strcmp(parentType, "variable_declarator") == 0)
			{
				TSNode parentName = ts_node_child_by_field_name(parent, "name", 4);
				if (!ts_node_is_null(parentName))
					return parentName;
			}
		}
		return nullNode;
	}

	// import_statement
	if (IsImportStatement(node))
	{
		uint32_t childCount = ts_node_child_count(node);
		for (uint32_t i = 0; i < childCount; i++)
		{
			TSNode child = ts_node_child(node, i);
			const char* childType = ts_node_type(child);

			if (strcmp(childType, "identifier") == 0)
				return child;
			if (strcmp(childType, "import_clause") == 0)
			{
				uint32_t clauseChildCount = ts_node_child_count(child);
				for (uint32_t j = 0; j < clauseChildCount; j++)
				{
					TSNode clauseChild = ts_node_child(child, j);
					const char* ccType = ts_node_type(clauseChild);

					if (strcmp(ccType, "identifier") == 0)
						return clauseChild;
					if (strcmp(ccType, "namespace_import") == 0)
					{
						TSNode nsName = ts_node_child_by_field_name(clauseChild, "name", 4);
						if (ts_node_is_null(nsName))
						{
							uint32_t nsChildCount = ts_node_child_count(clauseChild);
							for (uint32_t k = 0; k < nsChildCount; k++)
							{
								TSNode nsChild = ts_node_child(clauseChild, k);
								if (strcmp(ts_node_type(nsChild), "identifier") == 0)
								{
									nsName = nsChild;
									break;
								}
							}
						}
						if (!ts_node_is_null(nsName))
							return nsName;
					}
					if (strcmp(ccType, "named_imports") == 0)
					{
						uint32_t namedChildCount = ts_node_child_count(clauseChild);
						for (uint32_t k = 0; k < namedChildCount; k++)
						{
							TSNode namedChild = ts_node_child(clauseChild, k);
							if (strcmp(ts_node_type(namedChild), "import_specifier") == 0)
							{
								TSNode specName = ts_node_child_by_field_name(namedChild, "name", 4);
								if (!ts_node_is_null(specName))
									return specName;
							}
						}
					}
				}
			}
		}
		return nullNode;
	}

	// export_statement（独立导出）
	if (IsExportStatement(node))
	{
		uint32_t childCount = ts_node_child_count(node);
		for (uint32_t i = 0; i < childCount; i++)
		{
			TSNode child = ts_node_child(node, i);
			const char* childType = ts_node_type(child);

			if (strcmp(childType, "export_clause") == 0)
			{
				uint32_t clauseChildCount = ts_node_child_count(child);
				for (uint32_t j = 0; j < clauseChildCount; j++)
				{
					TSNode spec = ts_node_child(child, j);
					if (strcmp(ts_node_type(spec), "export_specifier") == 0)
					{
						TSNode specName = ts_node_child_by_field_name(spec, "name", 4);
						if (ts_node_is_null(specName))
							specName = spec;
						return specName;
					}
				}
			}
			if (strcmp(childType, "default") == 0)
				return child;
		}
		return nullNode;
	}

	// 兜底：在子节点中寻找 identifier 或 property_identifier
	uint32_t childCount = ts_node_child_count(node);
	for (uint32_t i = 0; i < childCount; i++)
	{
		TSNode child = ts_node_child(node, i);
		const char* childType = ts_node_type(child);

		if (strcmp(childType, "identifier") == 0 ||
			strcmp(childType, "property_identifier") == 0)
			return child;
	}

	return nullNode;
}

const std::vector<std::string>& CLanguageSupportTypeScript::GetFileExtensions() const
{
	return _extensions;
}

SymbolKind CLanguageSupportTypeScript::GetSymbolKind(TSNode node) const
{
	// === TS 专属（优先判定，避免被 JS 共有逻辑截获） ===
	if (IsInterfaceDeclaration(node))
		return SymbolKind::Interface;

	if (IsTypeAliasDeclaration(node))
		return SymbolKind::TypeAlias;

	if (IsEnumDeclaration(node))
		return SymbolKind::Enum;

	if (IsAbstractClassDeclaration(node))
		return SymbolKind::Class;

	if (IsModuleDeclaration(node))
		return SymbolKind::Namespace;

	if (IsAbstractMethodDefinition(node))
		return SymbolKind::Method;

	// === JS/TS 共有 ===
	if (IsFunctionDeclaration(node))
		return SymbolKind::Function;

	if (IsGeneratorFunctionDeclaration(node))
		return SymbolKind::JavaScript_GeneratorFunction;

	if (IsClassDeclaration(node))
		return SymbolKind::Class;

	if (IsMethodDefinition(node))
		return SymbolKind::Method;

	if (IsFieldDefinition(node))
	{
		TSNode valueNode = ts_node_child_by_field_name(node, "value", 5);
		if (!ts_node_is_null(valueNode))
		{
			const char* valueType = ts_node_type(valueNode);
			if (strcmp(valueType, "arrow_function") == 0 ||
				strcmp(valueType, "function_expression") == 0 ||
				strcmp(valueType, "function") == 0 ||
				strcmp(valueType, "generator_function") == 0)
				return SymbolKind::Method;
		}
		return SymbolKind::Field;
	}

	if (IsArrowFunction(node))
	{
		TSNode parent = ts_node_parent(node);
		if (!ts_node_is_null(parent))
		{
			const char* parentType = ts_node_type(parent);
			if (strcmp(parentType, "class_body") == 0 ||
				strcmp(parentType, "field_definition") == 0)
				return SymbolKind::Method;
		}
		return SymbolKind::JavaScript_ArrowFunction;
	}

	if (IsVariableDeclarator(node))
	{
		TSNode valueNode = ts_node_child_by_field_name(node, "value", 5);
		if (!ts_node_is_null(valueNode))
		{
			const char* valueType = ts_node_type(valueNode);
			if (strcmp(valueType, "arrow_function") == 0)
				return SymbolKind::JavaScript_ArrowFunction;
			if (strcmp(valueType, "generator_function") == 0)
				return SymbolKind::JavaScript_GeneratorFunction;
			if (strcmp(valueType, "function_expression") == 0 ||
				strcmp(valueType, "function") == 0)
				return SymbolKind::Function;
		}

		TSNode parent = ts_node_parent(node);
		if (!ts_node_is_null(parent))
		{
			TSNode cur = parent;
			while (!ts_node_is_null(cur))
			{
				const char* curType = ts_node_type(cur);
				if (strcmp(curType, "class_body") == 0)
					return SymbolKind::Field;
				if (strcmp(curType, "program") == 0)
					break;
				cur = ts_node_parent(cur);
			}
		}
		return SymbolKind::Variable;
	}

	if (IsImportStatement(node))
		return SymbolKind::Import;

	if (IsExportStatement(node))
		return SymbolKind::JavaScript_Export;

	return SymbolKind::Invalid;
}

LineRange CLanguageSupportTypeScript::GetNodeLineRange(TSNode node, const std::string& sourceCode) const
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

// === JS/TS 共有 ===

bool CLanguageSupportTypeScript::IsFunctionDeclaration(TSNode node) const
{
	return strcmp(ts_node_type(node), "function_declaration") == 0;
}

bool CLanguageSupportTypeScript::IsGeneratorFunctionDeclaration(TSNode node) const
{
	return strcmp(ts_node_type(node), "generator_function_declaration") == 0;
}

bool CLanguageSupportTypeScript::IsClassDeclaration(TSNode node) const
{
	return strcmp(ts_node_type(node), "class_declaration") == 0;
}

bool CLanguageSupportTypeScript::IsMethodDefinition(TSNode node) const
{
	return strcmp(ts_node_type(node), "method_definition") == 0;
}

bool CLanguageSupportTypeScript::IsFieldDefinition(TSNode node) const
{
	const char* nodeType = ts_node_type(node);
	return strcmp(nodeType, "field_definition") == 0 ||
		   strcmp(nodeType, "public_field_definition") == 0;
}

bool CLanguageSupportTypeScript::IsArrowFunction(TSNode node) const
{
	return strcmp(ts_node_type(node), "arrow_function") == 0;
}

bool CLanguageSupportTypeScript::IsVariableDeclarator(TSNode node) const
{
	return strcmp(ts_node_type(node), "variable_declarator") == 0;
}

bool CLanguageSupportTypeScript::IsImportStatement(TSNode node) const
{
	return strcmp(ts_node_type(node), "import_statement") == 0;
}

bool CLanguageSupportTypeScript::IsExportStatement(TSNode node) const
{
	return strcmp(ts_node_type(node), "export_statement") == 0;
}

// === TS 专属 ===

bool CLanguageSupportTypeScript::IsInterfaceDeclaration(TSNode node) const
{
	return strcmp(ts_node_type(node), "interface_declaration") == 0;
}

bool CLanguageSupportTypeScript::IsTypeAliasDeclaration(TSNode node) const
{
	return strcmp(ts_node_type(node), "type_alias_declaration") == 0;
}

bool CLanguageSupportTypeScript::IsEnumDeclaration(TSNode node) const
{
	return strcmp(ts_node_type(node), "enum_declaration") == 0;
}

bool CLanguageSupportTypeScript::IsAbstractClassDeclaration(TSNode node) const
{
	return strcmp(ts_node_type(node), "abstract_class_declaration") == 0;
}

bool CLanguageSupportTypeScript::IsModuleDeclaration(TSNode node) const
{
	const char* nodeType = ts_node_type(node);
	return strcmp(nodeType, "module_declaration") == 0 ||
		   strcmp(nodeType, "internal_module") == 0;
}

bool CLanguageSupportTypeScript::IsAbstractMethodDefinition(TSNode node) const
{
	return strcmp(ts_node_type(node), "abstract_method_definition") == 0;
}

// === 内部辅助 ===

bool CLanguageSupportTypeScript::_IsStandaloneExport(TSNode node) const
{
	TSNode decl = ts_node_child_by_field_name(node, "declaration", 11);
	if (!ts_node_is_null(decl))
		return false;
	return true;
}

bool CLanguageSupportTypeScript::_IsValueOfVariableDeclarator(TSNode node) const
{
	TSNode parent = ts_node_parent(node);
	if (ts_node_is_null(parent))
		return false;
	if (strcmp(ts_node_type(parent), "variable_declarator") != 0)
		return false;
	TSNode valueNode = ts_node_child_by_field_name(parent, "value", 5);
	return !ts_node_is_null(valueNode) && ts_node_eq(valueNode, node);
}

bool CLanguageSupportTypeScript::_IsTopLevelOrClassLevel(TSNode node) const
{
	TSNode cur = ts_node_parent(node);

	while (!ts_node_is_null(cur))
	{
		const char* curType = ts_node_type(cur);

		if (strcmp(curType, "program") == 0)
			return true;

		if (strcmp(curType, "class_body") == 0)
			return true;

		if (strcmp(curType, "function_declaration") == 0 ||
			strcmp(curType, "generator_function_declaration") == 0 ||
			strcmp(curType, "method_definition") == 0 ||
			strcmp(curType, "abstract_method_definition") == 0 ||
			strcmp(curType, "arrow_function") == 0 ||
			strcmp(curType, "function_expression") == 0)
			return false;

		if (strcmp(curType, "statement_block") == 0)
			return false;

		cur = ts_node_parent(cur);
	}

	return false;
}

void CLanguageSupportTypeScript::GetNodeRange(TSNode node, int& startLine, int& endLine, const std::string& sourceCode) const
{
	TSPoint start = ts_node_start_point(node);
	TSPoint end = ts_node_end_point(node);

	startLine = start.row;
	endLine = end.row;
}

TreeSitterSymbol_End
