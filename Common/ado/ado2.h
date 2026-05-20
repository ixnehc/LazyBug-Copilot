//
// MODULE: Ado2.h
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

#pragma once
#include <math.h>

#include <string>

#pragma warning (disable: 4146)
// CG : In order to use this code against a different version of ADO, the appropriate
// ADO library needs to be used in the #import statement
#pragma message ("Make sure you go to Tools.Options.Directories.Library files and add the paths to msado15.dll and msjro.dll will usually be in C:\\Program Files\\Common Files\\System\\ado")
#import "C:\Program Files\Common Files\System\ado\msado15.dll" rename("EOF", "EndOfFile")
//#import "C:\Program Files\Common Files\System\ado\MSJRO.DLL" no_namespace rename("ReplicaTypeEnum", "_ReplicaTypeEnum") 

//#import <msado15.dll> rename("EOF", "EndOfFile")
#import <C:\Program Files\Common Files\System\ado\MSJRO.DLL> no_namespace rename("ReplicaTypeEnum", "_ReplicaTypeEnum") 

using namespace ADODB;


#pragma warning (default: 4146)

#include "icrsint.h"

class CADOCommand;

struct CADOFieldInfo
{
	char m_strName[30]; 
	short m_nType;
	long m_lSize; 
	long m_lDefinedSize;
	long m_lAttributes;
	short m_nOrdinalPosition;
	BOOL m_bRequired;   
	BOOL m_bAllowZeroLength; 
	long m_lCollatingOrder;  
};



class CADODatabase
{
public:
	enum cadoConnectModeEnum
    {	
		connectModeUnknown = adModeUnknown,
		connectModeRead = adModeRead,
		connectModeWrite = adModeWrite,
		connectModeReadWrite = adModeReadWrite,
		connectModeShareDenyRead = adModeShareDenyRead,
		connectModeShareDenyWrite = adModeShareDenyWrite,
		connectModeShareExclusive = adModeShareExclusive,
		connectModeShareDenyNone = adModeShareDenyNone
    };

	CADODatabase()
	{
		::CoInitialize(NULL);
			
		m_pConnection = NULL;
		m_strConnection = ("");
		m_strLastError = ("");
		m_dwLastError = 0;
		m_pConnection.CreateInstance(__uuidof(Connection));
		m_nRecordsAffected = 0;
		m_nConnectionTimeout = 0;
	}
	
	virtual ~CADODatabase()
	{
		Close();
		m_pConnection.Release();
		m_pConnection = NULL;
		m_strConnection = ("");
		m_strLastError = ("");
		m_dwLastError = 0;
		::CoUninitialize();
	}
	
	BOOL Open(LPCTSTR lpstrConnection = (""), LPCTSTR lpstrUserID = (""), LPCTSTR lpstrPassword = (""));
	_ConnectionPtr GetActiveConnection() 
		{return m_pConnection;};
	int GetRecordsAffected()
		{return m_nRecordsAffected;};
	DWORD GetRecordCount(_RecordsetPtr m_pRs);
	long BeginTransaction() 
		{return m_pConnection->BeginTrans();};
	long CommitTransaction() 
		{return m_pConnection->CommitTrans();};
	long RollbackTransaction() 
		{return m_pConnection->RollbackTrans();};
	BOOL IsOpen();
	void Close();
	void SetConnectionMode(cadoConnectModeEnum nMode)
		{m_pConnection->PutMode((enum ConnectModeEnum)nMode);};
	void SetConnectionString(LPCTSTR lpstrConnection)
		{m_strConnection = lpstrConnection;};
	std::string GetConnectionString()
		{return m_strConnection;};
	std::string GetLastErrorString() 
		{return m_strLastError;};
	DWORD GetLastError()
		{return m_dwLastError;};
	std::string GetErrorDescription() 
		{return m_strErrorDescription;};
	void SetConnectionTimeout(long nConnectionTimeout = 30)
		{m_nConnectionTimeout = nConnectionTimeout;};

protected:
	void dump_com_error(_com_error &e);

public:
	_ConnectionPtr m_pConnection;
	
protected:
	std::string m_strConnection;
	std::string m_strLastError;
	std::string m_strErrorDescription;
	DWORD m_dwLastError;
	int m_nRecordsAffected;
	long m_nConnectionTimeout;
};

class CADORecordset
{
public:

