/*
 Copyright (c) 2001 - 2002
 Author: Konstantin Boukreev
 E-mail: konstantin@mail.primorye.ru
 Created: 25.12.2001 19:41:07
 Version: 1.0.2

 Permission to use, copy, modify, distribute and sell this software
 and its documentation for any purpose is hereby granted without fee,
 provided that the above copyright notice appear in all copies and
 that both that copyright notice and this permission notice appear
 in supporting documentation.  Konstantin Boukreev makes no representations
 about the suitability of this software for any purpose.
 It is provided "as is" without express or implied warranty.

*/

#include "stdh.h"
#include "sym_engine.h"

#include <crtdbg.h>
#include <malloc.h>
#include <tlhelp32.h>
#include <tchar.h>

#include "../Log/LogFile.h"
#include "../stringparser/stringparser.h"

#pragma comment (lib, "dbghelp")

#ifdef VERIFY
#undef VERIFY
#endif // VERIFY

#ifdef _DEBUG
#define VERIFY(x) _ASSERTE(x)
#else
#define VERIFY(x) (x)
#endif //_DEBUG

#ifdef _DEBUG
 #define SYM_ENGINE_TRACE_SPIN_COUNT
#endif //_DEBUG

#define WORK_AROUND_SRCLINE_BUG

///////////////////////////////////////////////////////////////////////
#include <iostream>
#include <fstream>
#include <ostream>
//#include "LogStream.h"


bool IsNT()
{
	OSVERSIONINFO vi = { sizeof(vi)};
	::GetVersionEx(&vi);
	return vi.dwPlatformId == VER_PLATFORM_WIN32_NT;
}

HANDLE GetSymEngineHandle()
{
	if (IsNT())
		return GetCurrentProcess();
	else
		return (HANDLE)((char*)0+GetCurrentProcessId());
}

BOOL __stdcall My_ReadProcessMemory (HANDLE, LPCVOID lpBaseAddress, LPVOID lpBuffer, DWORD nSize, LPDWORD lpNumberOfBytesRead)
{
    return ReadProcessMemory(GetCurrentProcess(), lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesRead);
}
static sym_engine g_sym_engine(0);

static LogFile g_log("");

/*-------------------------------------------------------------------
*	class sym_engine
*/
sym_engine::sym_engine (uInt address)
	: m_address(address), m_ok(g_sym_engine.check()), m_pframe(0)
{
}

sym_engine::~sym_engine()
{
	delete m_pframe;
}

uInt sym_engine::module(char * const buf, cuInt len)
{
	if (!len || !buf || IsBadWritePtr(buf, len))
		return 0;

	if (!g_sym_engine.check())
		return 0;

	HANDLE hProc = GetSymEngineHandle();
	HMODULE hMod = (HMODULE)((char*)0+SymGetModuleBase (hProc, m_address));
	if (!hMod) return 0;
	return get_module_basename(hMod, buf, len);
}

uInt sym_engine::symbol(char * const buf, cuInt len, uLong * const pdisplacement)
{
	if (!len || !buf ||
		IsBadWritePtr(buf, len) ||
		(pdisplacement && IsBadWritePtr(pdisplacement, sizeof(uInt))))
		return 0;

	if (!g_sym_engine.check())
		return 0;

	BYTE symbol [ 512 ] ;
	PIMAGEHLP_SYMBOL pSym = (PIMAGEHLP_SYMBOL)&symbol;
	memset(pSym, 0, sizeof(symbol));
	pSym->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
    pSym->MaxNameLength = sizeof(symbol) - sizeof(IMAGEHLP_SYMBOL);

	HANDLE hProc = GetSymEngineHandle();
	uLong displacement = 0;
	int r = SymGetSymFromAddr(hProc, m_address, &displacement, pSym);
	if (!r) return 0;
	if (pdisplacement)
		*pdisplacement = displacement;

	r = _snprintf(buf, len, "%s()", pSym->Name);

	r = r == -1 ? len - 1 : r;
	buf[r] = 0;	
	return r;
}

