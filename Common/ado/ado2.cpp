//
// MODULE: Ado2.cpp
//
// AUTHOR: Carlos Antollini <cantollini@hotmail.com>
//
// Copyright (c) 2001-2004. All Rights Reserved.
//
// Date: August 01, 2005
//
// Version 2.20
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed unmodified by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name and all copyright 
// notices remains intact. 
//
// An email letting me know how you are using it would be nice as well. 
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability for any damage/loss of business that
// this product may cause.
//
//
//////////////////////////////////////////////////////////////////////

#include "ado2.h"
#include <vector>

static const char * IntToStr(int nVal)
{
	static char buff[10];

	itoa(nVal, buff, 10);
	return buff;
}

static const char * LongToStr(long lVal)
{
	static char buff[20];

	ltoa(lVal, buff, 10);
	return buff;
}

static const char * ULongToStr(unsigned long ulVal)
{
	static char buff[20];

	ultoa(ulVal, buff, 10);
	return buff;

}


static const char *DblToStr(double dblVal, int ndigits= 20)
{
	static char buff[50];

	_gcvt(dblVal, ndigits, buff);
	return buff;
}

static const char *DblToStr(float fltVal)
{
	static char buff[50];
	_gcvt(fltVal, 10, buff);
	return buff;
}




///////////////////////////////////////////////////////
//
// CADODatabase Class
//

DWORD CADODatabase::GetRecordCount(_RecordsetPtr m_pRs)
{
	DWORD numRows = 0;
	
	numRows = m_pRs->GetRecordCount();

	if(numRows == -1)
	{
		if(m_pRs->EndOfFile != VARIANT_TRUE)
			m_pRs->MoveFirst();

		while(m_pRs->EndOfFile != VARIANT_TRUE)
		{
			numRows++;
			m_pRs->MoveNext();
		}
		if(numRows > 0)
			m_pRs->MoveFirst();
	}
	return numRows;
}

BOOL CADODatabase::Open(LPCTSTR lpstrConnection, LPCTSTR lpstrUserID, LPCTSTR lpstrPassword)
{
	HRESULT hr = S_OK;

	if(IsOpen())
		Close();

	if(strcmp(lpstrConnection, ("")) != 0)
		m_strConnection = lpstrConnection;

//	ASSERT(!m_strConnection.empty());

	try
	{
		if(m_nConnectionTimeout != 0)
			m_pConnection->PutConnectionTimeout(m_nConnectionTimeout);
		hr = m_pConnection->Open(_bstr_t(m_strConnection.c_str()), _bstr_t(lpstrUserID), _bstr_t(lpstrPassword), NULL);
		return hr == S_OK;
	}
	catch(_com_error &)
	{
		return FALSE;
	}

}


BOOL CADODatabase::IsOpen()
{
	if(m_pConnection )
		return m_pConnection->GetState() != adStateClosed;
	return FALSE;
}

void CADODatabase::Close()
{
	if(IsOpen())
		m_pConnection->Close();
}


///////////////////////////////////////////////////////
//
// CADORecordset Class
//

CADORecordset::CADORecordset()
{
	m_pRecordset = NULL;
	m_pCmd = NULL;
	m_strQuery = ("");
	m_strLastError = ("");
	m_dwLastError = 0;
	m_pRecBinding = NULL;
	m_pRecordset.CreateInstance(__uuidof(Recordset));
	m_pCmd.CreateInstance(__uuidof(Command));
	m_nEditStatus = CADORecordset::dbEditNone;
	m_nSearchDirection = CADORecordset::searchForward;
}

CADORecordset::CADORecordset(CADODatabase* pAdoDatabase)
{
	m_pRecordset = NULL;
	m_pCmd = NULL;
	m_strQuery = ("");
	m_strLastError = ("");
	m_dwLastError = 0;
	m_pRecBinding = NULL;
	m_pRecordset.CreateInstance(__uuidof(Recordset));
	m_pCmd.CreateInstance(__uuidof(Command));
	m_nEditStatus = CADORecordset::dbEditNone;
	m_nSearchDirection = CADORecordset::searchForward;

	m_pConnection = pAdoDatabase->GetActiveConnection();
}

