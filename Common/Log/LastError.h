// RunLogFile.h: interface for the C_RunLogFile class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

extern void SetLastErrString(const char *sErr,... );
extern const char *GetLastErrString();
extern void ClearLastErrString();

extern void SetLastErrCode(DWORD err);
extern DWORD GetLastErrCode();
