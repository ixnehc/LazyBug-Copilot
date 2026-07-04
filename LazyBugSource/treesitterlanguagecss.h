#pragma once

#include "TreeSitterLanguage.h"
#include <memory>

#define TreeSitterSymbol_Begin namespace TreeSitterSymbol{
#define TreeSitterSymbol_End }


namespace TreeSitterSymbol
{
	// CSS 语言支持
	class CLanguageSupportCss : public ILanguageSupport
	{
	public:
		CLanguageSupportCss();
		virtual ~CLanguageSupportCss();

		virtual Language GetLanguage() const override { return Language::Css; }
		virtual const TSLanguage* GetTSLanguage() const override;

		virtual bool IsSymbolDefinition(TSNode node) const override;

		virtual std::string GetNodeName(TSNode node, const std::string& sourceCode) const override;

		virtual std::string GetNodeDisplayName(TSNode node, const std::string& sourceCode) const override;

		virtual bool GetNameNodeRange(TSNode node, TSPoint& startPoint, TSPoint& endPoint) const override;

		virtual const std::vector<std::string>& GetFileExtensions() const override;

		virtual SymbolKind GetSymbolKind(TSNode node) const override;

		virtual LineRange GetNodeLineRange(TSNode node, const std::string& sourceCode) const override;

		// 覆写：支持逗号分组选择器拆分为多个 symbol
		virtual void CollectSymbols(
			TSNode node,
			const std::string& sourceCode,
			std::vector<RawSymbolDefine>& outSymbols,
			std::string& currentPrefix) const override;

	private:
		// 取节点文本（字节截取）
		std::string _GetNodeText(TSNode node, const std::string& sourceCode) const;

		// 按类型查找第一个命名子节点，找不到返回 null 节点
		TSNode _GetChildByType(TSNode node, const char* type) const;

		// 判断是否为 at-rule 类节点
		static bool _IsAtRule(const char* nodeType);

		std::vector<std::string> _extensions;
	};
}
