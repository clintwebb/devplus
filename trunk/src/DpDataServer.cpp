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

#include <stdlib.h>

#include <DpDataServer.h>


//-----------------------------------------------------------------------------
// Constructor.  Initialise the data we will be using.
DpDataServer::DpDataServer() 
{	
	_Lock.Lock();
	_pDB = NULL;
	_pList = NULL;
	_nItems = 0;
	_szFilename = NULL;
	_nLocked = eOpen;
	_Lock.Unlock();
	
}
		
//-----------------------------------------------------------------------------
// Deconstructor.  If we are closing down this object, then all of the data 
// requests should have been completed by now.  We should go thru the items 
// left in the list to make sure there are none with actual data.
DpDataServer::~DpDataServer() 
{
 	WaitForThread();
	_Lock.Lock();
	
	ASSERT(_nLocked == eOpen);
	
	while(_nItems > 0) {
		_nItems --;	

		ASSERT(_pList != NULL);
		ASSERT(_pList[_nItems].szQuery == NULL);
		ASSERT(_pList[_nItems].pResult == NULL);
	}
	
	if (_pList != NULL) {
		ASSERT(_nItems == 0);
		free(_pList);
		_pList = NULL;
	}
	
	if (_pDB != NULL) {
		delete _pDB;
		_pDB = NULL;
	}
	
	_Lock.Unlock();
}


//-----------------------------------------------------------------------------
// Basically we need to open the database file and make sure we can access it.  
// We probably wont try to write to the database file yet.  If everything is 
// ok, we will start the processing thread, and return a true.  If it didnt 
// load, then there is no point in starting the processing thread.
//
//	NOTE: Since the database needs to be opened in the same thread that will be 
//		  accessing it, we must first start the thread, and then wait a few 
//		  seconds to make sure that the database file has been opened.
bool DpDataServer::Load(char *szFile)
{
	bool bLoaded = false;
	int nCounter;
	
	ASSERT(szFile != NULL);
	
	_Lock.Lock();
	ASSERT(_pDB == NULL);
	ASSERT(_szFilename == NULL);
	ASSERT(_nLocked == eOpen);
	
	_szFilename = szFile;
	
	_Lock.Unlock();
			
	SetCycleTime(5);
	Start();
	
	nCounter = 1000;
	while (nCounter > 0 && bLoaded == false) {
		_Lock.Lock();
		if (_pDB != NULL) {
			bLoaded = true;
		}
		_Lock.Unlock();
		nCounter--;
		if (bLoaded == false) { Sleep(100); }
	}

	return(bLoaded);
}


//-----------------------------------------------------------------------------
// CJW: When the thread first runs it will need to open the database file.  The 
// 		Load function will keep checking for a few seconds to make sure that 
// 		the database has been loaded.  If it hasnt been loaded in that time 
// 		then the application will quit.
//
// 		The thread will go thru the list of queries and will process all of 
// 		them.  
//
//	TODO: No new queries can be added while we are processing these ones.  If 
//		  we need to, we can improve performance by modifying this system so 
//		  that queries can be added while we are processing them.  That would 
//		  be a bit tricky to do, so we wont do it unless it becomes necessary.
void DpDataServer::OnThreadRun(void)
{
	int i;
	bool bStarted = false;
	
	_Lock.Lock();
	ASSERT(_szFilename != NULL);
	
	if (_pDB == NULL) {
		_pDB = new DpSqlite3;
		if (_pDB != NULL) {
			if (_pDB->Open(_szFilename) == false) {
				delete _pDB;
				_pDB = NULL;
			}
		}
	}
		
	if (_pDB != NULL) {
		if (_nItems > 0) {
			
			ASSERT(_nLocked != eCleared);
			
			// go thru the list and process each one.
			i = 0;
			while(i < _nItems) {
			
				if (_pList[i].bDone == false && _pList[i].szQuery != NULL) {
					ASSERT(_pList[i].pResult == NULL);
			
					if (bStarted == false && _nLocked == eOpen) {
						_pDB->ExecuteNR("BEGIN");
						bStarted = true;
					}
					
					_pList[i].pResult = _pDB->ExecuteStr(_pList[i].szQuery);
					ASSERT(_pList[i].pResult != NULL);
					_pList[i].bDone = true;
					if (_pList[i].bReturn == false) {
						delete _pList[i].pResult;
						_pList[i].pResult = NULL;
						sqlite3_free(_pList[i].szQuery);
						_pList[i].szQuery = NULL;
					}
				}
				
				i++;
			}
			
			if (bStarted == true) {
				_pDB->ExecuteNR("COMMIT");
			}
			
			if (_nLocked == ePending) {
				// We will use an Exclusive begin so that external sources also cant read the database either.  At some point, we may want to include some functionality to specify whether they want a read lock, or a write lock.   Its possible we want to make sure the data isnt modified between selects, but I would guess that is pretty rare.  Normally when we want to specify a begin and a commit we dont want other processes from accessing the incomplete data.
				_pDB->ExecuteNR("BEGIN EXCLUSIVE");
				_nLocked = eLocked;
			}
			else if (_nLocked == eCommit) {
				_pDB->ExecuteNR("COMMIT");
				_nLocked = eCleared;
			}
			
			if (_nItems > 1) {
				if (_pList[_nItems-1].bDone == true && _pList[_nItems-1].szQuery == NULL) {
					_nItems--;
				}
			}
		}
	}

	_Lock.Unlock();
}

