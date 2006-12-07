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

#include <DpServerInterface.h>



//------------------------------------------------------------------------------
// CJW: Constructor:  We initialise the thread stuff, but we dont actually start
//      the thread until we tell this object to listen.
// CJW: Changed the cycle time from 2 seconds to 50 miliseconds.  2 seconds is
//      way too long before accepting each incoming connection.  OnIdle()
//      functionality was added in version 136 which can be used to add some
//      extra wait time when no connections are received.
DpServerInterface::DpServerInterface()
{
    SetCycleTime(50);
	
	Lock();
	_nItems = 0;
	_pList = NULL;
	Unlock();
}




//------------------------------------------------------------------------------
// CJW: Deconstructor.  Clean up the socket and such if this is being
//      deconstructed.
DpServerInterface::~DpServerInterface()
{
	WaitForThread();

	Lock();
	ASSERT((_nItems > 0 && _pList != NULL) || (_nItems >= 0 && _pList == NULL));
	
	while(_nItems > 0) {
		_nItems--;
		if (_pList[_nItems] != NULL) {
			delete _pList[_nItems];
			_pList[_nItems] = NULL;
		}
	}
	
	if (_pList != NULL) {
		free(_pList);
		_pList = NULL;
	}
	
	Unlock();
}


//------------------------------------------------------------------------------
// CJW: Add a thread object to the list.  
void DpServerInterface::AddObject(DpThreadObject *pObject)
{
	bool bFound = false;
	int i;
	
	ASSERT(pObject != NULL);
	
	Lock();
	if (_nItems == 0) {
		ASSERT(_pList == NULL);
	}
	else {
		ASSERT(_nItems > 0);
		ASSERT(_pList != NULL);
	}
	
	for (i=0; i < _nItems && bFound == false; i++) {
		if (_pList[i] == NULL) {
			_pList[i] = pObject;
			bFound = true;
		}
	}
	
	if (bFound == false) {
		_pList = (DpThreadObject **) realloc(_pList, sizeof(DpThreadObject *) * (_nItems + 1));
		_pList[_nItems] = pObject;
		_nItems++;
	}
	
	ASSERT(_nItems > 0 && _pList != NULL);
	Unlock();
}




//------------------------------------------------------------------------------
// CJW: The server interface thread will attempt to accept connections from the
//      clients and spawn new threads to handle those connections.  This thread
//      will fire every cycle until this object is deleted or the thread
//      stopped.
// CJW: Modified functionality slightly to keep accepting new incoming
//      connections as long as there is one to accept.  If no connections were
//      accepted within a cycle, it will call the OnIdle virtual function.
// CJW: Now that this object is managing the object list, we need to do a bit 
// 		of clean up every once in a while.  This function is basically run 
// 		every 50 miliseconds.  So if we do the list check every 100 times, 
// 		that would be 5000 miliseconds which is every 5 seconds... roughly.  
// 		That should be plenty often enough to check the list and clean up any 
// 		objects that have completed.
void DpServerInterface::OnThreadRun(void)
{
    SOCKET nNewSock;
    bool bDone = false;
    int nTimes = 0;

    while (bDone == false) {
        nNewSock = _xSocket.Accept();
        if (nNewSock != INVALID_SOCKET) {
			
            OnAccept(nNewSock);
			
            if (nTimes < DP_SERVER_ACCEPT_CYCLES)   { nTimes++; }
            else                					{ bDone = true; }
        }
        else {
            bDone = true;
        }
    }
    
    if (nTimes == 0) { OnIdle(); }

	CheckList(); 
}


//------------------------------------------------------------------------------
// CJW: Set up the listening socket.   If we are using GCC on linux, then we
//      handle our sockets a little differently.  We are not going to set our
//      socket operations in non-blocking mode, and will instead check before
//      performing a socket operation to see if it will block before performing
//      it.  This is just another way of doing what we had already been doing.
bool DpServerInterface::Listen(int nPort)
{
    bool bReturn = false;

    // will not work with a 0 or a negative number
    ASSERT(nPort > 0);

    Lock();

    // CJW: Create the socket place holder
	if (_xSocket.Listen(nPort) == true) {
		// If we are not using GCC, then we will set the socket to non-blocking.
        _xSocket.SetNonBlocking();
        
        // And then start the thread.
        Start();
        bReturn = true;
    }
    
    Unlock();

    return (bReturn);
}


//------------------------------------------------------------------------------
// CJW: The thread will keep cycling, attempting to accept a socket on the
//      connection.  If there was nothing to accept, then this function will be
//      called instead.  By default, it doesnt do anything, but the derived
//      function could choose to add a bit of a sleep here.
void DpServerInterface::OnIdle(void)
{
}


//------------------------------------------------------------------------------
// CJW: This object will also keep track (optionally) of the threads that are 
// 		created from it (by our child classed).  Because of that, we need to 
// 		clean up the list periodically, and that is where this function comes 
// 		in.  It is needed to delete any thread objects that have completed.
void DpServerInterface::CheckList(void)
{
	int i;
	
	Lock();
	
	for (i=0; i<_nItems; i++) {
		if (_pList[i] != NULL) {
			if (_pList[i]->IsDone() == true) {
				if (OnObjectDelete(_pList[i]) == true) {
					delete _pList[i];
					_pList[i] = NULL;
				}
			}
		}
	}
	
	while(_nItems > 0 && _pList[_nItems-1] == NULL) {
		_nItems--;
	}
	
	if (_nItems == 0) {
		free(_pList);
		_pList = NULL;
	}
	
	Unlock();
}


//------------------------------------------------------------------------------
// CJW: This virtual function is called
bool DpServerInterface::OnObjectDelete(DpThreadObject *pObject)
{
	bool bDelete = true;
	ASSERT(pObject != NULL);
	return(bDelete);
}


