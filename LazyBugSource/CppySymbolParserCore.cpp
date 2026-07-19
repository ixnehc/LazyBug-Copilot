// Proj_LazyBugCppParser.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "stdh.h"

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>  // 用于 std::transform
#include <sys/stat.h> // 用于 stat 函数
#include <time.h>     // 用于 time_t 和 ctime 函数
// 包含所需的头文件
#include "datapacket/DataPacket.h"
#include "stringparser/stringparser.h"
#include "CppSymbolDefines.h"
#include "SolutionDBDefines.h"
#include "Utils.h"

#include <clang-c/Index.h>
#include <clang-c/CXString.h>
#include <clang-c/CXSourceLocation.h>
#include <clang-c/CXFile.h>

using namespace CppSymbol;


#define __M(str) MessageBoxA(NULL, str, "Current Parsing:", MB_OK);

class CSystemDirectories
{
public:
	static bool IsUnderSystemDir(const char* path)
	{
		if (!path || !*path)
			return false;

		// 获取常见系统目录前缀（懒加载，只算一次）
		// 覆盖：
		//   %ProgramFiles%\Microsoft Visual Studio\...
		//   %ProgramFiles(x86)%\Microsoft Visual Studio\...
		//   %ProgramFiles%\Windows Kits\...
		//   %ProgramFiles(x86)%\Windows Kits\...
		//   %ProgramFiles%\LLVM\...  (clang)
		static std::vector<std::string> s_sysPrefixes;
		static bool s_initialized = false;
		if (!s_initialized)
		{
			s_initialized = true;

			// 收集候选的环境变量
			const wchar_t* envVars[] = {
				L"ProgramFiles",
				L"ProgramFiles(x86)",
				L"ProgramW6432",
			};
			for (auto ev : envVars)
			{
				wchar_t buf[MAX_PATH] = {};
				if (GetEnvironmentVariableW(ev, buf, MAX_PATH) == 0)
					continue;

				std::wstring wbase = buf;
				std::transform(wbase.begin(), wbase.end(), wbase.begin(), ::towlower);
				// 确保末尾有斜杠，避免前缀误匹配
				if (!wbase.empty() && wbase.back() != L'\\')
					wbase += L'\\';

				// 转换为 UTF-8 std::string
				std::string base = widechar_to_utf8(wbase.c_str());

				// Visual Studio 安装目录
				{
					std::string p = base + "microsoft visual studio\\";
					if (std::find(s_sysPrefixes.begin(), s_sysPrefixes.end(), p) == s_sysPrefixes.end())
						s_sysPrefixes.push_back(p);
				}
				// Windows Kits (SDK)
				{
					std::string p = base + "windows kits\\";
					if (std::find(s_sysPrefixes.begin(), s_sysPrefixes.end(), p) == s_sysPrefixes.end())
						s_sysPrefixes.push_back(p);
				}
				// LLVM / Clang
				{
					std::string p = base + "llvm\\";
					if (std::find(s_sysPrefixes.begin(), s_sysPrefixes.end(), p) == s_sysPrefixes.end())
						s_sysPrefixes.push_back(p);
				}
			}
		}

		size_t pathLen = strlen(path);
		for (const auto& prefix : s_sysPrefixes)
		{
			if (pathLen >= prefix.size() &&
				_strnicmp(path, prefix.c_str(), prefix.size()) == 0)
			{
				return true;
			}
		}
		return false;
	}
};
CSystemDirectories g_sysDirs;

bool IsUnderSystemDir(const char* path)
{
	return g_sysDirs.IsUnderSystemDir(path);
}

time_t GetTimeT(const char* filePath)
{
	return Utils::GetFileTimeT(filePath);
}

static std::string StandardizeFilePath(const char* path_)
{
	std::string path = path_;
	StringLower(path);
	FixSlashInPath_Utf8((char*)path.c_str());
	path = ResolveRelativePathWithDots(path);
	return std::move(path);
}

