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

#include <stdio.h>
#include <stdarg.h>
#include <string.h>


#include <DpMySql.h>






//------------------------------------------------------------------------------
// CJW: Constructor.
DpSqlDB::DpSqlDB()
{
}

//------------------------------------------------------------------------------
// CJW: Deconstructor.
DpSqlDB::~DpSqlDB()
{
}


//------------------------------------------------------------------------------
// CJW: Before performing any client operations with the database, we need to
//      lock it so that only one instance can be using it at a time.  This is
//      probably very important, because we are using one database handle, and
//      we must get the results from a query before we try and make another
//      query.  If you need to optimise database accesses, then make more than
//      one connection object.  Otherwise, all database operations will be
//      serialised.
void DpSqlDB::ClientLock(void)
{
	Lock();
}

void DpSqlDB::ClientUnlock(void)
{
	Unlock();
}



//------------------------------------------------------------------------------
// CJW: Constructor.
DpMySqlDB::DpMySqlDB()
{
    Lock();
    _hSql = NULL;
    _pParent = NULL;
    _nChildren = 0;
    _nInsertID = 0;
    _nFields = 0;
    _pResult = NULL;
    _pFieldList = NULL;
    _nLastErr = 0;
    _Row = 0;
    _bTableLock = false;
    Unlock();
}

//------------------------------------------------------------------------------
// CJW: Deconstructor.
DpMySqlDB::~DpMySqlDB()
{
    Lock();

    ASSERT(_bTableLock == false);
    
    if (_pParent != NULL) {
        ASSERT(_hSql == NULL);
        _pParent->Unspawn();
        _pParent = NULL;
    }
    
    if (_hSql != NULL) {
        ASSERT(_pParent == NULL);
        ASSERT(_nChildren == 0);
        mysql_close(_hSql);
        _hSql = NULL;
    }

    if (_pResult != NULL) {
        mysql_free_result(_pResult);
        _pResult = NULL;
    }

    // CJW - Assuming that by clearing the result set, we also clear this item. The document doesnt say anything about having to clear this up too.
    _pFieldList = NULL;
    
    Unlock();
}

// we want to add some asserts to this function to verify that it is being called by a spawn and not from the parent.  Otherwise, we dont want to change any functionality.  This function should only every be called internally, so we can assume that the local object is locked.
void DpMySqlDB::ClientLock(void)
{
    DpSqlDB::ClientLock();
    ASSERT(_pParent == NULL);
    ASSERT(_hSql != NULL);
}

// same comment as ClientLock above.
void DpMySqlDB::ClientUnlock(void)
{
    ASSERT(_pParent == NULL);
    ASSERT(_hSql != NULL);
    DpSqlDB::ClientUnlock();
}


DpMySqlDB* DpMySqlDB::Spawn()
{
    DpMySqlDB *pNew;

    Lock();
    ASSERT(_hSql != NULL);
    ASSERT(_pParent == NULL);
    ASSERT(_nChildren >= 0);
    _nChildren++;
    ASSERT(_nChildren > 0);
    
    // CJW: Spawned objects should be created BEFORE any tables are locked.
    ASSERT(_bTableLock == false);
    Unlock();
    
    pNew = new DpMySqlDB;
    ASSERT(pNew != NULL);
    pNew->AttachParent(this);
    return(pNew);
}

void DpMySqlDB::Unspawn()
{
    Lock();
    ASSERT(_hSql != NULL);
    ASSERT(_pParent == NULL);
    ASSERT(_nChildren > 0);
    _nChildren--;
    ASSERT(_nChildren >= 0);
    Unlock();
}

void DpMySqlDB::AttachParent(DpMySqlDB *pParent)
{
    ASSERT(pParent != NULL);
    Lock();
    ASSERT(_pParent == NULL);
    ASSERT(_hSql == NULL);
    _pParent = pParent;
    Unlock();
}


//-----------------------------------------------------------------------------
// CJW: Connect to the database, and stay connected until the disconnect
//      function is called.  If we are already connected, then we should throw
//      a debug assertion actually.  We are passed the server, user and
//      password.
bool DpMySqlDB::Connect(char *server, char *user, char *password, char *db)
{
    bool bStatus = false;

    ASSERT(server != NULL);
    
    Lock();
    ASSERT(_bTableLock == false);
    ASSERT(_hSql == NULL);
    ASSERT(_pParent == NULL);
    _hSql = mysql_init(NULL);
    if (_hSql != NULL) {
        _hSql = mysql_real_connect(_hSql, server, user, password, db, 0, NULL, CLIENT_COMPRESS);
        if (_hSql != NULL) {
            bStatus = true;
        }
    }
    Unlock();

    return(bStatus);
}


