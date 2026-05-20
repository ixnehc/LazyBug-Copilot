#pragma once

#include "iTypes.h"



namespace i_math
{

typedef s32 fixed;

extern fixed _cos_tbl[512];
extern fixed _tan_tbl[256];
extern fixed _acos_tbl[513];

/* ftofix and fixtof are used in generic C versions of fixmul and fixdiv */
inline fixed ftofix(double x)
{
	if (x > 32767.0) 
		return 0x7FFFFFFF;

	if (x < -32767.0) 
		return -0x7FFFFFFF;
	return (fixed)(x * 65536.0 + (x < 0 ? -0.5 : 0.5));
}


inline double fixtof(fixed x)
{
	return (double)x / 65536.0;
}


inline fixed fixadd(fixed x, fixed y)
{
	fixed result = x + y;

	if (result >= 0) 
	{
		if ((x < 0) && (y < 0)) 
			return -0x7FFFFFFF;
		else
			return result;
	}
	else 
	{
		if ((x > 0) && (y > 0)) 
			return 0x7FFFFFFF;
		else
			return result;
	}
}


inline fixed fixsub(fixed x, fixed y)
{
	fixed result = x - y;

	if (result >= 0) 
	{
		if ((x < 0) && (y > 0)) 
			return -0x7FFFFFFF;
		else
			return result;
	}
	else 
	{
		if ((x > 0) && (y < 0)) 
			return 0x7FFFFFFF;
		else
			return result;
	}
}

inline fixed fixmul(fixed x, fixed y)
{
	s64 lx = x;
	s64 ly = y;
	s64 lres = (lx*ly);

	if (lres > 0x7FFFFFFF0000LL) 
		return 0x7FFFFFFF;
	else if (lres < -0x7FFFFFFF0000LL) 
		return 0x80000000;
	else 
	{
		int res = lres >> 16;
		return res;
	}
}

inline fixed fixdiv(fixed x, fixed y)
{
	if (y == 0) 
		return (x < 0) ? -0x7FFFFFFF : 0x7FFFFFFF;
	else
		return ftofix(fixtof(x) / fixtof(y));
}


inline int fixfloor(fixed x)
{
	/* (x >> 16) is not portable */
	if (x >= 0)
		return (x >> 16);
	else
		return ~((~x) >> 16);
}


inline int fixceil(fixed x)
{
	if (x > 0x7FFF0000) 
		return 0x7FFF;

	return fixfloor(x + 0xFFFF);
}

inline fixed itofix(int x)
{
	return x << 16;
}


inline int fixtoi(fixed x)
{
	return fixfloor(x) + ((x & 0x8000) >> 15);
}


inline fixed fixcos(fixed x)
{
	return _cos_tbl[((x + 0x4000) >> 15) & 0x1FF];
}


inline fixed fixsin(fixed x)
{
	return _cos_tbl[((x - 0x400000 + 0x4000) >> 15) & 0x1FF];
}


inline fixed fixtan(fixed x)
{
	return _tan_tbl[((x + 0x4000) >> 15) & 0xFF];
}


inline fixed fixacos(fixed x)
{
	if ((x < -65536) || (x > 65536)) 
		return 0;
	return _acos_tbl[(x+65536+127)>>8];
}


inline fixed fixasin(fixed x)
{
	if ((x < -65536) || (x > 65536)) 
		return 0;

	return 0x00400000 - _acos_tbl[(x+65536+127)>>8];
}

fixed fixsqrt(fixed x);
fixed fixhypot(fixed x, fixed y);


class fix      /* C++ wrapper for the fixed point routines */
{
public:
   fixed v;

   fix() : v(0)                                       {}
   fix(const fix &x) : v(x.v)                         {}
   explicit fix(const int x) : v(itofix(x))           {}
   explicit fix(const long x) : v(itofix(x))          {}
   explicit fix(const unsigned int x) : v(itofix(x))  {}
   explicit fix(const unsigned long x) : v(itofix(x)) {}
   explicit fix(const float x) : v(ftofix(x))         {}
   explicit fix(const double x) : v(ftofix(x))        {}

   operator int() const                      { return fixtoi(v); }
   operator long() const                     { return fixtoi(v); }
   operator unsigned int() const             { return fixtoi(v); }
   operator unsigned long() const            { return fixtoi(v); }
   operator float() const                    { return fixtof(v); }
   operator double() const                   { return fixtof(v); }

