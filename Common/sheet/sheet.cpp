
#include "stdh.h"


#include "sheet.h"
#include "stringparser/stringparser.h"
#include "Log/LogDump.h"

#include "commondefines/general_stl.h"

#include <assert.h>


//////////////////////////////////////////////////////////////////////////
//CSheetRow




//////////////////////////////////////////////////////////////////////////
//CSheet
CSheet::CSheet()
{
}


#pragma warning(disable :4311)


char *CSheet::_AddStr(const char *s)
{
	int len=strlen(s)+1;
	DWORD sz=_buf.size();
	_buf.resize(sz+len);
	memcpy(&_buf[sz],s,len);
	return &_buf[sz];
}

static void SplitStr(char sep,char *str,std::vector<char *>&pieces)
{
	pieces.clear();
	char *p=str;

	char *start=NULL;
	while(*p)
	{
		if (!start)
			start=p;

		if (*p==sep)
		{
			*p=0;
			pieces.push_back(start);
			start=NULL;
		}
		p++;
	}
	if (start)
		pieces.push_back(start);
}

static char *Trim(char *str)
{
	char *start=NULL,*end=NULL;
	char *p=str;
	char *t;
	while(*p)
	{
		t=p;
		BOOL bBlank=TRUE;
		if (IsDBCSLeadByte(*p))
		{
			bBlank=FALSE;
			p++;
		}
		else
		{
			if (!isspace(*p))
				bBlank=FALSE;
		}
		p++;

		if ((!start)&&(!bBlank))
			start=t;
		if (bBlank&&(!end))
			end=t;
		else
		{
			if (!bBlank)
				end=NULL;
		}
	}
	if (end)
		memset(end,0,p-end);

	if (start)
		return start;
	return str;
}

SheetCell *GetEmptyCell()
{
	static BOOL bInit=FALSE;
	static SheetCell cellEmpty;
	if (!bInit)
	{
		cellEmpty.v=0;
		cellEmpty.s="";
		cellEmpty.cSiblings=1;
	}
	return &cellEmpty;
}



//这个buffer包含了一个字符串以及它的结束符\0
BOOL CSheet::Load(const char *buf,DWORD szBuf)
{
	_error="";
	_buf.resize(szBuf);
	memcpy(_buf.data(),buf,szBuf);

	if (szBuf<=1)
		return TRUE;//为空


	std::vector<char *> lines;
	SplitStr('\n',_buf.data(),lines);

	if (lines.size()<=1)
		return TRUE;//空sheet

	std::vector<char*>cells;
	std::vector<BOOL>flags;//是否为复合cell的标志

	BOOL bRet=TRUE;

	//得到titles
	if (TRUE)
	{
		SplitStr('\t',lines[0],cells);

		if (cells.size()<=0)
			return TRUE;//空sheet

		flags.resize(cells.size());
		VEC_SET(flags,0);

		for (int i=0;i<cells.size();i++)
		{
			cells[i]=Trim(cells[i]);

			if (cells[i][0]=='+')
			{
				flags[i]=1;
				cells[i]++;
			}
			cells[i]=Trim(cells[i]);

			if (cells[i][0]==0)
			{
				AppendFmtString(_error,"表格里的标题栏第%d格为空\n",i+1);
				continue;
			}

			std::unordered_map<std::string,ShtColID>::iterator it=_cols.find(std::string(cells[i]));
			if(it!=_cols.end())
			{
				AppendFmtString(_error,"表格里的标题栏有名字重复的格子(%s)",cells[i]);
				bRet=FALSE;
				break;
			}

			_cols[std::string(cells[i])]=i;
		}

		_nCols=cells.size();
	}

	if (!bRet)
	{
		_cols.clear();
		_nCols=0;
		return FALSE;
	}

	_rows.resize(lines.size()-1);
	for (int i=0;i<_rows.size();i++)
		_rows[i]=Class_New2(SheetRow);

	_cells.reserve(_rows.size()*_nCols);

	//分析lines
	if (TRUE)
	{
		std::vector<char*>pieces;

		for (int j=1;j<lines.size();j++)
		{
			SheetRow* row=_rows[j-1];

			row->cells.reserve(_nCols);


			SplitStr('\t',lines[j],cells);

			for (int i=0;i<_nCols;i++)
			{
				BOOL bCompound=flags[i];


				if (i>cells.size())
				{
					row->cells.push_back((SheetCell*)0xffffffff);
					continue;
				}

				cells[i]=Trim(cells[i]);

				if (cells[i][0]==0)
				{
					row->cells.push_back((SheetCell*)0xffffffff);
					continue;
				}

				SheetCell cll;
				if (!bCompound)
				{
					cll.cSiblings=1;
					cll.s=cells[i];
					_cells.push_back(cll);
					row->cells.push_back((SheetCell*)(_cells.size()-1));//暂时先存一个索引
				}
				else
				{
					SplitStr(',',cells[i],pieces);

					assert(pieces.size()>0);

					for (int ii=0;ii<pieces.size();ii++)
					{
						pieces[ii]=Trim(pieces[ii]);

						cll.s=pieces[ii];

						if (ii==0)
							cll.cSiblings=pieces.size();
						else
							cll.cSiblings=0;

						_cells.push_back(cll);

						if (ii==0)
							row->cells.push_back((SheetCell*)(_cells.size()-1));//暂时先存一个索引
					}
				}

			}


		}

	}

	//将_rows里的暂存的索引转换为指针
	for (int j=0;j<_rows.size();j++)
	{
		SheetRow* row=_rows[j];

		for (int i=0;i<row->cells.size();i++)
		{
			unsigned __int64 idx=(unsigned __int64)row->cells[i];
			if (idx==0xffffffff)
				row->cells[i]=GetEmptyCell();
			else
				row->cells[i]=&_cells[idx];
		}
	}

	//将每个cell里的字符串转成数值(如果可以转的话)
	for (int i=0;i<_cells.size();i++)
		_cells[i].v=DoubleFromString(_cells[i].s);

	_caches.resize(_nCols);
	VEC_SET(_caches,0);

	return TRUE;

}


