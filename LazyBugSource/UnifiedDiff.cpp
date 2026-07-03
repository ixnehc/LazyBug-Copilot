#include "stdh.h"
#include "UnifiedDiff.h"

#include "stringparser/stringparser.h"
#include <sstream>  // 用于std::istringstream
#include <fstream>  // 用于std::ifstream, std::ofstream
#include <regex>    // 用于std::regex, std::smatch, std::regex_search
#include <ios>      // 用于std::ios::cur
#include <vector>   // 用于std::vector
#include <string>
#include <iostream>

#include "Utils.h"

//截取行尾的//0xXXXXXXXX 十六进制数值,并把它们从行尾删除
void CullLineSuffix(std::string& line, DWORD &suffix)
{
	// 初始化suffix为0
	suffix = 0;
	
	// 检查行长度是否足够包含后缀（至少需要"//0x"加8个十六进制字符）
	if (line.length() < 11)
	{
		return;
	}
	
	// 使用正则表达式在整行中查找"//0x"后跟8个十六进制字符
	std::regex suffixPattern("//0x([0-9A-Fa-f]{8})");
	std::smatch matches;
	
	if (std::regex_search(line, matches, suffixPattern))
	{
		// 将十六进制字符串转换为DWORD
		std::string hexStr = matches[1].str();
		std::stringstream ss;
		ss << std::hex << hexStr;
		ss >> suffix;
		
		// 从原字符串中删除整个后缀标记
		size_t pos = line.find(matches[0].str());
		if (pos != std::string::npos)
		{
			line.erase(pos, matches[0].length());
			
			// 删除后可能剩余的空白字符
			if (!line.empty() && pos > 0)
			{
				// 如果后缀前是空格，且删除后缀后这个空格成为行尾，则删除这个空格
				if (pos > 0 && line[pos-1] == ' ' && pos == line.length())
				{
					line.erase(pos-1, 1);
				}
			}
		}
	}
}

void CollectUnifiedDiffChunks(std::string& diffContent, std::string filePath,std::vector<UnifiedDiffChunk>& result)
{
	std::istringstream diffStream(diffContent);
	std::string line;

	// 正则表达式，用于匹配目标文件名（+++ 行）
	std::regex filePathRegex(u8"\\+\\+\\+ ([^\\t\\n]+)");

	// 准备处理多个chunk
	std::regex chunkHeaderRegex(u8"@@ -(\\d+),?(\\d*) \\+(\\d+),?(\\d*) @@");
	bool inTargetFile = false;

	while (std::getline(diffStream, line))
	{
// 		// 检查是否是新文件标记
// 		if (line.find(u8"+++ ") == 0)
// 		{
// 			std::smatch match;
// 			if (std::regex_search(line, match, filePathRegex))
// 			{
// 				std::string currentFile = match[1];
// 				if (currentFile.substr(0, 2) == u8"b/")
// 				{
// 					currentFile = currentFile.substr(2);
// 				}
// 				inTargetFile = (currentFile == filePath);
// 			}
// 			continue;
// 		}
// 
// 		// 如果不是当前处理的文件，跳过
// 		if (!inTargetFile)
// 		{
// 			continue;
// 		}

		// 解析chunk头
		std::smatch match;
		if (std::regex_search(line, match, chunkHeaderRegex))
		{
			int origStart = std::stoi(match[1]) - 1; // 0-based
			int origCount = match[2].length() > 0 ? std::stoi(match[2]) : 1;
			int newStart = std::stoi(match[3]) - 1;  // 0-based
			int newCount = match[4].length() > 0 ? std::stoi(match[4]) : 1;

			UnifiedDiffChunk chunk;
			chunk.filePath = filePath;

			// 收集chunk中的所有行
			std::string diffLine;
			while (std::getline(diffStream, diffLine) &&
				!diffLine.empty() &&
				diffLine[0] != '@')
			{
				char firstChar = diffLine[0];
				std::string content = diffLine.substr(1);

				UnifiedDiffChunk::DiffLine diff;

				if (firstChar == ' ') // 上下文行
				{
					diff.tp = UnifiedDiffChunk::DiffLine::NoChange;
					diff.line = content;
					CullLineSuffix(diff.line, diff.lineSuffix);
				}
				if (firstChar == '-') // 上下文行
				{
					diff.tp = UnifiedDiffChunk::DiffLine::Delete;
					diff.line = content;
					CullLineSuffix(diff.line, diff.lineSuffix);
				}
				if (firstChar == '+') // 上下文行
				{
					diff.tp = UnifiedDiffChunk::DiffLine::Add;
					diff.line = content;
				}
				chunk.diffLines.push_back(std::move(diff));
			}

			result.push_back(std::move(chunk));


			// 如果读取到了下一个chunk的头，把指针回退
			if (!diffLine.empty() && diffLine[0] == '@')
			{
				diffStream.seekg(-static_cast<int>(diffLine.length()) - 1, std::ios::cur);
			}
		}
	}
}