uInt sym_engine::fileline (char * const buf, cuInt len, uLong * const pline, uLong * const pdisplacement)
{
	if (!len || !buf ||
		IsBadWritePtr(buf, len) ||
		(pline && IsBadWritePtr(pline, sizeof(uInt))) ||
		(pdisplacement && IsBadWritePtr(pdisplacement, sizeof(uInt))))
		return 0;

	if (!g_sym_engine.check())
		return 0;

	IMAGEHLP_LINE img_line;
	memset(&img_line, 0, sizeof(IMAGEHLP_LINE));
	img_line.SizeOfStruct = sizeof(IMAGEHLP_LINE);

	HANDLE hProc = GetSymEngineHandle();
	uLong displacement = 0;
	if (!get_line_from_addr(hProc, m_address, &displacement, &img_line))
		return 0;
	if (pdisplacement)
		*pdisplacement = displacement;
	if (pline)
		*pline = img_line.LineNumber;
	lstrcpynA(buf, img_line.FileName, len);
	return lstrlenA(buf);
}

bool sym_engine::get_line_from_addr (HANDLE hProc, cuInt addr, uLong * const pdisplacement, IMAGEHLP_LINE * const pLine)
{
	#ifdef WORK_AROUND_SRCLINE_BUG

	// "Debugging Applications" John Robbins
    // The problem is that the symbol engine finds only those source
    // line addresses (after the first lookup) that fall exactly on
    // a zero displacement. I'll walk backward 100 bytes to
    // find the line and return the proper displacement.
    uLong displacement = 0 ;
	while (!SymGetLineFromAddr (hProc, addr - displacement, ((pdisplacement?((*pdisplacement)=0):0),pdisplacement), pLine))
    {
        if ((++displacement) >=100)
            return false;
    }

	// "Debugging Applications" John Robbins
    // I found the line, and the source line information is correct, so
    // change the displacement if I had to search backward to find the source line.
    if (pdisplacement)
        (*pdisplacement) |= (displacement*(1L<<24));
    return true;

	#else
    return 0 != SymGetLineFromAddr (hProc, addr, (DWORD *) pdisplacement, pLine);
	#endif
}

 
bool sym_engine::stack_first (int loglevl,CONTEXT* const pctx)
{
	if (!pctx)
	{
		g_log.Dump("pctx = NULL, stack_first return false!");
		return false;
	}
	if(IsBadReadPtr(pctx, sizeof(CONTEXT)))
	{
		g_log.Dump("bad Read Ptr, stack_first return false!");
		return false;
	}
	if (!g_sym_engine.check())
	{
		g_log.Dump("check() = false, stack_first return false!");
		return false;
	}
	if (!m_pframe)
	{
		m_pframe = new STACKFRAME;
		if (!m_pframe)
		{
			g_log.Dump("m_pframe = NULL, stack_first return false!");
			return false;
		}
	}

	memset(m_pframe, 0, sizeof(STACKFRAME));

//    #ifdef _X86_
    m_pframe->AddrPC.Offset       = pctx->Eip;
    m_pframe->AddrPC.Mode         = AddrModeFlat;
    m_pframe->AddrStack.Offset    = pctx->Esp;
    m_pframe->AddrStack.Mode      = AddrModeFlat;
    m_pframe->AddrFrame.Offset    = pctx->Ebp;
    m_pframe->AddrFrame.Mode      = AddrModeFlat;
//     #else
//     m_pframe->AddrPC.Offset       = (DWORD)pctx->Fir;
//     m_pframe->AddrPC.Mode         = AddrModeFlat;
//     m_pframe->AddrReturn.Offset   = (DWORD)pctx->IntRa;
//     m_pframe->AddrReturn.Mode     = AddrModeFlat;
//     m_pframe->AddrStack.Offset    = (DWORD)pctx->IntSp;
//     m_pframe->AddrStack.Mode      = AddrModeFlat;
//     m_pframe->AddrFrame.Offset    = (DWORD)pctx->IntFp;
//     m_pframe->AddrFrame.Mode      = AddrModeFlat;
//     #endif

	m_pctx = pctx;
	return stack_next(loglevl);
}