BOOL CADORecordset::OpenMDB(_ConnectionPtr mpdb, const char *table)
{	
	Close();
	
	if(strcmp(table, ("")) != 0)
		m_strQuery = table;

//	ASSERT(!m_strQuery.empty());

	if(m_pConnection == NULL)
		m_pConnection = mpdb;

	try
	{
		m_pRecordset->CursorType = adOpenKeyset;
		m_pRecordset->CursorLocation = adUseServer;
		m_pRecordset->Open((LPCSTR)table, _variant_t((IDispatch*)mpdb, TRUE), 
						adOpenKeyset, adLockOptimistic, adCmdTable);

		return m_pRecordset != NULL && m_pRecordset->GetState()!= adStateClosed;
	}
	catch(_com_error &)
	{
		return FALSE;
	}
}

BOOL CADORecordset::OpenXLS(_ConnectionPtr mpdb, const char *table)
{	
	Close();

	if(strcmp(table, ("")) != 0)
		m_strQuery = table;

// 	ASSERT(!m_strQuery.empty());

	if(m_pConnection == NULL)
		m_pConnection = mpdb;

	try
	{
		m_pRecordset->CursorType = adOpenDynamic;
		m_pRecordset->CursorLocation = adUseServer;

		std::string query;
		query="select * from [";
		query+=table;
		query+="$]";
		m_pRecordset->Open(query.c_str(), _variant_t((IDispatch*)mpdb, TRUE), 
			adOpenDynamic, adLockReadOnly, adCmdUnknown);

		return m_pRecordset != NULL && m_pRecordset->GetState()!= adStateClosed;
	}
	catch(_com_error &)
	{
		return FALSE;
	}
}


const char *CADORecordset::GetFieldValue(int nIndex)
{
	static std::vector<char> str;
	_variant_t vtFld;
	_variant_t vtIndex;

	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;
	

	vtFld = m_pRecordset->Fields->GetItem(vtIndex)->Value;
	switch(vtFld.vt) 
	{
	case VT_R4:
		return DblToStr(vtFld.fltVal);
	case VT_R8:
		return DblToStr(vtFld.dblVal);
	case VT_BSTR:
	{
		int len=SysStringLen(vtFld.bstrVal);
		str.resize(len*2+1);
		::WideCharToMultiByte(CP_ACP,            // ANSI Code Page
			0,                 // no flags
			vtFld.bstrVal,  // source widechar string
			-1,                // assume NUL-terminated
			&str[0],                 // target buffer
			(int)str.size(), // target buffer length
			NULL,              // use system default char
			NULL);             // don't care if default used
		return &str[0];
	}
	case VT_I2:
	case VT_UI1:
		return IntToStr(vtFld.iVal);
	case VT_INT:
		return IntToStr(vtFld.intVal);
	case VT_I4:
		return LongToStr(vtFld.lVal);
	case VT_UI4:
		return ULongToStr(vtFld.ulVal);
	case VT_DECIMAL:
		{
		//Corrected by Jos?Carlos Martínez Galán
		double val = vtFld.decVal.Lo32;
		val *= (vtFld.decVal.sign == 128)? -1 : 1;
		val /= pow(10, vtFld.decVal.scale); 
		return DblToStr(val);
		}
	case VT_DATE:
	case VT_CY:		//Added by John Andy Johnson!!!
	case VT_BOOL:
		return vtFld.boolVal == VARIANT_TRUE? "1":"0";
	}
	return "";
}


BOOL CADORecordset::IsFieldNull(LPCTSTR lpFieldName)
{
	_variant_t vtFld;
	
	vtFld = m_pRecordset->Fields->GetItem(lpFieldName)->Value;
	return vtFld.vt == VT_NULL;
}

BOOL CADORecordset::IsFieldNull(int nIndex)
{
	_variant_t vtFld;
	_variant_t vtIndex;

	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;
	
	vtFld = m_pRecordset->Fields->GetItem(vtIndex)->Value;
	return vtFld.vt == VT_NULL;
}

BOOL CADORecordset::IsFieldEmpty(LPCTSTR lpFieldName)
{
	_variant_t vtFld;
	
	vtFld = m_pRecordset->Fields->GetItem(lpFieldName)->Value;
	return vtFld.vt == VT_EMPTY || vtFld.vt == VT_NULL;
}

BOOL CADORecordset::IsFieldEmpty(int nIndex)
{
	_variant_t vtFld;
	_variant_t vtIndex;
	
	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;
	
	vtFld = m_pRecordset->Fields->GetItem(vtIndex)->Value;
	return vtFld.vt == VT_EMPTY || vtFld.vt == VT_NULL;
}


