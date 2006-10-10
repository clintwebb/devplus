//-----------------------------------------------------------------------------
// SqlDB.cpp: implementation of the CSqlDB class.
//
//	See "SqlDB.h" for more details.
//-----------------------------------------------------------------------------

#include <afxwin.h>
#include "SqlDB.h"
#include "SQLEXT.H"




//-----------------------------------------------------------------------------
// CJW: Constructor...
CSqlDB::CSqlDB()
{
	m_xLock.Lock();
	m_hEnv = 0;
	m_hConnection = 0;
	m_pResultList = NULL;
	m_nItems = 0;
	m_szLastQuery = NULL;
	m_bConnected = false;
	m_xLock.Unlock();
}




//-----------------------------------------------------------------------------
// CJW: Deconstructor.    Clean up everything.
CSqlDB::~CSqlDB()
{
	m_xLock.Lock();

	while (m_nItems > 0) {
		if (m_pResultList[m_nItems-1] != NULL) {
			delete m_pResultList[m_nItems-1];
//			m_pResultList[m_nItems-1] = NULL;
		}
	}

	if (m_pResultList) {
		free (m_pResultList);
		m_pResultList = NULL;
	}

	if (m_hConnection != 0) {
		if (m_bConnected) { 
			SQLDisconnect(m_hConnection);
		}
		SQLFreeHandle(SQL_HANDLE_DBC, m_hConnection);
		m_hConnection = 0;
	}

	if (m_hEnv != 0) {
		SQLFreeHandle(SQL_HANDLE_ENV, m_hEnv);
		m_hEnv = 0;
	}

	if (m_szLastQuery != NULL) {
		free(m_szLastQuery);
		m_szLastQuery = NULL;
	}

	m_xLock.Unlock();
}



//-----------------------------------------------------------------------------
// CJW: Connect to the indicated data source.  Return true if we connected, 
//		return false if we didnt.
bool CSqlDB::Connect(char *source, char *user, char *pass)
{
	SQLRETURN nReturn;

	m_xLock.Lock();

	// CJW: if the handles are already connected, then the programmer didnt 
	//		close the last one.  In reality a new object should be created 
	//		for a new connection.
	ASSERT(m_hEnv == 0);
	ASSERT(m_hConnection == 0);
	ASSERT(m_bConnected == false);

	ASSERT(source != NULL);
	ASSERT(user != NULL);
	ASSERT(pass != NULL);


	// In order to connect to the an ODBC data source, we need to first create some handles.

	nReturn = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_hEnv);
	if (nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO) {
		// we have a valid environment...

		nReturn = SQLSetEnvAttr(m_hEnv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0); 
		if (nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO) {

			nReturn = SQLAllocHandle(SQL_HANDLE_DBC, m_hEnv, &m_hConnection);
			if (nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO) {
				// we have a valid connection handle...
			
				nReturn = SQLConnect(m_hConnection, (unsigned char *) source, strlen(source), (unsigned char *) user, strlen(user), (unsigned char *) pass, strlen(pass));
				if (nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO) {
					m_bConnected = true;
				}
				else {
					// CJW: Failed while trying connect to the database.
					SQLFreeHandle(SQL_HANDLE_DBC, m_hConnection);
					m_hConnection = 0;

					SQLFreeHandle(SQL_HANDLE_ENV, m_hEnv);
					m_hEnv = 0;
				}
					
			}
			else {
				// CJW: Failed while trying create the connection environment..
				SQLFreeHandle(SQL_HANDLE_ENV, m_hEnv);
				m_hEnv = 0;
			}
		}
		else {
			// CJW: Failed while trying to set the environment.
			SQLFreeHandle(SQL_HANDLE_ENV, m_hEnv);
			m_hEnv = 0;
		}
	}

	m_xLock.Unlock();

	return(m_bConnected);
}


//-----------------------------------------------------------------------------
// CJW: Close the connection.
void CSqlDB::Close()
{
	m_xLock.Lock();
	ASSERT(m_hConnection != 0);
	SQLDisconnect(m_hConnection);
	SQLFreeHandle(SQL_HANDLE_DBC, m_hConnection);
	m_hConnection = 0;
	m_xLock.Unlock();
}



