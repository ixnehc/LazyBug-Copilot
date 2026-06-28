#include "stdh.h"
#include "TreeSitterLanguage.h"
#include "Utils.h"
#include "stringparser/stringparser.h"
#include <algorithm>

TreeSitterSymbol_Begin

//////////////////////////////////////////////////////////////////////////
// CLanguageFactory

CLanguageFactory& CLanguageFactory::Instance()
{
	static CLanguageFactory instance;
	return instance;
}

void CLanguageFactory::RegisterLanguage(std::shared_ptr<ILanguageSupport> support)
{
	if (!support)
		return;
	
	Language lang = support->GetLanguage();
	_languages[lang] = support;
}

std::shared_ptr<ILanguageSupport> CLanguageFactory::GetLanguageSupport(Language lang)
{
	auto it = _languages.find(lang);
	if (it != _languages.end())
		return it->second;
	return nullptr;
}

Language CLanguageFactory::DetectLanguage(const std::string& filePath)
{
	return GetLanguageFromFilePath(filePath);
}

//////////////////////////////////////////////////////////////////////////
// 辅助函数

Language GetLanguageFromExtension(const std::string& extension)
{
	std::string ext = extension;
	StringLower(ext);
	
	// C/C++
// 	if (ext == "c")
// 		return Language::C;
// 	if (ext == "cpp" || ext == "cc" || ext == "cxx" || ext == "hpp" || ext == "h" || ext == "hh" || ext == "hxx")
// 		return Language::Cpp;
	
	// C#
	if (ext == "cs")
		return Language::CSharp;
	
	// CSS
	if (ext == "css")
		return Language::Css;

	// HTML
	if (ext == "html" || ext == "htm")
		return Language::Html;
	
	// Java
	if (ext == "java")
		return Language::Java;
	
	// JavaScript
	if (ext == "js" || ext == "mjs")
		return Language::JavaScript;
	
	// TypeScript
	if (ext == "ts" || ext == "tsx")
		return Language::TypeScript;
	
	// Python
	if (ext == "py" || ext == "pyw")
		return Language::Python;
// 	
// 	// Go
// 	if (ext == "go")
// 		return Language::Go;
// 	
// 	// Rust
// 	if (ext == "rs")
// 		return Language::Rust;
// 	
// 	// Swift
// 	if (ext == "swift")
// 		return Language::Swift;
// 	
// 	// Kotlin
// 	if (ext == "kt" || ext == "kts")
// 		return Language::Kotlin;
	
	return Language::Unknown;
}

Language GetLanguageFromFilePath(const std::string& filePath)
{
	// 获取文件扩展名
	size_t dotPos = filePath.rfind('.');
	if (dotPos == std::string::npos)
		return Language::Unknown;
	
	std::string ext = filePath.substr(dotPos + 1);
	return GetLanguageFromExtension(ext);
}

//////////////////////////////////////////////////////////////////////////
// ILanguageSupport 默认实现

void ILanguageSupport::CollectSymbols(
	TSNode node,
	const std::string& sourceCode,
	std::vector<RawSymbolDefine>& outSymbols,
	std::string& currentPrefix) const
{
	if (!IsSymbolDefinition(node))
		return;

	RawSymbolDefine symbol;
	std::string nodeName = GetNodeName(node, sourceCode);

	if (!currentPrefix.empty() && !nodeName.empty()) {
		symbol.name = currentPrefix + "." + nodeName;
	} else {
		symbol.name = nodeName;
	}

	if (!nodeName.empty()) {
		currentPrefix = symbol.name;
	}

	symbol.showName = GetNodeDisplayName(node, sourceCode);
	symbol.kind = GetSymbolKind(node);
	symbol.language = GetLanguage();
	symbol.lineRange = GetNodeLineRange(node, sourceCode);

	// 设置位置
	TSPoint nameStart, nameEnd;
	if (GetNameNodeRange(node, nameStart, nameEnd))
	{
		symbol.lineLoc.line = nameStart.row;
		symbol.lineLoc.startColumn = nameStart.column;
		symbol.lineLoc.endColumn = nameEnd.column;
	}
	else
	{
		TSPoint startPoint = ts_node_start_point(node);
		symbol.lineLoc.line = startPoint.row;
		symbol.lineLoc.startColumn = startPoint.column;
		symbol.lineLoc.endColumn = startPoint.column + nodeName.length();
	}

	if (symbol.lineRange.start > symbol.lineLoc.line)
		symbol.lineRange.start = symbol.lineLoc.line;

	if (!symbol.name.empty())
	{
		outSymbols.push_back(symbol);
	}
}

TreeSitterSymbol_End