   fix& operator = (const fix &x)            { v = x.v;           return *this; }
   fix& operator = (const int x)             { v = itofix(x);     return *this; }
   fix& operator = (const long x)            { v = itofix(x);     return *this; }
   fix& operator = (const unsigned int x)    { v = itofix(x);     return *this; }
   fix& operator = (const unsigned long x)   { v = itofix(x);     return *this; }
   fix& operator = (const float x)           { v = ftofix(x);     return *this; }
   fix& operator = (const double x)          { v = ftofix(x);     return *this; }

   fix& operator +=  (const fix x)           { v += x.v;          return *this; }
   fix& operator +=  (const int x)           { v += itofix(x);    return *this; }
   fix& operator +=  (const long x)          { v += itofix(x);    return *this; }
   fix& operator +=  (const float x)         { v += ftofix(x);    return *this; }
   fix& operator +=  (const double x)        { v += ftofix(x);    return *this; }

   fix& operator -=  (const fix x)           { v -= x.v;          return *this; }
   fix& operator -=  (const int x)           { v -= itofix(x);    return *this; }
   fix& operator -=  (const long x)          { v -= itofix(x);    return *this; }
   fix& operator -=  (const float x)         { v -= ftofix(x);    return *this; }
   fix& operator -=  (const double x)        { v -= ftofix(x);    return *this; }

   fix& operator *=  (const fix x)           { v = fixmul(v, x.v);         return *this; }
   fix& operator *=  (const int x)           { v *= x;                     return *this; }
   fix& operator *=  (const long x)          { v *= x;                     return *this; }
   fix& operator *=  (const float x)         { v = ftofix(fixtof(v) * x);  return *this; }
   fix& operator *=  (const double x)        { v = ftofix(fixtof(v) * x);  return *this; }

   fix& operator /=  (const fix x)           { v = fixdiv(v, x.v);         return *this; }
   fix& operator /=  (const int x)           { v /= x;                     return *this; }
   fix& operator /=  (const long x)          { v /= x;                     return *this; }
   fix& operator /=  (const float x)         { v = ftofix(fixtof(v) / x);  return *this; }
   fix& operator /=  (const double x)        { v = ftofix(fixtof(v) / x);  return *this; }

   fix& operator <<= (const int x)           { v <<= x;           return *this; }
   fix& operator >>= (const int x)           { v >>= x;           return *this; }

   fix& operator ++ ()                       { v += itofix(1);    return *this; }
   fix& operator -- ()                       { v -= itofix(1);    return *this; }

   fix operator ++ (int)                     { fix t;  t.v = v;   v += itofix(1);  return t; }
   fix operator -- (int)                     { fix t;  t.v = v;   v -= itofix(1);  return t; }

   fix operator - () const                   { fix t;  t.v = -v;  return t; }

   inline friend fix operator +  (const fix x, const fix y);
   inline friend fix operator +  (const fix x, const int y);
   inline friend fix operator +  (const int x, const fix y);
   inline friend fix operator +  (const fix x, const long y);
   inline friend fix operator +  (const long x, const fix y);
   inline friend fix operator +  (const fix x, const float y);
   inline friend fix operator +  (const float x, const fix y);
   inline friend fix operator +  (const fix x, const double y);
   inline friend fix operator +  (const double x, const fix y);

   inline friend fix operator -  (const fix x, const fix y);
   inline friend fix operator -  (const fix x, const int y);
   inline friend fix operator -  (const int x, const fix y);
   inline friend fix operator -  (const fix x, const long y);
   inline friend fix operator -  (const long x, const fix y);
   inline friend fix operator -  (const fix x, const float y);
   inline friend fix operator -  (const float x, const fix y);
   inline friend fix operator -  (const fix x, const double y);
   inline friend fix operator -  (const double x, const fix y);

   inline friend fix operator *  (const fix x, const fix y);
   inline friend fix operator *  (const fix x, const int y);
   inline friend fix operator *  (const int x, const fix y);
   inline friend fix operator *  (const fix x, const long y);
   inline friend fix operator *  (const long x, const fix y);
   inline friend fix operator *  (const fix x, const float y);
   inline friend fix operator *  (const float x, const fix y);
   inline friend fix operator *  (const fix x, const double y);
   inline friend fix operator *  (const double x, const fix y);

   inline friend fix operator /  (const fix x, const fix y);
   inline friend fix operator /  (const fix x, const int y);
   inline friend fix operator /  (const int x, const fix y);
   inline friend fix operator /  (const fix x, const long y);
   inline friend fix operator /  (const long x, const fix y);
   inline friend fix operator /  (const fix x, const float y);
   inline friend fix operator /  (const float x, const fix y);
   inline friend fix operator /  (const fix x, const double y);
   inline friend fix operator /  (const double x, const fix y);

