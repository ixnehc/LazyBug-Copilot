#include "stdh.h"

#include "FileChange.h"
#include "Utils.h"
#include "stringparser/stringparser.h"



void FileChange::DiscardContent()
{
	// 清空内容数据，保留元数据
	newContent.clear();
	oldContent.clear();
}