//-----------------------------------------------------------------------------
// CJW: Sql result set constructor.
CSqlResult::CSqlResult(void *pParent)
{
	CSqlDB *pDB;

	ASSERT(pParent != NULL);

	m_xLock.Lock();

	m_pParent = pParent;
	m_szLastQuery = NULL;
	m_nColumns = 0;
	m_pColumnData = NULL;
	m_nCurrentRow = 0;

	m_lastState = NULL;
	m_lastErrorText = NULL;
	
	// CJW: now create the statement handle.
	pDB = (CSqlDB *) pParent;
	m_lastReturn = SQLAllocHandle(SQL_HANDLE_STMT, pDB->GetHandle(), &m_hStatement);

	// CJW: This should succeed... if not.. something is configured wrong.
	ASSERT(m_lastReturn == SQL_SUCCESS || m_lastReturn == SQL_SUCCESS_WITH_INFO);

	m_xLock.Unlock();
}


//-----------------------------------------------------------------------------
// CJW: Sql result set destructor.
CSqlResult::~CSqlResult()
{
	CSqlDB *pTmp;
	int nCount;

	m_xLock.Lock();

	if (m_hStatement != 0) {
		SQLFreeHandle(SQL_HANDLE_STMT, m_hStatement);
		m_hStatement = 0;
	}

	if (m_pParent != NULL) {
		pTmp = (CSqlDB *) m_pParent;
		pTmp->DeleteResult(this);
	}

	if (m_szLastQuery != NULL) {
		free(m_szLastQuery);
		m_szLastQuery = NULL;
	}

	if (m_pColumnData != NULL) {
		ASSERT(m_nColumns > 0);
		for (nCount = 0; nCount < m_nColumns; nCount++) {
			if (m_pColumnData[nCount].ColumnName != NULL) {
				free(m_pColumnData[nCount].ColumnName);
				m_pColumnData[nCount].ColumnName = NULL;
			}
		}
		free(m_pColumnData);
		m_pColumnData = NULL;
	}

	if (m_lastState) {
		free(m_lastState);
		m_lastState = NULL;
	}

	if (m_lastErrorText) {
		free(m_lastErrorText);
		m_lastErrorText = NULL;
	}

	m_xLock.Unlock();
}



//-----------------------------------------------------------------------------
// CJW: Execute the Query.  If everything is good, return a CSqlResult object 
//		pointer.  The object would actually be created in our results list.   
CSqlResult * CSqlDB::Execute(char *query, ...)
{
	CSqlResult *pResult = NULL;
	SQLRETURN vReturn;
	int err;
	va_list ap;
#ifdef _DEBUG 
	char *tmp=NULL;
	int nLoopCount=0;
#endif

	SQLCHAR		Sqlstate[16];
	SQLINTEGER	NativeError;
	SQLCHAR		MessageText[1024];
	SQLSMALLINT TextLength;


	m_xLock.Lock();

	ASSERT(m_hConnection != 0);
	ASSERT(query != NULL);

	// CJW: If this object was already used for a query, we need to clear the last query.
	if (m_szLastQuery != NULL) {
		free(m_szLastQuery);
	}

	m_szLastQuery = (char *) malloc(32767);
	if (m_szLastQuery)
	{
		va_start(ap, query);
		err = vsprintf(m_szLastQuery, query, ap);
		va_end(ap);
		m_szLastQuery = (char *) realloc(m_szLastQuery, strlen(m_szLastQuery)+1);
		ASSERT(m_szLastQuery != NULL);
		if (m_szLastQuery) {

#ifdef _DEBUG_SQL_TRACE
			tmp = (char *) malloc(512);
			strncpy(tmp, m_szLastQuery, 255);
			strcat(tmp, "\n");
			TRACE(tmp);
			free(tmp);
#endif

			if (err > 0) {

				pResult = NewResult();
				if (pResult) {
					vReturn = SQLExecDirect(pResult->GetHandle(), (unsigned char *) m_szLastQuery, strlen(m_szLastQuery));

					ASSERT(vReturn != SQL_INVALID_HANDLE);
					if (vReturn == SQL_INVALID_HANDLE) {
						// CJW: Invalid handle?  How did that happen?
						m_bConnected = false;
						m_hConnection = 0;
					}
					else if (vReturn == SQL_ERROR) {
						// CJW: We've got an error, now we need to see what the code was.  If it 
						//		was indicating that the connection was lost, then we need to close 
						//		the connection, and clear the handles.

						SQLGetDiagRec(SQL_HANDLE_STMT, pResult->GetHandle(), 1, Sqlstate, &NativeError, MessageText, 1024, &TextLength);
						if (strcmp((const char *) Sqlstate, "08S01") == 0) {
							m_bConnected = false;
							delete pResult;
							pResult = NULL;
						}
#ifdef _DEBUG
						else if (strcmp((const char *) Sqlstate, "HY000") == 0) {
							// We are waiting for an original query to complete that competes with this query...
							ASSERT(0);  
						}
#endif
						else {
							pResult->Process(vReturn, (char *) Sqlstate, (char *) MessageText);
							pResult->SetLastQuery(m_szLastQuery);
						}
					}
					else {
						pResult->Process(vReturn);
						pResult->SetLastQuery(m_szLastQuery);
					}
				}
			}
		}
	}
	
	m_xLock.Unlock();

	return(pResult);
}


