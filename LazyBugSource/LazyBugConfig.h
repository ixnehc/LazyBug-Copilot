#pragma once

#define EVERYTHING_EXENAME "everything.exe"

#ifdef LAZYBUG_RETAIL

#define LAZYBUG_CPP_PARSER_EXENAME "LazyBugCppParser.exe"
#define LAZYBUG_SERVICE_EXENAME "LazyBugService.exe"

#define SOLUTIONDB_SERVICE_PIPE_NAME_REQUEST L"\\\\.\\pipe\\LazyBugSolutionDBService_Request"
#define SOLUTIONDB_SERVICE_PIPE_NAME_RESPONSE L"\\\\.\\pipe\\LazyBugSolutionDBService_Response"

#define LAZYBUG_DB_FOLDER "LazyBugDB"
#define LAZYBUG_WEBVIEW_USERFOLDER L"LazyBugDB\\_webview"

#define LAZYBUG_CHATINPUT_HISTORY "LazyBugChatInputHistory"

#else

#define LAZYBUG_CPP_PARSER_EXENAME "LazyBugCppParser_RL.exe"
#define LAZYBUG_SERVICE_EXENAME "LazyBugService_RL.exe"

#define SOLUTIONDB_SERVICE_PIPE_NAME_REQUEST L"\\\\.\\pipe\\LazyBugSolutionDBService_Request_RL"
#define SOLUTIONDB_SERVICE_PIPE_NAME_RESPONSE L"\\\\.\\pipe\\LazyBugSolutionDBService_Response_RL"

#define LAZYBUG_DB_FOLDER "LazyBugDB_RL"
#define LAZYBUG_WEBVIEW_USERFOLDER L"LazyBugDB_RL\\_webview"

#define LAZYBUG_CHATINPUT_HISTORY "LazyBugChatInputHistory_RL"

#endif

#define LAZYBUG_CLI_WHITELIST_FILENAME "cli_whitelist.txt"

#define LAZYBUG_PINYIN_LIB_FILENAME "pinyin_lib.txt"

#define LAZYBUG_GLOBAL_CONTEXT_FILENAME "global_rules.md"
#define LAZYBUG_PROJECT_CONTEXT_FILENAME "project_rules.md"