#include "stdh.h"
#include "DXT1Encodes.h"

#pragma  warning(disable:4244)

#ifndef INT_MAX
#define INT_MAX       2147483647    /* maximum (signed) int value */
#endif

BYTE * globalOutData; 
void ExtractBlock( const BYTE *inPtr, int width, BYTE *colorBlock ); //1
void ExtractBlock565( const BYTE *inPtr, int width, BYTE *colorBlock ); //1
WORD ColorTo565( const BYTE *color ) ;								//1
void EmitWORD( WORD s ) ;											//1
void GetMinMaxColors( const BYTE *colorBlock, BYTE *minColor, BYTE *maxColor ); //3
void EmitColorIndices( const BYTE *colorBlock, const BYTE *minColor, const BYTE *maxColor ); //1

#define ALIGN16(x)  __declspec(align(16)) x

void  CompressBmpToDXT1(const BYTE *inBuf,void *outBuf,int width,int height)
{
	 ALIGN16( BYTE block[64] );   
	 ALIGN16( BYTE minColor[4] );
	 ALIGN16( BYTE maxColor[4] );     
	 globalOutData = (BYTE*)outBuf;   
	 for ( int j = 0; j < height; j += 4, inBuf += width * 4*4 ) 
	 {      
		 for ( int i = 0; i < width; i += 4 ) 
		 {       
			 ExtractBlock( inBuf + i * 4, width, block ); 
			 GetMinMaxColors( block, minColor, maxColor );     
			 EmitWORD( ColorTo565( maxColor ) );         
			 EmitWORD( ColorTo565( minColor ) );      
			 EmitColorIndices( block, minColor, maxColor );     
		 }  
	 }    
}

void CompressBmp565ToDXT1(const BYTE *inBuf,void *outBuf,int width,int height)
{
	ALIGN16( BYTE block[64] );   
	ALIGN16( BYTE minColor[4] );
	ALIGN16( BYTE maxColor[4] );     
	globalOutData = (BYTE*)outBuf;   
	for ( int j = 0; j < height; j += 4, inBuf += width * 4*sizeof(WORD) ) 
	{      
		for ( int i = 0; i < width; i += 4 ) 
		{       
			ExtractBlock565( inBuf + i * sizeof(WORD), width, block ); 
			GetMinMaxColors( block, minColor, maxColor );     
			EmitWORD( ColorTo565( maxColor ) );         
			EmitWORD( ColorTo565( minColor ) );      
			EmitColorIndices( block, minColor, maxColor );     
		}  
	}    
}


void ExtractBlock( const BYTE *inPtr, int width, BYTE *colorBlock ) //1
{   
	for ( int j = 0; j < 4; j++ ) 
	{      
		memcpy( &colorBlock[j*4*4],inPtr, 4*4 );     
		inPtr += width * 4;  
	} 
} 

void ExtractBlock565( const BYTE *inPtr, int width, BYTE *colorBlock )
{   
	DWORD *p=(DWORD *)colorBlock;
	WORD *q=(WORD*)inPtr;
	for ( int j = 0; j < 4; j++ ) 
	{      
		p[0]=From565(q[0]);
		p[1]=From565(q[1]);
		p[2]=From565(q[2]);
		p[3]=From565(q[3]);

		p+=4;//到下四个
		q+= width;//到下一行
	} 
} 


WORD ColorTo565( const BYTE *color )          //1
{     
	return( ( color[ 2 ] >> 3 ) << 11 ) | ( ( color[ 1 ] >> 2 ) << 5 ) | ( color[ 0 ] >> 3 ); 
} 
void EmitByte( BYTE b )
{  
	globalOutData[0] = b;  
	globalOutData += 1; 
} 
void EmitWORD( WORD s )                  //1
{    
	globalOutData[0] = ( s >>  0 ) & 255; 
	globalOutData[1] = ( s >>  8 ) & 255;  
	globalOutData += 2; 
}  
void EmitDoubleWORD( DWORD i ) 
{    
	globalOutData[0] = ( i >>  0 ) & 255; 
	globalOutData[1] = ( i >>  8 ) & 255;   
	globalOutData[2] = ( i >> 16 ) & 255; 
	globalOutData[3] = ( i >> 24 ) & 255; 
	globalOutData += 4; 
} 
int ColorDistance( const BYTE *c1, const BYTE *c2 ) 
{    
	return  ( ( c1[0] - c2[0] ) * ( c1[0] - c2[0] ) ) + ( ( c1[1] - c2[1] ) * ( c1[1] - c2[1] ) ) +  ( ( c1[2] - c2[2] ) * ( c1[2] - c2[2] ) ); 
} 
void SwapColors( BYTE *c1, BYTE *c2 ) 
{  
	BYTE tm[3];   
	memcpy( tm, c1, 3 ); 
	memcpy( c1, c2, 3 );  
	memcpy( c2, tm, 3 );
}  
void GetMinMaxColors( const BYTE *colorBlock, BYTE *minColor, BYTE *maxColor )  //1
{   
	int maxDistance = -1;    
	for ( int i = 0; i < 64 - 4; i += 4 ) 
	{        
		for ( int j = i + 4; j < 64; j += 4 ) 
		{         
			int distance = ColorDistance( &colorBlock[i], &colorBlock[j] );
			if ( distance > maxDistance ) 
			{             
				maxDistance = distance; 
				memcpy( minColor, colorBlock+i, 3 );  
				memcpy( maxColor, colorBlock+j, 3 );    
			}     
		}   
	}    
	if ( ColorTo565( maxColor ) < ColorTo565( minColor ) ) 
	{     
		SwapColors( minColor, maxColor ); 
	}
} 