//-----------------------------------------------------------------------------
// CJW: Select the database that we want to use.  This should only be done on
//      the main control object for the database connection.  It should not be
//      performed on any spawned objects.  Also, keep in mind that changing
//      the database will be effective for all spawned database objects, so
//      care should be taken that no active database objects are around when
//      changing to a different database.
bool DpMySqlDB::Use(char *db)
{
    bool bStatus = false;
    
    ASSERT(db != NULL);
    Lock();
    ASSERT(_bTableLock == false);
    ASSERT(_pParent == NULL);
    ASSERT(_hSql != NULL);
    ASSERT(_nChildren == 0);
    if (mysql_select_db(_hSql, db) == 0) {
        bStatus = true;
    }
    Unlock();
    return(bStatus);
}

//-----------------------------------------------------------------------------
// CJW: Make sure that the string is in a format that can be used in a MySQL
//      statement.  Will return a memory pointer that must be freed by the
//      calling function.  This function does NOT add single-quotes around
//      text.  The Perl quote DOES add single-quotes, so keep the difference
//      in mind.
// CJW: This operation should be done on the main connection object.  All the
//      quotation and other server/client work should be done from the main
//      object.   Really only the Execute, and the GetData functions should be
//      called on the spawned object.
char * DpMySqlDB::Quote(char *str, unsigned int length)
{
    char *q;
    unsigned int n;
    unsigned int len;

    ASSERT(str != NULL);

    if (length == 0) { len = strlen(str); }
    else             { len = length; }
    ASSERT(len >= 0);
    q = (char *) malloc((len * 2) + 1);
    ASSERT(q);

    // The escape function should be ok to run in more than one thread at a time.  The mysql library is supposed to be thread safe in general, so it should be ok.
    Lock();
    ASSERT(_hSql != NULL);
    n = mysql_real_escape_string(_hSql, q, str, len);
    Unlock();
    
//     printf("Mysql::Quote(\"%s\") - length:%d, n:%d, len:%d\n", str, length, n, len);
    
    ASSERT(n >= len);
    ASSERT(n < ((len * 2) + 1));
    ASSERT(n == strlen(q));
//     q[n] = '\0';
    q = (char *) realloc(q, n+1);
    ASSERT(q != NULL);

    return(q);
}

// For this function, we assume that a lock has already been made.  Especially a client lock.
MYSQL * DpMySqlDB::GetHandle(void)
{
//  ASSERT(_bLocked == true);
    ASSERT(_hSql != NULL);
    ASSERT(_pParent == NULL);
    return(_hSql);
}

//-----------------------------------------------------------------------------
// CJW: Execute a query to the database server.  We will put the results in a
//      DpMySqlResult object so that the other database operations can
//      continue.
bool DpMySqlDB::Execute(char *query, ...)
{
    va_list ap;
    char *buffer;
    bool bOK = false;

    ASSERT(query != NULL);

    buffer = (char *) malloc(32767);
    if (buffer) {
        va_start(ap, query);
        vsprintf(buffer, query, ap);
        va_end(ap);
        bOK = ExecuteStr(buffer);
        free(buffer);
    }
    
    return(bOK);
}

//-----------------------------------------------------------------------------
// CJW: Execute the SQL statement.  This function should not be called by the
//      object that made the connection.  Instead, another object should be
//      spawned to handle the query.  A new object should be spawned for each
//      query.
bool DpMySqlDB::ExecuteStr(char *query)
{
    int err;
    bool bOK = false;
    MYSQL *hSql;

    ASSERT(query != NULL);
    
    Lock();
    ASSERT(_hSql == NULL);
    ASSERT(_pParent != NULL);
    ASSERT(_pResult == NULL);

    _pParent->ClientLock();
    hSql = _pParent->GetHandle();
    ASSERT(hSql != NULL);
    err = mysql_real_query(hSql, query, strlen(query)+1);
    if(err == 0) {

        ASSERT(_nInsertID == 0);
        ASSERT(_nFields == 0);
        _nInsertID = mysql_insert_id(hSql);
//         printf("Mysql: InsertID: %d\n", _nInsertID);
        _pResult = mysql_store_result(hSql);
        if (_pResult != NULL) {
            _nFields = mysql_num_fields(_pResult);
            _pFieldList = mysql_fetch_fields(_pResult);
            bOK = true;
        }
    }
    else {
        _nLastErr = mysql_errno(hSql);
    }
    
    _pParent->ClientUnlock();
    Unlock();

    return(bOK);
}


//-----------------------------------------------------------------------------
// CJW: Assuming that an insert statement was used, this will return the auto 
//      generated InsertID that was used in the table.
my_ulonglong DpMySqlDB::GetInsertID(void) 
{
    my_ulonglong id;

    Lock();
    ASSERT(_hSql == NULL);
    ASSERT(_pParent != NULL);
    id = _nInsertID;
    Unlock();
    return(id);
}

