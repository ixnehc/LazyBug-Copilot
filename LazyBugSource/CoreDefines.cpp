#include "stdh.h"

#include "CoreDefines.h"

#include "datapacket/DataPacket.h" 
  
//////////////////////////////////////////////////////////////////////////
//FindInFileResults
void FindInFileResults::Save(CDataPacket& dp) const
{
	// 保存文件信息数量
	DWORD fileCount = (DWORD)fileInfos.size();
	dp.Data_NextDword() = fileCount;

	// 保存每个文件的信息
	for (const auto& fileInfo : fileInfos)
	{
		// 保存文件路径
		dp.Data_WriteString(fileInfo.filePath);

		// 保存该文件中的行信息数量
		DWORD lineCount = (DWORD)fileInfo.lineInfos.size();
		dp.Data_NextDword() = lineCount;

		// 保存每个行信息
		for (const auto& lineInfo : fileInfo.lineInfos)
		{
			// 保存行号
			dp.Data_NextInt() = lineInfo.lineNumber;

			// 保存行内容
			dp.Data_WriteString(lineInfo.lineContent);

			// 保存符号名
			dp.Data_WriteString(lineInfo.symbolName);
		}
	}
}

void FindInFileResults::Load(CDataPacket& dp)
{
	// 清空现有数据
	Clear();

	// 读取文件信息数量
	DWORD fileCount = dp.Data_NextDword();

	// 读取每个文件的信息
	for (DWORD i = 0; i < fileCount; i++)
	{
		FileInfo fileInfo;

		// 读取文件路径
		dp.Data_ReadString(fileInfo.filePath);

		// 读取该文件中的行信息数量
		DWORD lineCount = dp.Data_NextDword();

		// 读取每个行信息
		for (DWORD j = 0; j < lineCount; j++)
		{
			FileLineInfo lineInfo;

			// 读取行号
			lineInfo.lineNumber = dp.Data_NextInt();

			// 读取行内容
			dp.Data_ReadString(lineInfo.lineContent);

			// 读取符号名
			dp.Data_ReadString(lineInfo.symbolName);

			fileInfo.lineInfos.push_back(lineInfo);
		}

		fileInfos.push_back(fileInfo);
	}
}

//////////////////////////////////////////////////////////////////////////
//SearchFileResult
void SearchFileResult::Save(CDataPacket& dp) const
{
	// 保存文件信息数量
	DWORD fileCount = (DWORD)fileInfos.size();
	dp.Data_NextDword() = fileCount;

	// 保存每个文件的信息
	for (const auto& fileInfo : fileInfos)
	{
		// 保存文件路径
		dp.Data_WriteString(fileInfo.filePath);
	}
}

void SearchFileResult::Load(CDataPacket& dp)
{
	// 清空现有数据
	fileInfos.clear();

	// 读取文件信息数量
	DWORD fileCount = dp.Data_NextDword();

	// 读取每个文件的信息
	for (DWORD i = 0; i < fileCount; i++)
	{
		FileInfo fileInfo;

		// 读取文件路径
		dp.Data_ReadString(fileInfo.filePath);

		fileInfos.push_back(fileInfo);
	}
}