bool sym_engine::stack_next(int loglevl)
{
	if (!m_pframe) 
	{
		g_log.Dump("m_pframe =NULL, stack_next return false!");
		return false;
	}
	if(!m_pctx)
	{
		g_log.Dump("m_pctx =NULL, stack_next return false!");
		return false;
	}

	if (!g_sym_engine.m_ok)
	{
		g_log.Dump("g_sym_engine.m_ok =false, stack_next return false!");
		return false;
	}


	SetLastError(0);
	HANDLE hProc = GetSymEngineHandle();
	BOOL r = StackWalk (IMAGE_FILE_MACHINE_I386,hProc,GetCurrentThread(),m_pframe,m_pctx,(PREAD_PROCESS_MEMORY_ROUTINE)My_ReadProcessMemory,SymFunctionTableAccess,SymGetModuleBase,0);

	if (!r)
	{
		//LogLine l_ln(g_dbclog);
		//l_ln<< newlv(loglevl) << "StackWalk =false, stack_next return false!";
		return false;
	}
	if(!m_pframe->AddrFrame.Offset)
	{
		g_log.Dump("m_pframe->AddrFrame.Offset =0, stack_next return false!");
		return false;
	}

	// "Debugging Applications" John Robbins
	// Before I get too carried away and start calculating
	// everything, I need to double-check that the address returned
	// by StackWalk really exists. I've seen cases in which
	// StackWalk returns TRUE but the address doesn't belong to
	// a module in the process.

	DWORD dwModBase = SymGetModuleBase (hProc, m_pframe->AddrPC.Offset);
	if (!dwModBase)
	{
		g_log.Dump("SymGetModuleBase =0, stack_next return false!");
		return false;
	}

	address(m_pframe->AddrPC.Offset);
	return true;
}

uInt sym_engine::get_module_basename (HMODULE hMod, char * const buf, uInt len)
{
	char filename[MAX_PATH];
	DWORD r = GetModuleFileNameA(hMod, filename, MAX_PATH);
	if (!r) return 0;

	char * p = 0;

	// Find the last '\' mark.
	int i = r - 1;
	for (; i >= 0; i--)
	{
		if (filename[i] == '\\')
		{
			p = &filename[i + 1];
			break;
		}
	}

	if (!p)
	{
		i = 0;
		p = filename;
	}

	len = (len - 1 < r - i - 1) ? len - 1 : r - i - 1;
//	len = min(len - 1, r - i - 1);
	memcpy(buf, p, len);
	buf[len] = 0;
	return len;
}

bool sym_engine::check()
{
	if (!g_sym_engine.m_ok)g_sym_engine.m_ok =guard::instance().init();
	return g_sym_engine.m_ok;
}
/*-------------------------------------------------------------------
*	class sym_engine::guard
*/
sym_engine::guard & sym_engine::guard::instance()
{
	static guard g;
	return g;
}

sym_engine::guard::guard()
	: m_ref(0)
{}

sym_engine::guard::~guard()
{
	clear();
}

bool sym_engine::guard::init()
{
	if (!m_ref)
	{
		m_ref = -1;

		HANDLE hProc = GetSymEngineHandle();
		DWORD  dwPid = GetCurrentProcessId();

		// initializes
		SymSetOptions (SymGetOptions()|SYMOPT_DEFERRED_LOADS|SYMOPT_LOAD_LINES);
	//	SymSetOptions (SYMOPT_UNDNAME|SYMOPT_LOAD_LINES);
		if (!::SymInitialize(hProc, 0, TRUE))_ASSERTE(0);
		else
		{
			// enumerate modules
			if (IsNT())
			{
				typedef BOOL (WINAPI *ENUMPROCESSMODULES)(HANDLE, HMODULE*, DWORD, LPDWORD);

				HINSTANCE hInst = LoadLibrary(_T("psapi.dll"));
				if (hInst)
				{
					ENUMPROCESSMODULES fnEnumProcessModules =(ENUMPROCESSMODULES)GetProcAddress(hInst, "EnumProcessModules");
					DWORD cbNeeded = 0;
					if (fnEnumProcessModules &&	fnEnumProcessModules(GetCurrentProcess(), 0, 0, &cbNeeded) && cbNeeded)
					{
						HMODULE * pmod = (HMODULE *)alloca(cbNeeded);
						DWORD cb = cbNeeded;
						if (fnEnumProcessModules(GetCurrentProcess(), pmod, cb, &cbNeeded))
						{
							m_ref = 0;
							for (uInt i = 0; i < cb / sizeof (HMODULE); ++i)
							{
								if (!load_module(hProc, pmod[i]))
								{
								//	m_ref = -1;
								//	break;
									_ASSERTE(0);
								}
							}
						}
					}else
					{
						_ASSERTE(0);
					}
					VERIFY(FreeLibrary(hInst));
				}else
				{
					_ASSERTE(0);
				}
			}else
			{
				typedef HANDLE (WINAPI *CREATESNAPSHOT)(DWORD, DWORD);
				typedef BOOL (WINAPI *MODULEWALK)(HANDLE, LPMODULEENTRY32);

				HMODULE hMod = GetModuleHandle(_T("kernel32"));
				CREATESNAPSHOT fnCreateToolhelp32Snapshot = (CREATESNAPSHOT)GetProcAddress(hMod, "CreateToolhelp32Snapshot");
				MODULEWALK fnModule32First = (MODULEWALK)GetProcAddress(hMod, "Module32First");
				MODULEWALK fnModule32Next  = (MODULEWALK)GetProcAddress(hMod, "Module32Next");

				if (fnCreateToolhelp32Snapshot && fnModule32First && fnModule32Next)
				{
					HANDLE hModSnap = fnCreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwPid);
					if (hModSnap)
					{
						MODULEENTRY32 me32 = {0};
						me32.dwSize = sizeof(MODULEENTRY32);
						if (fnModule32First(hModSnap, &me32))
						{
							m_ref = 0;
							do{
								if (!load_module(hProc, me32.hModule))
								{
								//	m_ref = -1;
								//	break;
								}
							}while(fnModule32Next(hModSnap, &me32));
						}
						VERIFY(CloseHandle(hModSnap));
					}
				}
			}

			if (m_ref == -1)
			{
				VERIFY(SymCleanup(GetSymEngineHandle()));
			}
		}
	}
	if (m_ref == -1)
		return false;
	if (m_ref == 0)
		++m_ref; //lock it once
	//++m_ref;
	return true;
}