//-----------------------------------------------------------------------------
// CJW: Create a new 'result' object and add it to our results list so that it 
//		can be managed and removed when the connection is closed.
CSqlResult * CSqlDB::NewResult()
{
	CSqlResult *pResult;

	// CJW: If you have more than 20 results open, then maybe you are note 
	//		deleting the results returned from EVERY Execute().
	ASSERT(m_nItems < 20);


	// make sure that we are sync'd
#ifdef _DEBUG
	if (m_nItems == 0) {
		ASSERT(m_pResultList == NULL);
	}
	else {
		ASSERT(m_pResultList != NULL);
	}
#endif

	pResult = new CSqlResult(this);
	if (pResult != NULL) {
		m_pResultList = (CSqlResult **) realloc(m_pResultList, sizeof(CSqlResult *) * (m_nItems+1));
		if (m_pResultList) {
			m_pResultList[m_nItems] = pResult;
			m_nItems++;
		}
		else {
			m_nItems = 0;
		}
	}

	return(pResult);
}


//-----------------------------------------------------------------------------
// CJW: Set the return code of our last operation so that the upper-level logic 
//		can determine the error that was produced if things didnt go so well.
void CSqlResult::Process(SQLRETURN ret, char *state, char *text)
{
	SQLSMALLINT nCount;
	SQLRETURN nReturn;
	SQLSMALLINT nLen;

	m_xLock.Lock();

	ASSERT(m_hStatement != 0);
	ASSERT(m_nColumns == 0);
	ASSERT(m_pColumnData == NULL);

	m_lastReturn = ret;

	if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {

		// Get the column names.
		SQLNumResultCols(m_hStatement, &m_nColumns);
		if (m_nColumns > 0) {
			m_pColumnData = (_SQL_ColumnData *) malloc(sizeof(_SQL_ColumnData) * m_nColumns);

			for(nCount = 0; nCount < m_nColumns; nCount++) {
				m_pColumnData[nCount].ColumnNumber = (SQLSMALLINT) nCount+1;
				m_pColumnData[nCount].ColumnName = (unsigned char *) malloc(256);

				ASSERT(m_pColumnData[nCount].ColumnName != NULL);
				if (m_pColumnData[nCount].ColumnName != NULL) {
					nReturn = SQLDescribeCol(
						m_hStatement, 
						m_pColumnData[nCount].ColumnNumber, 
						m_pColumnData[nCount].ColumnName, 256, &nLen, 
						&(m_pColumnData[nCount].DataType),
						&(m_pColumnData[nCount].ColumnSize),
						&(m_pColumnData[nCount].DecimalDigits),
						&(m_pColumnData[nCount].Nullable));

					// display a debug assertion if this failed. 
					ASSERT(nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO);
					if (nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO) {
						m_pColumnData[nCount].ColumnName = (unsigned char *) realloc(m_pColumnData[nCount].ColumnName, nLen+1);
					}
					else {
						free(m_pColumnData[nCount].ColumnName);
						nCount = m_nColumns;
						m_nColumns = 0;
					}
				}
			}
		}
	}
	else {
		if (state != NULL && text != NULL) {
	
			ASSERT(m_lastState == NULL);
			ASSERT(m_lastErrorText == NULL);

			m_lastState = (char *) malloc(strlen(state) + 1);
			m_lastErrorText = (char *) malloc(strlen(text) + 1);

			ASSERT(m_lastState != NULL);
			ASSERT(m_lastErrorText != NULL);

			strcpy(m_lastState, state);
			strcpy(m_lastErrorText, text);
		}
	}

	m_xLock.Unlock();
}



