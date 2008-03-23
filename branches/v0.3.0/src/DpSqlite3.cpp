//-----------------------------------------------------------------------------
// DevPlus.   
//-----------------------------------------------------------------------------


/***************************************************************************
 *   Copyright (C) 2006-2007 by Hyper-Active Systems,,,                    *
 *   Copyright (C) 2003-2005 by Clinton Webb,,,                            *
 *   devplus@hyper-active.com.au                                           *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/




//-----------------------------------------------------------------------------
// CJW: If we are compiling under visual studio, then we need to include this
//      file or we will have linker problems regarding some already defined
//      functions.  This is rather annoying and really quite stupid, but that
//      is what you get I suppose.
#ifdef _MSC_VER
    #include "stdafx.h"
#endif
//-----------------------------------------------------------------------------

#include <unistd.h>
#include <string.h>

#include <DpSqlite3.h>



//------------------------------------------------------------------------------
// CJW: Constructor.  Initialise the variables we are going to use.
DpSqlite3::DpSqlite3()
{
	_pDb = NULL;
	_nLastResultCode = SQLITE_OK;
}

//------------------------------------------------------------------------------
// CJW: Deconstructor. if the database is still allocated, then we will close it.
DpSqlite3::~DpSqlite3()
{
	if (_pDb != NULL) {
		// close the database.
		sqlite3_close(_pDb);
		_pDb = NULL;
	}
}

//------------------------------------------------------------------------------
// CJW: Open the database.  If it doesnt exist, then we will create it.  If we 
// 		cannot create it, then we will return false, otherwise we will return 
// 		true.
bool DpSqlite3::Open(char *szFilename)
{
	int rc;
	
	ASSERT(szFilename != NULL);
	ASSERT(_pDb == NULL);
	
	rc = sqlite3_open(szFilename, &_pDb);
	if (rc != SQLITE_OK) {
		sqlite3_close(_pDb);
		_pDb = NULL;
		_nLastResultCode = rc;
		return(false);
	}
	else {
		_nLastResultCode = rc;
		ASSERT(_nLastResultCode == SQLITE_OK);
		return(true);
	}
}

#include <stdio.h>
#include <stdlib.h>

//------------------------------------------------------------------------------
// CJW: Execute the command, and we dont care what the result is, so we're not 
// 		going to bother returning a result.   We will use the sqlite3 functions 
// 		to convert the format string and the arguments into a fully formed 
// 		string.
void DpSqlite3::ExecuteNR(char *query, ...)
{
	va_list args;
	char *ptr;
	int rc;
	char **result=NULL, *errmsg;
	int rows, cols;
	bool bLoop;
	int nDelay = 1;
	
	ASSERT(query != NULL);
	ASSERT(_pDb != NULL);

	va_start(args, query);
	ptr = sqlite3_vmprintf(query, args);
	va_end(args);
	
	// Make the query.  Since we dont care about the return result, we are simply going to clear out the data that we receive.
	bLoop = true;
	while (bLoop == true) {
		rc = sqlite3_get_table(_pDb, ptr, &result, &rows, &cols, &errmsg);
		if (rc == SQLITE_OK) {
			ASSERT(result != NULL);
			bLoop = false;
		}
		else if (rc == SQLITE_BUSY || rc == SQLITE_LOCKED) {
			ASSERT(result == NULL);
			sleep(1);
			nDelay ++;
		}
		else {
			fprintf(stderr, "DevPlus: unexpected database failure: %d\n", rc);
			bLoop = false;
		}
		_nLastResultCode = rc;
	}
	sqlite3_free_table(result);
	
	sqlite3_free(ptr);
}



//------------------------------------------------------------------------------
// CJW: Execute the command and return a result object.  We will completely 
// 		process the command now, and get all the data from the query.  The data 
// 		will be stored directly and pulled out as needed.
DpSqlite3Result * DpSqlite3::Execute(char *query, ...)
{
	va_list args;
	char *ptr;
	DpSqlite3Result *pResult;
	
	ASSERT(query != NULL);
	ASSERT(_pDb != NULL);

	va_start(args, query);
	ptr = sqlite3_vmprintf(query, args);
	va_end(args);
	
	pResult = ExecuteStr(ptr);
	
	sqlite3_free(ptr);
	
	return(pResult);
}

//------------------------------------------------------------------------------
// CJW: Execute an actual string.  String is treated as literal.  If there are 
// 		'%' characters in the string as a real char, then you cannot use 
// 		Execute(), and must use this function instead.
//
// NOTE: If the database is busy or locked, then we will wait 1 second, and then 
// 		 try it again.  There is no time-limit on the wait, it will wait 
// 		 forever.
DpSqlite3Result * DpSqlite3::ExecuteStr(char *query)
{
	bool bLoop;
	int rc;
	char **result=NULL, *errmsg;
	int rows, cols;
	int nDelay = 1;
	DpSqlite3Result *pResult = NULL;
	int insert;
	
	ASSERT(query != NULL);
	ASSERT(_pDb != NULL);
	
	bLoop = true;
	while (bLoop == true) {
		rc = sqlite3_get_table(_pDb, query, &result, &rows, &cols, &errmsg);
		if (rc == SQLITE_OK) {
			ASSERT(result != NULL);
			pResult = new DpSqlite3Result;
			insert = sqlite3_last_insert_rowid(_pDb);
			pResult->SetResult(rows, cols, result, errmsg, insert);
// 			sqlite3_free_table(result);
			bLoop = false;
		}
		else if (rc == SQLITE_BUSY || rc == SQLITE_LOCKED) {
			ASSERT(result == NULL);
			sleep(1);
			nDelay ++;
		}
		else {
			fprintf(stderr, "DevPlus: unexpected database failure: %d\n", rc);
			bLoop = false;
		}
		_nLastResultCode = rc;
	}

	return(pResult);
}

/*
char DpSqlite3::GetValue(char *szTable, char *szColumn, char *szValue, char *szResult)
{
	DpSqlite3Result *pResult;
	char *str = NULL;
	
	ASSERT(_pDb != NULL);
	
	pResult = Execute("SELECT %q FROM %q WHERE %q='%q' LIMIT 1", szResult, szTable, szColumn, szValue);
	ASSERT(pResult != NULL);
	if(pResult->NextRow()) {
		str = pResult->GetStr(szResult);
		strncpy(_szSessionID, str, sizeof(_szSessionID));
	}
	else {
		_szSessionID[0] = '\0';
	}
	
	pDB->ExecuteNR("COMMIT");

}
*/


