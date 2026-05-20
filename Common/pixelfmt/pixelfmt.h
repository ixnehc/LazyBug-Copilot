#pragma once

#include <bitset>


//////////////////////////////////////////////////////////////////////////
//Pixel Formats

struct Pixel_R5G6B5
{
	WORD b:5;
	WORD g:6;
	WORD r:5;
};

struct Pixel_A8R8G8B8
{
	DWORD b:8;
	DWORD g:8;
	DWORD r:8;
	DWORD a:8;
};

struct Pixel_A16R16G16B16
{
	WORD r;
	WORD g;
	WORD b;
	WORD a;
};

struct Pixel_A32R32G32B32F
{
	float r;
	float g;
	float b;
	float a;
};


struct Pixel_V8U8
{
	char u;
	char v;
};

struct Pixel_A8L8
{
	char a;
	char l;
};

struct Pixel_L16
{
	WORD l;
};

inline DWORD C565ToA8R8G8B8(WORD col)
{
	Pixel_R5G6B5 * pR5G6B5 = (Pixel_R5G6B5 *)(&col);

	DWORD colAlpha = 0;
	Pixel_A8R8G8B8 * p = (Pixel_A8R8G8B8 *)(&colAlpha);
	p->a = 0;
	p->r = DWORD(pR5G6B5->r)<<3;
	p->g = DWORD(pR5G6B5->g)<<2;
	p->b = DWORD(pR5G6B5->b)<<3;

	return colAlpha;
}

inline DWORD DecodeDXTCol(void * block,int x,int y)
{
	WORD c0 = ((WORD *)block)[0];
	WORD c1 = ((WORD *)block)[1];

	DWORD col0 = C565ToA8R8G8B8(c0) ; // color0
	DWORD col1 = C565ToA8R8G8B8(c1); // color1

	int off = y*8 + 2*x;
	DWORD place_holders = ((DWORD *)(block))[1];
	DWORD place_mask = (0x3<<off);
	DWORD code = (place_holders&place_mask)>>off;

	DWORD col = 0;

	Pixel_A8R8G8B8 * p0 = (Pixel_A8R8G8B8 *)&col0;
	Pixel_A8R8G8B8 * p1 = (Pixel_A8R8G8B8 *)&col1;
	Pixel_A8R8G8B8 * pr = (Pixel_A8R8G8B8 *)&col;
	switch(code)
	{
	case 0x00:  col = col0;break;
	case 0x01:  col = col1;break;
	case 0x02:  
		{
			pr->r = (2*p0->r + 1*p1->r + 1)/3;
			pr->g = (2*p0->g + 1*p1->g + 1)/3;
			pr->b = (2*p0->b + 1*p1->b + 1)/3;
			break;
		}
	case 0x03:
		{
			pr->r = (1*p0->r + 2*p1->r + 1)/3;
			pr->g = (1*p0->g + 2*p1->g + 1)/3;
			pr->b = (1*p0->b + 2*p1->b + 1)/3;
			break;
		}

	default: break;
	}

	return (col&0x00ffffff);
}



inline DWORD DecodeDXT3(void * block,int x,int y)
{
	BYTE * pColBlock = ((BYTE *)(block)) + 8;
	DWORD col565 = DecodeDXTCol(pColBlock,x,y);

	WORD alphaWord = ((WORD *)(block))[x];
	int off = 4*y;

	WORD place_mask = (0x0f)<<off;

	WORD code = (place_mask&alphaWord)>>off;

	DWORD alpha = DWORD(code<<28);

	return col565|alpha;
}
inline DWORD  DecodeDXT5(void * block,int x,int y)
{
	BYTE * pColBlock = ((BYTE *)(block)) + 8;

	DWORD col565 = DecodeDXTCol(pColBlock,x,y);

	BYTE  alpha0 = ((BYTE *)(block))[0];
	BYTE  alpha1 = ((BYTE *)(block))[1];

	int off = y*12 + 3*x;

	BYTE * pCode = ((BYTE *)(block)) + 2;

	std::bitset<48> place_holder,place_mask;
	place_holder |= ((WORD *)(pCode))[2];
	place_holder = place_holder<<32;
	place_holder |= ((DWORD *)(pCode))[0];

	place_mask.set(off);
	place_mask.set(off+1);
	place_mask.set(off+2);

	place_holder = (place_holder&place_mask)>>off;

	DWORD code = place_holder.to_ulong();

	DWORD alpha = 0;

	if(alpha0>alpha1)
	{
		switch(code)
		{
		case 0x000: alpha = alpha0; break;
		case 0x001: alpha = alpha1; break;
		case 0x002: alpha = (6*alpha0 + 1*alpha1 + 3)/7; break;
		case 0x003: alpha = (5*alpha0 + 2*alpha1 + 3)/7; break;
		case 0x004: alpha = (4*alpha0 + 3*alpha1 + 3)/7; break;
		case 0x005: alpha = (3*alpha0 + 4*alpha1 + 3)/7; break;
		case 0x006: alpha = (2*alpha0 + 5*alpha1 + 3)/7; break;
		case 0x007: alpha = (1*alpha0 + 6*alpha1 + 3)/7; break;
		default: break;
		}
	}
	else
	{
		switch(code)
		{
		case 0x000: alpha = alpha0; break;
		case 0x001: alpha = alpha1; break;
		case 0x002: alpha = (4*alpha0 + 1*alpha1 + 2)/5; break;
		case 0x003: alpha = (3*alpha0 + 2*alpha1 + 2)/5; break;
		case 0x004: alpha = (2*alpha0 + 3*alpha1 + 2)/5; break;
		case 0x005: alpha = (1*alpha0 + 4*alpha1 + 2)/5; break;
		case 0x006: alpha = 0;   break;
		case 0x007: alpha = 255; break;
		default : break;
		}
	}

	return col565|alpha<<24;
}
inline DWORD DecodeDXT1(void * data,int x,int y)
{
	WORD c0 = ((WORD *)(data))[0];
	WORD c1 = ((WORD *)(data))[1];

	DWORD col = 0;
	BYTE alpha = 255;
	if(c0>c1){  // opaque foramt
		col = DecodeDXTCol(data,x,y);
	}
	else  // one bit transparent code
	{	
		DWORD col0 = C565ToA8R8G8B8(c0); // color0
		DWORD col1 = C565ToA8R8G8B8(c1); // color1

		int off = y*8+2*x;

		DWORD place_holders = ((DWORD *)(data))[1];
		DWORD place_mask = (0x3<<off);
		DWORD code = (place_holders&place_mask)>>off;

		switch(code)
		{
		case 0x00: col = col0; break;
		case 0x01: col = col1; break;
		case 0x02: col = (col0+col1)/2; break;
		case 0x03: alpha = 0; break;
		default: break;
		}
	}

	// R5G6B5--->A8R8G8 A:255;
	return DWORD(alpha)<<24|col ;
}
