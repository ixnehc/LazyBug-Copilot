
#pragma once

class CDBReader
{
public:
	void Init(const char *filename,const char *tblname);
	void Clear();

	DWORD GetColumnCount();//有几列
	DWORD GetRowCount();//有几行
	const char *GetColumnName(DWORD idxColumn);//返回某一列的名称

	void Reset();//回到第一行,第一列
	BOOL NextColumn();//指到当前行的下一列上
	BOOL NextRow();//指到下一行的第一列上

	BOOL IsNumber();//当前格是数值还是字符串
	double GetNumber();//返回当前格的数值
	const char *GetStr();//返回当前格的字符串
};