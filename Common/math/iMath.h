
#pragma once

#include <windows.h>
#include "iTypes.h"
#include <string>
#include <math.h>

#define CLAMP_FUNCS(suffix,type)\
	inline type clamp_##suffix(type v,type Min,type Max)\
{\
	if (v>Max)\
	return Max;\
	if (v<Min)\
	return Min;\
	return v;\
}\
	inline type clampup_##suffix(type v,type Min)\
{\
	if (v<Min)\
	return Min;\
	return v;\
}\
	inline type clampdown_##suffix(type v,type Max)\
{\
	if (v>Max)\
	return Max;\
	return v;\
}


namespace i_math
{
	//! Rounding error constant often used when comparing f32 values.
	const float ROUNDING_ERROR	= 0.0001f;

	//! Constant for PI.
	const float Pi				= 3.14159f;

	//! Constant for converting bettween degrees and radiants.
	const float GRAD_PI		= (float)(180.0 / 3.1415926535897932384626433832795);

	//! Constant for converting bettween degrees and radiants.
	const float GRAD_PI2		= (float)(3.1415926535897932384626433832795 / 180.0);

	inline float deg2rad(float v)	{		return v*GRAD_PI2;	}
	inline float rad2deg(float v)	{		return v*GRAD_PI;	}


	//! returns minimum of two values. Own implementation to get rid of the STL.
	template<class T>
	inline const T min_(const T a, const T b)
	{
		return a < b ? a : b;
	}
	
	//! returns minimum of two values. Own implementation to get rid of the STL
	template<class T>
	inline T max_(const T a, const T b)
	{
		return a < b ? b : a;
	}

	template< class T > inline T abs( const T A )
	{
		return (A>=(T)0) ? A : -A;
	}



	//! returns if a float equals the other one, taking floating 
	//! point rounding errors into account
	inline bool equals(f32 a, f32 b)
	{
		return (a + ROUNDING_ERROR > b) && (a - ROUNDING_ERROR < b);
	}

	inline bool equals(f32 a, f32 b,f32 epsilon)
	{
		return (a + epsilon > b) && (a - epsilon< b);
	}

	inline bool isnan(float v)
	{
		if (v==v)
			return false;
		return true;
		if ((*(unsigned int *)&v)==0x7fc00000)
			return true;
		return false;
	}


	//if r is 0,return v1,if r is 1,return v2
	template<typename T>
	inline T lerp(T v1,T v2,T r)
	{
		return v1*(((T)1.0)-r)+v2*r;
	}

	template <typename T>
	inline T wrap(T v,T r)
	{
		v=fmodf(v,r);
		if (v<0)
			v+=r;
		return v;
	}

	template <typename T>
	inline T wrap_angle(T a)//to 0..360
	{
		return wrap<T>(a,360.0f);
	}
	template <typename T>
	inline T wrap_radian(T r)//to 0..2*Pi
	{
		return wrap<T>(r,2*Pi);
	}

	//判断r要转向target的话,最短的话要从哪边转,返回true表示r要加一个正值来转向target
	inline bool judge_rotate_dir(f32 r,f32 target)
	{
		float d=target-r;
		d=i_math::wrap_radian(d);
		if (d<Pi)
			return true;
		return false;
	}

	//从角度r向着角度limit旋转,旋转的绝对值为delta
	//return whether reach limit
	inline bool rotate_limited(f32 &r,f32 limit,float delta)
	{
		float d=i_math::wrap_radian(limit-r);
		if (d<Pi)
		{
			if (delta>=d)
			{
				r=limit;
				return true;
			}
			else
			{
				r+=delta;
				return false;
			}
		}
		else
		{
			d-=2*Pi;
			d=-d;
			if (delta>=d)
			{
				r=limit;
				return true;
			}
			else
			{
				r-=delta;
				return false;
			}

		}

	}


	inline f32 normalize_angle(f32 a)//to -180..180
	{
		return wrap_angle(a+(f32)180.0)-(f32)180.0;
	}

	inline f32 normalize_radian(f32 r)//to -Pi~Pi
	{
		r=wrap_radian(r);
		if (r>Pi)
			r-=2*Pi;
		return r;
	}

	//返回两个弧度的夹角
	inline f32 get_radian_dist(float r1,float r2)
	{
		return fabsf(i_math::normalize_radian(r1-r2));
	}

	//rate==0  --> full r1
	//rate==1  --> full r2
	inline f32 lerp_angle(f32 r1,f32 r2,float rate)
	{
		float d=i_math::normalize_radian(r2-r1);
		return r1+d*rate;
	}

