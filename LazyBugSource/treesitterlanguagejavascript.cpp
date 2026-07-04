#include "stdh.h"
#include "TreeSitterLanguageJavaScript.h"
#include "Utils.h"
#include "stringparser/stringparser.h"

// TreeSitter C接口
#include "treesitter_api.h"

// TreeSitter语言库声明
extern "C" {
	const TSLanguage* tree_sitter_javascript();
}

TreeSitterSymbol_Begin

//////////////////////////////////////////////////////////////////////////
// CLanguageSupportJavaScript

CLanguageSupportJavaScript::CLanguageSupportJavaScript()
{
	_extensions = { "js", "mjs" };
}

CLanguageSupportJavaScript::~CLanguageSupportJavaScript()
{
}

const TSLanguage* CLanguageSupportJavaScript::GetTSLanguage() const
{
	return tree_sitter_javascript();
}

bool CLanguageSupportJavaScript::IsSymbolDefinition(TSNode node) const
{
	if (IsFunctionDeclaration(node)) return true;
	if (IsGeneratorFunctionDeclaration(node)) return true;
	if (IsClassDeclaration(node)) return true;
	if (IsMethodDefinition(node)) return true;
	if (IsFieldDefinition(node)) return true;
	// arrow_function：仅当它不是 variable_declarator 的直接初始值时才单独算符号，
	// 否则交由 variable_declarator 统一命名，避免重复符号
	if (IsArrowFunction(node))
		return !_IsValueOfVariableDeclarator(node);
	if (IsImportStatement(node)) return true;
	// export_statement：仅当它本身是导出列表 / default 导出（不包裹具名声明）时才算符号，
	// 包裹声明的情况（export function/class/const...）由内部声明节点处理
	if (IsExportStatement(node))
		return _IsStandaloneExport(node);



	// variable_declarator 仅在顶层/类层级才是符号
	if (IsVariableDeclarator(node))
		return _IsTopLevelOrClassLevel(node);

	return false;
}

std::string CLanguageSupportJavaScript::GetNodeName(TSNode node, const std::string& sourceCode) const
{
	TSNode nameNode = _GetNameNode(node);
	if (ts_node_is_null(nameNode))
		return "";

	// 直接用字节偏移截取，避免逐行扫描带来的性能问题
	uint32_t startByte = ts_node_start_byte(nameNode);
	uint32_t endByte = ts_node_end_byte(nameNode);
	if (startByte <= endByte && endByte <= sourceCode.length())
		return sourceCode.substr(startByte, endByte - startByte);

	return "";
}

std::string CLanguageSupportJavaScript::GetNodeDisplayName(TSNode node, const std::string& sourceCode) const
{
	return GetNodeName(node, sourceCode);
}

bool CLanguageSupportJavaScript::GetNameNodeRange(TSNode node, TSPoint& startPoint, TSPoint& endPoint) const
{
	TSNode nameNode = _GetNameNode(node);
	if (ts_node_is_null(nameNode))
		return false;

	startPoint = ts_node_start_point(nameNode);
	endPoint = ts_node_end_point(nameNode);
	return true;
}

