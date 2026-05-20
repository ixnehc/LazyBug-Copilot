
#pragma once

#ifdef _DEBUG
	#pragma comment(lib, "tbb_debug.lib")
	#pragma comment(lib, "tbbmalloc_debug.lib")
#else
	#pragma comment(lib, "tbb.lib")
	#pragma comment(lib, "tbbmalloc.lib")
#endif

