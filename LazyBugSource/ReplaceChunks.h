#pragma once


//ReplaceChunks 代表一组代码段,分别对原始文件的多个区块进行替换,以达到修改文件的目的
struct ReplaceChunks
{
	struct Anchor
	{
		Anchor()
		{
			lineIdx = -1;
			lineCount = 0;
			lineIdxInChunk = -1;
		}
		bool IsValid() const
		{
			return lineIdx >= 0;
		}
		int GetStartLine() const		{			return lineIdx;		}
		int GetEndLine() const		{			return lineIdx + lineCount;		}
		int GetEndLineInChunk() const		{			return lineIdxInChunk + lineCount;		}
		int lineIdx;
		int lineCount;
		int lineIdxInChunk;
	};

	struct Chunk
	{
		Chunk()
		{
		}
		Anchor startAnchor;//原文件里替换起始定位Anchor
		Anchor endAnchor;//原文件里替换终止定位Anchor

		std::vector<std::string> lines;
	};

	std::vector<Chunk> _chunks;

};

struct ReplaceTargetFileLines
{
	int GetLineCount() const
	{
		return _lines.size();
	}
	struct LineAttr
	{
		LineAttr()
		{
			sem = Semantic::None;
			indentLevel = 0;
		}
		enum class Semantic
		{
			None,
			FunctionImpl,//函数实现的第一行
			ClassDecl,//类声明的第一行
			CloseBracket,//单独一行右侧大括号
			Comment,//完整的注释行
			ConditionTitle,//if,for,while,switch的所在行
		};
		Semantic sem;
		int indentLevel;//缩进的层级
	};
	std::vector<std::string> _lines;
	std::vector<LineAttr> _lineAttrs;
};

extern void ParseReplaceChunks(const std::string& content, ReplaceChunks& chunks);

extern void ParseReplaceTargetFileLines(const std::string& content, ReplaceTargetFileLines& fileLines);
extern void DumpReplaceTargetFileLines(std::string& content, const ReplaceTargetFileLines& fileLines);

extern void LocateReplaceChunks(ReplaceChunks& chunks, const ReplaceTargetFileLines& fileLines);

extern void ApplyReplaceChunks(const ReplaceChunks& chunks, ReplaceTargetFileLines& fileLines);