//-----------------------------------------------------------------------------
// CJW: Return the number of items that were returned from a Query.
unsigned int CSqlResult::Items()
{
	SQLINTEGER nItems;

	m_xLock.Lock();

	ASSERT(m_hStatement != 0);

	m_lastReturn = SQLRowCount(m_hStatement, &nItems);
	m_xLock.Unlock();

	return(nItems);
}


//-----------------------------------------------------------------------------
// CJW: Set the last query so that we know what this record set was about.
void CSqlResult::SetLastQuery(char *query)
{
	ASSERT(m_szLastQuery == NULL);

	m_xLock.Lock();
	m_szLastQuery = (char *) malloc(strlen(query) + 1);
	if (m_szLastQuery != NULL) {
		strcpy(m_szLastQuery, query);
	}
	m_xLock.Unlock();
}



//-----------------------------------------------------------------------------
// CJW: Returns true if there is a row of data available.
bool CSqlResult::NextRow()
{
	bool bReturn = false;

	m_xLock.Lock();

	ASSERT(m_hStatement != 0);
	if (m_hStatement != 0) {
		ASSERT(m_lastReturn == SQL_SUCCESS || m_lastReturn == SQL_SUCCESS_WITH_INFO);

		m_lastReturn = SQLFetch(m_hStatement);
		if (m_lastReturn == SQL_SUCCESS || m_lastReturn == SQL_SUCCESS_WITH_INFO) {
			m_nCurrentRow ++;
			ASSERT(m_nCurrentRow > 0);
			bReturn = true;
		}

	}

	m_xLock.Unlock();

	return(bReturn);
}



