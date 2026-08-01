#include "stdh.h"
#include "ChatTask_ReadMedia.h"
#include "Utils.h"
#include "Utils_Image.h"

#include "LlmChat.h"
#include "LlmLib.h"


// 获取扩展名对应的 MIME type
const char* CChatTask_ReadMedia::_GetMimeType(const std::string& ext)
{
	if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
	if (ext == "png")  return "image/png";
	if (ext == "webp") return "image/webp";
	if (ext == "bmp")  return "image/bmp";
	if (ext == "gif")  return "image/gif";
	// 未来扩展视频:
	// if (ext == "mp4")  return "video/mp4";
	// if (ext == "avi")  return "video/x-msvideo";
	// if (ext == "mkv")  return "video/x-matroska";
	// if (ext == "mov")  return "video/quicktime";
	// if (ext == "webm") return "video/webm";
	return nullptr;
}

bool CChatTask_ReadMedia::_IsSupportedImage(const std::string& ext)
{
	return (ext == "jpg" || ext == "jpeg" || ext == "png" || ext == "webp" || ext == "bmp" || ext == "gif");
}


CChatTask_ReadMedia::CChatTask_ReadMedia()
{
	_workerThread = nullptr;
	_shouldStop = false;
	_threadFinished = false;
	_threadSuccess = false;
}

CChatTask_ReadMedia::~CChatTask_ReadMedia()
{
	Interrupt();
}

bool CChatTask_ReadMedia::DependsOn(CChatTask* task0)
{
	if (!task0->CheckType("ReadMedia"))
		return false;
	return true;
}

void CChatTask_ReadMedia::_Fail()
{
	_status = TaskStatus::Failure;
}

void CChatTask_ReadMedia::_Succeed()
{
	_status = TaskStatus::Success;
}