	enum cadoOpenEnum
	{
		openUnknown = 0,
		openQuery = 1,
		openTable = 2,
		openStoredProc = 3
	};

	enum cadoEditEnum
	{
		dbEditNone = 0,
		dbEditNew = 1,
		dbEdit = 2
	};
	
	enum cadoPositionEnum
	{
	
		positionUnknown = -1,
		positionBOF = -2,
		positionEOF = -3
	};
	
	enum cadoSearchEnum
	{	
		searchForward = 1,
		searchBackward = -1
	};

	enum cadoDataType
	{
		typeEmpty = ADODB::adEmpty,
		typeTinyInt = ADODB::adTinyInt,
		typeSmallInt = ADODB::adSmallInt,
		typeInteger = ADODB::adInteger,
		typeBigInt = ADODB::adBigInt,
		typeUnsignedTinyInt = ADODB::adUnsignedTinyInt,
		typeUnsignedSmallInt = ADODB::adUnsignedSmallInt,
		typeUnsignedInt = ADODB::adUnsignedInt,
		typeUnsignedBigInt = ADODB::adUnsignedBigInt,
		typeSingle = ADODB::adSingle,
		typeDouble = ADODB::adDouble,
		typeCurrency = ADODB::adCurrency,
		typeDecimal = ADODB::adDecimal,
		typeNumeric = ADODB::adNumeric,
		typeBoolean = ADODB::adBoolean,
		typeError = ADODB::adError,
		typeUserDefined = ADODB::adUserDefined,
		typeVariant = ADODB::adVariant,
		typeIDispatch = ADODB::adIDispatch,
		typeIUnknown = ADODB::adIUnknown,
		typeGUID = ADODB::adGUID,
		typeDate = ADODB::adDate,
		typeDBDate = ADODB::adDBDate,
		typeDBTime = ADODB::adDBTime,
		typeDBTimeStamp = ADODB::adDBTimeStamp,
		typeBSTR = ADODB::adBSTR,
		typeChar = ADODB::adChar,
		typeVarChar = ADODB::adVarChar,
		typeLongVarChar = ADODB::adLongVarChar,
		typeWChar = ADODB::adWChar,
		typeVarWChar = ADODB::adVarWChar,
		typeLongVarWChar = ADODB::adLongVarWChar,
		typeBinary = ADODB::adBinary,
		typeVarBinary = ADODB::adVarBinary,
		typeLongVarBinary = ADODB::adLongVarBinary,
		typeChapter = ADODB::adChapter,
		typeFileTime = ADODB::adFileTime,
		typePropVariant = ADODB::adPropVariant,
		typeVarNumeric = ADODB::adVarNumeric,
		typeArray = ADODB::adVariant
	};
	
	enum cadoSchemaType 
	{
		schemaSpecific = adSchemaProviderSpecific,	
		schemaAsserts = adSchemaAsserts,
		schemaCatalog = adSchemaCatalogs,
		schemaCharacterSet = adSchemaCharacterSets,
		schemaCollections = adSchemaCollations,
		schemaColumns = adSchemaColumns,
		schemaConstraints = adSchemaCheckConstraints,
		schemaConstraintColumnUsage = adSchemaConstraintColumnUsage,
		schemaConstraintTableUsage  = adSchemaConstraintTableUsage,
		shemaKeyColumnUsage = adSchemaKeyColumnUsage,
		schemaTableConstraints = adSchemaTableConstraints,
		schemaColumnsDomainUsage = adSchemaColumnsDomainUsage,
		schemaIndexes = adSchemaIndexes,
		schemaColumnPrivileges = adSchemaColumnPrivileges,
		schemaTablePrivileges = adSchemaTablePrivileges,
		schemaUsagePrivileges = adSchemaUsagePrivileges,
		schemaProcedures = adSchemaProcedures,
		schemaTables = adSchemaTables,
		schemaProviderTypes = adSchemaProviderTypes,
		schemaViews = adSchemaViews,
		schemaViewTableUsage = adSchemaViewTableUsage,
		schemaProcedureParameters = adSchemaProcedureParameters,
		schemaForeignKeys = adSchemaForeignKeys,
		schemaPrimaryKeys = adSchemaPrimaryKeys,
		schemaProcedureColumns = adSchemaProcedureColumns,
		schemaDBInfoKeywords = adSchemaDBInfoKeywords,
		schemaDBInfoLiterals = adSchemaDBInfoLiterals,
		schemaCubes = adSchemaCubes,
		schemaDimensions = adSchemaDimensions,
		schemaHierarchies  = adSchemaHierarchies, 
		schemaLevels = adSchemaLevels,
		schemaMeasures = adSchemaMeasures,
		schemaProperties = adSchemaProperties,
		schemaMembers = adSchemaMembers,
	}; 


