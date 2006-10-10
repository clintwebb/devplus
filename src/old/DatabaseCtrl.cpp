// DatabaseCtrl.cpp: implementation of the CDatabaseCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <afxmt.h>
#include "DatabaseCtrl.h"



#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


//-----------------------------------------------------------------------------
// Static data.
MYSQL *CDatabaseCtrl::hSql = NULL;
CCriticalSection CDatabaseCtrl::xLock;
CCriticalSection CDatabaseCtrl::xServerLock;




//-----------------------------------------------------------------------------
CDatabaseCtrl::CDatabaseCtrl()
{
	result = NULL;
	bTablesLocked = false;
	nInsertID = 0;
}

//-----------------------------------------------------------------------------
// Deconstructor.  Free any resources that this INSTANCE created.  Keep in mind that we have some static object.s
CDatabaseCtrl::~CDatabaseCtrl()
{
	if (result != NULL) 
	{
		xServerLock.Lock();
		mysql_free_result(result);
		xServerLock.Unlock();
		result = NULL;
	}

	if (bTablesLocked != false)
	{
		UnlockTables();
	}
}


//-----------------------------------------------------------------------------
// Connect to the database, and stay connected until the disconnect function 
// is called.   If we are already connected, then we should throw a debug 
// assertion actually.  We are passed the server, user and password, we should 
// keep these within our structure if it becomes necessary to re-connect 
// programmatically.
bool CDatabaseCtrl::Connect(char *server, char *user, char *password, char *db)
{
	bool bStatus = false;

	xLock.Lock();
	xServerLock.Lock();
	hSql = mysql_init(NULL);
	if (hSql != NULL) 
	{
		hSql = mysql_connect(hSql, server, user, password);
		if (hSql != NULL) 
		{
			if (db != NULL)
			{
				bStatus = Use(db);
			}
			else
			{
				bStatus = true;
			}
		}
	}
	xServerLock.Unlock();
	xLock.Unlock();

	return(bStatus);
}

//-----------------------------------------------------------------------------
// Disconnect from the database if we are currently connected.   Follow the 
// usual locking procedures so that other threads can finish first.
bool CDatabaseCtrl::Disconnect()
{
	bool bStatus = false;

	xLock.Lock();
	if (hSql != NULL)
	{
		xServerLock.Lock();
		mysql_close(hSql);
		xServerLock.Unlock();
		hSql = NULL;
		bStatus = true;
	}
	xLock.Unlock();

	return(bStatus);
}




//-----------------------------------------------------------------------------
// Execute a query to the database server.  Locally we will store the recordset 
// data so that the other database operations can continue.
//
// If this current object has already been used to execute a command, then we 
// will need to close that previous execute and initiate this new one.
bool CDatabaseCtrl::Execute(char *query, ...)
{
	bool bStatus = false;
	int err;
	va_list ap;
	char *buffer;
#ifdef _DEBUG
		char *tmp;
#endif

	ASSERT(query != NULL);

	buffer = (char *) malloc(32767);
	if (buffer)
	{
		va_start(ap, query);
		vsprintf(buffer, query, ap);
		va_end(ap);

#ifdef _DEBUG
		tmp = (char *) malloc(260);
		strncpy(tmp, buffer, 255);
		tmp[255] = 0;
		strcat(tmp, "\n");
		TRACE(tmp);
		free(tmp);
#endif

		xLock.Lock();
		ASSERT(hSql != NULL);
		if (hSql != NULL)
		{
			// If this object was already used for a query.  clean it up first.
			if (result != NULL)
			{
				xServerLock.Lock();
				mysql_free_result(result);
				xServerLock.Unlock();
				result = NULL;
			}

			xServerLock.Lock();
			err = mysql_real_query(hSql, buffer, strlen(buffer)+1);
			if(err == 0)
			{
				nInsertID = mysql_insert_id(hSql);
				result = mysql_store_result(hSql);
				if (result != NULL)
				{
					nFields = mysql_num_fields(result);
					fieldList = mysql_fetch_fields(result);
				}
				bStatus = true;
			}
			else 
			{
				printf("err %d", err);
			}
			xServerLock.Unlock();
		}
		xLock.Unlock();

		free(buffer);
	}
	
	return(bStatus);
}

//-----------------------------------------------------------------------------
// Discard the current row if we have one, and load up the next row.  Return 
// true if we have a row, return false if we dont have one.
bool CDatabaseCtrl::NextRow()
{
	bool bStatus = false;

	xLock.Lock();
	if (result != NULL) 
	{
		xServerLock.Lock();
		row = mysql_fetch_row(result);
		if (row != NULL)
		{
			bStatus = true;
		}
		xServerLock.Unlock();
	}
	xLock.Unlock();

	return(bStatus);
}