//-----------------------------------------------------------------------------
// CJW: We will be adding an entry to the list.  When we process an entry, we 
// 		will be needing to make sure that we process the entries in the order 
// 		that they were submitted.   Its probably actually not very important 
// 		that this happens, because we will actually only have one query per 
// 		thread anyway.
DpSqlite3Result * DpDataServer::Execute(char *szQuery, ...)
{
	va_list args;
	char *ptr;
	DpSqlite3Result *pResult = NULL;
	int nItem;
	bool bFound =  false;
	
	ASSERT(szQuery != NULL);

	va_start(args, szQuery);
	ptr = sqlite3_vmprintf(szQuery, args);
	va_end(args);
	
	_EndLock.Lock();
	ASSERT(_nLocked == eOpen || _nLocked == eLocked);
	_Lock.Lock();
	
	// add the string to the queue, and then wait.
	if (_pList == NULL) {
		ASSERT(_nItems == 0);
		_pList = (DpRequestResult *) malloc(sizeof(DpRequestResult));
		_nItems++;
		
		nItem = 0;
	}
	else {
		
		// go thru the list to see if there are any blank entries we can use.
		nItem = 0;
		while (nItem < _nItems && bFound == false) {
			if (_pList[nItem].szQuery == NULL && _pList[nItem].bDone == true) {
				ASSERT(_pList[nItem].pResult == NULL);
				bFound = true;
			}
			else {
				nItem++;
			}
		}
		
		if (bFound == false) {
			_pList = (DpRequestResult *) realloc(_pList, sizeof(DpRequestResult) * (_nItems+1));
			nItem = _nItems;
			_nItems++;
		}
	}
	_pList[nItem].szQuery = ptr;
	_pList[nItem].pResult = NULL;
	_pList[nItem].bDone = false;
	_pList[nItem].bReturn = true;
	
	ASSERT(_nItems > 0);
	ASSERT(nItem >= 0);
	ASSERT(nItem < _nItems);
	ASSERT(_pList != NULL);
	ASSERT(_pList[nItem].szQuery != NULL);
	ASSERT(_pList[nItem].pResult == NULL);
	ASSERT(_pList[nItem].bDone == false);
	ASSERT(_pList[nItem].bReturn == true);
	
	_Lock.Unlock();
	
	// when the result is found, stop waiting.
	Sleep(0);
	_Lock.Lock();
	while(_pList[nItem].bDone == false) {
		_Lock.Unlock();
		Sleep(1);
		_Lock.Lock();
	}
	
	ASSERT(_pList[nItem].pResult != NULL);
	pResult = _pList[nItem].pResult;
	_pList[nItem].pResult = NULL;
	_pList[nItem].szQuery = NULL;
	_pList[nItem].bDone = false;
	_Lock.Unlock();
	_EndLock.Unlock();
	
	sqlite3_free(ptr);
	
	return(pResult);
}



