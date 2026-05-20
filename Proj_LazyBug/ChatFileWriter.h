#pragma once

#include "Checkpoints.h"

#include "Utils.h"

enum class ChatRestoreMode
{
	UsingChangelists,
	UsingCheckpoints,
};

class CChangelists;
class CCheckpoints;
class CChatFileWriter
{
public:
	CChatFileWriter()
	{
	}



	bool Write(const char* filePath, const std::string& content,Utils::FileContentCodingFormat codingFmt);

protected:

	friend class CChatDialog;
	friend class CChatTask_FastApply;
};