void CSheet::Clear()
{
	for (int i=0;i<_rows.size();i++)
		Safe_Class_Delete(_rows[i]);

	for (int i=0;i<_caches.size();i++)
		Safe_Class_Delete(_caches[i]);

	_caches.clear();
	_rows.clear();
	_cols.clear();
	_cells.clear();
	_buf.clear();

	_nCols=0;

}


ShtColID CSheet::FindColumn(const char *title)
{
	_error="";

	std::unordered_map<std::string,ShtColID>::iterator it=_cols.find(std::string(title));
	if (it==_cols.end())
	{
		AppendFmtString(_error,"无法在表格中找到名为\"%s\"的栏位!",title);
		return -1;
	}
	return (*it).second;
}

ColumnCache*CSheet::_BuildColumnCache(ShtColID col)
{
	assert(col<_nCols);
	if (_caches[col])
		return _caches[col];

	ColumnCache *cache=_caches[col]=Class_New2(ColumnCache);

	for (int i=0;i<_rows.size();i++)
	{
		SheetRow *row=_rows[i];

		SheetCell *cell=row->cells[col];

		cache->mp0[std::string(cell->s)]=i;
		cache->mp1[(int)cell->v]=i;
	}

	return cache;
}

ShtRowID CSheet::FindRow(const char *key,ShtColID col)
{
	if ((DWORD)col>=_nCols)
		return RowID_Invalid;
	ColumnCache*cache=_BuildColumnCache(col);

	std::unordered_map<std::string,ShtRowID>::iterator it=cache->mp0.find(std::string(key));
	if (it==cache->mp0.end())
		return RowID_Invalid;

	return (*it).second;
}

ShtRowID CSheet::FindRow(int key,ShtColID col)
{
	if ((DWORD)col>=_nCols)
		return RowID_Invalid;
	ColumnCache*cache=_BuildColumnCache(col);

	std::unordered_map<int,ShtRowID>::iterator it=cache->mp1.find(key);
	if (it==cache->mp1.end())
		return RowID_Invalid;

	return (*it).second;
}

SheetCell*CSheet::GetCell(ShtColID col,ShtRowID row,int idx)
{
	if ((DWORD)col>=_nCols)
		return NULL;
	if ((DWORD)row>=_rows.size())
		return NULL;
	SheetCell *ret=_rows[row]->cells[col];
	if(!ret)
		return ret;
	if (idx>=ret->cSiblings)
		return NULL;
	return ret+idx;
}

CSheetRow *CSheet::GetRow(ShtRowID row)
{
	if ((DWORD)row>=_rows.size())
		return NULL;
	CSheetRow *ret=Class_New2(CSheetRow);

	ret->_sht=this;
	ret->_sht->AddRef();
	ret->_row=row;
	ret->AddRef();
	return ret;
}


const char *CSheet::GetErrorStr()
{
	//清除掉最后的'\n'
	DWORD sz=_error.size();
	if (sz>0)
	{
		if (_error.c_str()[sz-1]=='\n')
			_error.resize(sz-1);
	}
	return _error.c_str();
}


