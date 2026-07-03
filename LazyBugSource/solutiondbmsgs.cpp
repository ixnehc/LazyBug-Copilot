#include "stdh.h"
#include "SolutionDBMsgs.h"

PipeMsgPtr CreateSolutionDBMsg(PipeMsgType type)
{
	switch ((SolutionDBMsgType)type)
	{
	case SolutionDBMsgType::RequestOpen:
		return std::make_unique<SolutionDBMsg_RequestOpen>();
	case SolutionDBMsgType::Opened:
		return std::make_unique<SolutionDBMsg_Opened>();
	case SolutionDBMsgType::QueryNameItems:
		return std::make_unique<SolutionDBMsg_QueryNameItems>();
	case SolutionDBMsgType::NameItems:
		return std::make_unique<SolutionDBMsg_NameItems>();
	case SolutionDBMsgType::CollectRefs:
		return std::make_unique<SolutionDBMsg_CollectRefs>();
	case SolutionDBMsgType::Refs:
		return std::make_unique<SolutionDBMsg_Refs>();
	case SolutionDBMsgType::FindSymbolDefine:
		return std::make_unique<SolutionDBMsg_FindSymbolDefine>();
	case SolutionDBMsgType::SymbolDefineLocations:
		return std::make_unique<SolutionDBMsg_SymbolDefines>();
	case SolutionDBMsgType::FindInFiles:
		return std::make_unique<SolutionDBMsg_FindInFiles>();
	case SolutionDBMsgType::FindInFilesResults:
		return std::make_unique<SolutionDBMsg_FindInFilesResults>();
	case SolutionDBMsgType::SearchFile:
		return std::make_unique<SolutionDBMsg_SearchFile>();
	case SolutionDBMsgType::SearchFileResults:
		return std::make_unique<SolutionDBMsg_SearchFileResult>();
	case SolutionDBMsgType::SetEmbeddingModel:
		return std::make_unique<SolutionDBMsg_SetEmbeddingModel>();
	case SolutionDBMsgType::EmbeddingModelSet:
		return std::make_unique<SolutionDBMsg_EmbeddingModelSet>();
	case SolutionDBMsgType::ActivateFiles:
		return std::make_unique<SolutionDBMsg_ActivateFiles>();
	case SolutionDBMsgType::ActivateFilesResult:
		return std::make_unique<SolutionDBMsg_ActivateFilesResult>();
	//XXXXX: more SolutionDB message
	default:
		always_assert(false);
		return nullptr;
	}
}