   inline friend fix operator << (const fix x, const int y);
   inline friend fix operator >> (const fix x, const int y);

   inline friend int operator == (const fix x, const fix y);
   inline friend int operator == (const fix x, const int y);
   inline friend int operator == (const int x, const fix y);
   inline friend int operator == (const fix x, const long y);
   inline friend int operator == (const long x, const fix y);
   inline friend int operator == (const fix x, const float y);
   inline friend int operator == (const float x, const fix y);
   inline friend int operator == (const fix x, const double y);
   inline friend int operator == (const double x, const fix y);

   inline friend int operator != (const fix x, const fix y);
   inline friend int operator != (const fix x, const int y);
   inline friend int operator != (const int x, const fix y);
   inline friend int operator != (const fix x, const long y);
   inline friend int operator != (const long x, const fix y);
   inline friend int operator != (const fix x, const float y);
   inline friend int operator != (const float x, const fix y);
   inline friend int operator != (const fix x, const double y);
   inline friend int operator != (const double x, const fix y);

   inline friend int operator <  (const fix x, const fix y);
   inline friend int operator <  (const fix x, const int y);
   inline friend int operator <  (const int x, const fix y);
   inline friend int operator <  (const fix x, const long y);
   inline friend int operator <  (const long x, const fix y);
   inline friend int operator <  (const fix x, const float y);
   inline friend int operator <  (const float x, const fix y);
   inline friend int operator <  (const fix x, const double y);
   inline friend int operator <  (const double x, const fix y);

   inline friend int operator >  (const fix x, const fix y);
   inline friend int operator >  (const fix x, const int y);
   inline friend int operator >  (const int x, const fix y);
   inline friend int operator >  (const fix x, const long y);
   inline friend int operator >  (const long x, const fix y);
   inline friend int operator >  (const fix x, const float y);
   inline friend int operator >  (const float x, const fix y);
   inline friend int operator >  (const fix x, const double y);
   inline friend int operator >  (const double x, const fix y);

   inline friend int operator <= (const fix x, const fix y);
   inline friend int operator <= (const fix x, const int y);
   inline friend int operator <= (const int x, const fix y);
   inline friend int operator <= (const fix x, const long y);
   inline friend int operator <= (const long x, const fix y);
   inline friend int operator <= (const fix x, const float y);
   inline friend int operator <= (const float x, const fix y);
   inline friend int operator <= (const fix x, const double y);
   inline friend int operator <= (const double x, const fix y);

   inline friend int operator >= (const fix x, const fix y);
   inline friend int operator >= (const fix x, const int y);
   inline friend int operator >= (const int x, const fix y);
   inline friend int operator >= (const fix x, const long y);
   inline friend int operator >= (const long x, const fix y);
   inline friend int operator >= (const fix x, const float y);
   inline friend int operator >= (const float x, const fix y);
   inline friend int operator >= (const fix x, const double y);
   inline friend int operator >= (const double x, const fix y);