//-----------------------------------------------------------------------------
// CJW: Discard the current row if we have one, and load up the next row.
//      Return true if we have a row, return false if we dont have one.
bool DpMySqlDB::NextRow()
{
    bool bStatus = false;

    Lock();
    if (_pResult != NULL) {
        _Row = mysql_fetch_row(_pResult);
        if (_Row != NULL) {
            bStatus = true;
        }
    }
    Unlock();

    return(bStatus);
}


//-----------------------------------------------------------------------------
// CJW: Get the contents of the labeled field
bool DpMySqlDB::GetData(char *field, unsigned int *value)
{
    bool bStatus = false;
    unsigned int n=0;

    ASSERT(field != NULL);
    ASSERT(value != NULL);
    
    Lock();
    ASSERT(_hSql == NULL);
    ASSERT(_pParent != NULL);
    ASSERT(_pResult != NULL);
    
    if (_pFieldList != NULL) {
        for(n=0; n<_nFields; n++) {
            if (strcmp(_pFieldList[n].name, field) == 0) {
                if (_Row[n] == NULL) {
                    *value = 0;
                }
                else {
                    *value = atoi(_Row[n]);
                }
                n=_nFields; // break out of the loop.
                bStatus = true;
            }
        }
    }
    
    Unlock();

    return(bStatus);
}



//-----------------------------------------------------------------------------
// CJW: Get the contents of the labeled field
bool DpMySqlDB::GetData(char *field, int *value)
{
    bool bStatus = false;
    unsigned int n=0;

    ASSERT(field != NULL);
    ASSERT(value != NULL);
    
    Lock();
    ASSERT(_hSql == NULL);
    ASSERT(_pParent != NULL);
    ASSERT(_pResult != NULL);
    
    if (_pFieldList != NULL) {
        for(n=0; n<_nFields; n++) {
            if (strcmp(_pFieldList[n].name, field) == 0) {
                if (_Row[n] == NULL) {
                    *value = 0;
                }
                else {
                    *value = atoi(_Row[n]);
                }
                n=_nFields; // break out of the loop.
                bStatus = true;
            }
        }
    }
    
    Unlock();

    return(bStatus);
}



//-----------------------------------------------------------------------------
// CJW: Get the contents of the labeled field
bool DpMySqlDB::GetData(char *field, char *value, int buflen)
{
    bool bStatus = false;
    unsigned int n=0;

    ASSERT(value != NULL);
    ASSERT(field != NULL);
    ASSERT(buflen > 0);
    ASSERT(field[0] != '\0');
    ASSERT(field != value);

    Lock();
    ASSERT(_hSql == NULL);
    ASSERT(_pParent != NULL);
    ASSERT(_pResult != NULL);
    
    if (_pFieldList != NULL) {
        for(n=0; n<_nFields; n++) {
            if (strcmp(_pFieldList[n].name, field) == 0) {
                if (_Row[n] != NULL) {
                    strncpy(value, _Row[n], buflen);
                    value[buflen-1] = '\0';
                    n=_nFields; // break out of the loop.
                }
                bStatus = true;
            }
        }
    }
    Unlock();

    return(bStatus);
}

/*
    It is often necessary to lock tables when performing a series of operations.  This function will perform the lock tables function.  However, an UnlockTables function will need to be called to unlock the tables.  You cannot call this function twice without calling an Unlock.  This function should only be called from the main connection object, and will call the client lock to make sure that other threads dont interfere.  Yes, that is correct, if you perform this lock in one thread, other threads will not be able to perform any database calls until this lock is released.  If you need threads to not block each other, then you will need to use seperate connection objects.
*/
void DpMySqlDB::LockTables(char *szTables)
{
    DpMySqlDB *pResult;

    ASSERT(szTables != NULL);

    pResult = Spawn();
    ASSERT(pResult);
    if (pResult != NULL) {

        Lock();
        ASSERT(_bTableLock == false);
    
        ASSERT(_hSql != NULL);
        ASSERT(_pParent == NULL);
        ASSERT(_pResult == NULL);

        pResult->Execute("LOCK TABLES %s", szTables);
        delete pResult;
    
        _bTableLock = true;
    }

    // We performed a writelock, but we dont want to unlock it yet.
}

void DpMySqlDB::UnlockTables(void)
{
    DpMySqlDB *pResult;
    
    ASSERT(_bTableLock == true);

    ASSERT(_hSql != NULL);
    ASSERT(_pParent == NULL);
    ASSERT(_pResult == NULL);

    // We still have it thread-locked, but since we are about to clear the table lock, we need to clear this value first so that we can actually spawn this instance so that we can make the query.
    _bTableLock = false;
    
    pResult = Spawn();
    ASSERT(pResult);
    if (pResult != NULL) {
        pResult->Execute("UNLOCK TABLES");
        delete pResult;
    }
    Unlock();
}


void DpSqlDB::Lock(void)
{
	_xLock.Lock();
}

void DpSqlDB::Unlock(void)
{
	_xLock.Unlock();
}