void CollectUnifiedDiffChunk(std::string& diffContent, std::string filePath, UnifiedDiffChunk& result)
{
	std::istringstream diffStream(diffContent);

	result.filePath = filePath;

	std::string diffLine;
	while (std::getline(diffStream, diffLine))
	{
		if (diffLine.empty())
			continue;
		char firstChar = diffLine[0];
		std::string content = diffLine.substr(1);

		UnifiedDiffChunk::DiffLine diff;

		if (firstChar == '-') 
		{
			diff.tp = UnifiedDiffChunk::DiffLine::Delete;
			diff.line = content;
			CullLineSuffix(diff.line, diff.lineSuffix);
		}
		else
		{
			if (firstChar == '+')
			{
				diff.tp = UnifiedDiffChunk::DiffLine::Add;
				diff.line = content;
				CullLineSuffix(diff.line, diff.lineSuffix);
			}
			else
			{
				// 上下文行
				diff.tp = UnifiedDiffChunk::DiffLine::NoChange;
				diff.line = content;
				CullLineSuffix(diff.line, diff.lineSuffix);
			}
		}

		result.diffLines.push_back(std::move(diff));
	}
}

bool LoadDiffTargetFile(const std::string& filePath, DiffTargetFile& file)
{
	std::string content;
	Utils::GetFileContentIntoUTF8(filePath.c_str(), content, file.codingFmt);

	// 如果文件内容为空，直接返回false
	if (content.empty())
	{
		return false;
	}

	// 清空目标文件对象
	file.lines.clear();

	// 按行分割文件内容
	std::istringstream contentStream(content);
	std::string line;

	// 逐行读取并添加到file中
	int lineNumber=1;
	while (std::getline(contentStream, line))
	{
		DiffTargetFile::Line newLine;
		newLine.line = line;
		newLine.suffix = encrypt(lineNumber);
		file.lines.push_back(std::move(newLine));

		lineNumber++;
	}

	return true;
}

bool SaveDiffTargetFile(std::string& filePath, DiffTargetFile& file)
{
	std::string content;

	// 将文件内容按行合并到content字符串中
	for (size_t i = 0; i < file.lines.size(); i++)
	{
		// 添加当前行内容
		content += file.lines[i].line;
		
		// 如果不是最后一行，添加换行符
		if (i < file.lines.size() - 1)
		{
			content += "\r\n";
		}
	}

	if (file.codingFmt == Utils::FileContentCodingFormat::Local)
		content = utf8_to_local(content);

	// 打开文件进行写入
	std::ofstream outFile;
	Utils::OpenOFStream(outFile, filePath.c_str());

	// 检查文件是否成功打开
	if (!outFile.is_open())
	{
		return false;
	}

	if (file.codingFmt == Utils::FileContentCodingFormat::Utf8_WithSignature)
	{
		const BYTE utf8_bom[] = { 0xEF, 0xBB, 0xBF };
		outFile.write((const char*) & utf8_bom[0], sizeof(utf8_bom));
	}

	// 写入内容到文件
	outFile.write(content.c_str(), content.size());

	// 检查写入是否成功
	if (outFile.fail())
	{
		outFile.close();
		return false;
	}

	// 关闭文件
	outFile.close();

	return true;

}

void ApplyUnifiedDiff(DiffTargetFile& file, UnifiedDiffChunk& chunk)
{
	int insertLine = -1;

	for (int i = 0;i < chunk.diffLines.size();i++)
	{
		const UnifiedDiffChunk::DiffLine& diffLine = chunk.diffLines[i];
		if (diffLine.tp == UnifiedDiffChunk::DiffLine::NoChange)
		{
			int idx=file.FindLine(diffLine.lineSuffix);
			if (idx >= 0)
			{
				insertLine = idx;
				break;
			}
		}
		if (diffLine.tp == UnifiedDiffChunk::DiffLine::Delete)
		{
			int idx = file.FindLine(diffLine.lineSuffix);
			if (idx >= 0)
			{
				insertLine = idx;
				break;
			}
		}
	}

	if (insertLine < 0)
		insertLine = (int)file.lines.size();

	for (int i = 0;i < chunk.diffLines.size();i++)
	{
		const UnifiedDiffChunk::DiffLine& diffLine = chunk.diffLines[i];
		if (diffLine.tp == UnifiedDiffChunk::DiffLine::NoChange)
		{
			int idx = file.FindLine(diffLine.lineSuffix);
			if (idx >= 0)
				insertLine = idx + 1;
		}
		if (diffLine.tp == UnifiedDiffChunk::DiffLine::Delete)
		{
			int idx = file.FindLine(diffLine.lineSuffix);
			if (idx >= 0)
			{
				file.RemoveLine(idx);
				insertLine = idx;
			}
		}
		if (diffLine.tp == UnifiedDiffChunk::DiffLine::Add)
		{
			file.InsertLine(insertLine, diffLine.line);
			insertLine++;
		}

	}
}