   inline friend fix sqrt(fix x);
   inline friend fix cos(fix x);
   inline friend fix sin(fix x);
   inline friend fix tan(fix x);
   inline friend fix acos(fix x);
   inline friend fix asin(fix x);
   inline friend fix atan(fix x);
   inline friend fix atan2(fix x, fix y);

};

inline  fix operator +  (const fix x, const fix y)    { fix t;  t.v = x.v + y.v;        return t; }
inline  fix operator +  (const fix x, const int y)    { fix t;  t.v = x.v + itofix(y);  return t; }
inline  fix operator +  (const int x, const fix y)    { fix t;  t.v = itofix(x) + y.v;  return t; }
inline  fix operator +  (const fix x, const long y)   { fix t;  t.v = x.v + itofix(y);  return t; }
inline  fix operator +  (const long x, const fix y)   { fix t;  t.v = itofix(x) + y.v;  return t; }
inline  fix operator +  (const fix x, const float y)  { fix t;  t.v = x.v + ftofix(y);  return t; }
inline  fix operator +  (const float x, const fix y)  { fix t;  t.v = ftofix(x) + y.v;  return t; }
inline  fix operator +  (const fix x, const double y) { fix t;  t.v = x.v + ftofix(y);  return t; }
inline  fix operator +  (const double x, const fix y) { fix t;  t.v = ftofix(x) + y.v;  return t; }

inline  fix operator -  (const fix x, const fix y)    { fix t;  t.v = x.v - y.v;        return t; }
inline  fix operator -  (const fix x, const int y)    { fix t;  t.v = x.v - itofix(y);  return t; }
inline  fix operator -  (const int x, const fix y)    { fix t;  t.v = itofix(x) - y.v;  return t; }
inline  fix operator -  (const fix x, const long y)   { fix t;  t.v = x.v - itofix(y);  return t; }
inline  fix operator -  (const long x, const fix y)   { fix t;  t.v = itofix(x) - y.v;  return t; }
inline  fix operator -  (const fix x, const float y)  { fix t;  t.v = x.v - ftofix(y);  return t; }
inline  fix operator -  (const float x, const fix y)  { fix t;  t.v = ftofix(x) - y.v;  return t; }
inline  fix operator -  (const fix x, const double y) { fix t;  t.v = x.v - ftofix(y);  return t; }
inline  fix operator -  (const double x, const fix y) { fix t;  t.v = ftofix(x) - y.v;  return t; }

inline  fix operator *  (const fix x, const fix y)    { fix t;  t.v = fixmul(x.v, y.v);         return t; }
inline  fix operator *  (const fix x, const int y)    { fix t;  t.v = x.v * y;                  return t; }
inline  fix operator *  (const int x, const fix y)    { fix t;  t.v = x * y.v;                  return t; }
inline  fix operator *  (const fix x, const long y)   { fix t;  t.v = x.v * y;                  return t; }
inline  fix operator *  (const long x, const fix y)   { fix t;  t.v = x * y.v;                  return t; }
inline  fix operator *  (const fix x, const float y)  { fix t;  t.v = ftofix(fixtof(x.v) * y);  return t; }
inline  fix operator *  (const float x, const fix y)  { fix t;  t.v = ftofix(x * fixtof(y.v));  return t; }
inline  fix operator *  (const fix x, const double y) { fix t;  t.v = ftofix(fixtof(x.v) * y);  return t; }
inline  fix operator *  (const double x, const fix y) { fix t;  t.v = ftofix(x * fixtof(y.v));  return t; }

inline  fix operator /  (const fix x, const fix y)    { fix t;  t.v = fixdiv(x.v, y.v);         return t; }
inline  fix operator /  (const fix x, const int y)    { fix t;  t.v = x.v / y;                  return t; }
inline  fix operator /  (const int x, const fix y)    { fix t;  t.v = fixdiv(itofix(x), y.v);   return t; }
inline  fix operator /  (const fix x, const long y)   { fix t;  t.v = x.v / y;                  return t; }
inline  fix operator /  (const long x, const fix y)   { fix t;  t.v = fixdiv(itofix(x), y.v);   return t; }
inline  fix operator /  (const fix x, const float y)  { fix t;  t.v = ftofix(fixtof(x.v) / y);  return t; }
inline  fix operator /  (const float x, const fix y)  { fix t;  t.v = ftofix(x / fixtof(y.v));  return t; }
inline  fix operator /  (const fix x, const double y) { fix t;  t.v = ftofix(fixtof(x.v) / y);  return t; }
inline  fix operator /  (const double x, const fix y) { fix t;  t.v = ftofix(x / fixtof(y.v));  return t; }

inline  fix operator << (const fix x, const int y)    { fix t;  t.v = x.v << y;   return t; }
inline  fix operator >> (const fix x, const int y)    { fix t;  t.v = x.v >> y;   return t; }

inline  int operator == (const fix x, const fix y)    { return (x.v == y.v);       }
inline  int operator == (const fix x, const int y)    { return (x.v == itofix(y)); }
inline  int operator == (const int x, const fix y)    { return (itofix(x) == y.v); }
inline  int operator == (const fix x, const long y)   { return (x.v == itofix(y)); }
inline  int operator == (const long x, const fix y)   { return (itofix(x) == y.v); }
inline  int operator == (const fix x, const float y)  { return (x.v == ftofix(y)); }
inline  int operator == (const float x, const fix y)  { return (ftofix(x) == y.v); }
inline  int operator == (const fix x, const double y) { return (x.v == ftofix(y)); }
inline  int operator == (const double x, const fix y) { return (ftofix(x) == y.v); }

inline  int operator != (const fix x, const fix y)    { return (x.v != y.v);       }
inline  int operator != (const fix x, const int y)    { return (x.v != itofix(y)); }
inline  int operator != (const int x, const fix y)    { return (itofix(x) != y.v); }
inline  int operator != (const fix x, const long y)   { return (x.v != itofix(y)); }
inline  int operator != (const long x, const fix y)   { return (itofix(x) != y.v); }
inline  int operator != (const fix x, const float y)  { return (x.v != ftofix(y)); }
inline  int operator != (const float x, const fix y)  { return (ftofix(x) != y.v); }
inline  int operator != (const fix x, const double y) { return (x.v != ftofix(y)); }
inline  int operator != (const double x, const fix y) { return (ftofix(x) != y.v); }

inline  int operator <  (const fix x, const fix y)    { return (x.v < y.v);        }
inline  int operator <  (const fix x, const int y)    { return (x.v < itofix(y));  }
inline  int operator <  (const int x, const fix y)    { return (itofix(x) < y.v);  }
inline  int operator <  (const fix x, const long y)   { return (x.v < itofix(y));  }
inline  int operator <  (const long x, const fix y)   { return (itofix(x) < y.v);  }
inline  int operator <  (const fix x, const float y)  { return (x.v < ftofix(y));  }
inline  int operator <  (const float x, const fix y)  { return (ftofix(x) < y.v);  }
inline  int operator <  (const fix x, const double y) { return (x.v < ftofix(y));  }
inline  int operator <  (const double x, const fix y) { return (ftofix(x) < y.v);  }

inline  int operator >  (const fix x, const fix y)    { return (x.v > y.v);        }
inline  int operator >  (const fix x, const int y)    { return (x.v > itofix(y));  }
inline  int operator >  (const int x, const fix y)    { return (itofix(x) > y.v);  }
inline  int operator >  (const fix x, const long y)   { return (x.v > itofix(y));  }
inline  int operator >  (const long x, const fix y)   { return (itofix(x) > y.v);  }
inline  int operator >  (const fix x, const float y)  { return (x.v > ftofix(y));  }
inline  int operator >  (const float x, const fix y)  { return (ftofix(x) > y.v);  }
inline  int operator >  (const fix x, const double y) { return (x.v > ftofix(y));  }
inline  int operator >  (const double x, const fix y) { return (ftofix(x) > y.v);  }

inline  int operator <= (const fix x, const fix y)    { return (x.v <= y.v);       }
inline  int operator <= (const fix x, const int y)    { return (x.v <= itofix(y)); }
inline  int operator <= (const int x, const fix y)    { return (itofix(x) <= y.v); }
inline  int operator <= (const fix x, const long y)   { return (x.v <= itofix(y)); }
inline  int operator <= (const long x, const fix y)   { return (itofix(x) <= y.v); }
inline  int operator <= (const fix x, const float y)  { return (x.v <= ftofix(y)); }
inline  int operator <= (const float x, const fix y)  { return (ftofix(x) <= y.v); }
inline  int operator <= (const fix x, const double y) { return (x.v <= ftofix(y)); }
inline  int operator <= (const double x, const fix y) { return (ftofix(x) <= y.v); }

inline  int operator >= (const fix x, const fix y)    { return (x.v >= y.v);       }
inline  int operator >= (const fix x, const int y)    { return (x.v >= itofix(y)); }
inline  int operator >= (const int x, const fix y)    { return (itofix(x) >= y.v); }
inline  int operator >= (const fix x, const long y)   { return (x.v >= itofix(y)); }
inline  int operator >= (const long x, const fix y)   { return (itofix(x) >= y.v); }
inline  int operator >= (const fix x, const float y)  { return (x.v >= ftofix(y)); }
inline  int operator >= (const float x, const fix y)  { return (ftofix(x) >= y.v); }
inline  int operator >= (const fix x, const double y) { return (x.v >= ftofix(y)); }
inline  int operator >= (const double x, const fix y) { return (ftofix(x) >= y.v); }

inline  fix sqrt(fix x)          { fix t;  t.v = fixsqrt(x.v);        return t; }
inline  fix cos(fix x)           { fix t;  t.v = fixcos(x.v);         return t; }
inline  fix sin(fix x)           { fix t;  t.v = fixsin(x.v);         return t; }
inline  fix tan(fix x)           { fix t;  t.v = fixtan(x.v);         return t; }
inline  fix acos(fix x)          { fix t;  t.v = fixacos(x.v);        return t; }
inline  fix asin(fix x)          { fix t;  t.v = fixasin(x.v);        return t; }
inline  fix atan(fix x)          { fix t;  t.v = fixatan(x.v);        return t; }
inline  fix atan2(fix x, fix y)  { fix t;  t.v = fixatan2(x.v, y.v);  return t; }



}


