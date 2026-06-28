#pragma once

#include <functional>


typedef int StringIndex;
const StringIndex StringIndex_Null = 0;  // 无效的字符串索引

// 统一符号类型
enum class SymbolKind
{
	Invalid = 0,

	// === Common (C/C++, C#, Java, JS, etc.) ===
	Namespace,
	Class,
	Struct,
	Enum,
	EnumConstant,
	Function,
	Method,
	Constructor,
	Destructor,
	Variable,
	Field,
	Parameter,
	ReturnType,
	Typedef,
	TypeAlias,
	Template,
	TemplateParameter,
	Macro,
	UsingDirective,
	UsingDeclaration,
	Alias,
	Friend,
	Lambda,
	Interface,
	Property,
	Constant,
	Import,

	// Common 引用相关
	TypeRef,
	NamespaceRef,
	MemberRef,
	LabelRef,
	OverloadedDeclRef,
	VariableRef,
	DeclRefExpr,
	TemplateRef,
	CXXBaseSpecifier,
	CallExpr,
	CXXOperatorCallExpr,
	CXXMemberCallExpr,
	MacroExpansion,
	ConstructorRef,
	DestructorRef,
	FriendRef,
	UsingRef,
	EnumRef,
	EnumValueRef,
	TypedefRef,
	AliasRef,
	FieldRef,
	ParameterRef,
	ReturnTypeRef,
	BaseClassRef,
	VirtualFunctionRef,
	OperatorRef,
	ConversionRef,
	TemplateSpecializationRef,
	TemplateParameterRef,
	LambdaRef,
	ExceptionRef,
	AttributeRef,

	// === C# 专用 ===
	CSharp_Delegate,
	CSharp_Event,
	CSharp_Record,
	CSharp_Attribute,

	// C# 引用
	CSharp_DelegateRef,
	CSharp_EventRef,

	// === JavaScript 专用 ===
	JavaScript_Module,
	JavaScript_Export,
	JavaScript_ArrowFunction,
	JavaScript_GeneratorFunction,

	// JavaScript 引用
	JavaScript_ModuleRef,

	// === Java 专用 ===
	Java_Package,
	Java_Annotation,
	Java_Record,

	// Java 引用
	Java_PackageRef,
	Java_AnnotationRef,

	// === HTML 专用 ===
	Html_Id,
	Html_Class,

	// === CSS 专用 ===
	Css_Selector,
	Css_AtRule,
	Css_Variable,
};

struct SingleLineLoc
{
	SingleLineLoc()
	{
		line = 0;
		startColumn = endColumn = 0;
	}

	//都是0-base
	WORD line;
	WORD startColumn;
	WORD endColumn;
};

struct LineRange
{
	LineRange()
	{
		Zero();
	}
	void Zero()
	{
		start = 0xffff;
		end = 0;
	}
	bool IsValid() const
	{
		return end >= start;
	}
	//[start,end) , 0-base
	WORD start;
	WORD end;
};

struct SymbolRangeInfo
{
	SymbolKind _kind;      // symbol 类型
	LineRange  _lineRange; // symbol 实现代码行范围
};

struct FileLocation
{
	FileLocation()
	{
		filePath = StringIndex_Null;
		strFilePath = nullptr;
	}
	StringIndex filePath;
	const char* strFilePath;//用于调试

	SingleLineLoc lineLoc;
};

struct FileRange
{
	FileRange()
	{
		filePath = StringIndex_Null;
	}

	StringIndex filePath;
	LineRange lineRange;
};


class CDataPacket;

// 文件过滤回调函数类型
// 返回true表示忽略该文件，false表示不忽略
typedef std::function<bool(const char* filePath)> FindInFileFilter;

struct FindInFileResults
{
	struct FileLineInfo
	{
		int lineNumber;//1-base
		std::string lineContent;
		std::string symbolName;
	};

	struct FileInfo
	{
		std::string filePath;
		std::vector<FileLineInfo> lineInfos;
	};

	std::vector<FileInfo> fileInfos;

	// 添加结果的方法
	void AddResult(const std::string& filePath, int lineNumber, const std::string& lineContent)
	{
		// 查找是否已有该文件的记录
		for (auto& fileInfo : fileInfos)
		{
			if (fileInfo.filePath == filePath)
			{
				fileInfo.lineInfos.push_back({ lineNumber, lineContent });
				return;
			}
		}

		// 如果没有找到，创建新的文件记录
		FileInfo newFileInfo;
		newFileInfo.filePath = filePath;
		newFileInfo.lineInfos.push_back({ lineNumber, lineContent });
		fileInfos.push_back(newFileInfo);
	}

	// 清空结果
	void Clear()
	{
		fileInfos.clear();
	}

	// 获取总结果数
	size_t GetTotalResults() const
	{
		size_t total = 0;
		for (const auto& fileInfo : fileInfos)
		{
			total += fileInfo.lineInfos.size();
		}
		return total;
	}

	void Save(CDataPacket& dp) const;
	void Load(CDataPacket& dp);

};

struct SearchFileResult
{
	struct FileInfo
	{
		std::string filePath;

	};

	std::vector<FileInfo> fileInfos;

	void Save(CDataPacket& dp) const;
	void Load(CDataPacket& dp);

};


enum class ThreadPriority
{
	LOWEST,
	BELOW_NORMAL,
	NORMAL,
	ABOVE_NORMAL,
	HIGHEST,
	TIME_CRITICAL
};


// 语言类型枚举
enum class Language
{
	Unknown = 0,
	C,
	Cpp,
	Python,
	Java,
	JavaScript,
	TypeScript,
	Go,
	Rust,
	CSharp,
	Swift,
	Kotlin,
	Css,
	Html,
	// 可扩展更多语言...
};