// 将 CXCursorKind 转换为 SymbolKind（可选带 cursor，用于区分虚函数等情况）
inline SymbolKind ToSymbolKind(CXCursorKind kind, CXCursor cursor = clang_getNullCursor())
{
	switch (kind)
	{
		// 定义相关
		case CXCursor_Namespace:             return SymbolKind::Namespace;
		case CXCursor_ClassDecl:             return SymbolKind::Class;
		case CXCursor_StructDecl:            return SymbolKind::Struct;
		case CXCursor_EnumDecl:              return SymbolKind::Enum;
		case CXCursor_EnumConstantDecl:      return SymbolKind::EnumConstant;
		case CXCursor_FunctionDecl:          return SymbolKind::Function;
		case CXCursor_CXXMethod:             return SymbolKind::Method;
		case CXCursor_Constructor:           return SymbolKind::Constructor;
		case CXCursor_Destructor:            return SymbolKind::Destructor;
		case CXCursor_FieldDecl:             return SymbolKind::Field;
		case CXCursor_VarDecl:               return SymbolKind::Variable;
		case CXCursor_ParmDecl:              return SymbolKind::Parameter;
		case CXCursor_TypedefDecl:           return SymbolKind::Typedef;
		case CXCursor_TypeAliasDecl:         return SymbolKind::TypeAlias;
		case CXCursor_FunctionTemplate:
		case CXCursor_ClassTemplate:         return SymbolKind::Template;
		case CXCursor_TemplateTypeParameter:
		case CXCursor_NonTypeTemplateParameter:
		case CXCursor_TemplateTemplateParameter:return SymbolKind::TemplateParameter;
		case CXCursor_MacroDefinition:       return SymbolKind::Macro;
		case CXCursor_UsingDirective:        return SymbolKind::UsingDirective;
		case CXCursor_UsingDeclaration:      return SymbolKind::UsingDeclaration;
		case CXCursor_FriendDecl:            return SymbolKind::Friend;
		case CXCursor_LambdaExpr:            return SymbolKind::Lambda;

		// 引用相关
		case CXCursor_TypeRef:               return SymbolKind::TypeRef;
		case CXCursor_NamespaceRef:          return SymbolKind::NamespaceRef;
		case CXCursor_MemberRefExpr:         return SymbolKind::MemberRef;
		case CXCursor_LabelRef:              return SymbolKind::LabelRef;
		case CXCursor_OverloadedDeclRef:     return SymbolKind::OverloadedDeclRef;
		case CXCursor_VariableRef:           return SymbolKind::VariableRef;
		case CXCursor_DeclRefExpr:           return SymbolKind::DeclRefExpr;
		case CXCursor_TemplateRef:           return SymbolKind::TemplateRef;
		case CXCursor_CXXBaseSpecifier:      return SymbolKind::CXXBaseSpecifier;
		case CXCursor_CallExpr:              return SymbolKind::CallExpr;
		case CXCursor_MacroExpansion:        return SymbolKind::MacroExpansion;
		case CXCursor_ConversionFunction:    return SymbolKind::ConversionRef;

		default:                             return SymbolKind::Invalid;
	}
}

static void GetCursorLocation(CXCursor c, CXFile& file, CXSourceLocation&loc,unsigned& line, unsigned& column, unsigned& length)
{
	CXSourceRange range = clang_getCursorExtent(c);
	CXSourceLocation start = clang_getRangeStart(range);
	CXSourceLocation end = clang_getRangeEnd(range);

	unsigned startLine, startColumn, startOffset;
	unsigned endLine, endColumn, endOffset;
	clang_getFileLocation(start, &file, &startLine, &startColumn, &startOffset);
	clang_getFileLocation(end, nullptr, &endLine, &endColumn, &endOffset);

	line = startLine - 1;
	column = startColumn - 1;
	length = endColumn - startColumn;
	loc = start;
}

