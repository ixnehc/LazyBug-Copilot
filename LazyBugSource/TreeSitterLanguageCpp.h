#pragma once

#include "TreeSitterLanguage.h"
#include <memory>

#define TreeSitterSymbol_Begin namespace TreeSitterSymbol{
#define TreeSitterSymbol_End }


namespace TreeSitterSymbol
{
	// C++ 语言支持
	class CLanguageSupportCpp : public ILanguageSupport
	{
	public:
		CLanguageSupportCpp();
		virtual ~CLanguageSupportCpp();

		// ILanguageSupport 接口实现
		virtual Language GetLanguage() const override { return Language::Cpp; }

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
		bool IsClassSpecifier(TSNode node) const;
		bool IsStructSpecifier(TSNode node) const;
		bool IsEnumSpecifier(TSNode node) const;
		bool IsFunctionDefinition(TSNode node) const;
		bool IsDeclaration(TSNode node) const;
		bool IsNamespaceDefinition(TSNode node) const;
		bool IsTemplateDeclaration(TSNode node) const;
		bool IsClassDeclaration(TSNode node) const;

		// 获取节点的完整代码范围
		void GetNodeRange(TSNode node, int& startLine, int& endLine, const std::string& sourceCode) const;

	private:
		std::vector<std::string> _extensions;
	};

}