//-----------------------------------------------------------------------------
// Get the contents of the labeled field 
bool CDatabaseCtrl::GetData(char *field, unsigned int *value)
{
	bool bStatus = false;
	unsigned int n=0;

	xLock.Lock();
	if (fieldList != NULL)
	{
		for(n=0; n<nFields; n++)
		{
			if (strcmp(fieldList[n].name, field) == 0)
			{
				if (row[n] == NULL) {
					*value = 0;
				}
				else {
					*value = atoi(row[n]);
				}
				n=nFields; // break out of the loop.
				bStatus = true;
			}
		}
	}
	xLock.Unlock();

	return(bStatus);
}


//-----------------------------------------------------------------------------
// Get the contents of the labeled field 
bool CDatabaseCtrl::GetData(char *field, short *value)
{
	bool bStatus = false;
	unsigned int n=0;

	xLock.Lock();
	if (fieldList != NULL)
	{
		for(n=0; n<nFields; n++)
		{
			if (strcmp(fieldList[n].name, field) == 0)
			{
				if (row[n] == NULL) {
					*value = 0;
				}
				else {

#ifdef _DEBUG
					// should also check the data type to see if it fits.
					enum_field_types tt;
					tt = fieldList[n].type;

					// types that are too big
					ASSERT(tt != FIELD_TYPE_LONG);
					ASSERT(tt != FIELD_TYPE_DOUBLE);
					ASSERT(tt != FIELD_TYPE_FLOAT);
					ASSERT(tt != FIELD_TYPE_LONGLONG);
					ASSERT(tt != FIELD_TYPE_INT24);

					// types that dont match
					ASSERT(tt != FIELD_TYPE_NULL);
					ASSERT(tt != FIELD_TYPE_TIMESTAMP);
					ASSERT(tt != FIELD_TYPE_DATE);
					ASSERT(tt != FIELD_TYPE_TIME);
					ASSERT(tt != FIELD_TYPE_DATETIME);
					ASSERT(tt != FIELD_TYPE_NEWDATE);

					// types that we might have to handle specially.
					ASSERT(tt != FIELD_TYPE_TINY_BLOB);
					ASSERT(tt != FIELD_TYPE_MEDIUM_BLOB);
					ASSERT(tt != FIELD_TYPE_LONG_BLOB);
					ASSERT(tt != FIELD_TYPE_BLOB);

					// strings can contain a short value, so we will do a little checking...
					if(tt == FIELD_TYPE_VAR_STRING || tt == FIELD_TYPE_STRING)  {
						ASSERT(strlen(row[n]) > 0);
						ASSERT(strlen(row[n]) < 6);
					}

					ASSERT((row[n][0] >= '0' && row[n][0] <= '9') || row[n][0] == '-' || row[n][0] == NULL || row[n][0] == ' ');

					int tmp;
					tmp = atoi(row[n]);
					
					// -32,768 to 32,767
					ASSERT(tmp >= -32768 && tmp <= 32767);
					*value = (short) tmp;
#else
					*value = atoi(row[n]);
#endif

				}
				n=nFields; // break out of the loop.
				bStatus = true;
			}
		}
	}
	xLock.Unlock();

	return(bStatus);
}



//-----------------------------------------------------------------------------
// Get the contents of the labeled field 
bool CDatabaseCtrl::GetData(char *field, unsigned long *value)
{
	bool bStatus = false;
	unsigned int n=0;

	xLock.Lock();
	if (fieldList != NULL)
	{
		for(n=0; n<nFields; n++)
		{
			if (strcmp(fieldList[n].name, field) == 0)
			{
				if (row[n] == NULL) {
					*value = 0;
				}
				else {
					*value = atol(row[n]);
				}
				n=nFields; // break out of the loop.
				bStatus = true;
			}
		}
	}
	xLock.Unlock();

	return(bStatus);
}



//-----------------------------------------------------------------------------
// Get the contents of the labeled field 
bool CDatabaseCtrl::GetData(char *field, int *value)
{
	bool bStatus = false;
	unsigned int n=0;

	xLock.Lock();
	if (fieldList != NULL)
	{
		for(n=0; n<nFields; n++)
		{
			if (strcmp(fieldList[n].name, field) == 0)
			{
				if (row[n] == NULL) {
					*value = 0;
				}
				else {
					*value = atoi(row[n]);
				}
				n=nFields; // break out of the loop.
				bStatus = true;
			}
		}
	}
	xLock.Unlock();

	return(bStatus);
}




