#pragma once

#include <vector>

struct Bitset2d
{
	struct contour_point	{
		contour_point(){row=column=0;}
		contour_point(int _Row,int _Column){row = _Row; column = _Column;}
		int row;
		int column;
	};
	struct contour{
		std::vector<contour_point> _points;
		contour_point * operator [](int i);
		int count() const;
	};
	Bitset2d();
	Bitset2d(int rows,int columns);
	virtual ~Bitset2d(void);
	bool operator ()(int row,int column) const;
	bool operator[](int i) const;
	void resize(int rows,int columns,int pitch=0);
	void set(int row,int column);//把某一位设成1
	void reset(int row,int column);//把某一位设成0
	void reset();//把整张图设成0
	void assign(int row,int column,bool value);//把某一位赋成某值
	void assign(const void * data,int len);//从另一个buffer拷贝数据到本Bitset2d上
	void assign(const Bitset2d & map,int l,int t);//把另一张Bitset2d拷到本Bitset2d上的某位置,l,t是本Bitset2d上的点
	int count(int l,int t,int w,int h);//本Bitset2d上某一块区域内1的个数
	bool any(int l,int t,int w,int h);//本Bitset2d上某一块区域内是否有1
	int size() const;//返回本Bitset2d的buffer占用的字节数
	int bits() const;//返回本Bitset2d的bit数
	int width() const;
	int height() const;
	bool isOut(int row,int column) const;

	void getContour(Bitset2d & glyph_map) const;
	void getContour(Bitset2d & glyph_map,int l,int t,int w,int h) const;
	void getContour(contour & gly) const;
	void getContour(contour & gly,int l,int t,int w,int h) const;
private:
	std::vector<unsigned char> _bytes;	
	int _w,_h;
	int _pitch;
};