void CChatTask_ReadMedia::_ThreadFunc()
{
	// 获取文件路径
	std::string filePath;
	if (!_toolCall.GetStringParam("filePath", filePath))
	{
		std::lock_guard<std::mutex> lock(_resultMutex);
		_threadResult = "Error: Missing filePath parameter";
		_threadMessage = "ReadMedia: Missing filePath parameter";
		_threadSuccess = false;
		_threadFinished = true;
		return;
	}

	FixSlashInPath_Utf8((char*)filePath.c_str());

	// 检查路径是否为完全路径
	if (!IsFullPath(filePath.c_str()))
	{
		std::lock_guard<std::mutex> lock(_resultMutex);
		_threadResult = "Error: filePath must be a full path: '" + filePath + "'";
		_threadMessage = "ReadMedia: path is not a full path";
		_threadSuccess = false;
		_threadFinished = true;
		return;
	}

	// 获取扩展名并校验
	std::string ext = GetFileSuffix(filePath);
	StringLower(ext);

	const char* mimeType = _GetMimeType(ext);
	if (!mimeType)
	{
		std::lock_guard<std::mutex> lock(_resultMutex);
		_threadResult = "Error: Unsupported media format: '" + ext + "'. Currently supported image formats: jpg, jpeg, png, webp, bmp, gif";
		_threadMessage = "ReadMedia: unsupported format \"" + ext + "\"";
		_threadSuccess = false;
		_threadFinished = true;
		return;
	}

	// 获取可选参数
	int maxWidth = 0;
	int maxHeight = 0;
	bool hasThumbnailParams = false;

	if (_toolCall.ExistParam("maxWidth") && _toolCall.ExistParam("maxHeight"))
	{
		_toolCall.GetIntParam("maxWidth", maxWidth);
		_toolCall.GetIntParam("maxHeight", maxHeight);
		hasThumbnailParams = (maxWidth > 0 && maxHeight > 0);
	}

	// 检查是否被中断
	if (_shouldStop)
	{
		std::lock_guard<std::mutex> lock(_resultMutex);
		_threadResult = "Task interrupted";
		_threadMessage = "ReadMedia: interrupted";
		_threadSuccess = false;
		_threadFinished = true;
		return;
	}

	// 读取完整图片的 base64
	std::string fullBase64;
	bool readSuccess = Utils::GetFileContentIntoBase64(filePath.c_str(), fullBase64);

	// 再次检查是否被中断
	if (_shouldStop)
	{
		std::lock_guard<std::mutex> lock(_resultMutex);
		_threadResult = "Task interrupted";
		_threadMessage = "ReadMedia: interrupted";
		_threadSuccess = false;
		_threadFinished = true;
		return;
	}

	// 构建返回结果
	std::string resultStr;
	std::string resultStrSimple;
	std::string messageStr;

	if (!readSuccess)
	{
		resultStr = "Error: Failed to read media file: '" + filePath + "'";
		resultStrSimple = resultStr;
		messageStr = "Failed to read media file: \"" + filePath + "\"";
	}
	else if (fullBase64.empty())
	{
		resultStr = "Error: Media file is empty: '" + filePath + "'";
		resultStrSimple = resultStr;
		messageStr = "Read empty media file: \"" + filePath + "\"";
	}
	else
	{
		// 构建 data URI
		resultStr = "data:" + std::string(mimeType) + ";base64," + fullBase64;

		// Simple result: 生成缩略图 base64
		if (hasThumbnailParams)
		{
			std::string thumbBase64;
			if (Utils::LoadImageThumbnailIntoBase64(filePath.c_str(), maxWidth, maxHeight, thumbBase64) && !thumbBase64.empty())
			{
				resultStrSimple = "data:" + std::string(mimeType) + ";base64," + thumbBase64;
			}
			else
			{
				// 缩略图生成失败，回退到截断原 base64（取前 512 字符）
				resultStrSimple = resultStr.substr(0, std::min<size_t>(resultStr.size(), 512));
			}
		}
		else
		{
			// 未指定缩略图参数，使用默认缩略图尺寸
			std::string thumbBase64;
			if (Utils::LoadImageThumbnailIntoBase64(filePath.c_str(), 256, 256, thumbBase64) && !thumbBase64.empty())
			{
				resultStrSimple = "data:" + std::string(mimeType) + ";base64," + thumbBase64;
			}
			else
			{
				resultStrSimple = resultStr.substr(0, std::min<size_t>(resultStr.size(), 512));
			}
		}

		messageStr = "Successfully read media file: \"" + filePath + "\"";
	}

	// 保存结果
	std::lock_guard<std::mutex> lock(_resultMutex);
	_threadResult = resultStr;
	_threadResultSimple = resultStrSimple;
	_threadMessage = messageStr;
	_threadSuccess = readSuccess;
	_threadFinished = true;
}

void CChatTask_ReadMedia::Start()
{
	_status = TaskStatus::Running;

	// 重置状态
	_shouldStop = false;
	_threadFinished = false;
	_threadSuccess = false;
	_threadResult.clear();
	_threadResultSimple.clear();
	_threadMessage.clear();

	// 启动工作线程
	_workerThread = new std::thread(&CChatTask_ReadMedia::_ThreadFunc, this);
}

void CChatTask_ReadMedia::Update()
{
	if (_status != TaskStatus::Running)
		return;

	// 检查线程是否完成
	if (_threadFinished)
	{
		// 等待线程结束
		if (_workerThread && _workerThread->joinable())
		{
			_workerThread->join();
		}

		// 获取结果并发送
		{
			std::lock_guard<std::mutex> lock(_resultMutex);
			_SendToolCallResult(_threadResult.c_str(), _threadResultSimple.c_str());
			_SendToolCallMessage_Exploring(_threadMessage.c_str());
		}

		// 清理线程
		if (_workerThread)
		{
			delete _workerThread;
			_workerThread = nullptr;
		}

		// 设置最终状态
		if (_threadSuccess)
			_Succeed();
		else
			_Fail();
	}
}

void CChatTask_ReadMedia::Interrupt()
{
	// 设置停止标志
	_shouldStop = true;

	// 等待线程结束
	if (_workerThread && _workerThread->joinable())
	{
		_workerThread->join();
	}

	// 清理线程
	if (_workerThread)
	{
		delete _workerThread;
		_workerThread = nullptr;
	}

	_status = TaskStatus::Failure;
}
