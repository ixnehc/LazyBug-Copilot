#include "stdh.h"

//#include "Changelists.h"

#include "ChatFileWriter.h"

#include "Utils.h"

ChatRestoreMode GetChatRestoreMode()
{
	return ChatRestoreMode::UsingCheckpoints;
}

bool CChatFileWriter::Write(const char* filePath, const std::string& content, Utils::FileContentCodingFormat codingFmt)
{
	const int maxRetries = 3;
	const int retryDelayMs = 200;

	for (int i = 0; i < maxRetries; ++i)
	{
		if (Utils::SetFileContentFromUTF8(filePath, content, codingFmt))
			return true;

		if (i < maxRetries - 1)
			Sleep(retryDelayMs);
	}

	return false;
}