static std::unordered_map<CXFile, std::string> fileCache;
static bool GetStandardFullPath(CXFile file,std::string &fullPath)
{
	// 首先检查缓存中是否已有该文件的标准化路径
	
	auto cacheIt = fileCache.find(file);
	if (cacheIt != fileCache.end())
	{
		fullPath = cacheIt->second;
		return !fullPath.empty();
	}

	CXString file_name = clang_getFileName(file);
	const char* file_path = clang_getCString(file_name);

	if (!file_path) 
	{
		clang_disposeString(file_name);
		// 缓存空结果以避免重复处理
		fileCache[file] = "";
		return false;
	}

	if ((file_path[0])&&(file_path[1] != ':'))
	{
		clang_disposeString(file_name);

		//不是绝对路径
		file_name = clang_File_tryGetRealPathName(file);
		file_path = clang_getCString(file_name);
		if ((file_path[0]) && (file_path[1] != ':'))
		{
			//还是不是绝对路径
			clang_disposeString(file_name);
			// 缓存空结果以避免重复处理
			fileCache[file] = "";
			return false;
		}
	}

	if (g_sysDirs.IsUnderSystemDir(file_path))
	{
		clang_disposeString(file_name);
		// 缓存空结果以避免重复处理
		fileCache[file] = "";
		return false;
	}

	fullPath = StandardizeFilePath(file_path);
	clang_disposeString(file_name);
	
	// 缓存结果
	fileCache[file] = fullPath;
	return true;
}

//-------------------------------------------------------------------------------------------------
// VisitContext_Ref : 专门用于 Ref 收集
struct VisitContext_Ref
{
	RawSymbolRefs* rawRefs = nullptr;
	std::unordered_map<std::string, WORD> refTargetFilesLookUp;
	std::string targetFile; // 只收集目标文件

	WORD AddRefTargetFile(const char* filePath)
	{
		if (!filePath || !rawRefs)
			return 0xffff;
		std::string path = filePath;
		auto it = refTargetFilesLookUp.find(path);
		if (it != refTargetFilesLookUp.end())
			return it->second;
		WORD newIndex = static_cast<WORD>(rawRefs->targetFiles.size());
		RawSymbolRefs::File file;
		file.path = path;
		file.time = GetTimeT(path.c_str());
		rawRefs->targetFiles.push_back(std::move(file));
		refTargetFilesLookUp[path] = newIndex;
		return newIndex;

	}
};

bool CollectRefTarget(CXCursor referenced, RawSymbolRef::RefTarget& refTarget, VisitContext_Ref& ctx)
{
	refTarget.Zero();

	if (!clang_Cursor_isNull(referenced))
	{
		CXSourceRange refRange = clang_getCursorExtent(referenced);
		CXSourceLocation refStart = clang_getRangeStart(refRange);
		CXSourceLocation refEnd = clang_getRangeEnd(refRange);

		CXFile defFile;
		unsigned defStartLine, defEndLine, tmpCol, tmpOff;
		clang_getFileLocation(refStart, &defFile, &defStartLine, &tmpCol, &tmpOff);
		clang_getFileLocation(refEnd, nullptr, &defEndLine, nullptr, nullptr);

		std::string path;
		if (GetStandardFullPath(defFile, path))
		{
			refTarget.fileIndex = ctx.AddRefTargetFile(path.c_str());

			refTarget.lineRange.start = (WORD)(defStartLine - 1);
			refTarget.lineRange.end = (WORD)(defEndLine - 1 + 1);//-1为了转为0-base, +1 为了转为non-inclusive end

			// 新增：收集 refTarget 的符号类型
			CXCursorKind k = clang_getCursorKind(referenced);
			refTarget.kind = ToSymbolKind(k, referenced);
		}
	}
	return refTarget.IsValid();
}


static CXChildVisitResult VisitNode_Ref_Raw(CXCursor c, CXCursor parent, CXClientData client_data)
{
	VisitContext_Ref* ctx = (VisitContext_Ref*)client_data;
	CXSourceLocation loc = clang_getCursorLocation(c);
	CXFile file;
	unsigned line, col, offset;
	clang_getFileLocation(loc, &file, &line, &col, &offset);

	std::string filePath;
	if (!GetStandardFullPath(file, filePath))
		return CXChildVisit_Continue;

	if (filePath != ctx->targetFile) // 只采集目标文件中的引用
		return CXChildVisit_Recurse;

	CXCursorKind kind = clang_getCursorKind(c);
	CXString spelling = clang_getCursorSpelling(c);
	const char* name = clang_getCString(spelling);

	if (name && strlen(name) > 0)
	{
		RawSymbolRef ref;
		ref.name = name;
		ref.lineLoc.line = (WORD)(line - 1);
		ref.lineLoc.startColumn = (WORD)(col - 1);
		ref.lineLoc.endColumn = (WORD)(col);

		CXCursor targetCursor = clang_getCursorReferenced(c);
		if (CollectRefTarget(targetCursor, ref.refTarget, *ctx))
		{
			CollectRefTarget(clang_getCursorSemanticParent(targetCursor), ref.refTargetParent, *ctx);
			// 使用 ToSymbolKind() 代替手写 switch
			ref.kind = ToSymbolKind(kind, c);
			if (ref.kind != SymbolKind::Invalid)
				ctx->rawRefs->refs.push_back(ref);
		}
	}
	clang_disposeString(spelling);
	return CXChildVisit_Recurse;
}