//void GetMinMaxColors( const BYTE *colorBlock, BYTE *minColor, BYTE *maxColor )  //2
//{  
//	int maxLuminance = -1, minLuminance = MAX_INT;  
//	for ( i = 0; i < 16; i++ )
//	{        
//		int luminance = ColorLuminance( colorBlock+i*4 );  
//		if ( luminance > maxLuminance )
//		{          
//			maxLuminance = luminance;     
//			memcpy( maxColor, colorBlock+i*4, 3 );    
//		}        
//		if ( luminance < minLuminance )
//		{         
//			minLuminance = luminance;    
//			memcpy( minColor, colorBlock+i*4, 3 );   
//		}   
//	}    
//	if ( ColorTo565( maxColor ) < ColorTo565( minColor ) ) 
//	{    
//		SwapColors( minColor, maxColor );  
//	} 
//} 



//#define INSET_SHIFT     4       // inset the bounding box with ( range >> shift )  
//void GetMinMaxColors( const BYTE *colorBlock, BYTE *minColor, BYTE *maxColor )   //3
//{   
//	int i;    
//	BYTE inset[3];  
//	minColor[0] = minColor[1] = minColor[2] = 255; 
//	maxColor[0] = maxColor[1] = maxColor[2] = 0;   
//	for ( i = 0; i < 16; i++ ) 
//	{       
//		if ( colorBlock[i*4+0] < minColor[0] )
//		{
//			minColor[0] = colorBlock[i*4+0];
//		}        
//		if ( colorBlock[i*4+1] < minColor[1] ) 
//		{ 
//			minColor[1] = colorBlock[i*4+1]; 
//		}      
//		if ( colorBlock[i*4+2] < minColor[2] )
//		{ 
//			minColor[2] = colorBlock[i*4+2];
//		}     
//		if ( colorBlock[i*4+0] > maxColor[0] ) 
//		{ 
//			maxColor[0] = colorBlock[i*4+0]; 
//		}     
//		if ( colorBlock[i*4+1] > maxColor[1] ) 
//		{
//			maxColor[1] = colorBlock[i*4+1];
//		}        
//		if ( colorBlock[i*4+2] > maxColor[2] )
//		{ 
//			maxColor[2] = colorBlock[i*4+2];
//		}   
//	}   
//	inset[0] = ( maxColor[0] - minColor[0] ) >> INSET_SHIFT;   
//	inset[1] = ( maxColor[1] - minColor[1] ) >> INSET_SHIFT;   
//	inset[2] = ( maxColor[2] - minColor[2] ) >> INSET_SHIFT; 
//    minColor[0] = ( minColor[0] + inset[0] <= 255 ) ? minColor[0] + inset[0] : 255;    
//	minColor[1] = ( minColor[1] + inset[1] <= 255 ) ? minColor[1] + inset[1] : 255;    
//	minColor[2] = ( minColor[2] + inset[2] <= 255 ) ? minColor[2] + inset[2] : 255;  
//	maxColor[0] = ( maxColor[0] >= inset[0] ) ? maxColor[0] - inset[0] : 0;   
//	maxColor[1] = ( maxColor[1] >= inset[1] ) ? maxColor[1] - inset[1] : 0;     
//	maxColor[2] = ( maxColor[2] >= inset[2] ) ? maxColor[2] - inset[2] : 0; 
//} 


 int ColorLuminance( const BYTE *color ) 
 {   
	 return ( color[0] + color[1] * 2 + color[2] ); 
 } 

#define C565_5_MASK         0xF8    
 // 0xFF minus last three bits