//------------------------------------------------------------------------------
// CJW: Send a BEGIN command.
void DpSqlite3::Begin(bool bNow)
{	
	char *cmd;
	
	if (bNow == false)	{ cmd = "BEGIN"; }
	else				{ cmd = "BEGIN IMMEDIATE"; }
	ExecuteNR(cmd);
}

//------------------------------------------------------------------------------
// CJW: Send a COMMIT command.
void DpSqlite3::Commit()
{
	ExecuteNR("COMMIT");
}




//------------------------------------------------------------------------------
// CJW: Constructor.  Initialise the variables we are going to use.
DpSqlite3Result::DpSqlite3Result()
{
	_nRow = 0;
	_nColumns = 0;
	_nRows = 0;
	_szResult = NULL;
	_szErr = NULL;
	_nInsertID = 0;
}


DpSqlite3Result::~DpSqlite3Result()
{
	ASSERT(_szResult != NULL);
	sqlite3_free_table(_szResult);
	_szResult = NULL;
	
	if (_szErr != NULL) {
		sqlite3_free(_szErr);
		_szErr = NULL;
	}
}

// #include <stdio.h>

void DpSqlite3Result::SetResult(int rows, int cols, char **results, char *szErr, int insert)
{
	ASSERT(results != NULL);
	ASSERT(_nRow == 0);
	ASSERT(_nColumns == 0 && _nRows == 0 && _szResult == NULL && _szErr == NULL);
	ASSERT(_nInsertID == 0);
	
	_nRows = rows;
	_nColumns = cols;
	_szResult = results;
	_szErr = szErr;
	_nInsertID = insert;
}


//-----------------------------------------------------------------------------
// CJW: move our row counter forward one row.  If there are no more rows, then 
// 		return false.  Otherwise return true.
bool DpSqlite3Result::NextRow()
{
	bool bMore = false;
	
	ASSERT(_nRow <= _nRows);
	_nRow ++;
	if (_nRow <= _nRows) { bMore = true; }
	
	return(bMore);
}


//-----------------------------------------------------------------------------
// CJW: Get the integer translation of a field.  Since all the data actually 
// 		comes in as strings, we convert it to an int.  Need to be careful of a 
// 		NULL string, because we will get those from time to time too. If we get 
// 		a NULL, then we will just return a 0, which should be ok. 
int DpSqlite3Result::GetInt(char *name)
{
	int value = 0;
	char *str;
	
	ASSERT(name != NULL);
	ASSERT(_nRow > 0 && _nRow <= _nRows);
	ASSERT(_szResult != NULL);
	
	str = GetStr(name);
	if (str != NULL) {
		value = atoi(str);
	}
	else { value = 0; }
	
	return(value);
}
		
		
//-----------------------------------------------------------------------------
// CJW: Get the string result from 
char *DpSqlite3Result::GetStr(char *name)
{
	char *value = NULL;
	int col=-1;
	int i;
	
	ASSERT(name != NULL);
	ASSERT(_nRow > 0 && _nRow <= _nRows);
	ASSERT(_nColumns > 0);
	ASSERT(_szResult != NULL);
	
	for(i=0; i<_nColumns; i++) {
		if (strcmp(name, _szResult[i]) == 0) {
			col = i;
			i = _nColumns;
		} 
	}
	
	if (col >= 0) {
		value = _szResult[_nColumns+(_nColumns * (_nRow-1))+col];
// 		ASSERT(value != NULL);
	}
		
	return(value);
}

//-----------------------------------------------------------------------------
// CJW: When records are inserted, the primary key is calculated (normally), 
// 		and would have been stored, so this function will return it if we have 
// 		it.  A primary key should not be 0, so if we return a 0, then it means 
// 		that no primary key value was returned.
int DpSqlite3Result::GetInsertID(void)
{
	return(_nInsertID);
}