void sym_engine::guard::clear()
{
	if (m_ref ==  0) return;
	if (m_ref == -1) return;
	if (--m_ref == 0)
	{
		VERIFY(SymCleanup(GetSymEngineHandle()));
	}
}

bool sym_engine::guard::load_module(HANDLE hProcess, HMODULE hMod)
{
	char filename[MAX_PATH];
	if (!GetModuleFileNameA(hMod, filename, MAX_PATH))
		return false;

	HANDLE hFile = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
	if (hFile == INVALID_HANDLE_VALUE) return false;

	// "Debugging Applications" John Robbins
    // For whatever reason, SymLoadModule can return zero, but it still loads the modules. Sheez.
	SetLastError(ERROR_SUCCESS);
    if (!SymLoadModule(hProcess, hFile, filename, 0, (DWORD)((char*)hMod -(char*)0), 0) && ERROR_SUCCESS != GetLastError())
	{
		return false;
	}

	return true;
}

/*-------------------------------------------------------------------
*	prints a current thread's stack
*/

struct current_context : CONTEXT
{
	HANDLE	thread;
	volatile int signal;
};

static DWORD WINAPI tproc(void * const pv)
{
	current_context * p = reinterpret_cast<current_context*>(pv);
	__try
	{
		// Konstantin, 14.01.2002 17:21:32
		// must wait in spin lock until main thread will leave a ResumeThread (must return back to user context)
		uInt debug_only = 0;
		while (p->signal)
		{
			//if (!SwitchToThread())
			Sleep(0); // forces switch to another thread
			++debug_only;
		}
		#ifdef SYM_ENGINE_TRACE_SPIN_COUNT	//:==_DEBUG
		char s[256];
		wsprintf(s, "sym_engine::tproc, spin count %u\n", debug_only);
		OutputDebugString(s);
		#endif;

		if (-1 == SuspendThread(p->thread))
		{
			p->signal  = -1;
			__leave;
		}

		__try
		{
			p->signal = GetThreadContext(p->thread, p) ? 1 : -1;
		}
		__finally
		{
			VERIFY(-1 != ResumeThread(p->thread));
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		p->signal  = -1;
	}
	return 0;
}

bool sym_engine::stack_trace(int loglevl,cChar *const caption, cuInt skip, cChar * const fmt)
{
	if (!fmt) return false;

	// attempts to get current thread's context
	
	current_context ctx;
	memset(&ctx, 0, sizeof current_context);

	BOOL r = DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(), &ctx.thread, 0, 0, DUPLICATE_SAME_ACCESS);

	_ASSERTE(r);
	_ASSERTE(ctx.thread);

	if (!r || !ctx.thread)
		return false;

	ctx.ContextFlags = CONTEXT_CONTROL; // CONTEXT_FULL;
	ctx.signal = -1;

	DWORD dummy;
	HANDLE worker = CreateThread(0, 0, tproc, &ctx, CREATE_SUSPENDED, &dummy);
	_ASSERTE(worker);

	if (worker)
	{
		 VERIFY(SetThreadPriority(worker, THREAD_PRIORITY_ABOVE_NORMAL)); //  THREAD_PRIORITY_HIGHEST
		if (-1 != ResumeThread(worker))
		{
			uInt debug_only = 0;
										// Konstantin, 14.01.2002 17:21:32
			ctx.signal = 0;				// only now the worker thread can get this thread context
			while (!ctx.signal)
				++debug_only; // wait in spin
			#ifdef SYM_ENGINE_TRACE_SPIN_COUNT	//:==_DEBUG
			char s[256];
			wsprintf(s, "sym_engine::stack_trace, spin count %u\n", debug_only);
			OutputDebugString(s);
			#endif
		}else
		{
			VERIFY(TerminateThread(worker, 0));
		}

		VERIFY(CloseHandle(worker));
	}

	VERIFY(CloseHandle(ctx.thread));

	if (ctx.signal == -1)
	{
		_ASSERTE(0);
		return false;
	}

	// now it can print stack
	stack_trace(loglevl,&ctx, caption, skip, fmt);
	return true;
}