#define C565_6_MASK  0xFC    
 // 0xFF minus last two bits 
 void EmitColorIndices( const BYTE *colorBlock, const BYTE *minColor, const BYTE *maxColor )  //1
 {    
	 BYTE colors[4][4];    
	 unsigned int indices[16];  
	 colors[0][0] = ( maxColor[0] & C565_5_MASK ) | ( maxColor[0] >> 5 );  
	 colors[0][1] = ( maxColor[1] & C565_6_MASK ) | ( maxColor[1] >> 6 );   
	 colors[0][2] = ( maxColor[2] & C565_5_MASK ) | ( maxColor[2] >> 5 );
	 colors[1][0] = ( minColor[0] & C565_5_MASK ) | ( minColor[0] >> 5 ); 
	 colors[1][1] = ( minColor[1] & C565_6_MASK ) | ( minColor[1] >> 6 );  
	 colors[1][2] = ( minColor[2] & C565_5_MASK ) | ( minColor[2] >> 5 );  
	 colors[2][0] = ( 2 * colors[0][0] + 1 * colors[1][0] ) / 3;  
	 colors[2][1] = ( 2 * colors[0][1] + 1 * colors[1][1] ) / 3;  
	 colors[2][2] = ( 2 * colors[0][2] + 1 * colors[1][2] ) / 3;   
	 colors[3][0] = ( 1 * colors[0][0] + 2 * colors[1][0] ) / 3;     
	 colors[3][1] = ( 1 * colors[0][1] + 2 * colors[1][1] ) / 3;    
	 colors[3][2] = ( 1 * colors[0][2] + 2 * colors[1][2] ) / 3;     
	 for ( int i = 0; i < 16; i++ ) 
	 {     
		 unsigned int minDistance = INT_MAX;    
		 for ( int j = 0; j < 4; j++ )
		 {        
			 unsigned int dist = ColorDistance( &colorBlock[i*4], &colors[j][0] );   
			 if ( dist < minDistance ) 
			 {              
				 minDistance = dist;  
				 indices[i] = j;       
			 }      
		 }  
	 }    
	 DWORD result = 0;    
	 for ( int i = 0; i < 16; i++ ) 
	 {    
		 result |= ( indices[i] << (unsigned int)( i << 1 ) ); 
	 }      EmitDoubleWORD( result );
 } 

//void EmitColorIndices( const BYTE *colorBlock, const BYTE *minColor, const BYTE *maxColor ) //2
//{  
//	WORD colors[4][4];  
//	DWORD result = 0;    
//	colors[0][0] = ( maxColor[0] & C565_5_MASK ) | ( maxColor[0] >> 5 );  
//	colors[0][1] = ( maxColor[1] & C565_6_MASK ) | ( maxColor[1] >> 6 );  
//	colors[0][2] = ( maxColor[2] & C565_5_MASK ) | ( maxColor[2] >> 5 );   
//	colors[1][0] = ( minColor[0] & C565_5_MASK ) | ( minColor[0] >> 5 );     
//	colors[1][1] = ( minColor[1] & C565_6_MASK ) | ( minColor[1] >> 6 );    
//	colors[1][2] = ( minColor[2] & C565_5_MASK ) | ( minColor[2] >> 5 );  
//	colors[2][0] = ( 2 * colors[0][0] + 1 * colors[1][0] ) / 3;   
//	colors[2][1] = ( 2 * colors[0][1] + 1 * colors[1][1] ) / 3;   
//	colors[2][2] = ( 2 * colors[0][2] + 1 * colors[1][2] ) / 3;     
//	colors[3][0] = ( 1 * colors[0][0] + 2 * colors[1][0] ) / 3;     
//	colors[3][1] = ( 1 * colors[0][1] + 2 * colors[1][1] ) / 3;     
//	colors[3][2] = ( 1 * colors[0][2] + 2 * colors[1][2] ) / 3;    
//	for ( int i = 15; i >= 0; i-- ) 
//	{         
//		int c0 = colorBlock[i*4+0];  
//		int c1 = colorBlock[i*4+1];     
//		int c2 = colorBlock[i*4+2];      
//		int d0 = abs( colors[0][0] - c0 ) + abs( colors[0][1] - c1 ) + abs( colors[0][2] - c2 );    
//		int d1 = abs( colors[1][0] - c0 ) + abs( colors[1][1] - c1 ) + abs( colors[1][2] - c2 );    
//		int d2 = abs( colors[2][0] - c0 ) + abs( colors[2][1] - c1 ) + abs( colors[2][2] - c2 );
//	    int d3 = abs( colors[3][0] - c0 ) + abs( colors[3][1] - c1 ) + abs( colors[3][2] - c2 );      
//		int b0 = d0 > d3;      
//		int b1 = d1 > d2;      
//		int b2 = d0 > d2;       
//		int b3 = d1 > d3;   
//		int b4 = d2 > d3;      
//		int x0 = b1 & b2;     
//		int x1 = b0 & b3;         
//		int x2 = b0 & b4;        
//		result |= ( x2 | ( ( x0 | x1 ) << 1 ) ) << ( i << 1 );    
//	}   
//	EmitDoubleWORD( result );
//} 
//