//-----------------------------------------------------------------------------
// CJW: Get data from the current row and create some memory for this item, 
//		storing the pointer in value.  if this operation fails, do not modify 
//		value.
bool CSqlResult::GetData(const char *field, char **value)
{
	bool bReturn = FALSE;
	int nCount;
	SQLRETURN nSqlReturn;
	SQLINTEGER nLen;
	SQLINTEGER nStrLen;
	char *tmp;
	SQL_TIMESTAMP_STRUCT *ts;
	

#ifdef _DEBUG
	SQLCHAR		Sqlstate[16];
	SQLINTEGER	NativeError;
	SQLCHAR		MessageText[1024];
	SQLSMALLINT TextLength;

#endif

	ASSERT(field != NULL);
	ASSERT(value != NULL);
	
	m_xLock.Lock();

	ASSERT(m_hStatement != 0);
	ASSERT(m_nColumns > 0);
	ASSERT(m_pColumnData != NULL);
	ASSERT(m_nCurrentRow > 0);

	ASSERT(m_lastReturn == SQL_SUCCESS || m_lastReturn == SQL_SUCCESS_WITH_INFO);


	// CJW: we have the name of a column... we need to go thru the columns list 
	//		to find the one we want.
	
	for (nCount = 0; nCount < m_nColumns; nCount++) {
		if (stricmp(field, (const char *) m_pColumnData[nCount].ColumnName) == 0) {
			// we found the match...

			if (m_pColumnData[nCount].DataType == SQL_TYPE_TIMESTAMP) {
				// This is a timestamp.

				ts = (SQL_TIMESTAMP_STRUCT *) malloc(sizeof(SQL_TIMESTAMP_STRUCT));
				ASSERT(ts != NULL);
				nSqlReturn = SQLGetData(
					m_hStatement,						// SQLHSTMT		StatementHandle
					m_pColumnData[nCount].ColumnNumber,	// SQLUSMALLINT	ColumnNumber
					SQL_TYPE_TIMESTAMP,					// SQLSMALLINT	TargetType
					ts,									// SQLPOINTER	TargetValuePtr
					sizeof(SQL_TIMESTAMP_STRUCT),		// SQLINTEGER	BufferLength
					&nLen);								// SQLINTEGER   *StrLen_or_IndPtr

				if (nSqlReturn == SQL_SUCCESS || nSqlReturn == SQL_SUCCESS_WITH_INFO) {
					bReturn = true;
					tmp = (char *) malloc(64);
					if (tmp) {
						sprintf(tmp, "%04d-%02d-%02d %02d:%02d:%02d %s",
							ts->year, ts->month,  ts->day,
							ts->hour, ts->minute, ts->second,
							ts->hour < 12 ? "AM" : "PM");
						tmp = (char *) realloc(tmp, strlen(tmp)+1);
						value[0] = tmp;
					}
				}
#ifdef _DEBUG
				else {
					// get some more information about why this failed.
					SQLGetDiagRec(SQL_HANDLE_STMT, m_hStatement, 1, Sqlstate, &NativeError, MessageText, 1024, &TextLength);
					// Code of 07009 indicates that the data was retreived out of column sequence.
				}
#endif

				free(ts);

			}
			else {
				// this is not a timestamp.

				nStrLen = sizeof(char) * (m_pColumnData[nCount].ColumnSize + 1) * 2;
				ASSERT(nStrLen > 0);
				tmp = (char *) malloc(nStrLen);
				if (tmp) {

					nSqlReturn = SQLGetData(
						m_hStatement,						// SQLHSTMT		StatementHandle
						m_pColumnData[nCount].ColumnNumber,	// SQLUSMALLINT	ColumnNumber
						SQL_CHAR,							// SQLSMALLINT	TargetType
						tmp,								// SQLPOINTER	TargetValuePtr
						nStrLen,							// SQLINTEGER	BufferLength
						&nLen);								// SQLINTEGER   *StrLen_or_IndPtr

					ASSERT(nSqlReturn == SQL_SUCCESS || nSqlReturn == SQL_SUCCESS_WITH_INFO);

					if (nSqlReturn == SQL_SUCCESS || nSqlReturn == SQL_SUCCESS_WITH_INFO) {
						bReturn = true;
						tmp = (char *) realloc(tmp, nLen + 1);
						value[0] = tmp;
					}
					else {
#ifdef _DEBUG
						// get some more information about why this failed.
						SQLGetDiagRec(SQL_HANDLE_STMT, m_hStatement, 1, Sqlstate, &NativeError, MessageText, 1024, &TextLength);
						// Code of 07009 indicates that the data was retreived out of column sequence.
#endif
						free(tmp);
					}
				}
			}

			nCount = m_nColumns;		// break out of the loop.
		}
	}


	m_xLock.Unlock();
	return(bReturn);
}