struct VisitContext
{
	VisitContext()
	{
		lowerCasedParseFilePath = "";
		rawInclusions = nullptr;
		rawFileTimes = nullptr;
	}

	const char* lowerCasedParseFilePath;

	std::unordered_map<std::string, std::vector<RawSymbolDefine>> rawDefines;
	std::unordered_map<std::string, time_t>* rawFileTimes;
	std::unordered_map<std::string, RawInclusion>* rawInclusions;
	std::unordered_set<CXFile> inclusions;

	int debugCount = 0;
	int debugCount2 = 0;

};

static void AddRawSymbolDefine(VisitContext& ctx,
	SymbolKind kind, const char* filePath, unsigned line, unsigned column, unsigned length, const char* name,
	unsigned startLine, unsigned endLine)
{
	if (line > 0xffff)
		return;
	if ((startLine > 0xffff) || (endLine > 0xffff))
		return;

	// 创建新的原始symbol定义记录
	RawSymbolDefine define;

	// 设置symbol名称
	define.name = name;

	// 设置symbol类型
	define.kind = kind;


	// 设置symbol位置
//	define.location.filePath = filePath;
	define.lineLoc.line = (WORD)(line - 1);
	define.lineLoc.startColumn = (WORD)(column - 1);
	define.lineLoc.endColumn = (WORD)(column - 1 + length);

	// 设置代码行范围
	define.lineRange.start = (WORD)(startLine - 1);
	define.lineRange.end = (WORD)(endLine - 1 + 1);


	// 添加到定义列表
	ctx.rawDefines[std::string(filePath)].push_back(define);
}