	inline f32 div_safe(f32 a,f32 b)
	{
		if (b>=0)
		{
			if (b<ROUNDING_ERROR)
				b=ROUNDING_ERROR;
		}
		else
		{
			if (b>ROUNDING_ERROR)
				b=ROUNDING_ERROR;
		}
		return a/b;
	}

	template <typename T>
	inline f32 calc_faded_weight(T tStart,T durFadeIn0,T tStop,T durFadeOut0,T t)
	{
		f32 wt=0.0f;
		f32 durFadeIn=(f32 )durFadeIn0;
		if (durFadeIn==0)
			durFadeIn=0.0001f;
		f32 durFadeOut=(f32 )durFadeOut0;
		if (durFadeOut==0)
			durFadeOut=0.0001f;

		if (t<tStart)
			t=tStart;

		if (tStop<tStart+durFadeIn)
		{
			if (t<tStop)
				wt=((f32)(t-tStart))/(f32)durFadeIn;
			else
			{
				wt=1.0f-((f32)(t-tStop))/(f32)durFadeOut;
				if (wt<0.0f)
					wt=0.0f;
				wt*=((f32)(tStop-tStart))/(f32)durFadeIn;
			}
		}
		else
		{
			if (t<tStart+durFadeIn)
				wt=((f32)(t-tStart))/(f32)durFadeIn;
			else
			{
				if (t>tStop)
				{
					wt=1.0f-((f32)(t-tStop))/(f32)durFadeOut;
					if (wt<0.0f)
						wt=0.0f;
				}
				else
					wt=1.0f;
			}
		}
		if (wt>1.0f)
			wt=1.0f;
		if (wt<0.0f)
			wt=0.0f;
		return wt;
	}

	// returns accurate values only for integers that are power of 2
	__forceinline u32 fastlog2( u32 x)
	{
		if (x == 0) return 0;  // 处理0的情况

		DWORD index;
		_BitScanReverse(& index, x);
		return (u32)index;
	}

	// returns log2 of first larger value that is a power of 2
	__forceinline u32 fastmaxlog2(u32 x)
	{
		if (x == 0) return 0; // Handle the case when x is 0

		DWORD index; // Variable to store the result of _BitScanReverse
		_BitScanReverse(&index, x); // Find the highest set bit

		// Check if x is a power of 2
		if ((x & (x - 1)) == 0)
		{
			return static_cast<u32>(index); // x is a power of 2, return the index
		}
		else
		{
			return static_cast<u32>(index + 1); // x is not a power of 2, return index + 1
		}
	}

	__forceinline bool ispower2( u32 x)
	{
		u32 p=fastlog2(x);
		return ((1<<p)==x);
	}


	CLAMP_FUNCS(dbl,f64)
	CLAMP_FUNCS(f,f32)
	CLAMP_FUNCS(i,s32)
	CLAMP_FUNCS(u,u32)
	CLAMP_FUNCS(sh,s16)

	//divide signed s by u ,and floor the result
	__forceinline s32 idiv_signed(s32 s,u32 u)
	{
		if (s>=0)
			return s/(s32)u;

		return (s-(((s32)u)-1))/(s32)u;
	}

	//mod signed s by u.
	__forceinline s32 imod_signed(s32 s,u32 u)
	{
		return s-idiv_signed(s,u)*(s32)u;
	}

	//rate==0  --> full c1
	//rate==1  --> full c2
	__forceinline u32 lerp_color( u32 c1, u32 c2, float rate )
	{
		u32 result;
		unsigned char *pCol1 = (unsigned char *)&( c1 );
		unsigned char *pCol2 = (unsigned char *)&( c2 );
		unsigned char *pResult = (unsigned char *)&( result );
		for ( int i = 0; i < 4; ++i )
		{
			pResult[i] = ( unsigned char )( (s32)pCol1[i] * ( 1.0f - rate ) + (s32)pCol2[i] * rate );
		}
		return result;
	}

	template<class T>
	inline void in_range( T &val, T start, T end )
	{
		if ( val < start )	val = start;
		if ( val > end )	val = end;
	}
//--------------------------------------------------------------------------------------------------------------------------------
// #define SMALL_NUMBER		(1.e-8)
// #define KINDA_SMALL_NUMBER	(1.e-4)
// #define BIG_NUMBER			(3.4e+38f)
// 
// 
// template< class T > inline T Square( const T A )
// {
// 	return A*A;
// }
// 
// template< class T > inline T Clamp( const T X, const T Min, const T Max )
// {
// 	return X<Min ? Min : X<Max ? X : Max;
// }
// 
// inline void * appMemmove( void* Dest, const void* Src, s32 Count )
// {
// 	return  memmove( Dest, Src, Count );
// }
// //void * appMemmove( void* Dest, const void* Src, s32 Count );
// 


//
}

