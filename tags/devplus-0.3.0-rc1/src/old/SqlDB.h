// SqlDB.h: interface for the CSqlDB class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SQLDB_H__FD95BA0E_7B56_4B01_BDCB_BA9C8628872F__INCLUDED_)
#define AFX_SQLDB_H__FD95BA0E_7B56_4B01_BDCB_BA9C8628872F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <sql.h>
#include <afxmt.h>	// for CCriticalSection.



struct _SQL_ColumnData {
	SQLSMALLINT	 ColumnNumber;
	SQLCHAR		*ColumnName;
	SQLSMALLINT  DataType;
	SQLUINTEGER  ColumnSize;
	SQLSMALLINT  DecimalDigits;
	SQLSMALLINT  Nullable;
};



//-----------------------------------------------------------------------------
// CJW: Result Set from an SQL query.   Every Executed statement should return 
//		a result object which has all the information about the result.
//		When the statement handle is passed to this object, it gets information 
//		about the rows and items.  
class CSqlResult 
{

public:
	char * LastState();
	char * LastText();
	SQLHANDLE GetHandle();
	SQLRETURN Status();
	bool GetData(const char *field, char **value);
	bool GetData(const char *field, int *value);

	bool NextRow();
	void SetLastQuery(char *query);
	unsigned int Items();
	void Process(SQLRETURN ret, char *state=NULL, char *text=NULL);

	CSqlResult(void *pParent);
	virtual ~CSqlResult();

private:
	char * m_lastErrorText;
	char * m_lastState;
	unsigned int m_nCurrentRow;			// 1 based... 1 is first row.
	_SQL_ColumnData *m_pColumnData;
	SQLSMALLINT m_nColumns;
	char * m_szLastQuery;
	CCriticalSection m_xLock;
	void * m_pParent;
	SQLHANDLE m_hStatement;
	SQLRETURN m_lastReturn;
};


// CJW: Database object.
class CSqlDB  
{
public:
	bool Commit();
	void DeleteResult(CSqlResult *ptr);
	SQLHANDLE GetHandle();
	bool Connected();
	CSqlResult * Execute(char *query, ...);
	void Close();
	virtual bool Connect(char *source, char *user, char *pass);
	CSqlDB();
	virtual ~CSqlDB();


private:
	CSqlResult * NewResult();

private:
	bool m_bConnected;
	char * m_szLastQuery;
	SQLHANDLE m_hEnv;
	SQLHANDLE m_hConnection;
	CCriticalSection m_xLock;
	CSqlResult **m_pResultList;
	unsigned int m_nItems;
};

#endif // !defined(AFX_SQLDB_H__FD95BA0E_7B56_4B01_BDCB_BA9C8628872F__INCLUDED_)