//-----------------------------------------------------------------------------
// CJW: Get data from the current row and create some memory for this item, 
//		storing the pointer in value.  if this operation fails, do not modify 
//		value.
bool CSqlResult::GetData(const char *field, int *value)
{
	bool bReturn = FALSE;
	int nCount;
	SQLRETURN nSqlReturn;
	SQLINTEGER nLen;
	int tmp;

	ASSERT(field != NULL);
	ASSERT(value != NULL);

	m_xLock.Lock();

	ASSERT(m_hStatement != 0);
	ASSERT(m_nColumns > 0);
	ASSERT(m_pColumnData != NULL);
	ASSERT(m_nCurrentRow > 0);

	ASSERT(m_lastReturn == SQL_SUCCESS || m_lastReturn == SQL_SUCCESS_WITH_INFO);


	// CJW: we have the name of a column... we need to go thru the columns list 
	//		to find the one we want.
	
	for (nCount = 0; nCount < m_nColumns; nCount++) {
		if (stricmp(field, (const char *) m_pColumnData[nCount].ColumnName) == 0) {
			// we found the match...

			nSqlReturn = SQLGetData(
				m_hStatement,						// SQLHSTMT		StatementHandle
				m_pColumnData[nCount].ColumnNumber,	// SQLUSMALLINT	ColumnNumber
				SQL_INTEGER,						// SQLSMALLINT	TargetType
				&tmp,								// SQLPOINTER	TargetValuePtr
				sizeof(tmp),						// SQLINTEGER	BufferLength
				&nLen);								// SQLINTEGER   *StrLen_or_IndPtr

			// CJW: There should always be data.. if we get this, then we've done something wrong.
			ASSERT(nSqlReturn != SQL_NO_DATA);

			ASSERT(nSqlReturn == SQL_SUCCESS || nSqlReturn == SQL_SUCCESS_WITH_INFO);
			if (nSqlReturn == SQL_SUCCESS || nSqlReturn == SQL_SUCCESS_WITH_INFO) {
				bReturn = true;
				*value = tmp;
			}
			nCount = m_nColumns;		// break out of the loop.
		}
	}

	m_xLock.Unlock();
	return(bReturn);
}


//-----------------------------------------------------------------------------
// CJW: Return the status of the last operation.
SQLRETURN CSqlResult::Status()
{
	SQLRETURN tt;

	m_xLock.Lock();
	tt = m_lastReturn;
	m_xLock.Unlock();

	return(tt);
}


//-----------------------------------------------------------------------------
// CJW: Return true if we are currently connected to a database.
bool CSqlDB::Connected()
{
	bool bb;

	m_xLock.Lock();
	bb = m_bConnected;
	m_xLock.Unlock();

	return(bb);
}



//-----------------------------------------------------------------------------
// CJW: Return the connection handle.  Should only be used when a new result 
//		statement objec tis being created.
SQLHANDLE CSqlDB::GetHandle()
{
	SQLHANDLE hh;

	m_xLock.Lock();
	hh = m_hConnection;
	m_xLock.Unlock();

	return(hh);
}


//-----------------------------------------------------------------------------
// CJW: When a result is being deleted, its reference also needs to be removed 
//		from the results list that is kept in this database object (so it can 
//		clean up at the end if necessary).
void CSqlDB::DeleteResult(CSqlResult *ptr)
{
	unsigned int nCount;
	unsigned int nIndex;

	m_xLock.Lock();

	ASSERT(m_nItems > 0);
	ASSERT(m_pResultList != NULL);

	nIndex = 0;
	for(nCount = 0; nCount < m_nItems; nCount++)
	{
		if(m_pResultList[nCount] != ptr) {
			if (nIndex < nCount) { m_pResultList[nIndex] = m_pResultList[nCount]; }
			nIndex++;
		}
	}
	m_pResultList = (CSqlResult **) realloc(m_pResultList, sizeof(CSqlResult *) * nIndex);
	m_nItems = nIndex;

	m_xLock.Unlock();
}


//-----------------------------------------------------------------------------
// CJW: Return the statement handle.
SQLHANDLE CSqlResult::GetHandle()
{
	SQLHANDLE hh;

	m_xLock.Lock();
	hh = m_hStatement;
	m_xLock.Unlock();

	return(hh);
}


//-----------------------------------------------------------------------------
// CJW: Commit the changes to the database.
bool CSqlDB::Commit()
{
	bool done = false;
	SQLRETURN nReturn;

	m_xLock.Lock();
	nReturn = SQLEndTran(SQL_HANDLE_DBC, m_hConnection, SQL_COMMIT);
	if (nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO) {
		done = true;
	}
	else {
		ASSERT(0);
	}
	m_xLock.Unlock();

	return (done);
}


char * CSqlResult::LastState()
{
	char *ptr;

	m_xLock.Lock();
	ptr = m_lastState;
	m_xLock.Unlock();

	return(ptr);
}

char * CSqlResult::LastText()
{
	char *ptr;

	m_xLock.Lock();
	ptr = m_lastErrorText;
	m_xLock.Unlock();

	return(ptr);
}
