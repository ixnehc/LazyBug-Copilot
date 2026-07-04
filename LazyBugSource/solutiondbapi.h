#pragma once

#include "PipeMsg.h"
#include <string>
#include <memory>

#include "SolutionDBMsgs.h"

struct EmbedModelParam;

extern void SolutionDB_EnsureConnected();

extern void SolutionDB_Connect();
extern void SolutionDB_Disconnect();

extern SolutionDBMsg_Opened SolutionDB_Open(const char* slnPath);
extern void SolutionDB_QueryNameItems(const char* dbFolderPath,const char *query, SolutionDBMsg_NameItems&result);

extern bool SolutionDB_CollectRefs(const char* dbFolderPath, const CppSymbol::CollectRefsParam &collectRefsParam);
extern void SolutionDB_FindSymbolDefines(const char* dbFolderPath, const char* symbolName, int maxResult, SolutionDBMsg_SymbolDefines& result);

extern void SolutionDB_FindInFiles(const char* dbFolderPath, const char* keyword, int maxResults, SolutionDBMsg_FindInFilesResults& result);
extern void SolutionDB_SearchFile(const char* dbFolderPath, const char* keyword, int maxResults, SolutionDBMsg_SearchFileResult& result);

extern SolutionDBMsg_EmbeddingModelSet SolutionDB_SetEmbeddingModel(const char* dbFolderPath, const EmbedModelParam& modelParam);
extern SolutionDBMsg_ActivateFilesResult SolutionDB_ActivateFiles(const char* dbFolderPath, const std::vector<std::string>& filePaths);

