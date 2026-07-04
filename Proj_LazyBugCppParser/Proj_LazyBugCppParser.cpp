// Proj_LazyBugCppParser.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "stdh.h"

#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <sys/stat.h> // 用于 stat 函数
#include <time.h>     // 用于 time_t 和 ctime 函数
#include <io.h>       // 用于 _setmode 和 _fileno
#include <fcntl.h>    // 用于 _O_BINARY
#include <stdio.h>    // 用于标准I/O函数
#include <fstream>
// 包含所需的头文件
#include "../Common/datapacket/DataPacket.h"
#include "../LazyBugSource/CppSymbolDefines.h"
#include "../LazyBugSource/SolutionDBDefines.h"
#include "../LazyBugSource/Utils_File.h"

#include <clang-c/Index.h>
#include <clang-c/CXString.h>
#include <clang-c/CXSourceLocation.h>
#include <clang-c/CXFile.h>

using namespace CppSymbol;


// 辅助函数：将十六进制字符串转换为二进制数据
std::vector<BYTE> HexStringToBytes(const std::string& hexStr)
{
	std::vector<BYTE> bytes;
	if (hexStr.size() % 2 != 0)
		return bytes;

	bytes.resize(hexStr.size() / 2);
	for (size_t i = 0; i < hexStr.size(); i += 2)
	{
		std::string hexByte = hexStr.substr(i, 2);
		bytes[i / 2] = (BYTE)strtol(hexByte.c_str(), NULL, 16);
	}
	return bytes;
}


int main(int argc, char* argv[])
{
    try
    {
        // 检查命令行参数
        if (argc != 2)
        {
            std::cerr << "Usage: Proj_LazyBugCppParser.exe \"d<hex_encoded_request>\" or \"f<temp_file_path>\"" << std::endl;
            return 1;
        }

		ParseResult result;

		result.AddDebugTime("C-A");

        std::string arg = argv[1];
        std::vector<BYTE> requestBytes;

        if (arg.size() >= 2 && arg[0] == 'd')
        {
            // 直接传递十六进制字符串格式: "d<hex_encoded_request>"
            std::string hexRequest = arg.substr(1);
            requestBytes = HexStringToBytes(hexRequest);
            if (requestBytes.empty())
            {
                std::cerr << "Error: Invalid hex data" << std::endl;
                return 1;
            }
        }
        else if (arg.size() >= 2 && arg[0] == 'f')
        {
            // 通过临时文件传递格式: "f<temp_file_path>"
            std::string tempFilePath = arg.substr(1);
            
            // 读取临时文件内容
            std::ifstream file;
			Utils::OpenIFStream(file, tempFilePath.c_str());

            if (!file.is_open())
            {
                std::cerr << "Error: Cannot open temp file: " << tempFilePath << std::endl;
                return 1;
            }
            
            file.seekg(0, std::ios::end);
            size_t fileSize = file.tellg();
            file.seekg(0, std::ios::beg);
            
            requestBytes.resize(fileSize);
            file.read(reinterpret_cast<char*>(requestBytes.data()), fileSize);
            
            if (!file)
            {
                std::cerr << "Error: Failed to read temp file: " << tempFilePath << std::endl;
                return 1;
            }
        }
        else
        {
            std::cerr << "Error: Invalid command format. Use \"d<hex>\" or \"f<file>\"" << std::endl;
            return 1;
        }


        // 创建ProjSetting对象
        ProjSetting setting;
        
        // 反序列化ParseRequest
        ParseRequest request;
        CDataPacket dpRequest;
        dpRequest.SetDataBufferPointer(requestBytes.data());
        request.Load(dpRequest, setting);

		result.AddDebugTime("C-B");

        // 处理请求
		extern void ProcessRequest(const ParseRequest & request, ParseResult & result);
		extern void ProcessRequest_CollectRef(const ParseRequest & request, ParseResult & result);
		if (request.collectRefParam.IsValid())
			ProcessRequest_CollectRef(request, result);
		else
			ProcessRequest(request, result);

		result.AddDebugTime("C-C");

        // 序列化结果
        std::vector<BYTE> resultBuffer;
        {
            CDataPacket dp;
            DP_BeginSave(dp, resultBuffer);
            result.Save(dp);
            DP_EndSave();
        }

        // 设置stdout为二进制模式
        _setmode(_fileno(stdout), _O_BINARY);
        
        // 直接输出二进制数据到stdout
        std::cout.write(reinterpret_cast<const char*>(resultBuffer.data()), resultBuffer.size());
        std::cout.flush();  // 强制刷新缓冲区


        return 0;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    catch (...)
    {
        std::cerr << "Unknown error occurred" << std::endl;
        return 1;
    }
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
