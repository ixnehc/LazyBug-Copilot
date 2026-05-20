/********************************************************************
	created:	2010/7/7   12:02
	file path:	d:\IxEngine\Common\dbreader
	author:		chenxi
	
	purpose:	data base reader
*********************************************************************/
#include "stdh.h"
#include "dbreader.h"


void CDBReader::Init(const char *filename,const char *tblname)
{
}

void CDBReader::Clear()
{

}

DWORD CDBReader::GetColumnCount()
{
	return 0;
}

DWORD CDBReader::GetRowCount()
{
	return 0;
}
const char *CDBReader::GetColumnName(DWORD idxColumn)
{
	return "";
}

void CDBReader::Reset()
{

}

BOOL CDBReader::NextColumn()
{
	return FALSE;
}

BOOL CDBReader::NextRow()
{
	return FALSE;
}

BOOL CDBReader::IsNumber()
{
	return TRUE;
}

double CDBReader::GetNumber()
{
	return 0.0;
}

const char *CDBReader::GetStr()
{
	return "";
}
