
#include "stdh.h"

#include "Bitset2d.h"

#include <assert.h>

Bitset2d::Bitset2d()
{
	_w = 0;
	_h = 0;
	_pitch=0;
}

Bitset2d::Bitset2d(int rows,int columns)
{
	resize(rows,columns);
}

Bitset2d::~Bitset2d(void)
{
}

#define bitset2d_check(row,column){	 \
	if(row>=_h&&column>=_w)			\
		assert(FALSE);		\
}\

#define BYTEBIT(row__,col__,iByte__,iBit__)									\
int iByte__=((row__)*_pitch)+(col__)/8;										\
int iBit__=((col__)%8);


bool Bitset2d::operator ()(int row,int column) const
{	
	bitset2d_check(row,column);

	BYTEBIT(row,column,ibyte,ibit);

	return ((_bytes[ibyte])&(0x1<<ibit))>0;
}

bool Bitset2d::operator[](int i) const
{
	return (*this)(i/_w,i%_w);
}

void Bitset2d::set(int row,int column)
{
	bitset2d_check(row,column);

	BYTEBIT(row,column,ibyte,ibit);

	_bytes[ibyte] = (_bytes[ibyte])|(0x1<<ibit);
}
void Bitset2d::reset(int row,int column)
{
	bitset2d_check(row,column);

	BYTEBIT(row,column,ibyte,ibit);

	_bytes[ibyte] = (_bytes[ibyte])&BYTE(~(0x1<<ibit));
}
int Bitset2d::count(int l,int t,int w,int h)
{
	bitset2d_check(l,t);
	bitset2d_check(l+w,t+h);
	
	int count = 0;

	for(int i = l;i<l+w;i++)
		for(int j = 0;j<t+h;j++)
			count += (*this)(i,j);
	
	return count;
}
bool Bitset2d::any(int l,int t,int w,int h)
{
	bitset2d_check(l,t);
	bitset2d_check(l+w,t+h);

	for(int i = l;i<l+w;i++)
		for(int j = 0;j<t+h;j++)
			if((*this)(i,j)==true)
				return true;
	
	return false;
}
void Bitset2d::assign(int row,int column,bool value)
{
	if(value)
		set(row,column);
	else
		reset(row,column);
}	
void Bitset2d::assign(const void * data,int len)
{
	int num_bytes = size();
	int bytes_copy = (len>num_bytes)?num_bytes:len;
	memcpy(_bytes.data(),data,bytes_copy);
}
void Bitset2d::assign(const Bitset2d & map,int l,int t)
{
	int w = map.width();
	int h = map.height();

	int ex = (w + l>_w)?_w:(w+l);
	int ey = (h + t>_h)?_h:(h+t);

	for(int i = l,x = 0;i<ex;i++,x++)
		for (int j = t,y = 0;j<ey;j++,y++) {
			assign(i,j,map(x,y));
		}
}
int Bitset2d::size() const
{
	return _pitch*_h;
}
void Bitset2d::reset()
{
	memset(_bytes.data(),0,_bytes.size());
}
int Bitset2d::width() const
{
	return _w;
}
int Bitset2d::height() const
{
	return _h;
}
void Bitset2d::getContour(Bitset2d & glyph_map) const
{
	getContour(glyph_map,0,0,_w,_h);
}
void Bitset2d::getContour(Bitset2d & glyph_map, int l, int t,int w,int h) const
{
	int width  = (l+w>_w)?_w:(l+w);
	int height = (t+h>_h)?_h:(t+h);
	
	if(l>=width||t>=height)
		return;

	glyph_map._w = width  - l;
	glyph_map._h = height - t;

	glyph_map._bytes.resize(glyph_map.size());
	glyph_map.reset();

	const int moveX[] = {1,0,-1, 0,-1, 0, 1,1};
	const int moveY[] = {1,1, 1,-1,-1,-1,-1,0};

	for(int i = t;i<height;i++)
		for (int j = l;j<width;j++) {
			if((*this)(i,j)){
				for(int m = 0;m<8;m++){
					int x = i+moveX[m];
					int y = j+moveY[m];
					if(isOut(x,y)||(*this)(x,y)==false)
					{
						glyph_map.set(i-t,j-l);
						break;
					}
				}
			}
		}
}
bool Bitset2d::isOut(int row,int column) const
{
	return (row>=_h||column>=_w);
}
int Bitset2d::bits() const
{
	return _w*_h;
}
void Bitset2d::getContour(contour & gly) const
{
	getContour(gly,0,0,_w,_h);
}
void Bitset2d::getContour(contour & gly, int l, int t, int w, int h) const
{
	gly._points.resize(0);
	
	int width  = (l+w>_w)?_w:(l+w);
	int height = (t+h>_h)?_h:(t+h);

	const int moveX[] = {1,0,-1, 0,-1, 0, 1,1};
	const int moveY[] = {1,1, 1,-1,-1,-1,-1,0};

	for(int i = t ;i<height;i++)
	for (int j = l;j<width;j++)
	{
		if((*this)(i,j)){
			for(int m = 0;m<8;m++)
			{
				int x = i+moveX[m];
				int y = j+moveY[m];
				if(isOut(x,y)||(*this)(x,y)==false)
				{
					gly._points.push_back(contour_point(i,j));
					break;
				}
			}
		}
	}
}
//////////////////////////////////////////////////////////////////////////
Bitset2d::contour_point * Bitset2d::contour::operator [](int i)
{
	if(i>=_points.size())
		return NULL;
	return &_points[i];
}
int Bitset2d::contour::count() const
{
	return _points.size();
} 
void Bitset2d::resize(int rows,int columns,int pitch)
{
	_w = columns;
	_h = rows;	
	if (pitch==0)
		_pitch=(_w+7)/8;
	else
		_pitch=pitch;

	int bytes = size();
	_bytes.resize(bytes);
	reset();
}
