#pragma once

#include "TreeSitterLanguage.h"
#include <memory>

#define TreeSitterSymbol_Begin namespace TreeSitterSymbol{
#define TreeSitterSymbol_End }


namespace TreeSitterSymbol
{
	// JavaScript 语言支持
	class CLanguageSupportJavaScript : public ILanguageSupport
	{
	public:
		CLanguageSupportJavaScript();
		virtual ~CLanguageSupportJavaScript();

		// ILanguageSupport 接口实现
		virtual Language GetLanguage() const override { return Language::JavaScript; }

		virtual const TSLanguage* GetTSLanguage() const override;

		virtual bool IsSymbolDefinition(TSNode node) const override;

		virtual std::string GetNodeName(TSNode node, const std::string& sourceCode) const override;

		virtual std::string GetNodeDisplayName(TSNode node, const std::string& sourceCode) const override;

		virtual bool GetNameNodeRange(TSNode node, TSPoint& startPoint, TSPoint& endPoint) const override;



		virtual const std::vector<std::string>& GetFileExtensions() const override;

		virtual SymbolKind GetSymbolKind(TSNode node) const override;

		virtual LineRange GetNodeLineRange(TSNode node, const std::string& sourceCode) const override;

	private:
		// 判断节点类型
		bool IsFunctionDeclaration(TSNode node) const;
		bool IsGeneratorFunctionDeclaration(TSNode node) const;
		bool IsClassDeclaration(TSNode node) const;
		bool IsMethodDefinition(TSNode node) const;
		bool IsFieldDefinition(TSNode node) const;
		bool IsArrowFunction(TSNode node) const;
		bool IsVariableDeclarator(TSNode node) const;
		bool IsImportStatement(TSNode node) const;
		bool IsExportStatement(TSNode node) const;

		// 返回节点用于命名的"名称节点"（找不到返回 null 节点）
		TSNode _GetNameNode(TSNode node) const;



		// 判断 export_statement 是否为"独立导出"（export {..} / export default），
		// 而非包裹具名声明的导出（export function/class/const...）
		bool _IsStandaloneExport(TSNode node) const;



		// 判断函数节点是否为某个 variable_declarator 的初始值（value 字段）
		bool _IsValueOfVariableDeclarator(TSNode node) const;

		// 判断变量声明是否在顶层或类层级（非函数内部）
		bool _IsTopLevelOrClassLevel(TSNode node) const;


		// 获取节点的完整代码范围
		void GetNodeRange(TSNode node, int& startLine, int& endLine, const std::string& sourceCode) const;

	private:
		std::vector<std::string> _extensions;
	};
}
