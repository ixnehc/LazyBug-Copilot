#pragma once

namespace i_math
{
typedef unsigned char		u8; 

typedef signed char			s8; 

typedef char				c8; 


typedef unsigned short		u16;

typedef signed short		s16; 


typedef unsigned int		u32;

typedef signed int			s32; 



typedef __int64				s64; 
typedef unsigned __int64	u64;



typedef float				f32; 

typedef double				f64; 

//enum {MAXDWORD		= 0xffffffffU};
//enum {MAXSBYTE		= 0x7f       };
//enum {MAXSWORD		= 0x7fff     };
//enum {MAXINT		= 0x7fffffff };
enum {INDEX_NONE	= -1         };
enum {UNICODE_BOM   = 0xfeff     };

//enum {MAXBYE=0xff};
enum ENoInit {E_NoInit = 0};

} // end namespace