//-----------------------------------------------------------------------------
// Get the contents of the labeled field 
bool CDatabaseCtrl::GetData(char *field, char **value)
{
	bool bStatus = false;
	unsigned int n=0;

	xLock.Lock();
	if (fieldList != NULL)
	{
		for(n=0; n<nFields; n++)
		{
			if (strcmp(fieldList[n].name, field) == 0)
			{
				if (row[n] != NULL) {
					*value = (char *) malloc(strlen(row[n])+1);
					strcpy(*value, row[n]);
					n=nFields; // break out of the loop.
					bStatus = true;
				}
				else {
					*value = NULL;
					bStatus = true;
				}
			}
		}
	}
	xLock.Unlock();

	return(bStatus);
}


//-----------------------------------------------------------------------------
// Get the contents of the labeled field 
bool CDatabaseCtrl::GetData(char *field, char *value, int buflen)
{
	bool bStatus = false;
	unsigned int n=0;

	ASSERT(value != NULL);
	ASSERT(field != NULL);
	ASSERT(buflen > 0);
	ASSERT(field[0] != NULL);
	ASSERT(field != value);

	xLock.Lock();
	if (fieldList != NULL)
	{
		for(n=0; n<nFields; n++)
		{
			if (strcmp(fieldList[n].name, field) == 0)
			{
				if (row[n] != NULL) {
					strncpy(value, row[n], buflen);
					n=nFields; // break out of the loop.
				}
				bStatus = true;
			}
		}
	}
	xLock.Unlock();

	return(bStatus);
}





//-----------------------------------------------------------------------------
// Lock the table so that no other process can access it.  We will just do a 
// blind read/write lock for now.   We might want to make a note that we have 
// locked a table, so that when this instance is deleted, it knows that it 
// should first unlock the tables.
bool CDatabaseCtrl::LockTable(char *table)
{
	bool bStatus = false;
	char *buffer;

	ASSERT(table != NULL);
	ASSERT(strlen(table) < (1024-18));

	xLock.Lock();
	ASSERT(bTablesLocked == false);
	buffer = (char *) malloc(1024);
	if (buffer)
	{
		sprintf(buffer, "LOCK TABLES %s WRITE", table);
		if (Execute(buffer) != false)
		{
			bStatus = true;
			bTablesLocked = true;
		}
		free(buffer);
	}
	xLock.Unlock();

	return(bStatus);
}


//-----------------------------------------------------------------------------
// unlock all the tables that have been locked by this instance.  We dont 
// actually keep track of which tables are locked, that is all done by the db 
// server.
bool CDatabaseCtrl::UnlockTables()
{
	bool bStatus = false;

	xLock.Lock();
	ASSERT(bTablesLocked == true);
	if (Execute("UNLOCK TABLES") != false)
	{
		bStatus = true;
		bTablesLocked = false;
	}
	xLock.Unlock();

	return(bStatus);
}


//-----------------------------------------------------------------------------
// CJW: Return the insert ID for a transaction that was made.  Will return 0 if 
//		the transaction was not an INSERT with an auto_increment.
my_ulonglong CDatabaseCtrl::GetInsertID()
{
	my_ulonglong nID =0;

	xLock.Lock();
	nID = nInsertID;
	xLock.Unlock();

	return(nID);
}


//-----------------------------------------------------------------------------
// CJW: Make sure that the string is in a format that can be used in a MySQL 
//		statement.  Will return a memory pointer that must be freed by the 
//		calling function.  This function does NOT add single-quotes around 
//		text.  The Perl quote DOES add single-quotes, so keep the difference 
//		in mind.
char * CDatabaseCtrl::Quote(char *str, unsigned int length)
{
	char *q;
	unsigned int n;

	ASSERT(str != NULL);
	ASSERT(length > 0);

	q = (char *) malloc((length * 2) + 1);
    n = mysql_real_escape_string(hSql, q, str, length);
	ASSERT(n >= length);
	q[n] = NULL;
	q = (char *) realloc(q, n+1);
	ASSERT(q != NULL);

	return(q);
}


//-----------------------------------------------------------------------------
// CJW: Select the database that we want to use.  pass in the name...
bool CDatabaseCtrl::Use(char *db)
{
	bool bStatus = false;
	
	ASSERT(db != NULL);

	if (mysql_select_db(hSql, db) == 0)
	{
		bStatus = true;
	}
	
	return(bStatus);
}