//-----------------------------------------------------------------------------
// CJW: We need to execute a query, but we dont want the return value.  This 
// 		function will not wait for the query to complete, it will simply return 
// 		to process more stuff.  This should only be used to INSERT and UPDATE 
// 		functions where you dont need to check the results and you are pretty 
// 		sure the database is available.
//
//		Note: Since we are exiting after we pass the query on, we cannot clean 
//			  it up, so the thread will need to do that.
void DpDataServer::ExecuteNR(char *szQuery, ...)
{
	va_list args;
	char *ptr;
	int nItem;
	bool bFound;
	
	ASSERT(szQuery != NULL);

	va_start(args, szQuery);
	ptr = sqlite3_vmprintf(szQuery, args);
	va_end(args);
	ASSERT(ptr != NULL);
	
	_EndLock.Lock();
	ASSERT(_nLocked == eOpen || _nLocked == eLocked);
	_Lock.Lock();
	
	// add the string to the queue, and then wait.
	if (_pList == NULL) {
		ASSERT(_nItems == 0);
		_pList = (DpRequestResult *) malloc(sizeof(DpRequestResult));
		_nItems++;
		
		nItem = 0;
	}
	else {
		
		// go thru the list to see if there are any blank entries we can use.
		nItem = 0;
		bFound = false;
		while (nItem < _nItems && bFound == false) {
			if (_pList[nItem].szQuery == NULL && _pList[nItem].bDone == true) {
				ASSERT(_pList[nItem].pResult == NULL);
				bFound = true;
			}
			else { nItem++; }	
		}
		
		if (bFound == false) {
			ASSERT(nItem == _nItems);
			_pList = (DpRequestResult *) realloc(_pList, sizeof(DpRequestResult) * (_nItems+1));
			nItem = _nItems;
			_nItems++;
		}
	}
	
	ASSERT(nItem >= 0 && nItem < _nItems);
	_pList[nItem].szQuery = ptr;
	_pList[nItem].pResult = NULL;
	_pList[nItem].bDone = false;
	_pList[nItem].bReturn = false;
	
	ASSERT(_nItems > 0);
	ASSERT(nItem >= 0);
	ASSERT(nItem < _nItems);
	ASSERT(_pList != NULL);
	ASSERT(_pList[nItem].szQuery != NULL);
	ASSERT(_pList[nItem].pResult == NULL);
	ASSERT(_pList[nItem].bDone == false);
	ASSERT(_pList[nItem].bReturn == false);
	
	_Lock.Unlock();
	_EndLock.Unlock();
}

//-----------------------------------------------------------------------------
// CJW: Sometimes when a client is performing operations, it needs to ensure 
// 		that the data thread doesnt execute the commands out of sequence, so we 
// 		need to provide a method where clients can do all their operations 
// 		without another client butting in.  This is actually kind of difficult 
// 		because we need to ensure that the db thread can keep running, and 
// 		process the commands, and we want our thread to be able to perform 
// 		database operations, but we dont want other threads to do anything.  
// 		This kind of functionality should be used sparingly, and only when it 
// 		could cause data integrity to fail.
void DpDataServer::Begin(void)
{
	_EndLock.Lock();
	_Lock.Lock();
	ASSERT(_nLocked == eOpen);
	_nLocked = ePending;
	_Lock.Unlock();
	
	// Context switch so that the db thread can process anything that was 
	// already in the queue.
	Sleep(0);
	
	_Lock.Lock();
	while(_nLocked == ePending) {
		_Lock.Unlock();
		Sleep(50);
		_Lock.Lock();
	}
	ASSERT(_nLocked == eLocked);
	_Lock.Unlock();
	
}

//-----------------------------------------------------------------------------
// CJW:
void DpDataServer::Commit(void)
{
	// Now we do a commit, and we 
	_Lock.Lock();
	ASSERT(_nLocked == eLocked);
	_nLocked = eCommit;
	_Lock.Unlock();
	
	// Context switch so that the db thread can process anything that was 
	// already in the queue.
	Sleep(0);
	
	_Lock.Lock();
	while(_nLocked == eCommit) {
		_Lock.Unlock();
		Sleep(50);
		_Lock.Lock();
	}
	ASSERT(_nLocked == eCleared);
	_nLocked = eOpen;
	_Lock.Unlock();

	_EndLock.Unlock();
}



