#pragma once

#include "TreeSitterLanguage.h"
#include <memory>

#define TreeSitterSymbol_Begin namespace TreeSitterSymbol{
#define TreeSitterSymbol_End }


namespace TreeSitterSymbol
{
	// Java 语言支持
	class CLanguageSupportJava : public ILanguageSupport
	{
	public:
		CLanguageSupportJava();
		virtual ~CLanguageSupportJava();

		virtual Language GetLanguage() const override { return Language::Java; }
		virtual const TSLanguage* GetTSLanguage() const override;

		virtual bool IsSymbolDefinition(TSNode node) const override;

		virtual std::string GetNodeName(TSNode node, const std::string& sourceCode) const override;

		virtual std::string GetNodeDisplayName(TSNode node, const std::string& sourceCode) const override;

		virtual bool GetNameNodeRange(TSNode node, TSPoint& startPoint, TSPoint& endPoint) const override;

		virtual const std::vector<std::string>& GetFileExtensions() const override;

		virtual SymbolKind GetSymbolKind(TSNode node) const override;

		virtual LineRange GetNodeLineRange(TSNode node, const std::string& sourceCode) const override;

		// Java 支持「一行声明多个字段变量」，需覆写以产出多个 symbol；
		// 同时维护类内嵌套前缀（package 不参与前缀）
		virtual void CollectSymbols(
			TSNode node,
			const std::string& sourceCode,
			std::vector<RawSymbolDefine>& outSymbols,
			std::string& currentPrefix) const override;

	private:
		// 判断节点类型
		bool IsClassDeclaration(TSNode node) const;
		bool IsInterfaceDeclaration(TSNode node) const;
		bool IsEnumDeclaration(TSNode node) const;
		bool IsRecordDeclaration(TSNode node) const;
		bool IsAnnotationTypeDeclaration(TSNode node) const;
		bool IsMethodDeclaration(TSNode node) const;
		bool IsFieldDeclaration(TSNode node) const;
		bool IsEnumConstant(TSNode node) const;
		bool IsPackageDeclaration(TSNode node) const;

		// 是否是会引入嵌套命名前缀的类型容器（class/interface/enum/record/annotation）
		bool IsTypeContainer(TSNode node) const;

		// 获取节点的完整代码范围
		void GetNodeRange(TSNode node, int& startLine, int& endLine, const std::string& sourceCode) const;

		// 由文本范围(TSPoint)从源码取出对应字符串
		std::string GetTextByPointRange(const TSPoint& start, const TSPoint& end, const std::string& sourceCode) const;

		// 产出一个 symbol（按 currentPrefix 拼名）；返回拼好的全名
		std::string MakeSymbol(
			TSNode node,
			const std::string& nodeName,
			const TSPoint& nameStart,
			const TSPoint& nameEnd,
			SymbolKind kind,
			const std::string& currentPrefix,
			const std::string& sourceCode,
			std::vector<RawSymbolDefine>& outSymbols) const;

	private:
		std::vector<std::string> _extensions;
	};
}