bool sym_engine::stack_trace(int loglevl,CONTEXT * const pctx, cChar *const caption, cuInt skip, cChar * const fmt)
{
	if (!fmt) return false;
	sym_engine sym(0);
	return stack_trace(loglevl,sym, pctx, caption, skip, fmt);
}

bool sym_engine::stack_trace(int loglevl,sym_engine& sym, CONTEXT * const pctx, cChar * const caption, uInt skip, cChar * const fmt)
{
	SYSTEMTIME time;
	::GetSystemTime(&time);
	char str[128];
	int nSize = _snprintf(str,sizeof(str),"crashdump_%02d%02d_%02d%02d",time.wMonth,time.wDay,time.wHour+8,time.wMinute);
	
	g_log.Create(str);

    if (!sym.stack_first(loglevl,pctx))
	{
		g_log.Dump("%s:stack_first return false!",caption);
		return false;
	}

	char buf [512] = {0};
	char fbuf[512] = {0};
	char sbuf[512] = {0};

	g_log.Dump(caption);
	do
	{
		if (!skip)
		{
			uLong ln = 0;
			uLong ld = 0;
			uLong sd = 0;
			char *pf	= 0;
			char *ps = 0;

			std::string s;

			s="\t\t";//ÐÐÊ×Ëõ½ø

			for (cChar * p = fmt; *p; (*p)?++p:p)
			{
				if (*p == '%')
				{
					if(!*++p)
						break; // skips '%'
					char c = *p;
					switch (c)
					{
					case 'm':
					{
						if (sym.module(buf, sizeof(buf)))
							s+=buf;
						else
							s+="?.?";
						break;
					}
					case 'f':
					{
						if (!pf)
							pf = (sym.fileline(fbuf, sizeof(fbuf), &ln, &ld)) ? fbuf : " ";
						s+=pf;
						break;
					}
					case 'l':
					{
						if (!pf)
							pf = (sym.fileline(fbuf, sizeof(fbuf), &ln, &ld)) ? fbuf : " ";
						if (*(p + 1) == 'd') 
						{ 
							AppendFmtString(s,"-%d+%d",(int)(ld/(1L<<24)),(int)(ld%(1L<<24)));
							++p; 
						}
						else 
							AppendFmtString(s,"%d",ln);
						break;
					}
					case 's':
						if (!ps)
							ps = sym.symbol(sbuf, sizeof(sbuf), &sd) ? sbuf : "?()";
						if (*(p + 1) == 'd') 
						{ 
							AppendFmtString(s,"%s%d",(sd>=0?"+":""),(int)sd);
							++p; 
						}
						else 
							s+=ps;
						break;
					case '%':
						s+="%";
						break;
					default:
						s+="%";
						s+=c;
						break;
					}
				}
				else
				{
					s+= *p;
				}
			}

			g_log.Dump(s.c_str());
		}
		else
		{
			--skip;
		}
	}while (sym.stack_next(loglevl));
	return true;
}

