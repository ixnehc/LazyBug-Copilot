/*
Copyright (c) 2001 - 2002
Author: Konstantin Boukreev
E-mail: konstantin@mail.primorye.ru
Created: 25.12.2001 19:41:00
Version: 1.0.2

Permission to use, copy, modify, distribute and sell this software
and its documentation for any purpose is hereby granted without fee,
provided that the above copyright notice appear in all copies and
that both that copyright notice and this permission notice appear
in supporting documentation.  Konstantin Boukreev makes no representations
about the suitability of this software for any purpose.
It is provided "as is" without express or implied warranty.

sym_engine class incapsulate Symbol Handler and Debugging Service API

the list of used API:
SymInitialize, SymCleanup, SymGetLineFromAddr, SymGetModuleBase,
SymGetSymbolInfo, SymGetSymFromAddr, SymGetSymFromName, SymGetSymNext,
SymLoadModule, SymSetOptions
StackWalk

based on code\ideas from John Robbins's book "Debugging application"
http://www.wintellect.com/robbins
*/

#ifndef _sym_engine_e4b31bc5_8e01_4cda_b5a4_905dde52ac01
#define _sym_engine_e4b31bc5_8e01_4cda_b5a4_905dde52ac01

#if _MSC_VER > 1000 
#pragma once
#endif // _MSC_VER > 1000

#ifdef _WIN64
#error Win64 is not supported
#endif
//#include "DBCCommon.h"
// #include "Mutex.h"
#include "se_translator.h"
#include <DbgHelp.h>

class sym_engine
{
public:
	sym_engine (uInt address);
	~sym_engine();

	/*
	format argument
	%m  : module
	%f  : file
	%l  : line
	%ld : line's displacement
	%s  : symbol
	%sd : symbol's displacement

	for example
	"%f(%l) : %m at %s"
	"%m.%s + %sd bytes, in %f:line %l"
	*/
	static bool stack_trace(int loglevl,cChar *caption ="", uInt skip = 1, cChar * fmt = default_fmt());
	static bool stack_trace(int loglevl,CONTEXT *pctx, cChar *caption ="", uInt skip = 0, cChar * fmt = default_fmt());
	static bool stack_trace(int loglevl,sym_engine & sym, CONTEXT *pctx, cChar *caption ="", uInt skip = 1, cChar * fmt = default_fmt());
private:
	static cChar * default_fmt() { return "%m(%s%sd) : (ÐÐ:%l%ld)%f"; }
	static bool get_line_from_addr (HANDLE hProc, uInt addr, uLong * pdisplacement, IMAGEHLP_LINE * pLine);
	static uInt get_module_basename (HMODULE hMod, char * buf, uInt len);

	bool check();

	// stack walk
	bool stack_first (int loglevl,CONTEXT* pctx);
	bool stack_next  (int loglevl);

	void address(uInt address)	{ m_address = address; }
	uInt address(void) const	{ return m_address; }

	// symbol handler queries
	uInt module  (char * buf, uInt len);
	uInt symbol  (char * buf, uInt len, uLong * pdisplacement = 0);
	uInt fileline(char * buf, uInt len, uLong * pline, uLong * pdisplacement = 0);
private:
	uInt			m_address;
	bool			m_ok;
	STACKFRAME *	m_pframe;
	CONTEXT *		m_pctx;


private:
	class guard
	{
	private:
		guard();
	public:
		~guard();
		bool init();
		static guard & instance();
	private:
		void clear();
		bool load_module(HANDLE, HMODULE);
		int  m_ref;
		se_translator m_se_trans;
	};
};

// Change Log:
//		25.12.2001  Konstantin, Initial version.
//		11.01.2002	Konstantin, added skip arg to stack_trace()
//		11.01.2002	Konstantin, format argument to stack_trace()
//		14.01.2002	Konstantin, fixed bug in thread's wait logic
//		16.01.2002, Konstantin, fixed bug in stack_trace(std::ostream&, CONTEXT *, cChar*) with skip value
//		16.01.2002, Konstantin, made guard as singelton


#endif //_sym_engine_e4b31bc5_8e01_4cda_b5a4_905dde52ac01