TSNode CLanguageSupportJavaScript::_GetNameNode(TSNode node) const
{
	TSNode nullNode = {};

	// 大多数 JS 声明都有 "name" 字段
	TSNode nameNode = ts_node_child_by_field_name(node, "name", 4);
	if (!ts_node_is_null(nameNode))
		return nameNode;

	// field_definition 使用 "property" 字段（property_identifier 或 private_property_identifier）
	if (IsFieldDefinition(node))
	{
		TSNode propNode = ts_node_child_by_field_name(node, "property", 8);
		if (!ts_node_is_null(propNode))
			return propNode;
		// 兜底：在子节点中查找 property_identifier / private_property_identifier
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

	// import_statement: 命名策略——明确优先取 default 导入名；
	// 若无 default，则依次回退到 namespace 导入名(* as x)、首个具名导入。
	// （单一名称范围 API 无法表达多名称，故不展开 import { a, b } 的全部成员）
	if (IsImportStatement(node))
	{
		uint32_t childCount = ts_node_child_count(node);
		for (uint32_t i = 0; i < childCount; i++)
		{
			TSNode child = ts_node_child(node, i);
			const char* childType = ts_node_type(child);

			// import x from '...' → identifier
			if (strcmp(childType, "identifier") == 0)
				return child;
			// import { x } from '...' → import_clause → named_imports → import_specifier
			if (strcmp(childType, "import_clause") == 0)
			{
				uint32_t clauseChildCount = ts_node_child_count(child);
				for (uint32_t j = 0; j < clauseChildCount; j++)
				{
					TSNode clauseChild = ts_node_child(child, j);
					const char* ccType = ts_node_type(clauseChild);

					if (strcmp(ccType, "identifier") == 0)
						return clauseChild;
					// namespace_import: * as name → identifier
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
					// named_imports: { foo, bar }
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

	// export_statement（独立导出）：export { a, b } / export default ...
	if (IsExportStatement(node))
	{
		uint32_t childCount = ts_node_child_count(node);
		for (uint32_t i = 0; i < childCount; i++)
		{
			TSNode child = ts_node_child(node, i);
			const char* childType = ts_node_type(child);

			// export { a, b, c } → export_clause → export_specifier
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
			// export default ... → 使用 "default" 关键字作为名称
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



const std::vector<std::string>& CLanguageSupportJavaScript::GetFileExtensions() const
{
	return _extensions;
}

SymbolKind CLanguageSupportJavaScript::GetSymbolKind(TSNode node) const
{
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
		// 字段的值若是函数（箭头函数 / 函数表达式），语义上视为方法
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
			// 在 class_body 或 field_definition 内 → Method
			const char* parentType = ts_node_type(parent);
			if (strcmp(parentType, "class_body") == 0 ||
				strcmp(parentType, "field_definition") == 0)
				return SymbolKind::Method;
		}
		return SymbolKind::JavaScript_ArrowFunction;
	}

	if (IsVariableDeclarator(node))
	{
		// 若变量的初始值是函数（箭头函数 / 函数表达式 / 生成器），语义上视为函数
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
			// 向上查找，判断是否在 class_body 内
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

LineRange CLanguageSupportJavaScript::GetNodeLineRange(TSNode node, const std::string& sourceCode) const
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

bool CLanguageSupportJavaScript::IsFunctionDeclaration(TSNode node) const
{
	const char* nodeType = ts_node_type(node);
	return strcmp(nodeType, "function_declaration") == 0;
}

bool CLanguageSupportJavaScript::IsGeneratorFunctionDeclaration(TSNode node) const
{
	const char* nodeType = ts_node_type(node);
	return strcmp(nodeType, "generator_function_declaration") == 0;
}

bool CLanguageSupportJavaScript::IsClassDeclaration(TSNode node) const
{
	const char* nodeType = ts_node_type(node);
	return strcmp(nodeType, "class_declaration") == 0;
}

bool CLanguageSupportJavaScript::IsMethodDefinition(TSNode node) const
{
	const char* nodeType = ts_node_type(node);
	return strcmp(nodeType, "method_definition") == 0;
}

bool CLanguageSupportJavaScript::IsFieldDefinition(TSNode node) const
{
	const char* nodeType = ts_node_type(node);
	// 不同 grammar 版本中类字段的节点名可能不同
	return strcmp(nodeType, "field_definition") == 0 ||
		   strcmp(nodeType, "public_field_definition") == 0;
}


bool CLanguageSupportJavaScript::IsArrowFunction(TSNode node) const
{
	const char* nodeType = ts_node_type(node);
	return strcmp(nodeType, "arrow_function") == 0;
}

bool CLanguageSupportJavaScript::IsVariableDeclarator(TSNode node) const
{
	const char* nodeType = ts_node_type(node);
	return strcmp(nodeType, "variable_declarator") == 0;
}

bool CLanguageSupportJavaScript::IsImportStatement(TSNode node) const
{
	const char* nodeType = ts_node_type(node);
	return strcmp(nodeType, "import_statement") == 0;
}

bool CLanguageSupportJavaScript::IsExportStatement(TSNode node) const
{
	const char* nodeType = ts_node_type(node);
	return strcmp(nodeType, "export_statement") == 0;
}

bool CLanguageSupportJavaScript::_IsStandaloneExport(TSNode node) const
{
	// 若包含 declaration 字段（export function/class/lexical_declaration...），
	// 则由内部声明节点产生符号，export_statement 本身不再重复
	TSNode decl = ts_node_child_by_field_name(node, "declaration", 11);
	if (!ts_node_is_null(decl))
		return false;
	return true;
}


bool CLanguageSupportJavaScript::_IsValueOfVariableDeclarator(TSNode node) const
{
	TSNode parent = ts_node_parent(node);
	if (ts_node_is_null(parent))
		return false;
	if (strcmp(ts_node_type(parent), "variable_declarator") != 0)
		return false;
	TSNode valueNode = ts_node_child_by_field_name(parent, "value", 5);
	return !ts_node_is_null(valueNode) && ts_node_eq(valueNode, node);
}


bool CLanguageSupportJavaScript::_IsTopLevelOrClassLevel(TSNode node) const
{
	TSNode cur = ts_node_parent(node);

	while (!ts_node_is_null(cur))
	{
		const char* curType = ts_node_type(cur);

		// 程序根节点 → 顶层
		if (strcmp(curType, "program") == 0)
			return true;

		// 类体 → 字段
		if (strcmp(curType, "class_body") == 0)
			return true;

		// 碰到任何函数、方法、箭头函数等 → 局部变量，不是符号
		if (strcmp(curType, "function_declaration") == 0 ||
			strcmp(curType, "generator_function_declaration") == 0 ||
			strcmp(curType, "method_definition") == 0 ||
			strcmp(curType, "arrow_function") == 0 ||
			strcmp(curType, "function_expression") == 0)
			return false;

		// 块语句 { ... }（statement_block）→ 块级作用域，其中的 let/const 为局部变量。
		// 即使该块直接挂在 program/class_body 下（顶层裸块），也应视为局部，不作为符号。
		if (strcmp(curType, "statement_block") == 0)
			return false;


		cur = ts_node_parent(cur);
	}

	return false;
}

void CLanguageSupportJavaScript::GetNodeRange(TSNode node, int& startLine, int& endLine, const std::string& sourceCode) const
{
	TSPoint start = ts_node_start_point(node);
	TSPoint end = ts_node_end_point(node);

	startLine = start.row;
	endLine = end.row;
}

TreeSitterSymbol_End
