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




//-----------------------------------------------------------------------------
#include <stdlib.h>
#include <string.h>
#include <DpSocketEx.h>




//-----------------------------------------------------------------------------
// CJW: Constructor.  Initialise the socket.
DpSocketEx::DpSocketEx()
{
	_MainLock.Lock();
    _pSocket = NULL;
    
    _BufferLock.Lock();
    _pBuffer = NULL;
	_nBufferLength = 0;
	_BufferLock.Unlock();
	
	_bClosed = false;
	_pReadBuffer = NULL;
    _MainLock.Unlock();
}

//-----------------------------------------------------------------------------
// CJW: Deconstructor.  If we still have a connected socket, then close it.
DpSocketEx::~DpSocketEx()
{
	_MainLock.Lock();
    if (_pSocket != NULL) {
        delete _pSocket;
        _pSocket = NULL;
    }
    ASSERT(_pSocket == NULL);
    
    _BufferLock.Lock();
    ASSERT((_pBuffer == NULL && _nBufferLength == 0) || (_pBuffer != NULL && _nBufferLength >= 0));
    if (_pBuffer != NULL) {
    	free(_pBuffer);
    	_pBuffer = NULL;
    	_nBufferLength = 0;
    }
    ASSERT(_pBuffer == NULL && _nBufferLength == 0);
    _BufferLock.Unlock();
    
    _ExternalLock.Lock();
    _bClosed = true;
    _ExternalLock.Unlock();
    
    if (_pReadBuffer != NULL) {
    	free(_pReadBuffer); _pReadBuffer = NULL;
    }
    
    _MainLock.Unlock();
    
    WaitForThread();
}


//-----------------------------------------------------------------------------
// CJW: Connect to the host....
bool DpSocketEx::Connect(char *szHost, int nPort)
{
	bool bOK = false;
	
	ASSERT(szHost != NULL && nPort > 0);
	
	_MainLock.Lock();
	ASSERT(_pSocket == NULL);
	_pSocket = new DpSocket;
	ASSERT(_pSocket != NULL);
	bOK = _pSocket->Connect(szHost, nPort);
	_MainLock.Unlock();
	
	// start the thread.
	Start();
	
	return(bOK);
}

//-----------------------------------------------------------------------------
// CJW: Return true if the connection has been closed.  Keep in mind that we 
// 		wont actually close the connection until all the data has been 
// 		processed.
bool DpSocketEx::IsClosed(void)
{
	bool bClosed;
	
    _ExternalLock.Lock();
    bClosed = _bClosed;
    _ExternalLock.Unlock();

	return(bClosed);
}


void DpSocketEx::OnThreadStart(void)
{
	_MainLock.Lock();
	ASSERT(_pReadBuffer == NULL);
	ASSERT(DP_MAX_PACKET_SIZE >= 32);
	_pReadBuffer = (char *) malloc(DP_MAX_PACKET_SIZE);
	ASSERT(_pReadBuffer != NULL);
	_MainLock.Unlock();
}


//-----------------------------------------------------------------------------
// CJW: Thread to handle the receiving of data from the socket.  
//	** Should we add new data to a data array, or should we create a series of 
//	   chunks so that we dont have to keep re-sizing and allocating memory?  
//	   Well, if the child class didnt process the data on the first reception 
//	   of it, then it probably needed more data.  So we should just add more 
//	   data.  Since this function will be called often even if there is no data, 
//	   then we should use an internal buffer that is created once for the 
//	   object, and then re-used, rather than continually allocating and then 
//	   freeing data.
void DpSocketEx::OnThreadRun(void)
{
	bool bIdle;
	int done;
	
	_MainLock.Lock();
	
	ASSERT(_pSocket != NULL);
	ASSERT(_pBuffer == NULL && _nBufferLength == 0);
	
	while (_pSocket != NULL || _nBufferLength > 0) {
		bIdle = true;
		
		// read data.
		ASSERT(_pReadBuffer != NULL);
		if (_pSocket != NULL) {
			done = _pSocket->Receive(_pReadBuffer, DP_MAX_PACKET_SIZE);
			if (done < 0) {
				// socket closed.
				delete _pSocket;
				_pSocket = NULL;
			}
			else if (done > 0) {
				_BufferLock.Lock();
				_pBuffer = (char *) realloc(_pBuffer, _nBufferLength + done + 1);
				memcpy(&_pBuffer[_nBufferLength], _pReadBuffer, done);
				_nBufferLength += done;
				_BufferLock.Unlock();
				bIdle = false;
			}
		}
		_MainLock.Unlock();
		
		// call virtual function.
		_BufferLock.Lock();
		if (_nBufferLength > 0) {
			ASSERT(_pBuffer != NULL);
			done = OnReceive(_pBuffer, _nBufferLength);
			ASSERT(done <= _nBufferLength);
			if (done == _nBufferLength) {
				free(_pBuffer);
				_pBuffer = NULL;
				_nBufferLength = 0;
				bIdle = false;
			}
			else if (done > 0) {
				ASSERT(_nBufferLength > done);
				_nBufferLength -= done;
				ASSERT(_nBufferLength > 0);
				memmove(_pBuffer, _pBuffer + done, _nBufferLength);
				_pBuffer = (char *) realloc(_pBuffer, _nBufferLength);
				ASSERT(_pBuffer != NULL);
				bIdle = false;
			}
			else {
				ASSERT(done == 0);
				if (_pSocket == NULL) {
					OnStalled(_pBuffer, _nBufferLength);
					free(_pBuffer); _pBuffer = NULL;
					_nBufferLength = 0;
				}
			}
		}
		
	    ASSERT((_pBuffer == NULL && _nBufferLength == 0) || (_pBuffer != NULL && _nBufferLength >= 0));
		if (_nBufferLength == 0 && _pSocket == NULL) {
			_ExternalLock.Lock();
			_bClosed = true;
			_ExternalLock.Unlock();
			OnClosed();
			bIdle = false;
		}
		
		_BufferLock.Unlock();
		
		if (bIdle == true) {
			Sleep(50);
		}
		_MainLock.Lock();
	}
	_MainLock.Unlock();
}


//-----------------------------------------------------------------------------
// CJW: This virtual function is called after a socket has been closed (from 
// 		the peer), and no data has been queued to be received.  Not called when 
// 		closed directly).
void DpSocketEx::OnClosed(void)
{
	_MainLock.Lock();
	ASSERT(_pSocket == NULL);
	_MainLock.Unlock();
}




//-----------------------------------------------------------------------------
// CJW: if the peer has closed the socket but there is still data that hasnt 
// 		been processed, we still call OnReceive so that the child process can 
// 		deal with it.  if the Onreceive does not process any of the data from 
// 		the queue, then this virtual function is called to let the child know 
// 		that there will not be any more data coming from the socket.  This 
// 		function will happen once, and then the remaining data will be purged 
// 		and the socket completed.  The object itself will then be able to be 
// 		deleted (if it was created as a session for DpServerInterface it would 
// 		be cleaned up automatically).  OnClosed will be called after.
void DpSocketEx::OnStalled(char *pData, int nLength)
{
	ASSERT(pData != NULL && nLength > 0);
	_MainLock.Lock();
	ASSERT(_pSocket == NULL);
	_MainLock.Unlock();
}


int DpSocketEx::Send(char *pData, int nLength)
{
	int nTotal=0;
	
	ASSERT(pData != NULL && nLength > 0);
	_MainLock.Lock();
	
	// we need to keep sending until all the data has been sent.
	#error not complete
	
	_MainLock.Unlock();
}