static CXChildVisitResult VisitNode_Define_Raw(CXCursor c, CXCursor parent, CXClientData client_data)
{
	VisitContext* ctx = (VisitContext*)client_data;

	ctx->debugCount++;

	// 获取节点类型和位置信息
	CXCursorKind kind = clang_getCursorKind(c);
	SymbolKind sk = ToSymbolKind(kind, c);

	if (false)
	{
		CXString spelling = clang_getCursorSpelling(c);
		const char* name = clang_getCString(spelling);

		if (std::string("Curl_dyn_addf") == name)
		{
			__M("Here2");

		}


		clang_disposeString(spelling);

	}


	switch (sk)
	{
	case SymbolKind::Namespace:
	case SymbolKind::Class:
	case SymbolKind::Struct:
	case SymbolKind::Enum:
	case SymbolKind::EnumConstant:
	case SymbolKind::Function:
	case SymbolKind::Method:
// 	case SymbolKind::Constructor:
// 	case SymbolKind::Destructor:
	case SymbolKind::Variable:
	case SymbolKind::Field:
	case SymbolKind::Typedef:
	case SymbolKind::TypeAlias:
	case SymbolKind::Template:
	case SymbolKind::Macro:
//	case SymbolKind::MacroExpansion:
		break;
	default:
		return CXChildVisit_Recurse;
	}

	// 特殊规则：跳过局部变量
	if (kind == CXCursor_VarDecl)
	{
		CXCursor parent = clang_getCursorSemanticParent(c);
		CXCursorKind parentKind = clang_getCursorKind(parent);
		if (parentKind == CXCursor_FunctionDecl ||
			parentKind == CXCursor_CXXMethod ||
			parentKind == CXCursor_Constructor ||
			parentKind == CXCursor_Destructor ||
			parentKind == CXCursor_CompoundStmt)
		{
			return CXChildVisit_Continue;
		}
	}

	// 特殊规则：前向声明不记录
	if ((kind == CXCursor_ClassDecl || kind == CXCursor_StructDecl) && !clang_isCursorDefinition(c))
		return CXChildVisit_Continue;

	// 获取定义位置
	CXFile file;
	unsigned line, column, length;
	CXSourceLocation loc = clang_getCursorLocation(c);
	clang_getFileLocation(loc, &file, &line, &column, nullptr);

	if (ctx->inclusions.find(file) == ctx->inclusions.end())
	{
		// 文件不在包含文件列表中
		if (!clang_Location_isFromMainFile(loc))
		{
			return CXChildVisit_Continue;
		}
	}

	CXString spelling = clang_getCursorSpelling(c);
	const char* name = clang_getCString(spelling);
	
	// 计算 length：如果有 spelling，使用其长度；否则使用 extent 范围
	if (name && strlen(name) > 0)
	{
		length = (unsigned)strlen(name);
	}
	else
	{
		CXSourceRange range = clang_getCursorExtent(c);
		CXSourceLocation start = clang_getRangeStart(range);
		CXSourceLocation end = clang_getRangeEnd(range);
		unsigned startCol, endCol;
		clang_getFileLocation(start, nullptr, nullptr, &startCol, nullptr);
		clang_getFileLocation(end, nullptr, nullptr, &endCol, nullptr);
		length = endCol - startCol;
	}

	// 检查是否为匿名或无名symbol，如果是则跳过
	bool isAnonymousSymbol = false;
	if (name && strlen(name) > 0)
	{
		// 检查是否包含"(anonymous"字样，表示匿名结构
		if (strstr(name, "(anonymous") != nullptr)
		{
			isAnonymousSymbol = true;
		}
	}
	else
	{
		// 名称为空，检查是否为匿名结构
		if (kind == CXCursor_UnionDecl || kind == CXCursor_StructDecl || kind == CXCursor_ClassDecl)
		{
			isAnonymousSymbol = true;
		}
	}

	if (name && strlen(name) > 0 && !isAnonymousSymbol)
	{

		std::string path;
		if (!GetStandardFullPath(file, path))
		{
			return CXChildVisit_Continue;
		}

		// 首次遇到该文件时，记录其修改时间
		if (ctx->rawFileTimes->find(path) == ctx->rawFileTimes->end())
		{
			(*ctx->rawFileTimes)[path] = clang_getFileTime(file);
		}

		// 获取代码范围（当前声明/定义自身）
		CXSourceRange range = clang_getCursorExtent(c);
		CXSourceLocation start = clang_getRangeStart(range);
		CXSourceLocation end = clang_getRangeEnd(range);

		unsigned startLine, endLine;
		clang_getFileLocation(start, nullptr, &startLine, nullptr, nullptr);
		clang_getFileLocation(end, nullptr, &endLine, nullptr, nullptr);

		if (true)
		{
			if (sk != SymbolKind::Invalid)
			{

				// 获取完整symbol路径
				std::string fullName;
				if (true)
				{
					CXCursor current = c;
					while (!clang_Cursor_isNull(current) && clang_getCursorKind(current) != CXCursor_TranslationUnit)
					{
						CXString currentSpelling = clang_getCursorSpelling(current);
						const char* currentName = clang_getCString(currentSpelling);

						// 检查是否为匿名union/struct
						bool isAnonymous = false;
						if (currentName && strlen(currentName) > 0)
						{
							// 检查是否包含"(anonymous"字样，表示匿名结构
							if (strstr(currentName, "(anonymous") != nullptr)
							{
								isAnonymous = true;
							}
						}
						else
						{
							// 名称为空，也可能是匿名结构
							CXCursorKind currentKind = clang_getCursorKind(current);
							if (currentKind == CXCursor_UnionDecl || currentKind == CXCursor_StructDecl)
							{
								isAnonymous = true;
							}
						}

						// 只有非匿名的容器才添加到路径中
						if (!isAnonymous && currentName && strlen(currentName) > 0)
						{
							if (!fullName.empty())
							{
								fullName.insert(0, ".");
							}
							fullName.insert(0, currentName);
						}

						clang_disposeString(currentSpelling);
						current = clang_getCursorSemanticParent(current);
					}
					if (fullName.empty() && name)
					{
						fullName = name;
					}
				}

				ctx->debugCount2++;
				AddRawSymbolDefine(*ctx, sk, path.c_str(), line, column, length, fullName.c_str(), startLine, endLine);
			}

		}
	}

	clang_disposeString(spelling);

	return CXChildVisit_Recurse;
}

