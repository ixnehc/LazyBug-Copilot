#pragma once

#include "encrypt/encrypt.h"

#include "Utils.h"

#include <vector>
#include <string>

struct DiffTargetFile
{
	DiffTargetFile()
	{
		codingFmt = Utils::FileContentCodingFormat::None;
	}
	struct Line
	{
		Line()
		{
			suffix = encrypt(0x7fffffff);
		}
		DWORD suffix;
		std::string line;
	};
	int FindLine(DWORD suffix)
	{
		for (int i = 0;i < lines.size();i++)
		{
			if (lines[i].suffix == suffix)
				return i;
		}
		return -1;
	}
	void RemoveLine(DWORD lineIndex)
	{
		lines.erase(lines.begin() + lineIndex);
	}
	void InsertLine(DWORD lineIndex,std::string line)
	{
		Line newLine;
		newLine.line = line;
	
		if(lineIndex>=lines.size()	)
			lines.push_back(newLine);
		else
			lines.insert(lines.begin() + lineIndex, newLine);
	}
	std::vector<Line> lines;
	Utils::FileContentCodingFormat codingFmt;
};

struct UnifiedDiffChunk
{
	std::string filePath;
	struct DiffLine
	{
		enum Type
		{
			NoChange,
			Add,
			Delete,
		};
		DWORD lineSuffix;
		Type tp;
		std::string line;
	};

	std::vector<DiffLine> diffLines;
};

extern void CollectUnifiedDiffChunks(std::string& diffContent, std::string filePath, std::vector<UnifiedDiffChunk>& result);
extern void CollectUnifiedDiffChunk(std::string& diffContent, std::string filePath, UnifiedDiffChunk& result);
extern bool LoadDiffTargetFile(const std::string& filePath, DiffTargetFile& file);
extern bool SaveDiffTargetFile(std::string& filePath, DiffTargetFile& file);
extern void ApplyUnifiedDiff(DiffTargetFile& file, UnifiedDiffChunk& chunk);