	CADORecordset();

	CADORecordset(CADODatabase* pAdoDatabase);

	virtual ~CADORecordset()
	{
		Close();
		if(m_pRecordset)
			m_pRecordset.Release();
		if(m_pCmd)
			m_pCmd.Release();
		m_pRecordset = NULL;
		m_pCmd = NULL;
		m_pRecBinding = NULL;
		m_strQuery = ("");
		m_strLastError = ("");
		m_dwLastError = 0;
		m_nEditStatus = dbEditNone;
	}

	std::string GetQuery() 
		{return m_strQuery;};
	void SetQuery(LPCSTR strQuery) 
		{m_strQuery = strQuery;};
	DWORD GetRecordCount();
	BOOL IsOpen();
	void Close();
	BOOL OpenMDB(_ConnectionPtr mpdb, const char *table);
	BOOL OpenXLS(_ConnectionPtr mpdb, const char *table);
	long GetFieldCount()
		{return m_pRecordset->Fields->GetCount();};
	const char *GetFieldValue(int nIndex);
	
	BOOL IsFieldNull(LPCTSTR lpFieldName);
	BOOL IsFieldNull(int nIndex);
	BOOL IsFieldEmpty(LPCTSTR lpFieldName);
	BOOL IsFieldEmpty(int nIndex);	
	BOOL IsEof()		{return m_pRecordset->EndOfFile == VARIANT_TRUE;};
	BOOL IsEOF()		{return m_pRecordset->EndOfFile == VARIANT_TRUE;};
	BOOL IsBof()		{return m_pRecordset->BOF == VARIANT_TRUE;};
	BOOL IsBOF()		{return m_pRecordset->BOF == VARIANT_TRUE;};
	void MoveFirst() 		{m_pRecordset->MoveFirst();};
	void MoveNext() 		{m_pRecordset->MoveNext();};
	void MovePrevious() 		{m_pRecordset->MovePrevious();};
	void MoveLast() 		{m_pRecordset->MoveLast();};
	long GetAbsolutePage()		{return m_pRecordset->GetAbsolutePage();};
	void SetAbsolutePage(int nPage)		{m_pRecordset->PutAbsolutePage((enum PositionEnum)nPage);};
	long GetPageCount()		{return m_pRecordset->GetPageCount();};
	long GetPageSize()		{return m_pRecordset->GetPageSize();};
	void SetPageSize(int nSize)		{m_pRecordset->PutPageSize(nSize);};
	long GetAbsolutePosition()		{return m_pRecordset->GetAbsolutePosition();};
	void SetAbsolutePosition(int nPosition)		{m_pRecordset->PutAbsolutePosition((enum PositionEnum)nPosition);};
	BOOL GetFieldInfo(LPCTSTR lpFieldName, CADOFieldInfo* fldInfo);
	BOOL GetFieldInfo(int nIndex, CADOFieldInfo* fldInfo);


	std::string GetString(LPCTSTR lpCols, LPCTSTR lpRows, LPCTSTR lpNull, long numRows = 0);
	std::string GetLastErrorString() 
		{return m_strLastError;};
	DWORD GetLastError()
		{return m_dwLastError;};
	BOOL IsConnectionOpen()
		{return m_pConnection != NULL && m_pConnection->GetState() != adStateClosed;};
	_RecordsetPtr GetRecordset()
		{return m_pRecordset;};
	_ConnectionPtr GetActiveConnection() 
		{return m_pConnection;};

public:
	_RecordsetPtr m_pRecordset;
	_CommandPtr m_pCmd;
	
protected:

	_ConnectionPtr m_pConnection;
	int m_nSearchDirection;
	std::string m_strFind;
	_variant_t m_varBookFind;
	_variant_t m_varBookmark;
	int m_nEditStatus;
	std::string m_strLastError;
	DWORD m_dwLastError;
	void dump_com_error(_com_error &e);
	IADORecordBinding *m_pRecBinding;
	std::string m_strQuery;

protected:
	BOOL GetFieldInfo(FieldPtr pField, CADOFieldInfo* fldInfo);
		
};