void ProcessRequest_CollectRef(const ParseRequest& request, ParseResult& result)
{
// 	__M("Here");

	if (!request.collectRefParam.IsValid())
		return;

	// Load unsaved content if necessary
	CXUnsavedFile* unsaved_files = nullptr;
	unsigned num_unsaved_files = 0;
	std::string content;

	if (!request.collectRefParam.unsavedContentTempFilePath.empty())
	{
		if (!Utils::LoadFileContent(request.collectRefParam.unsavedContentTempFilePath.c_str(), content))
			return;
		unsaved_files = new CXUnsavedFile[1];
		unsaved_files[0].Filename = request.collectRefParam.filePath.c_str();
		unsaved_files[0].Contents = content.c_str();
		unsaved_files[0].Length = (int)content.length();
		num_unsaved_files = 1;
	}

	// Create index
	CXIndex index = clang_createIndex(0, 0);
	if (!index)
	{
		if (unsaved_files) delete[] unsaved_files;
		return;
	}

	bool isGeneratingPCH = request.NeedGenPCH();

	// include args (基本和 ProcessRequest 一致)
	std::vector<const char*> args;
	args.push_back("-x");
	args.push_back("c++");
	args.push_back("-std=c++17");
	args.push_back("-I.");
	if (request.setting)
	{
		for (const std::string& path : request.setting->additionalIncludeFullPathes)
		{
			args.push_back("-I");
			args.push_back(path.c_str());
		}
	}

	if (!isGeneratingPCH)
	{
		if (!request.setting->lowerCasedPchOutputFullPath.empty())
		{
			args.push_back("-include-pch");
			args.push_back(request.setting->lowerCasedPchOutputFullPath.c_str());
		}
	}


	// parse TU
	CXTranslationUnit unit = clang_parseTranslationUnit(
		index,
		request.lowerCasedParseFilePath.c_str(),
		args.data(),
		(int)args.size(),
		unsaved_files,
		num_unsaved_files,
		CXTranslationUnit_None
	);

	RawSymbolRefs refs;
	if (unit)
	{
		CXCursor cursor = clang_getTranslationUnitCursor(unit);
		if (!clang_Cursor_isNull(cursor))
		{
			VisitContext_Ref ctx;
			ctx.rawRefs = &refs;
			ctx.targetFile = request.collectRefParam.filePath;
			clang_visitChildren(cursor, VisitNode_Ref_Raw, &ctx);
		}
	}

	// save result
	if (!request.collectRefParam.resultTempFilePath.empty())
	{
		std::vector<BYTE> buf;
		DP_BeginSave(dp, buf);
		{
			refs.Save(dp);
		}
		DP_EndSave();

		if (Utils::SaveFileContent(request.collectRefParam.resultTempFilePath.c_str(), buf))
		{
			result.success = true;
//			__M("Here");
		}
	}

	if (unit) clang_disposeTranslationUnit(unit);
	clang_disposeIndex(index);

	if (unsaved_files) delete[] unsaved_files;
}

void VisitNode_Inclusion_Raw(CXFile included_file, CXSourceLocation* include_stack, unsigned include_stack_size, CXClientData client_data)
{
	VisitContext* ctx = (VisitContext*)client_data;
	if (!ctx->rawInclusions)
		return;
	std::string path;
	if (!GetStandardFullPath(included_file, path))
		return;

	// 创建包含文件信息并添加到map中
	(*ctx->rawInclusions)[path].time = clang_getFileTime(included_file);
	ctx->inclusions.insert(included_file);
}