DWORD CADORecordset::GetRecordCount()
{
	DWORD nRows = 0;
	
	nRows = m_pRecordset->GetRecordCount();

	if(nRows == -1)
	{
		nRows = 0;
		if(m_pRecordset->EndOfFile != VARIANT_TRUE)
			m_pRecordset->MoveFirst();
		
		while(m_pRecordset->EndOfFile != VARIANT_TRUE)
		{
			nRows++;
			m_pRecordset->MoveNext();
		}
		if(nRows > 0)
			m_pRecordset->MoveFirst();
	}
	
	return nRows;
}

BOOL CADORecordset::IsOpen()
{
	if(m_pRecordset != NULL && IsConnectionOpen())
		return m_pRecordset->GetState() != adStateClosed;
	return FALSE;
}

void CADORecordset::Close()
{
	if(IsOpen())
	{
		m_pRecordset->PutSort((""));
		m_pRecordset->Close();	
	}
}


BOOL CADORecordset::GetFieldInfo(LPCTSTR lpFieldName, CADOFieldInfo* fldInfo)
{
	FieldPtr pField = m_pRecordset->Fields->GetItem(lpFieldName);
	
	return GetFieldInfo(pField, fldInfo);
}

BOOL CADORecordset::GetFieldInfo(int nIndex, CADOFieldInfo* fldInfo)
{
	_variant_t vtIndex;
	
	vtIndex.vt = VT_I2;
	vtIndex.iVal = nIndex;

	FieldPtr pField = m_pRecordset->Fields->GetItem(vtIndex);

	return GetFieldInfo(pField, fldInfo);
}


BOOL CADORecordset::GetFieldInfo(FieldPtr pField, CADOFieldInfo* fldInfo)
{
	memset(fldInfo, 0, sizeof(CADOFieldInfo));

	strcpy(fldInfo->m_strName, (LPCTSTR)pField->GetName());
	fldInfo->m_lDefinedSize = pField->GetDefinedSize();
	fldInfo->m_nType = pField->GetType();
	fldInfo->m_lAttributes = pField->GetAttributes();
	if(!IsEof())
		fldInfo->m_lSize = pField->GetActualSize();
	return TRUE;
}


std::string CADORecordset::GetString(LPCTSTR lpCols, LPCTSTR lpRows, LPCTSTR lpNull, long numRows)
{
	_bstr_t varOutput;
	_bstr_t varNull("");
	_bstr_t varCols("\t");
	_bstr_t varRows("\r");

	if(strlen(lpCols) != 0)
		varCols = _bstr_t(lpCols);

	if(strlen(lpRows) != 0)
		varRows = _bstr_t(lpRows);
	
	if(numRows == 0)
		numRows =(long)GetRecordCount();			
			
	varOutput = m_pRecordset->GetString(adClipString, numRows, varCols, varRows, varNull);

	return (LPCTSTR)varOutput;
}


BOOL DumpDBTable(BOOL bMDBorXLS,const char *pathDB,const char *table,std::string &result)
{
	result="";

	CADODatabase db;
	if (bMDBorXLS)
	{//MDB
		std::string s;
		s="Provider=Microsoft.Jet.OLEDB.4.0;Data Source=";
		s+=pathDB;
		db.SetConnectionString(s.c_str());
	}
	else
	{//XLS
		std::string s;
		s="Provider=Microsoft.Jet.OLEDB.4.0;Extended Properties=Excel 8.0;Data Source=";
		s+=pathDB;
		db.SetConnectionString(s.c_str());
	}
	db.SetConnectionMode((CADODatabase::cadoConnectModeEnum)(CADODatabase::connectModeRead));
	if (!db.Open())
		return FALSE;

	CADORecordset rcd;
	if (bMDBorXLS)
	{
		if(!rcd.OpenMDB(db.GetActiveConnection(),table))
		{
			db.Close();
			return FALSE;
		}
	}
	else
	{
		if(!rcd.OpenXLS(db.GetActiveConnection(),table))
		{
			db.Close();
			return FALSE;
		}
	}


	int nFields=rcd.GetFieldCount();

	CADOFieldInfo info;
	for (int i=0;i<nFields;i++)
	{
		if (i>0)
			result+="\t";
		rcd.GetFieldInfo(i,&info);
		result+=info.m_strName;
	}
	while(!rcd.IsEof())
	{

		result+="\r\n";
		BOOL bEmptyRecord=TRUE;
		for (int i=0;i<nFields;i++)
		{
			if (i>0)
				result+="\t";
			const char *s=rcd.GetFieldValue(i);
			if (s[0])
				bEmptyRecord=FALSE;
			result+=s;
		}
		if (bEmptyRecord)
			break;
		rcd.MoveNext();
	}
	rcd.Close();


	db.Close();

	return TRUE;
}