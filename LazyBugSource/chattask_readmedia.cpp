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
	if (ext == "tiff" || ext == "tif") return "image/tiff";
	if (ext == "ico")  return "image/x-icon";
	if (ext == "dds")  return "image/x-dds";
	if (ext == "tga")  return "image/x-tga";
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
	return _GetMimeType(ext) != nullptr;
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

	if (!_IsSupportedImage(ext))
	{
		std::lock_guard<std::mutex> lock(_resultMutex);
		_threadResult = "Error: Unsupported media format: '" + ext + "'. Currently supported image formats: jpg, jpeg, png, webp, bmp, gif, tiff, ico, dds, tga";
		_threadMessage = "ReadMedia: unsupported format \"" + ext + "\"";
		_threadSuccess = false;
		_threadFinished = true;
		return;
	}

	// 获取可选参数
	int maxWidth = 0;
	int maxHeight = 0;
	bool hasMaxSize = false;

	if (_toolCall.ExistParam("maxWidth") && _toolCall.ExistParam("maxHeight"))
	{
		_toolCall.GetIntParam("maxWidth", maxWidth);
		_toolCall.GetIntParam("maxHeight", maxHeight);
		hasMaxSize = (maxWidth > 0 && maxHeight > 0);
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

	// 读取图片为 base64（自动转换不支持的格式为 PNG/JPEG）
	std::string fullBase64;
	std::string fullMime;
	bool readSuccess = Utils::LoadImageIntoBase64(
		filePath.c_str(),
		hasMaxSize ? maxWidth : 0,
		hasMaxSize ? maxHeight : 0,
		fullBase64, fullMime);

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
		// 获取原图尺寸用于 token 估算
		int imgWidth = 0, imgHeight = 0;
		Utils::GetImageSize(filePath.c_str(), imgWidth, imgHeight);

		// 构建 content block 数组（text + image_url）
		auto buildImageResult = [&](const std::string& base64, const std::string& mime) -> std::string
		{
			nlohmann::json arr = nlohmann::json::array();

			nlohmann::json textBlock;
			textBlock["type"] = "text";
			textBlock["text"] = "Image read: " + filePath + " (" + std::to_string(imgWidth) + "x" + std::to_string(imgHeight) + ")";
			arr.push_back(textBlock);

			nlohmann::json imageBlock;
			imageBlock["type"] = "image_url";
			imageBlock["image_url"]["url"] = "data:" + mime + ";base64," + base64;
			arr.push_back(imageBlock);

			return arr.dump();
		};

		// 全尺寸结果
		resultStr = buildImageResult(fullBase64, fullMime);

		// Simple result: 始终生成固定小缩略图用作 partial
		{
			std::string thumbBase64;
			std::string thumbMime;
			if (Utils::LoadImageIntoBase64(filePath.c_str(), 256, 256, thumbBase64, thumbMime) && !thumbBase64.empty())
				resultStrSimple = buildImageResult(thumbBase64, thumbMime);
			else
				resultStrSimple = resultStr.substr(0, std::min<size_t>(resultStr.size(), 512));
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
