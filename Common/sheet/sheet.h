#pragma once




#include <vector>
#include <string>
#include <unordered_map>

#include "class/class.h"

typedef int ShtRowID;
typedef int ShtColID;

#define RowID_Invalid (-1)
#define ColumnID_Invalid (-1)

#define IS_EMPTY_CELL(cell) ((cell)?((cell)->s[0]==0):TRUE)


class CDeviceObject;
class CSheetMgr;


struct SheetCell
{
	const char *s;
	double v;
	DWORD cSiblings;
};

struct SheetRow
{
	DEFINE_CLASS(SheetRow);
	std::vector<SheetCell*> cells;
};

struct ColumnCache
{
	DEFINE_CLASS(ColumnCache);
	std::unordered_map<std::string,ShtRowID>mp0;
	std::unordered_map<int,ShtRowID>mp1;
};



class CSheetRow;
class CSheet
{
public:
	DEFINE_CLASS(CSheet);

	IMPLEMENT_REFCOUNT_C;

	CSheet();
	~CSheet()
	{
		Clear();
	}

	BOOL Load(const char *buf,DWORD szBuf);
	void Clear();

	void SetPath(const char *path)	{		_path=path;	}
	const char *GetPath()	{		return _path.c_str();	}

	DWORD GetRowCount()	{		return (DWORD)_rows.size();	}
	DWORD GetColumnCount()	{		return _nCols;	}
	ShtColID FindColumn(const char *title);
	ShtRowID FindRow(const char *key,ShtColID col=0);
	ShtRowID FindRow(int key,ShtColID col=0);
	SheetCell*GetCell(ShtColID col,ShtRowID row,int idx=0);
	CSheetRow *GetRow(ShtRowID row);
	CSheetRow *ObtainRow(int key,ShtColID col=0)	{		return GetRow(FindRow(key,col));	}
	CSheetRow *ObtainRow(const char*key,ShtColID col=0){		return GetRow(FindRow(key,col));	}
	const char *GetErrorStr();

protected:


	char *_AddStr(const char *s);
	DWORD _nCols;
	std::vector<SheetRow*> _rows;

	std::unordered_map<std::string,ShtColID>_cols;

	std::vector<SheetCell> _cells;


	std::vector<char>_buf;

	ColumnCache*_BuildColumnCache(ShtColID col);
	std::vector<ColumnCache*>_caches;

	std::string _path;//用于调试信息
	std::string _error;


};

class CSheetRow
{
public:
	DEFINE_CLASS(CSheetRow)
	CSheetRow()
	{
		_sht=NULL;
		_row=-1;
	}
	~CSheetRow()
	{
		SAFE_RELEASE(_sht);
	}
	IMPLEMENT_REFCOUNT_C

	virtual DWORD GetColumnCount()
	{
		return _sht->GetColumnCount();
	}
	virtual ShtColID FindColumn(const char *title)
	{
		return _sht->FindColumn(title);
	}
	virtual SheetCell *FindCell(const char *title,int idx)
	{
		ShtColID col=FindColumn(title);
		if (col!=ColumnID_Invalid)
			return _sht->GetCell(col,_row,idx);
		return NULL;
	}

	virtual BOOL Find(const char *&ret,const char *title,int idx=0)
	{
		SheetCell *cell=FindCell(title,idx);
		if (!cell)
			return FALSE;
		ret=cell->s;
		return TRUE;
	}
	virtual BOOL Find(int&ret,const char *title,int idx=0)
	{
		SheetCell *cell=FindCell(title,idx);
		if (!cell)
			return FALSE;
		ret=(int)cell->v;
		return TRUE;
	}
	virtual BOOL Find(float&ret,const char *title,int idx=0)
	{
		SheetCell *cell=FindCell(title,idx);
		if (!cell)
			return FALSE;
		ret=(float)cell->v;
		return TRUE;
	}

	virtual SheetCell *GetCell(ShtColID col,int idx=0)
	{
		return _sht->GetCell(col,_row,idx);
	}
	virtual CSheet *GetOwner()	{		return _sht;	}
	virtual ShtRowID GetRowID()	{		return _row;	}

protected:
	ShtRowID _row;
	CSheet *_sht;

	friend class CSheet;

};

