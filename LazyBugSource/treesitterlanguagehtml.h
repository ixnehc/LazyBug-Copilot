#pragma once

#include "TreeSitterLanguage.h"
#include <memory>

#define TreeSitterSymbol_Begin namespace TreeSitterSymbol{
#define TreeSitterSymbol_End }


namespace TreeSitterSymbol
{
	// HTML 语言支持
	class CLanguageSupportHtml : public ILanguageSupport
	{
	public:
		CLanguageSupportHtml();
		virtual ~CLanguageSupportHtml();

		virtual Language GetLanguage() const override { return Language::Html; }
		virtual const TSLanguage* GetTSLanguage() const override;

		virtual bool IsSymbolDefinition(TSNode node) const override;

		virtual std::string GetNodeName(TSNode node, const std::string& sourceCode) const override;

		virtual std::string GetNodeDisplayName(TSNode node, const std::string& sourceCode) const override;

		virtual bool GetNameNodeRange(TSNode node, TSPoint& startPoint, TSPoint& endPoint) const override;

		virtual const std::vector<std::string>& GetFileExtensions() const override;

		virtual SymbolKind GetSymbolKind(TSNode node) const override;

		virtual LineRange GetNodeLineRange(TSNode node, const std::string& sourceCode) const override;

		// 覆写：支持 class 多值拆分为多个 symbol
		virtual void CollectSymbols(
			TSNode node,
			const std::string& sourceCode,
			std::vector<RawSymbolDefine>& outSymbols,
			std::string& currentPrefix) const override;

	private:
		// 取 attribute 节点下 attribute_name 子节点的文本
		std::string _GetAttributeNameText(TSNode node, const std::string& sourceCode) const;

		// 取 attribute 节点下 attribute_value 子节点（找不到返回 null 节点）
		TSNode _GetAttributeValueNode(TSNode node) const;

		// 向上遍历父节点找到所属 element/script_element/style_element，返回其行范围
		LineRange _GetElementLineRange(TSNode node, const std::string& sourceCode) const;

	private:
		std::vector<std::string> _extensions;
	};
}
