#pragma once

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

// TreeSitter C接口
#include "treesitter_api.h"

#include "TreeSitterSymbolDefines.h"

#define TreeSitterSymbol_Begin namespace TreeSitterSymbol{
#define TreeSitterSymbol_End }


namespace TreeSitterSymbol
{
	// 语言支持抽象接口
	class ILanguageSupport
	{
	public:
		virtual ~ILanguageSupport() = default;

		// 获取语言类型
		virtual Language GetLanguage() const = 0;

		// 获取TreeSitter语言对象
		virtual const TSLanguage* GetTSLanguage() const = 0;

		// 判断节点是否是符号定义
		virtual bool IsSymbolDefinition(TSNode node) const = 0;

		// 获取节点名称
		virtual std::string GetNodeName(TSNode node, const std::string& sourceCode) const = 0;

		// 获取节点显示名称（可能包含类型信息等）
		virtual std::string GetNodeDisplayName(TSNode node, const std::string& sourceCode) const
		{
			return GetNodeName(node, sourceCode);
		}

		// 获取符号名称子节点的范围
		virtual bool GetNameNodeRange(TSNode node, TSPoint& startPoint, TSPoint& endPoint) const = 0;

		// 获取文件扩展名列表
		virtual const std::vector<std::string>& GetFileExtensions() const = 0;

		// 获取符号类型
		virtual SymbolKind GetSymbolKind(TSNode node) const = 0;

		// 获取节点的代码范围
		virtual LineRange GetNodeLineRange(TSNode node, const std::string& sourceCode) const = 0;

		// 收集一个节点产生的所有符号（默认实现：单 symbol，子类可覆写以支持多 symbol）
		virtual void CollectSymbols(
			TSNode node,
			const std::string& sourceCode,
			std::vector<RawSymbolDefine>& outSymbols,
			std::string& currentPrefix) const;
	};

	// 语言支持工厂
	class CLanguageFactory
	{
	public:
		static CLanguageFactory& Instance();

		// 注册语言支持
		void RegisterLanguage(std::shared_ptr<ILanguageSupport> support);

		// 根据语言类型获取支持对象
		std::shared_ptr<ILanguageSupport> GetLanguageSupport(Language lang);

		// 根据文件路径自动识别语言
		Language DetectLanguage(const std::string& filePath);

		// 获取所有注册的语言
		const std::unordered_map<Language, std::shared_ptr<ILanguageSupport>>& GetAllLanguages() const
		{
			return _languages;
		}

	private:
		CLanguageFactory() = default;
		~CLanguageFactory() = default;
		CLanguageFactory(const CLanguageFactory&) = delete;
		CLanguageFactory& operator=(const CLanguageFactory&) = delete;

		std::unordered_map<Language, std::shared_ptr<ILanguageSupport>> _languages;
	};

	// 辅助函数：从文件扩展名判断语言
	Language GetLanguageFromExtension(const std::string& extension);

	// 辅助函数：从文件路径判断语言
	Language GetLanguageFromFilePath(const std::string& filePath);

}