void ProcessRequest(const ParseRequest& request, ParseResult& result)
{
	if (request.collectRefParam.IsValid())
		return;

//	__M(request.lowerCasedParseFilePath.c_str());
// 	MessageBoxA(NULL, request.parseFilePath.c_str(), "Current Parsing:", MB_OK);

	result.parseFilePath = request.lowerCasedParseFilePath;
	result.success = false;
	result.requestId = request.requestId;

	bool isGeneratingPCH = request.NeedGenPCH();

	// 创建索引
	CXIndex index = clang_createIndex(0, 0);
	if (!index)
		return;

	result.AddDebugTime("D-B");

	// 准备编译参数
	std::vector<const char*> args;
	args.reserve(4 + (request.setting ? request.setting->additionalIncludeFullPathes.size() : 0) * 2);

	// 基本参数
	args.push_back("-x");
	args.push_back("c++");
	args.push_back("-std=c++17");
	args.push_back("-I.");

	// 附加include路径
	if (request.setting)
	{
		for (const std::string& path : request.setting->additionalIncludeFullPathes)
		{
			args.push_back("-I");
			args.push_back(path.c_str());
		}
	}

	// 	args.push_back("-v");

	time_t pchTime = 0;
	// 添加预编译头文件参数
	if (!isGeneratingPCH)
	{
		if (!request.setting->lowerCasedPchOutputFullPath.empty())
		{
			args.push_back("-include-pch");
			args.push_back(request.setting->lowerCasedPchOutputFullPath.c_str());
			pchTime = GetTimeT(request.setting->lowerCasedPchOutputFullPath.c_str());
		}
	}

// 	if (isGeneratingPCH)
// 		__M(request.lowerCasedParseFilePath.c_str());

	unsigned option = isGeneratingPCH ? (CXTranslationUnit_ForSerialization | CXTranslationUnit_DetailedPreprocessingRecord | CXTranslationUnit_KeepGoing )
		: CXTranslationUnit_DetailedPreprocessingRecord | CXTranslationUnit_KeepGoing ;
	option |= CXTranslationUnit_RetainExcludedConditionalBlocks;
	 
	CXTranslationUnit unit = clang_parseTranslationUnit(
		index,
		request.lowerCasedParseFilePath.c_str(),
		args.data(),
		(int)args.size(),
		nullptr,
		0,
		option
	);
	result.AddDebugTime("D-C");

// 	if (isGeneratingPCH)
// 		__M("ok2");

	if (isGeneratingPCH)
	{
		if (unit)
		{
			clang_saveTranslationUnit(unit, request.setting->lowerCasedPchOutputFullPath.c_str(), clang_defaultSaveOptions(unit));
			pchTime = GetTimeT(request.setting->lowerCasedPchOutputFullPath.c_str());
			result.isPCHGenerated = true;
		}
	}

// 	if (!unit)
// 	{
// 		MessageBoxA(nullptr, request.setting->lowerCasedPchOutputFullPath.c_str(), "Fail!", MB_OK);
// 	}

	result.success = (unit != nullptr);
	result.pchTime = pchTime;

	// 新增：在这里进行 AST 遍历，提取符号定义和包含文件
	if (result.success)
	{
		time_t fileTime = 0;
		CXFile mainFile = clang_getFile(unit, request.lowerCasedParseFilePath.c_str());
		if (mainFile)
		{
			fileTime = clang_getFileTime(mainFile);
		}

		// 获取根节点
		CXCursor cursor = clang_getTranslationUnitCursor(unit);
		if (!clang_Cursor_isNull(cursor))
		{
			// 设置访问上下文
			VisitContext ctx;
			ctx.lowerCasedParseFilePath = request.lowerCasedParseFilePath.c_str();
			ctx.rawInclusions = &result.inclusions;
			ctx.rawFileTimes = &result.fileTimes;
			ctx.rawDefines[request.lowerCasedParseFilePath].clear();//确保会把main file加入到结果中(main file有可能没有任何symbol)

			// 预先记录主文件的时间，确保它一定被包含在结果中
			result.fileTimes[request.lowerCasedParseFilePath] = fileTime;

			// 先获取所有包含文件
			clang_getInclusions(unit, VisitNode_Inclusion_Raw, &ctx);
			result.AddDebugTime("D-CA");

			// 然后遍历 AST 提取符号定义（此时会过滤掉不在包含文件中的符号）
			clang_visitChildren(cursor, VisitNode_Define_Raw, &ctx);

			std::string dbg;
			FormatString(dbg, "D-D(%d/%d/%d)", ctx.debugCount, ctx.debugCount2,fileCache.size());
			result.AddDebugTime(dbg.c_str());

			// 访问结束后，移动结果
			result.definesByFile = std::move(ctx.rawDefines);
		}
	}


//	result.AddDebugTime("D-D");

	// 清理资源
	clang_disposeTranslationUnit(unit);
	clang_disposeIndex(index);
}

