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

	Utils::SetFileContentFromUTF8(filePath, content, codingFmt);

	return true;
}
