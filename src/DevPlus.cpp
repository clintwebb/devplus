//-----------------------------------------------------------------------------
// DevPlus.   
//-----------------------------------------------------------------------------


/***************************************************************************
 *   Copyright (C) 2003-2005 by Clinton Webb,,,                            *
 *   devplus@cjdj.org                                                      *
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
// CJW: Include the header file.
#include <DevPlus.h>
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// CJW: Sanity check to make sure that the header and source that we are using 
// 		are the same version.  Otherwise some weird behaviour could be 
// 		experienced and difficult to track down.
#if (VERSION_DEVPLUS_THIS != 147) 
#error Header/Source version mismatch.
#endif
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// CJW: Include the required header files for each class that is not excluded.

#ifndef _EXCLUDE_THREADBASE
    #ifdef __GNUC__
        #include <time.h>
    #endif
#endif

#ifndef _EXCLUDE_THREADOBJECT
    #ifdef __GNUC__
        #include <pthread.h>
    #else
        #include <process.h>
    #endif
    #include <time.h>
#endif

#ifndef _EXCLUDE_INI
    #include <stdio.h>
#endif

#ifndef _EXCLUDE_DB
    #ifndef _EXCLUDE_DB_ODBC
        #ifndef __GNUC__
            #include <afxwin.h>
            #include "SqlDB.h"
            #include "SQLEXT.H"
        #endif
    #endif
#endif

#ifndef _EXCLUDE_SOCKET
    #ifdef __GNUC__
        #include <sys/types.h>
        #include <sys/socket.h>
        #include <arpa/inet.h>
        #include <netdb.h>
        #include <unistd.h>
        #include <errno.h>
        #include <fcntl.h>
    #else
        #include <winsock2.h>
    #endif
    
    #include <string.h>
#endif

#ifndef _EXCLUDE_DATAQUEUE
    #include <stdlib.h>
	#include <stdio.h>
#endif
//-----------------------------------------------------------------------------







// #############################################################################
// #############################################################################
// #############################################################################

#ifndef _EXCLUDE_SOCKET

//-----------------------------------------------------------------------------
// CJW: Constructor.  Initialise the socket.
DpSocket::DpSocket()
{
    _nSocket = 0;
}

//-----------------------------------------------------------------------------
// CJW: Deconstructor.  If we still have a connected socket, then close it.
DpSocket::~DpSocket()
{
    if (_nSocket != 0) {
        Close();
    }
    ASSERT(_nSocket == 0);
}

//-----------------------------------------------------------------------------
// CJW: Initialise the sockets.
//
//  TODO    Is there some way we can test to see if this has already been done
//          before we try and do it?  This object should co-exist peacefully
//          with other socket handling code and therefore, this initialisation
//          might already have been done.
void DpSocket::Init(void)
{
    #ifdef __GNUC__
        // In Linux, we dont need to initialise the socket library, it is
        // built in to the kernel.
    #else
        WORD wVersionRequested;
        WSADATA wsaData;
        wVersionRequested = MAKEWORD( 2, 2 );
        WSAStartup( wVersionRequested, &wsaData );
    #endif
}



//-----------------------------------------------------------------------------
// CJW: In order to connect to a remote host, we have to resolve the name or IP
//      address and port into a structure.
int DpSocket::Resolve(const char szAddr[], int iPort, struct sockaddr_in *pSin, char *szType)
{
    unsigned long ulAddress;
    struct hostent *hp;

    ASSERT(szAddr != NULL && szAddr[0] != '\0' && iPort > 0);
	ASSERT(pSin != NULL && szType != NULL);
    
    // First, assign the family and port.
    pSin->sin_family = AF_INET;
    pSin->sin_port = htons(iPort);

    // Look up by standard notation (xxx.xxx.xxx.xxx) first.
    ulAddress = inet_addr(szAddr);
    if ( ulAddress != (unsigned long)(-1) )  {
        // Success. Assign, and we're done.  Since it was an actual IP address, then we dont do any DNS lookup for that, so we cant do any checking for any other address type (such as MX).
        pSin->sin_addr.s_addr = ulAddress;
        return 0;
    }

    // If that didn't work, try to resolve host name by DNS.
    hp = gethostbyname(szAddr);
    if( hp == NULL ) {
        // Didn't work. We can't resolve the address.
        return -1;
    }

    // Otherwise, copy over the converted address and return success.
    memcpy( &(pSin->sin_addr.s_addr), &(hp->h_addr[0]), hp->h_length);
    return 0;
}


//-----------------------------------------------------------------------------
// Connect to the host and port.  Return true if we are connected.  Returns
// false if we couldnt connect.
bool DpSocket::Connect(char *szHost, int nPort)
{
    bool bConnected=false;
    struct sockaddr_in sin;

    ASSERT(szHost != NULL);
    ASSERT(nPort > 0);
    ASSERT(_nSocket == 0);

    if (Resolve(szHost,nPort,&sin) >= 0) {
        // CJW: Create the socket
        _nSocket = socket(AF_INET,SOCK_STREAM,0);
        if (_nSocket >= 0) {
            // CJW: Connect to the server
            if (connect(_nSocket, (struct sockaddr*)&sin, sizeof(struct sockaddr)) >= 0) {
                SetNonBlocking();   // and set the socket for non-blocking mode.
                bConnected = true;
            }
            else {
                Close();
                bConnected = false;
            }
        }
    }

    return(bConnected);
}


//-----------------------------------------------------------------------------
// CJW: If we are connected, then close the connection and initialise the
//      socket value.
void DpSocket::Close()
{
    int nResult;
    
	if (_nSocket > 0) {
    
#ifdef __GNUC__
        nResult = close(_nSocket);
#else
        nResult = closesocket(_nSocket);
#endif
        ASSERT(nResult == 0);
        _nSocket = 0;
    }
}


//-----------------------------------------------------------------------------
// CJW: Receive some chars from the socket if there are any there.
//
//  ** Under linux, is the socket handling thread safe?   We need to have a
//     define somewhere that indicates if we are compiling with threads or
//     not.  So that we can put a lock around all the socket routines to make
//     sure that they are not run at the same time.  As far as I can tell the
//     most important problem is that if a socket operation would block, it
//     would return a -1, and then we are supposed to check some global
//     variable of some sort. This itself doesn't sound very thread safe. At
//     this point, I think I can put a critical lock around it so that only
//     one socket operation is done at a time. A better solution would be to
//     perform a select() on the socket to see if it has data ready to read.
//     That way we are letting the OS perform any mutex operations to ensure
//     thread (or process) safety.
//
//  Although it is not obvious, errno is thread-local which means that we can
//  safely look at errno in this thread without concern that it is being updated
//  in another thread.
//
int DpSocket::Receive(char *data, int len)
{
	int nError;
	#ifdef __GNUC__
        ssize_t nResult = -1;
    #else
        int nResult = -1;
    #endif
    
    
    ASSERT(_nSocket > 0);
    ASSERT(data != NULL && len > 0);
    
    if ((_nSocket > 0) && (len > 0) && (data != NULL)) {
        nResult = recv(_nSocket, data, len, 0);
        if (nResult == 0) {
            _nSocket = 0;
            nResult = -1;
        }
        else if (nResult < 0) {
            // we got an error on the socket, so make a note of the error number.
            #ifdef __GNUC__
                nError = errno;
                if (nError == EAGAIN) {
                    nResult = 0;
                }
                else {
                    ASSERT(nResult == -1);
                }
            #else
                nError = WSAGetLastError();
                if (nError == WSAEWOULDBLOCK) {
                    nResult = 0;
                }
                else {
                    ASSERT(nResult == -1);
                }
            #endif
        }
    }

    return(nResult);
}


//-----------------------------------------------------------------------------
// CJW: Send some characters to the remote connection.  If we cant send at this
//      time, exit and report.  This function is NON-BLOCKING.
int DpSocket::Send(char *data, int len)
{
    int nResult = -1;
    int nError;
    
    ASSERT(_nSocket > 0);
    ASSERT(data != NULL && len > 0);

    if ((_nSocket > 0) && (len > 0) && (data != NULL)) {
        nResult = send(_nSocket, data, len, 0);
        if (nResult == 0) {
            _nSocket = 0;
            nResult = -1;
        }
        else if (nResult < 0) {
            // we got an error on the socket, so make a note of the error number.
            #ifdef __GNUC__
                nError = errno;
                if (nError == EAGAIN) {
                    nResult = 0;
                }
                else {
                    ASSERT(nResult == -1);
                }
            #else
                nError = WSAGetLastError();
                if (nError == WSAEWOULDBLOCK) {
                    nResult = 0;
                }
                else {
                    ASSERT(nResult == -1);
                }
            #endif
        }
    }

    return(nResult);
}



//-----------------------------------------------------------------------------
// CJW: Accept the socket so that we can use it.  If we are already handling a
//      socket, then close it, but that is bad programming.
void DpSocket::Accept(SOCKET nSocket)
{
    ASSERT(nSocket > 0);
    ASSERT(_nSocket == 0);
	
    if (_nSocket > 0) {
        Close();
    }
	
    _nSocket = nSocket;
    SetNonBlocking();
}



//-----------------------------------------------------------------------------
// CJW: Set socket for non-blocking.
void DpSocket::SetNonBlocking()
{
    #ifdef __GNUC__
        int opts;
    
        opts = fcntl(_nSocket, F_GETFL);
        if (opts >= 0) {
            opts = (opts | O_NONBLOCK);
            fcntl(_nSocket, F_SETFL, opts);
        }
    #else
        u_long nSetting = 1;
    
        ASSERT(_nSocket > 0);
        if (_nSocket > 0) {
            ioctlsocket(_nSocket, FIONBIO, &nSetting);
        }
    #endif
}



//-----------------------------------------------------------------------------
// CJW: Detach the socket handle from this object.  Once this function is 
//      called, this object will not be able to perform any functionality on
//      the socket.  It is intended to be used to transfer control to a
//      different DpSocket derived object (or otherwise) that performs a
//      different set of operations on the socket.  For example, a socket
//      object can be used to initialise and handshake, but then when the type
//      of functionality required has been determined, pass it over to an
//      object that can handle it.
SOCKET DpSocket::Detach()
{
    SOCKET s;
    
    ASSERT(_nSocket != 0);
    
    s = _nSocket;
    _nSocket = 0;

    return(s);
}

//-----------------------------------------------------------------------------
// CJW: Accept a pending socket.  The returned socket is not being controlled 
//      by this object.  Actually it is assumed that this object is the one
//      listening for new connections.
SOCKET DpSocket::Accept()
{
    SOCKET nNewSock = INVALID_SOCKET;
#ifdef __GNUC__
    socklen_t len;
    sockaddr_in sa;

    len = sizeof(sa);
#endif
    if (_nSocket > 0) {
        // CJW: Accept the incoming connection.
        nNewSock = accept(_nSocket, NULL, NULL);
    }

    return(nNewSock);
}

//-----------------------------------------------------------------------------
// CJW: Return true if we are actually connected.  We will use the socket 
//      handle to determine this, because in all instances, if we are not
//      connected then we would have cleared the socket handle.   A large
//      number of ASSERT's littered throughout the code is intended to make
//      sure that this is true, but be careful if adding functionality that
//      could cause the socket to close.
bool DpSocket::IsConnected()
{
    return(_nSocket > 0 ? true : false);
}


//-----------------------------------------------------------------------------
// CJW: Sometimes it is nice to know what is on the other end of the socket 
//      that we are controlling.  This function will return a fully qualified
//      network name if we know it.  Otherwise we will have an IP address.
//
//      NOTE: Do not rely on the name resolving, as in some cases even if the
//            name is known, the IP will still be returned).
void DpSocket::GetPeerName(char *pStr, int nMax)
{
    struct sockaddr name;
    socklen_t namelen;
    int nResult;
    
    ASSERT(pStr != NULL);
    ASSERT(nMax > 0);
    
    ASSERT(_nSocket > 0);
    
    namelen = sizeof(name);
    nResult = getpeername(_nSocket, &name, &namelen);
    
    ASSERT(0);
    // Need to finish this.  Once we have something running that can be used to test it, we need to put some print information so we can see what this functional actually returns.
}


//------------------------------------------------------------------------------
// CJW: Set up the listening socket.   If we are using GCC on linux, then we
//      handle our sockets a little differently.  We are not going to set our
//      socket operations in non-blocking mode, and will instead check before
//      performing a socket operation to see if it will block before performing
//      it.  This is just another way of doing what we had already been doing.
bool DpSocket::Listen(int nPort)
{
	struct sockaddr_in sin;
	bool bReturn = false;

    // will not work with a 0 or a negative number
	ASSERT(nPort > 0);
	ASSERT(_nSocket == 0);

    // CJW: Create the socket place holder
	_nSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_nSocket > 0) {
		sin.sin_family = AF_INET;
		sin.sin_port = htons(nPort);
		sin.sin_addr.s_addr = INADDR_ANY;

		if (bind(_nSocket, (struct sockaddr *) &sin, sizeof(struct sockaddr)) == 0) {
            // Can be in new thread
			if (listen(_nSocket, 5) == 0) {
				bReturn = true;
			}
			else {
#ifdef __GNUC__
                    close(_nSocket);
#else
                    closesocket(_nSocket);
#endif
                _nSocket = 0;
			}
		}
		else {
			_nSocket = 0;
		}
	}
    
	return (bReturn);
}





#endif

// #############################################################################
// #############################################################################
// #############################################################################








// #############################################################################
// #############################################################################
// #############################################################################


#ifndef _EXCLUDE_DATAQUEUE

#include <stdarg.h>


//-----------------------------------------------------------------------------
// CJW: Start the object with an empty buffer.
DpDataQueue::DpDataQueue()
{
    _pBuffer = NULL;
    _nLength = 0;
}


//-----------------------------------------------------------------------------
// CJW: Deconstructor.  Clean up the buffer if there is anything in it.
DpDataQueue::~DpDataQueue()
{
    if (_pBuffer != NULL) {
        ASSERT(_nLength > 0);
        free(_pBuffer);
        _pBuffer = NULL;
        _nLength = 0;
    }
    ASSERT(_nLength == 0);
}


//-----------------------------------------------------------------------------
// return true if the queue is empty.
bool DpDataQueue::IsEmpty()
{
    bool res;

    if (_nLength > 0)
        res = false;
    else
        res = true;

    return(res);
}


//-----------------------------------------------------------------------------
// CJW: Add some data to the queue.
void DpDataQueue::Add(char *data, int len)
{
    ASSERT(data != NULL);
    ASSERT(len > 0);

#ifdef _DEBUG
    int tmp = _nLength;
#endif

	ASSERT((_pBuffer == NULL && _nLength == 0) || (_nLength != 0 && _pBuffer != NULL));

    _pBuffer = (char *) realloc(_pBuffer, _nLength + len + 1);
    ASSERT(_pBuffer != NULL);
    if (_pBuffer != NULL) {
        memcpy(&_pBuffer[_nLength], data, len);
        _nLength += len;
#ifdef _DEBUG
        ASSERT(_nLength > tmp);
#endif
    }
}


//-----------------------------------------------------------------------------
// CJW: Clear the queue.
void DpDataQueue::Clear()
{
    if (_pBuffer != NULL) {
        free(_pBuffer);
        _pBuffer = NULL;
        _nLength = 0;
    }
    ASSERT(_nLength == 0);
}

// CJW: Remove 'cnt' number of chars from the front of the queue.
void DpDataQueue::Remove(int cnt)
{
    char *pNew;

    ASSERT(cnt > 0);
    ASSERT(_nLength >= cnt && _pBuffer != NULL);

    if (_nLength <= cnt) {
        if (_pBuffer != NULL) {
            free(_pBuffer);
            _pBuffer = NULL;
        }
        _nLength = 0;
    }
    else {
        pNew = (char *) malloc((_nLength-cnt)+1);
        ASSERT(pNew != NULL);
        if (pNew != NULL) {
            memcpy(pNew, &_pBuffer[cnt], _nLength-cnt);
            _nLength -= cnt;
            free(_pBuffer);
            _pBuffer = pNew;
        }
        else {
            _nLength = 0;
        }
    }
}

//-----------------------------------------------------------------------------
// CJW: Return the length of the data in the queue.
int DpDataQueue::Length()
{
    return(_nLength);
}

//-----------------------------------------------------------------------------
// CJW: Push data onto the queue... same as Add really.
void DpDataQueue::Push(char *data, int len)
{
    char *pOldBuffer;

    ASSERT(data != NULL && len > 0);

    #ifdef _DEBUG
        int tmp = _nLength;
	#endif

	ASSERT((_pBuffer == NULL && _nLength == 0) || (_nLength != 0 && _pBuffer != NULL));

    if (_nLength == 0) {
        Add(data, len);
    }
    else {
        pOldBuffer = _pBuffer;
        _pBuffer = (char *) malloc(_nLength + len + 1);
        ASSERT(_pBuffer != NULL);
        if (_pBuffer != NULL) {

            memcpy(_pBuffer, data, len);
            memcpy(&_pBuffer[len], pOldBuffer, _nLength);
            _nLength += len;
            free(pOldBuffer);

#ifdef _DEBUG
            ASSERT(_nLength > tmp);
#endif
        }
        else {
            _pBuffer = pOldBuffer;
        }
    }
}


//-----------------------------------------------------------------------------
// CJW: Inserts a single char to the top of the queue.
//
//  TODO: This implementation is not very efficient.  It should be re-written
//        if this function is going to be used extensively.  It is implemented
//        this way because the only time this function is currently used is if
//        a char is pulled from the queue, but it is determined that it cant be
//        processed, it is put back.  This is only used when other buffers get
//        full, and it is not expected to actually happen very often in
//        normally running code.
void DpDataQueue::Push(char ch)
{
    char buffer[2];
    
    buffer[0] = ch;
    buffer[1] = '\0';
    
    Push(buffer, 1);
}




//-----------------------------------------------------------------------------
// CJW: Pop some data off the queue.   There MUST be enough data in there to pop off.
char * DpDataQueue::Pop(int nLength)
{
    char *pTmp = NULL;

    ASSERT(nLength > 0 && nLength <= _nLength);
    if (nLength <= _nLength) {
        pTmp = (char *) malloc(nLength+2);
        ASSERT(pTmp != NULL);
        if (pTmp != NULL) {
            memcpy(pTmp, _pBuffer, nLength);
            Remove(nLength);
        }
    }

    return (pTmp);
}


//-----------------------------------------------------------------------------
// CJW: Look in the data that we have for this particular char.  if we find it,
//      return the char position that it was found.
int DpDataQueue::FindChar(char ch)
{
    int nPos = 0;
    int nCount;
    char *pTmp;
    
    pTmp = _pBuffer;
    for(nCount=0; nCount<_nLength; nCount++) {
        if (*pTmp == ch) {
            nPos = nCount;
            nCount = _nLength;
        }
        pTmp++;
    }
    
    return(nPos);
}



//------------------------------------------------------------------------------
// CJW: Using a regular printf format string, add data to the data queue.  
// 		Useful for when the dataqueue is being used for outputting a stream of 
// 		text.
int DpDataQueue::Print(char *fmt, ...)
{
	va_list args;
	char *ptr;
	int len;
	
	ASSERT(fmt != NULL);

	ptr = (char *) malloc(32767);
	ASSERT(ptr != NULL);
	
	va_start(args, fmt);
	len = vsprintf(ptr, fmt, args);
	ASSERT(len <= 32767);
	va_end(args);
	
	Add(ptr, len);
	free(ptr);
	
	return(len);
}


//------------------------------------------------------------------------------
// CJW: Look in the data queue and find the end of a line.  If there isnt a 
// 		whole line there, then we return a null.  Otherwise we return a complete 
// 		line (including the CRLF).
char *DpDataQueue::GetLine(void)
{
	char *result = NULL;
	int nLength;
	bool bFound;
	int i,j;
	
	ASSERT((_pBuffer == NULL && _nLength == 0) || (_pBuffer != NULL && _nLength > 0));
	
	if (_nLength > 0) {
		
		bFound = false;
		nLength = 0;
		while (nLength < _nLength && bFound == false) {
			if (_pBuffer[nLength] == '\r') {
				bFound = true;				
			}
			nLength++;
		}
		
		if (bFound == true) {
			ASSERT(nLength >= 0);
			result = Pop(nLength);
			ASSERT(result != NULL);
			
			for (i=0,j=0; i<nLength; i++) {
				if (result[i] != '\r' && result[i] != '\n') {
					if (j != i) {
						result[j] = result[i];
					}
					j++;
				}
			}
			result[j] = '\0';
		}
	}
		
	return(result);
}


#endif

// #############################################################################
// #############################################################################
// #############################################################################


#ifndef _EXCLUDE_THREADBASE


//------------------------------------------------------------------------------
// CJW: Constructor for DpThreadbase... nothing so far.
DpThreadBase::DpThreadBase()
{
}


//------------------------------------------------------------------------------
DpThreadBase::~DpThreadBase()
{
}


//------------------------------------------------------------------------------
// CJW: If we are using GNUC then we need to provide our own sleep function
//      that uses miliseconds.
#ifdef __GNUC__
void DpThreadBase::Sleep(DWORD dTime)
{
    struct timespec strTime;
    
	if (dTime < 1000) 	{ strTime.tv_sec = 0; }
	else 				{ strTime.tv_sec = dTime / 1000; }
	
	strTime.tv_nsec = (dTime % 1000) * 1000000;
    
    nanosleep(&strTime, NULL);
}
#endif



#endif





// #############################################################################
// #############################################################################
// #############################################################################









// #############################################################################
// #############################################################################
// #############################################################################
#ifndef _EXCLUDE_LOCK

//------------------------------------------------------------------------------
// CJW: Constructor.  We need to initialise the object by initialising our
//      critical sections (mutexes) and clearing our counters if we have any.
DpLock::DpLock()
{
	_nWriteCount = 0;
	
	#ifdef __GNUC__
 		pthread_mutexattr_t attr;
    
        pthread_mutexattr_init(&attr);
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
		pthread_mutex_init(&_csLock, &attr);
		pthread_mutexattr_destroy(&attr);
	#else
        InitializeCriticalSection(&_csLock);
	#endif
}



//-----------------------------------------------------------------------------
// CJW: Deconstructor.    Before we can destroy the object, our counter needs 
// 		to be down to zero.  If it isnt, then we need to fire an assertion.
DpLock::~DpLock()
{
	ASSERT(_nWriteCount == 0);
	
	#ifdef __GNUC__
        pthread_mutex_destroy(&_csLock);
	#else
	// do we need to do anything to destroy a critical section handle in windows?
	#endif
}




//-----------------------------------------------------------------------------
// CJW: This function is used to indicate that a function wants to lock the
//      object for exclusive access.
void DpLock::Lock()
{
	#ifdef __GNUC__
        pthread_mutex_lock(&_csLock);
	#else
        EnterCriticalSection(&_csLock);
	#endif
	
	ASSERT(_nWriteCount >= 0);
	_nWriteCount ++;
	ASSERT(_nWriteCount > 0);
}


//-----------------------------------------------------------------------------
// CJW: The object does not need to be locked anymore for this thread.  If the
//      readcount is greater than zero, then this must be a readlock.  So all
//      we need to do is decrement the readcount value.  if the readcount is
//      zero, then this must have been a write lock.  To verify the integrity,
//      we will assert if the write flag is not sert to true.  This will
//      indicate that somewhere the locks and unlocks got out of sync.
void DpLock::Unlock()
{
	ASSERT(_nWriteCount > 0);
	_nWriteCount --;
	ASSERT(_nWriteCount >= 0);
	
	#ifdef __GNUC__
		pthread_mutex_unlock(&_csLock);
	#else
        LeaveCriticalSection(&_csLock);
	#endif
}

#endif

// #############################################################################
// #############################################################################
// #############################################################################

















// #############################################################################
// #############################################################################
// #############################################################################
#ifndef _EXCLUDE_READWRITELOCK
#include <signal.h>

//------------------------------------------------------------------------------
// CJW: Constructor.  We need to initialise the object by initialising our
//      critical sections (mutexes) and clearing our counters.
DpReadWriteLock::DpReadWriteLock()
{
    #ifdef __GNUC__
        pthread_mutexattr_t attr;
    
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&_csLockData, &attr);
        pthread_mutexattr_destroy(&attr);
    #else
        InitializeCriticalSection(&_csLockData);
    #endif
	
	DpLock::Lock();
	DataLock();
    
    _nReadCount = 0;
    _nWriteCount = 0;
    
	DataUnlock();
	DpLock::Unlock();
}



//-----------------------------------------------------------------------------
// CJW: Deconstructor.    Before we can destroy the object, we need to make
//      sure all the reads and writes are finished.  So we will keep looping
//      until we get to a point where we can quit.
DpReadWriteLock::~DpReadWriteLock()
{
    #ifdef __GNUC__
		DpLock::Lock();
        pthread_mutex_lock(&_csLockData);
    #else
		DpLock::Lock();
        EnterCriticalSection(&_csLockData);
    #endif

	ASSERT(_nWriteCount == 0 && _nReadCount == 0);

    #ifdef __GNUC__
        pthread_mutex_unlock(&_csLockData);
		pthread_mutex_destroy(&_csLockData);
		DpLock::Unlock();
    #else
        LeaveCriticalSection(&_csLockData);
		DpLock::Unlock();
    #endif
}


void DpReadWriteLock::DataLock(void)
{
	#ifdef __GNUC__
		pthread_mutex_lock(&_csLockData);
	#else
		EnterCriticalSection(&_csLockData);
	#endif
}

void DpReadWriteLock::DataUnlock(void)
{
	#ifdef __GNUC__
		pthread_mutex_unlock(&_csLockData);
	#else
    	LeaveCriticalSection(&_csLockData);
	#endif
}


//-----------------------------------------------------------------------------
// CJW: Use this function to indicate that you want to read from the object.
//      This will not actually prevent you from writing to it, but if you do
//      need to write, make sure you use WriteLock().   The first lock would
//      halt execution until all the write-locks have released.  Therefore this
//      function can only complete if the real lock is not in place.
//
//      NOTE: if the main lock completes, yet write is true, then we are
//            currently in a write lock.  If there is no write locks in the
//            path of execution, then we have a logic error and have left-over
//            write-lock that should not be there.
void DpReadWriteLock::ReadLock()
{
    // CJW: So that we can avoid some sync'ing problems we need to actually
    //      Lock the object while doing this function.   If we pass this
    //      lock, then we should have exclusive rights to the object..  We
    //      would not actually get this lock if there is a write lock until
    //      the write lock is finished.
	DpLock::Lock();
	DataLock();
    
    ASSERT(_nWriteCount == 0);
	ASSERT(_nReadCount >= 0);
    _nReadCount++;             // CJW: Now increment the read count.
    ASSERT(_nReadCount > 0);
    
	DataUnlock();
	DpLock::Unlock();
}


//-----------------------------------------------------------------------------
// CJW: This function is used to indicate that a function wants to lock the
//      object for exclusive access.   It will first lock the object.   Then it
//      will loop until all the reads have stopped, unlocking the object before
//      it does a sleep and then it will lock it again.   Once it has gone thru
//      that and have indicated that it is writing, it will exit the function
//      without unlocking it.   When the user unlocks. It will then be unlocked.
void DpReadWriteLock::WriteLock()
{
#ifdef _DEBUG
    unsigned int nLoops = 0;
#endif

    // CJW: Now that all the read-locks have released their hold, we can lock the object.
	DpLock::Lock();

    // CJW: Now we are the next in line to write, but we first need to
    //      wait until all the read locks have expired.
    DataLock();
    
    while (_nReadCount > 0)
    {
        ASSERT(_nWriteCount == 0);
		DataUnlock();

#ifdef _DEBUG
        // CJW: assert if we have looped 100 times (20 seconds).
        nLoops++;
        if (nLoops > 200) {

            // CJW: This assertion might indicate that we have a read-lock
            //      followed by a write-lock in the same thread which would
            //      never clear.
            ASSERT(0);
            nLoops = 0;
        }
#endif

        Sleep(100);
        DataLock();
    }

    // CJW: If the following assertion fails, then we have a syncing problem.
    ASSERT(_nReadCount == 0);
	ASSERT(_nWriteCount >= 0);
    _nWriteCount++;
    ASSERT(_nWriteCount > 0);

	DataUnlock();
}


//-----------------------------------------------------------------------------
// CJW: The object does not need to be locked anymore for this thread.  If the
//      readcount is greater than zero, then this must be a readlock.  So all
//      we need to do is decrement the readcount value.  if the readcount is
//      zero, then this must have been a write lock.  To verify the integrity,
//      we will assert if the write flag is not sert to true.  This will
//      indicate that somewhere the locks and unlocks got out of sync.
void DpReadWriteLock::Unlock()
{
	DataLock();
    
    if (_nReadCount > 0) {
        // CJW: This is a read lock, so we dont have to actually unlock anything.
        ASSERT(_nWriteCount == 0);
        _nReadCount--;
    }
    else {
        // CJW: This must be a write lock.  So we need to make sure that our
        //      write-count is positive and then actually unlock.
        ASSERT(_nWriteCount > 0);
    
        // CJW: We are not writing any more.
        _nWriteCount--;

		DpLock::Unlock();
    }
	
	DataUnlock();    
}

#endif

// #############################################################################
// #############################################################################
// #############################################################################









// #############################################################################
// #############################################################################
// #############################################################################

#ifndef _EXCLUDE_SERVERINTERFACE

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
	ASSERT((_nItems > 0 && _pList != NULL) || (_nItems >= 0 && _pList == NULL));

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
	static int nCheck = 0;

    while (bDone == false) {
        nNewSock = _xSocket.Accept();
        if (nNewSock != INVALID_SOCKET) {
			
            OnAccept(nNewSock);
			
            if (nTimes < 100)   { nTimes++; }
            else                { bDone = true; }
        }
        else {
            bDone = true;
        }
    }
    
#if (VERSION_DEVPLUS >= 136)
    if (nTimes == 0) {
        OnIdle();
		if (nCheck > 100)	{ 
			CheckList(); 
			nCheck = 0; 
		}
		else { 
			nCheck++; 
		}
	}
#endif
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
        Start();
        bReturn = true;
    }
    
    Unlock();

    return (bReturn);
}


#if (VERSION_DEVPLUS >= 136)
//------------------------------------------------------------------------------
// CJW: The thread will keep cycling, attempting to accept a socket on the
//      connection.  If there was nothing to accept, then this function will be
//      called instead.  By default, it doesnt do anything, but the derived
//      function could choose to add a bit of a sleep here.
void DpServerInterface::OnIdle(void)
{
}
#endif


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
				delete _pList[i];
				_pList[i] = NULL;
			}
		}
	}
	
	while(_nItems > 0 && _pList[_nItems-1] == NULL) {
		_nItems--;
	}
	
	Unlock();
}


#endif

// #############################################################################
// #############################################################################
// #############################################################################





























// #############################################################################
// #############################################################################
// #############################################################################

#ifndef _EXCLUDE_THREADOBJECT

//------------------------------------------------------------------------------
// CJW: Constructor.  Initialise our thread.  We will start with a default cycle
//      time, but the mode will be set to RunOnce.  Therefore you cannot compare
//      the cycle time to anything for comparison
DpThreadObject::DpThreadObject()
{
    Lock();

    _pToFunction = NULL;
    _pToParam = NULL;
    _dToCycleTime = DP_DEFAULT_CYCLE_TIME;
	_eToMode = dptRunOnce;
	_eToControl = dptControlRun;
	_eToStatus = dptUnknown;

    #ifdef __GNUC__
        pthread_attr_init(&_attr);
    #else
        _nToStackSize = 0;
        _hToHandle = 0;
    #endif

    Unlock();
}

//------------------------------------------------------------------------------
// CJW: Deconstructor.  We are going out of scope, we need to indicate we are
//      closing, then wait for the thread function to exit if it is already
//      exiting.
DpThreadObject::~DpThreadObject()
{
    WaitForThread();

    Lock();
    #ifdef __GNUC__
        pthread_attr_destroy(&_attr);
    #endif
	_eToStatus = dptDone;
    Unlock();
}


//------------------------------------------------------------------------------
// CJW: This function is supposed to wait for the thread to stop processing if 
// 		it is processing.
void DpThreadObject::WaitForThread(void)
{
    Lock();
	if (_eToStatus != dptPending && _eToStatus != dptUnknown && _eToControl == dptControlRun) {
        Stop();
    }
	
	if (_eToStatus != dptPending && _eToStatus != dptUnknown) {
		while(_eToStatus != dptStopped) {
            Unlock();
            DpThreadBase::Sleep(200);
            Lock();
        }
    }
    Unlock();
}



//------------------------------------------------------------------------------
// CJW: Set the cycle time to call the OnThreadRun() function.
void DpThreadObject::SetCycleTime(DWORD dTime)
{
	ASSERT(dTime > 0);
    Lock();
	_eToMode = dptCycle;
    _dToCycleTime = dTime;
    Unlock();
}


//------------------------------------------------------------------------------
// CJW: Start the thread.  This assumes that we are using DpThreadObject as a
//      base class.   Otherwise there would be no use for this function.
void DpThreadObject::Start(void)
{
    Lock();
    
	ASSERT(_eToControl == dptControlRun);
	ASSERT(_eToStatus == dptPending || _eToStatus == dptUnknown);
    
	_eToStatus = dptStarting;
    #ifdef __GNUC__
        pthread_attr_setdetachstate(&_attr, PTHREAD_CREATE_DETACHED);
        pthread_create(&_thread, &_attr, &(DpThreadObject::ThreadProcessFunction), this);
    #else
        ASSERT(_hToHandle == 0);
        _hToHandle = (HANDLE) _beginthread(DpThreadObject::ThreadProcessFunction, _nToStackSize, this);
    #endif
    
    Unlock();
}




//------------------------------------------------------------------------------
// CJW: This sets the call-back function and starts the thread.  The call-back
//      function will be called every X miliseconds.  It will have a cycle-time
//      set.  The cycle-time will mean that if you set it for 250 miliseconds,
//      and it takes 40 miliseconds to process the cycle, then it will sleep for
//      210 miliseconds and then start the cycle again.
//
//      The call-back function is not called directly for the thread, instead it
//      is called by the OnThreadRun() function inside this class.  If you
//      derive this class and over-ride the OnThreadRun() function, you can
//      provide a one-cycle thread by simple doing all your upper-level
//      processing in that one task.
void DpThreadObject::Start(void (*fn)(void *), void *pParam)
{
    Lock();

	ASSERT(_eToControl == dptControlRun);
	ASSERT(_eToStatus == dptPending || _eToStatus == dptUnknown);
    
    _pToFunction = fn;
    _pToParam = pParam;
	_eToStatus = dptStarting;

    #ifdef __GNUC__
        pthread_attr_setdetachstate(&_attr, PTHREAD_CREATE_DETACHED);
        pthread_create(&_thread, &_attr, &(DpThreadObject::ThreadProcessFunction), this);
    #else
        _hToHandle = (HANDLE) _beginthread(DpThreadObject::ThreadProcessFunction, _nToStackSize, this);
    #endif
    
    Unlock();
}



//------------------------------------------------------------------------------
// CJW: This function contains the thread that is managed by this object.  It
//      will have the pointer to the DpThreadObject that it refers to.  It will
//      pass control back to the object.  When that is done, it will end the
//      thread.
#ifdef __GNUC__
void* DpThreadObject::ThreadProcessFunction(void *pParam)
#else
void DpThreadObject::ThreadProcessFunction(void *pParam)
#endif
{
    DpThreadObject *pThread;
    
    pThread = (DpThreadObject *) pParam;
    if (pThread != NULL) {
        pThread->ProcessThread();
    }
#ifdef __GNUC__
    return(NULL);
#endif
}



//------------------------------------------------------------------------------
// CJW: Called by the static object that doesnt really know anything about this
//      object.  This will run the call-back function based on whether we are in
//      Cycle or RunOnce mode.
void DpThreadObject::ProcessThread(void)
{
    Lock();
	ASSERT(_eToStatus == dptStarting);
    OnThreadStart();
	_eToStatus = dptStarted;
	
	if (_eToMode == dptCycle) {
        Unlock();
        ProcessThreadCycle();
		Lock();
    }
    else {
		_eToStatus = dptRunning;
		if (_eToControl == dptControlRun) {
            Unlock();
            OnThreadRun();
            Lock();
        }
		_eToStatus = dptClosing;
    }
    
    OnThreadStop();
	_eToStatus = dptStopped;
    Unlock();
}



//------------------------------------------------------------------------------
// CJW: This function will handle threads that are set to cycle.   If we are in
//      cycle mode, then we will run the call-back function each cycle.  It will
//      take a note of the current clock ticks, and then run the function.  When
//      returned, it will make a note of the clock ticks one more time, and
//      compare the two.  It would then sleep enough time to make the next
//      cycle.  If the function took longer than the cycle, then it will run
//      again immediately.
void DpThreadObject::ProcessThreadCycle(void)
{
    #ifndef __GNUC__
    struct timeval start, finish;
	struct timezone tz;
	DWORD used;
    #endif
    DWORD delay;

    Lock();
	while(_eToControl == dptControlRun) {
		ASSERT(_eToMode == dptCycle);
		
		_eToStatus = dptRunning;
        Unlock();
    
        #ifndef __GNUC__
		gettimeofday(&start, &tz);
        #endif
        OnThreadRun();
    
        Lock();
		_eToStatus = dptWaiting;
        delay = _dToCycleTime;
        Unlock();
    
        #ifndef __GNUC__
		gettimeofday(&finish, &tz);
		used = (finish.tv_sec * 1000) + (finish.tv_usec / 1000);
		used -= (start.tv_sec * 1000) + (start.tv_usec / 1000);
        if (used < delay) { delay -= used; }
        else 			  { delay = 0;     }
        #endif
    
        DpThreadBase::Sleep(delay);
        Lock();
    }
	
	_eToStatus = dptClosing;
    Unlock();
}


//------------------------------------------------------------------------------
// CJW: When a thread is started, it will create a new stack for it.  This
//      specifies the stack size that should be declared for it.  If you are
//      able to determine what the stack size would likely be required while the
//      thread is running, then specify a number that will cover it.  Otherwise,
//      by default (or if passed in a 0), it will use the same size as the
//      calling thread.   In windows this is likely to be 1mb.  Which means that
//      if your application starts up 200 threads that will continue running,
//      you would have 200mb of stack size taken up.  This is not too much of a
//      problem with general threading, especially since most of that 1mb would
//      be in virtual ram and would be rather efficient, if you are writing an
//      application that should have a small footprint, you can tune your
//      threads memory usage.
//
//      I havent seen any mention of stack size for threads in linux, so this
//      function does nothing if compiled with GCC.
void DpThreadObject::SetStackSize(unsigned nStackSize)
{
	ASSERT(nStackSize > 0);
	
    #ifndef __GNUC__
        Lock();
        _nToStackSize = nStackSize;
        Unlock();
    #endif
}


//------------------------------------------------------------------------------
// CJW: Virtual function that should be over-ridden in the derived class.  This
//      will run INSIDE the thread when it has first been created.  Use this
//      function to initialise data that should be initialised inside the
//      thread.
void DpThreadObject::OnThreadStart(void)
{
}


//------------------------------------------------------------------------------
// CJW: Virtual function that should be over-ridden in a derived class.  This
//      will run before the object is deleted, and the thread should not
//      actually be processing at this point, and we are locked with exclusive
//      access.
void DpThreadObject::OnThreadStop(void)
{
}


//------------------------------------------------------------------------------
// CJW:
void DpThreadObject::OnThreadRun(void)
{
    Lock();
    ASSERT(_pToFunction != NULL);
    if (_pToFunction != NULL) {
        (*_pToFunction)(_pToParam);
    }
    Unlock();
}


//------------------------------------------------------------------------------
// CJW: We need to indicate that the thread should stop.  We do not wait for it 
// 		to stop... for that we use WaitForThread();
void DpThreadObject::Stop(void)
{
    Lock();
// 	ASSERT((_eToControl == ControlRun && _eToStatus != Done && _eToStatus != Stopped) 
// 			|| (_eToControl == ControlStop && (_eToStatus == Done || _eToStatus == Stopped)));
	if (_eToControl == dptControlRun) {
		_eToControl = dptControlStop;
	}
    Unlock();
}



#endif

// #############################################################################
// #############################################################################
// #############################################################################













// #############################################################################
// #############################################################################
// #############################################################################

#ifndef _EXCLUDE_DB

// ##########################################################################

#ifndef _EXCLUDE_DB_SQLITE3

#include <unistd.h>
#include <string.h>

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

#endif


// ##########################################################################

#ifndef _EXCLUDE_DB_MYSQL


#include <stdarg.h>


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
    ASSERT(len > 0);
    q = (char *) malloc((length * 2) + 1);
    ASSERT(q);

    // The escape function should be ok to run in more than one thread at a time.  The mysql library is supposed to be thread safe in general, so it should be ok.
    Lock();
    ASSERT(_hSql != NULL);
    n = mysql_real_escape_string(_hSql, q, str, len);
    Unlock();
    
    ASSERT(n >= len);
    q[n] = '\0';
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
        _pResult = mysql_store_result(hSql);
        if (_pResult != NULL) {
            _nFields = mysql_nu_fields(_pResult);
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


#endif

#endif

#ifdef _INCLUDE_DB_OLD
//-----------------------------------------------------------------------------
// CJW: Constructor...
CSqlDB::CSqlDB()
{
    _hEnv = 0;
    _hConnection = 0;
    _pResultList = NULL;
    _nItems = 0;
    _bConnected = false;
}




//-----------------------------------------------------------------------------
// CJW: Deconstructor.    Clean up everything.
CSqlDB::~CSqlDB()
{

    while (_nItems > 0) {
        if (_pResultList[_nItems-1] != NULL) {
            delete _pResultList[_nItems-1];
        }
    }

    if (_pResultList) {
        free (_pResultList);
        _pResultList = NULL;
    }

    if (_hConnection != 0) {
        if (_bConnected) {
            SQLDisconnect(_hConnection);
        }
        SQLFreeHandle(SQL_HANDLE_DBC, _hConnection);
        _hConnection = 0;
    }

    if (_hEnv != 0) {
        SQLFreeHandle(SQL_HANDLE_ENV, _hEnv);
        _hEnv = 0;
    }
}



//-----------------------------------------------------------------------------
// CJW: Connect to the indicated data source.  Return true if we connected,
//      return false if we didnt.
bool CSqlDB::Connect(char *source, char *user, char *pass)
{
    SQLRETURN nReturn;

    // CJW: if the handles are already connected, then the programmer didnt
    //      close the last one.  In reality a new object should be created
    //      for a new connection.
    ASSERT(_hEnv == 0);
    ASSERT(_hConnection == 0);
    ASSERT(_bConnected == false);

    ASSERT(source != NULL);
    ASSERT(user != NULL);
    ASSERT(pass != NULL);


    // In order to connect to the an ODBC data source, we need to first create some handles.

    nReturn = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &_hEnv);
    if (nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO) {
        // we have a valid environment...

        nReturn = SQLSetEnvAttr(_hEnv, SQL_ATTR_ODBC_VERSION, (void*)SQL_OV_ODBC3, 0);
        if (nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO) {

            nReturn = SQLAllocHandle(SQL_HANDLE_DBC, _hEnv, &_hConnection);
            if (nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO) {
                // we have a valid connection handle...
    
                nReturn = SQLConnect(_hConnection, (unsigned char *) source, strlen(source), (unsigned char *) user, strlen(user), (unsigned char *) pass, strlen(pass));
                if (nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO) {
                    _bConnected = true;
                }
                else {
                    // CJW: Failed while trying connect to the database.
                    SQLFreeHandle(SQL_HANDLE_DBC, _hConnection);
                    _hConnection = 0;

                    SQLFreeHandle(SQL_HANDLE_ENV, _hEnv);
                    _hEnv = 0;
                }
    
            }
            else {
                // CJW: Failed while trying create the connection environment..
                SQLFreeHandle(SQL_HANDLE_ENV, _hEnv);
                _hEnv = 0;
            }
        }
        else {
            // CJW: Failed while trying to set the environment.
            SQLFreeHandle(SQL_HANDLE_ENV, _hEnv);
            _hEnv = 0;
        }
    }

    return(_bConnected);
}


//-----------------------------------------------------------------------------
// CJW: Close the connection.
void CSqlDB::Close()
{
    ASSERT(_hConnection != 0);
    SQLDisconnect(_hConnection);
    SQLFreeHandle(SQL_HANDLE_DBC, _hConnection);
    _hConnection = 0;
}



//-----------------------------------------------------------------------------
// CJW: Sql result set constructor.
CSqlResult::CSqlResult(void *pParent)
{
    CSqlDB *pDB;

    ASSERT(pParent != NULL);

    _pParent = pParent;
    _nColumns = 0;
    _pColumnData = NULL;
    _nCurrentRow = 0;

    _lastState = NULL;
    _lastErrorText = NULL;
    
    // CJW: now create the statement handle.
    pDB = (CSqlDB *) pParent;
    _lastReturn = SQLAllocHandle(SQL_HANDLE_STMT, pDB->GetHandle(), &_hStatement);

    // CJW: This should succeed... if not.. something is configured wrong.
    ASSERT(_lastReturn == SQL_SUCCESS || _lastReturn == SQL_SUCCESS_WITH_INFO);
}


//-----------------------------------------------------------------------------
// CJW: Sql result set destructor.
CSqlResult::~CSqlResult()
{
    CSqlDB *pTmp;
    int nCount;

    if (_hStatement != 0) {
        SQLFreeHandle(SQL_HANDLE_STMT, _hStatement);
        _hStatement = 0;
    }

    if (_pParent != NULL) {
        pTmp = (CSqlDB *) _pParent;
        pTmp->DeleteResult(this);
    }

    if (_pColumnData != NULL) {
        ASSERT(_nColumns > 0);
        for (nCount = 0; nCount < _nColumns; nCount++) {
            if (_pColumnData[nCount].ColumnName != NULL) {
                free(_pColumnData[nCount].ColumnName);
                _pColumnData[nCount].ColumnName = NULL;
            }
        }
        free(_pColumnData);
        _pColumnData = NULL;
    }

    if (_lastState) {
        free(_lastState);
        _lastState = NULL;
    }

    if (_lastErrorText) {
        free(_lastErrorText);
        _lastErrorText = NULL;
    }
}



//-----------------------------------------------------------------------------
// CJW: Execute the Query.  If everything is good, return a CSqlResult object
//      pointer.  The object would actually be created in our results list.
CSqlResult * CSqlDB::Execute(char *query, ...)
{
    CSqlResult *pResult = NULL;
    int err;
    va_list ap;
    char *buffer;

    ASSERT(query != NULL);

    buffer = (char *) malloc(10000);
    ASSERT(buffer != NULL);
    if (buffer != NULL) {
        va_start(ap, query);
        err = vsprintf(buffer, query, ap);
        va_end(ap);

        buffer = (char *) realloc(buffer, strlen(buffer)+1);
        ASSERT(buffer != NULL);
        if (buffer != NULL) {
            if (err > 0) {
                pResult = ExecuteStr(buffer);
            }

            free(buffer);
        }
    }

    return(pResult);
}


//-----------------------------------------------------------------------------
CSqlResult * CSqlDB::ExecuteStr(char *query)
{
    CSqlResult *pResult = NULL;

    SQLRETURN vReturn;
    SQLCHAR     Sqlstate[16];
    SQLINTEGER  NativeError;
    SQLCHAR     MessageText[1024];
    SQLSMALLINT TextLength;

    ASSERT(_hConnection != 0);
    ASSERT(query != NULL);

#ifdef _DEBUG
    int nLoopCount=0;
    nLoopCount = strlen(query) - 1;
    ASSERT(nLoopCount > 0);
    while(nLoopCount > 0) {
        if( (query[nLoopCount] < 32) || (query[nLoopCount] >= 127)) {
            // if this assertion fires, then we are trying to insert data that is not readible.
            ASSERT(0);
        }
        nLoopCount--;
    }
#endif

    pResult = NewResult();
    if (pResult != NULL) {
        vReturn = SQLExecDirect(pResult->GetHandle(), (unsigned char *) query, strlen(query));

        ASSERT(vReturn != SQL_INVALID_HANDLE);
        if (vReturn == SQL_INVALID_HANDLE) {
            // CJW: Invalid handle?  How did that happen?
            _bConnected = false;
            _hConnection = 0;
        }
        else if (vReturn == SQL_NO_DATA) {
            pResult->Process(vReturn);
        }
        else if (vReturn != SQL_SUCCESS) {
            // CJW: We've got an error, now we need to see what the code was.  If it
            //      was indicating that the connection was lost, then we need to close
            //      the connection, and clear the handles.

            SQLGetDiagRec(SQL_HANDLE_STMT, pResult->GetHandle(), 1, Sqlstate, &NativeError, MessageText, 1024, &TextLength);
            if (strcmp((const char *) Sqlstate, "08S01") == 0) {
                _bConnected = false;
                delete pResult;
                pResult = NULL;
            }
#ifdef _DEBUG
//          else if (strcmp((const char *) Sqlstate, "HY000") == 0) {
//              // We are waiting for an original query to complete that competes with this query...
//              ASSERT(0);
//          }
#endif
            else {
                pResult->Process(vReturn, (char *) Sqlstate, (char *) MessageText);
            }
        }
        else {
            pResult->Process(vReturn);
        }
    }

    return(pResult);
}


//-----------------------------------------------------------------------------
// CJW: Create a new 'result' object and add it to our results list so that it
//      can be managed and removed when the connection is closed.
CSqlResult * CSqlDB::NewResult()
{
    CSqlResult *pResult;

    // CJW: If you have more than 20 results open, then maybe you are note
    //      deleting the results returned from EVERY Execute().
    ASSERT(_nItems < 20);


    // make sure that we are sync'd
#ifdef _DEBUG
    if (_nItems == 0) {
        ASSERT(_pResultList == NULL);
    }
    else {
        ASSERT(_pResultList != NULL);
    }
#endif

    pResult = new CSqlResult(this);
    if (pResult != NULL) {
        _pResultList = (CSqlResult **) realloc(_pResultList, sizeof(CSqlResult *) * (_nItems+1));
        if (_pResultList) {
            _pResultList[_nItems] = pResult;
            _nItems++;
        }
        else {
            _nItems = 0;
        }
    }

    return(pResult);
}
MYSQL_RES

//-----------------------------------------------------------------------------
// CJW: Set the return code of our last operation so that the upper-level logic
//      can determine the error that was produced if things didnt go so well.
void CSqlResult::Process(SQLRETURN ret, char *state, char *text)
{
    SQLSMALLINT nCount;
    SQLRETURN nReturn;
    SQLSMALLINT nLen;

    ASSERT(_hStatement != 0);
    ASSERT(_nColumns == 0);
    ASSERT(_pColumnData == NULL);

    _lastReturn = ret;

    if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {

        // Get the column names.
        SQLNumResultCols(_hStatement, &_nColumns);
        if (_nColumns > 0) {
            _pColumnData = (_SQL_ColumnData *) malloc(sizeof(_SQL_ColumnData) * _nColumns);

            for(nCount = 0; nCount < _nColumns; nCount++) {
                _pColumnData[nCount].ColumnNumber = (SQLSMALLINT) nCount+1;
                _pColumnData[nCount].ColumnName = (unsigned char *) malloc(256);

                ASSERT(_pColumnData[nCount].ColumnName != NULL);
                if (_pColumnData[nCount].ColumnName != NULL) {
                    nReturn = SQLDescribeCol(
                        _hStatement,
                        _pColumnData[nCount].ColumnNumber,
                        _pColumnData[nCount].ColumnName, 256, &nLen,
                        &(_pColumnData[nCount].DataType),
                        &(_pColumnData[nCount].ColumnSize),
                        &(_pColumnData[nCount].DecimalDigits),
                        &(_pColumnData[nCount].Nullable));

                    // display a debug assertion if this failed.
                    ASSERT(nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO);
                    if (nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO) {
                        _pColumnData[nCount].ColumnName = (unsigned char *) realloc(_pColumnData[nCount].ColumnName, nLen+1);
                    }
                    else {
                        free(_pColumnData[nCount].ColumnName);
                        nCount = _nColumns;
                        _nColumns = 0;
                    }
                }
            }
        }
    }
    else {
        if (state != NULL && text != NULL) {
    
            ASSERT(_lastState == NULL);
            ASSERT(_lastErrorText == NULL);

            _lastState = (char *) malloc(strlen(state) + 1);
            _lastErrorText = (char *) malloc(strlen(text) + 1);

            ASSERT(_lastState != NULL);
            ASSERT(_lastErrorText != NULL);

            strcpy(_lastState, state);
            strcpy(_lastErrorText, text);
        }
    }
}



//-----------------------------------------------------------------------------
// CJW: Return the number of items that were returned from a Query.
unsigned int CSqlResult::Items()
{
    SQLINTEGER nItems;

    ASSERT(_hStatement != 0);

    _lastReturn = SQLRowCount(_hStatement, &nItems);

    return(nItems);
}




//-----------------------------------------------------------------------------
// CJW: Returns true if there is a row of data available.
bool CSqlResult::NextRow()
{
    bool bReturn = false;

    ASSERT(_hStatement != 0);
    if (_hStatement != 0) {
        ASSERT(_lastReturn == SQL_SUCCESS || _lastReturn == SQL_SUCCESS_WITH_INFO);

        _lastReturn = SQLFetch(_hStatement);
        if (_lastReturn == SQL_SUCCESS || _lastReturn == SQL_SUCCESS_WITH_INFO) {
            _nCurrentRow ++;
            ASSERT(_nCurrentRow > 0);
            bReturn = true;
        }
    }

    return(bReturn);
}



//-----------------------------------------------------------------------------
// CJW: Get data from the current row and create some memory for this item,
//      storing the pointer in value.  if this operation fails, do not modify
//      value.
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
    SQLCHAR     Sqlstate[16];
    SQLINTEGER  NativeError;
    SQLCHAR     MessageText[1024];
    SQLSMALLINT TextLength;

#endif

    ASSERT(field != NULL);
    ASSERT(value != NULL);
    
    // Initialise our return result.
    *value = NULL;

    ASSERT(_hStatement != 0);
    ASSERT(_nColumns > 0);
    ASSERT(_pColumnData != NULL);
    ASSERT(_nCurrentRow > 0);

    ASSERT(_lastReturn == SQL_SUCCESS || _lastReturn == SQL_SUCCESS_WITH_INFO);


    // CJW: we have the name of a column... we need to go thru the columns list
    //      to find the one we want.
    
    for (nCount = 0; nCount < _nColumns; nCount++) {
        if (stricmp(field, (const char *) _pColumnData[nCount].ColumnName) == 0) {
            // we found the match...

            if (_pColumnData[nCount].DataType == SQL_TYPE_TIMESTAMP) {
                // This is a timestamp.

                ts = (SQL_TIMESTAMP_STRUCT *) malloc(sizeof(SQL_TIMESTAMP_STRUCT));
                ASSERT(ts != NULL);
                nSqlReturn = SQLGetData(
                    _hStatement,                       // SQLHSTMT     StatementHandle
                    _pColumnData[nCount].ColumnNumber, // SQLUSMALLINT ColumnNumber
                    SQL_TYPE_TIMESTAMP,                 // SQLSMALLINT  TargetType
                    ts,                                 // SQLPOINTER   TargetValuePtr
                    sizeof(SQL_TIMESTAMP_STRUCT),       // SQLINTEGER   BufferLength
                    &nLen);                             // SQLINTEGER   *StrLen_or_IndPtr

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
                    SQLGetDiagRec(SQL_HANDLE_STMT, _hStatement, 1, Sqlstate, &NativeError, MessageText, 1024, &TextLength);
                    // Code of 07009 indicates that the data was retreived out of column sequence.
                }
#endif

                free(ts);

            }
            else {
                // this is not a timestamp.

                nStrLen = sizeof(char) * (_pColumnData[nCount].ColumnSize + 1) * 2;
                ASSERT(nStrLen > 0);
                tmp = (char *) malloc(nStrLen);
                if (tmp) {

                    nSqlReturn = SQLGetData(
                        _hStatement,                       // SQLHSTMT     StatementHandle
                        _pColumnData[nCount].ColumnNumber, // SQLUSMALLINT ColumnNumber
                        SQL_CHAR,                           // SQLSMALLINT  TargetType
                        tmp,                                // SQLPOINTER   TargetValuePtr
                        nStrLen,                            // SQLINTEGER   BufferLength
                        &nLen);                             // SQLINTEGER   *StrLen_or_IndPtr

                    ASSERT(nSqlReturn == SQL_SUCCESS || nSqlReturn == SQL_SUCCESS_WITH_INFO);

                    if (nSqlReturn == SQL_SUCCESS || nSqlReturn == SQL_SUCCESS_WITH_INFO) {
                        bReturn = true;
                        tmp = (char *) realloc(tmp, nLen + 1);
                        value[0] = tmp;
                    }
                    else {
#ifdef _DEBUG
                        // get some more information about why this failed.
                        SQLGetDiagRec(SQL_HANDLE_STMT, _hStatement, 1, Sqlstate, &NativeError, MessageText, 1024, &TextLength);
                        // Code of 07009 indicates that the data was retreived out of column sequence.
#endif
                        free(tmp);
                    }
                }
            }

            nCount = _nColumns;        // break out of the loop.
        }
    }

    return(bReturn);
}



//-----------------------------------------------------------------------------
// CJW: Get data from the current row and create some memory for this item,
//      storing the pointer in value.  if this operation fails, do not modify
//      value.
bool CSqlResult::GetData(const char *field, int *value)
{
    bool bReturn = FALSE;
    int nCount;
    SQLRETURN nSqlReturn;
    SQLINTEGER nLen;
    int tmp;

    ASSERT(field != NULL);
    ASSERT(value != NULL);

    ASSERT(_hStatement != 0);
    ASSERT(_nColumns > 0);
    ASSERT(_pColumnData != NULL);
    ASSERT(_nCurrentRow > 0);

    ASSERT(_lastReturn == SQL_SUCCESS || _lastReturn == SQL_SUCCESS_WITH_INFO);

    // initialise the return result.
    *value = 0;

    // CJW: we have the name of a column... we need to go thru the columns list
    //      to find the one we want.
    
    for (nCount = 0; nCount < _nColumns; nCount++) {
        if (stricmp(field, (const char *) _pColumnData[nCount].ColumnName) == 0) {
            // we found the match...

            nSqlReturn = SQLGetData(
                _hStatement,                       // SQLHSTMT     StatementHandle
                _pColumnData[nCount].ColumnNumber, // SQLUSMALLINT ColumnNumber
                SQL_INTEGER,                        // SQLSMALLINT  TargetType
                &tmp,                               // SQLPOINTER   TargetValuePtr
                sizeof(tmp),                        // SQLINTEGER   BufferLength
                &nLen);                             // SQLINTEGER   *StrLen_or_IndPtr

            // CJW: There should always be data.. if we get this, then we've done something wrong.
            ASSERT(nSqlReturn != SQL_NO_DATA);

            ASSERT(nSqlReturn == SQL_SUCCESS || nSqlReturn == SQL_SUCCESS_WITH_INFO);
            if (nSqlReturn == SQL_SUCCESS || nSqlReturn == SQL_SUCCESS_WITH_INFO) {
                bReturn = true;
                *value = tmp;
            }
            nCount = _nColumns;        // break out of the loop.
        }
    }

    return(bReturn);
}


//-----------------------------------------------------------------------------
// CJW: Return the status of the last operation.
SQLRETURN CSqlResult::Status()
{
    return(_lastReturn);
}


//-----------------------------------------------------------------------------
// CJW: Return true if we are currently connected to a database.
bool CSqlDB::Connected()
{
    return(_bConnected);
}



//-----------------------------------------------------------------------------
// CJW: Return the connection handle.  Should only be used when a new result
//      statement objec tis being created.
SQLHANDLE CSqlDB::GetHandle()
{
    return(_hConnection);
}


//-----------------------------------------------------------------------------
// CJW: When a result is being deleted, its reference also needs to be removed
//      from the results list that is kept in this database object (so it can
//      clean up at the end if necessary).
void CSqlDB::DeleteResult(CSqlResult *ptr)
{
    unsigned int nCount;
    unsigned int nIndex;

    ASSERT(_nItems > 0);
    ASSERT(_pResultList != NULL);

    nIndex = 0;
    for(nCount = 0; nCount < _nItems; nCount++)
    {
        if(_pResultList[nCount] != ptr) {
            if (nIndex < nCount) { _pResultList[nIndex] = _pResultList[nCount]; }
            nIndex++;
        }
    }
    _pResultList = (CSqlResult **) realloc(_pResultList, sizeof(CSqlResult *) * nIndex);
    _nItems = nIndex;
}


//-----------------------------------------------------------------------------
// CJW: Return the statement handle.
SQLHANDLE CSqlResult::GetHandle()
{
    return(_hStatement);
}


//-----------------------------------------------------------------------------
// CJW: Commit the changes to the database.
bool CSqlDB::Commit()
{
    bool done = false;
    SQLRETURN nReturn;

    nReturn = SQLEndTran(SQL_HANDLE_DBC, _hConnection, SQL_COMMIT);
    if (nReturn == SQL_SUCCESS || nReturn == SQL_SUCCESS_WITH_INFO) {
        done = true;
    }
    else {
        ASSERT(0);
    }

    return (done);
}


char * CSqlResult::LastState()
{
    return(_lastState);
}

char * CSqlResult::LastText()
{
    return(_lastErrorText);
}

#endif

// #############################################################################
// #############################################################################
// #############################################################################


























// #############################################################################
// #############################################################################
// #############################################################################

/******************************************************************************/
/******************************************************************************/
/**********                                                          **********/
/**********         DpSettings                                        **********/
/**********                                                          **********/
/******************************************************************************/
/******************************************************************************/

#ifndef _EXCLUDE_SETTINGS

int DpSettings::_nInstances = 0;
int DpSettings::_nItems = 0;
SettingsData *DpSettings::_pDataList = NULL;


//------------------------------------------------------------------------------
// CJW: Constructor.
DpSettings::DpSettings()
{
    Lock();
    _nInstances ++;
    Unlock();
}

//------------------------------------------------------------------------------
// CJW: Deconstructor.
DpSettings::~DpSettings()
{
    int nCount, i;
    
    Lock();
    
    ASSERT(_nInstances > 0);
    _nInstances --;
    if (_nInstances == 0) {
    
        // ** clear out all the settings.
        for(nCount = 0; nCount < _nItems; nCount++) {
            if (_pDataList[nCount].szName != NULL) {
                free(_pDataList[nCount].szName);
                _pDataList[nCount].szName = NULL;
            }
    
            if (_pDataList[nCount].eType == TYPE_STRING) {
                i = _pDataList[nCount].nItems;
                while(i > 0) {
                    i--;
    
                    if (_pDataList[nCount].pData[i].string != NULL) {
                        free(_pDataList[nCount].pData[i].string);
                        _pDataList[nCount].pData[i].string = NULL;
                    }
                }
            }
    
            if (_pDataList[nCount].pData != NULL) {
                free(_pDataList[nCount].pData);
            }
        }
    
        if (_pDataList != NULL) {
            free(_pDataList);
            _nItems = 0;
            _pDataList = NULL;
        }
    
    }
    Unlock();
}

//------------------------------------------------------------------------------
// CJW: Set a string value for the name.
bool DpSettings::Set(char *name, char *value, int index)
{
    bool bResult = false;
    int entry;
    
    ASSERT(name != NULL);
    ASSERT(index >= 0);
    
    Lock();
    
    // first we need to find the named setting in our list.  If it is not there,
    // an empty entry will be created for it.
    entry = FindName(name);
    
    if (entry < 0) {
        entry = CreateName(name);
    }
    
    if (entry >= 0) {
        // make sure there are enough entries in our list for this object.
        if (VerifyIndex(entry, index) == false) {
            CreateIndex(entry, index);
        }
    
        _pDataList[entry].eType = TYPE_STRING;
        _pDataList[entry].pData[index].string = (char *) malloc(strlen(value)+1);
        strcpy(_pDataList[entry].pData[index].string, value);
    
        bResult = true;
    }
    
    Unlock();
    
    return(bResult);
}


//------------------------------------------------------------------------------
// CJW: Set a string value for the name.
bool DpSettings::Set(char *name, int value, int index)
{
    bool bResult = false;
    int entry;
    
    ASSERT(name != NULL);
    ASSERT(index >= 0);
    
    Lock();
    
    // first we need to find the named setting in our list.  If it is not there,
    // an empty entry will be created for it.
    entry = FindName(name);
    
    if (entry < 0) {
        entry = CreateName(name);
    }
    
    if (entry >= 0) {
        // make sure there are enough entries in our list for this object.
        if (VerifyIndex(entry, index) == false) {
            CreateIndex(entry, index);
        }
    
        _pDataList[entry].eType = TYPE_INTEGER;
        _pDataList[entry].pData[index].integer = value;
    
        bResult = true;
    }
    
    Unlock();
    
    return(bResult);
}


bool DpSettings::Get(char *name, char *value,  int index)
{
    bool bResult = false;
    int entry;
    
    ASSERT(name != NULL);
    ASSERT(index >= 0);
    ASSERT(value != NULL);
    
    Lock();
    
    // first we need to find the named setting in our list.
    entry = FindName(name);
    if (entry >= 0) {
        // make sure there are enough entries in our list for this object.
        if (VerifyIndex(entry, index) != false) {
            if (_pDataList[entry].eType == TYPE_STRING) {
                strcpy(value, _pDataList[entry].pData[index].string);
                bResult = true;
            }
        }
    }
    
    Unlock();
    
    return(bResult);
}

bool DpSettings::Get(char *name, int *value,  int index)
{
    bool bResult = false;
    int entry;
    
    ASSERT(name != NULL);
    ASSERT(index >= 0);
    ASSERT(value != NULL);
    
    Lock();
    
    // first we need to find the named setting in our list.
    entry = FindName(name);
    if (entry >= 0) {
        // make sure there are enough entries in our list for this object.
        if (VerifyIndex(entry, index) != false) {
            if (_pDataList[entry].eType == TYPE_INTEGER) {
                *value = _pDataList[entry].pData[index].integer;
                bResult = true;
            }
            else if (_pDataList[entry].eType == TYPE_STRING) {
                *value = atoi(_pDataList[entry].pData[index].string);
                bResult = true;
            }
        }
    }
    
    Unlock();
    
    return(bResult);
}

//------------------------------------------------------------------------------
// CJW: We have a named setting, and we want to find the entry number for it so
//      that we can find it in our array.  Basically we will go thru each
//      element and check the name of each one.
int DpSettings::FindName(char *name)
{
    int nEntry=-1;
    int nCount = 0;
    
    while(nCount < _nItems) {
        if(strcmp(name, _pDataList[nCount].szName) == 0) {
            nEntry = nCount;
            nCount = _nItems;
        }
        nCount++;
    }
    
    return(nEntry);
}


//------------------------------------------------------------------------------
// CJW: The name evidently is not in our list yet, so we are going to create it.
int DpSettings::CreateName(char *name)
{
    int nEntry;
    
    nEntry = _nItems;
    _nItems++;
    _pDataList = (SettingsData *) realloc(_pDataList, sizeof(SettingsData) * _nItems);
    _pDataList[nEntry].szName = (char *) malloc(strlen(name) + 1);
    strcpy(_pDataList[nEntry].szName, name);
    _pDataList[nEntry].nItems = 0;
    
    return(nEntry);
}


//------------------------------------------------------------------------------
// CJW: We need to verify that there are enough indexes for this entry.  If
//      there are, then return true, otherwise return false.
bool DpSettings::VerifyIndex(int entry, int index)
{
    bool bValid = false;
    
    ASSERT(entry >= 0);
    ASSERT(index >= 0);
    
    ASSERT(_pDataList != NULL);
    ASSERT(entry < _nItems);
    
    if (_pDataList[entry].nItems > index) {
        bValid = true;
    }
    
    return(bValid);
}


//------------------------------------------------------------------------------
// CJW: Create the right number of blank index entries if we dont already have
//      enough.
void DpSettings::CreateIndex(int entry, int index)
{
    ASSERT(_pDataList != NULL);
    ASSERT(entry < _nItems);

    if (index >= _pDataList[entry].nItems) {
        _pDataList[entry].pData = (union SettingsUnion *) realloc(_pDataList[entry].pData, sizeof(union SettingsUnion) * (index + 1));
    
        while(_pDataList[entry].nItems <= index) {
            _pDataList[entry].pData[_pDataList[entry].nItems].pointer = NULL;
            _pDataList[entry].nItems++;
        }
    }
}


#endif

// #############################################################################
// #############################################################################
// #############################################################################




























// #############################################################################
// #############################################################################
// #############################################################################

#ifndef _EXCLUDE_LOGGER

//------------------------------------------------------------------------------
// CJW: Initialise the logger data that will be used locally for this object.
DpLogger::DpLogger()
{
}

//------------------------------------------------------------------------------
// CJW: Deconstructor.
DpLogger::~DpLogger()
{
}

    
//------------------------------------------------------------------------------
// CJW: Create a console log session.  This means that all logs to this mask
//      will go to the console.
DpLogger::CreateConsole(BYTE mask)
{
}



#endif




#ifndef _EXCLUDE_INI


//------------------------------------------------------------------------------
// CJW: Constructor.  The Ini file object will be used to open an ini file and get the values out of it.  All values in the file will be contained in a group.
DpIniFile::DpIniFile()
{
    _nGroups = 0;
    _pGroupList = NULL;
    _nCurrentGroup = 0;
    
}

//------------------------------------------------------------------------------
// CJW: Deconstructor.  Clear out any resources that we have been allocated.
DpIniFile::~DpIniFile()
{
    while(_nGroups > 0) {
        _nGroups--;
        ASSERT(_pGroupList != NULL);
        delete _pGroupList[_nGroups];
        _pGroupList[_nGroups] = NULL;
    }
    
    if (_pGroupList != NULL) {
        free(_pGroupList);
        _pGroupList = NULL;
    }
}


//------------------------------------------------------------------------------
// CJW: Load the file into an array.  We want to save the name of the file, and
//      we will read in all the stuff, ignoring comments, setting groups, etc.
bool DpIniFile::Load(char *path)
{
    bool ok = false;
    FILE *fp;
    char *buffer = NULL, ch;
    int length = 0;
    int len;
    int count;
    char szGroup[256], szName[256], szValue[5000];
    
    enum {
        unknown,
        newline,
        comment,
        group,
        name,
        value,
        eol,
        fail
    } state = unknown;
    
    ASSERT(path != NULL);
    fp = fopen(path, "r");
    if (fp != NULL) {
    
        // first load the entire contents of the file into a buffer.
        buffer = (char *) malloc(1024);
        len = fread(buffer, 1, 1024, fp);
        while(len > 0) {
            length += len;
            buffer = (char *) realloc(buffer, length+1024);
            len = fread(&buffer[length], 1, 1024, fp);
        }
        fclose(fp);
    
        // then go thru the buffer looking for groups.
        state = newline;
        len = 0;
        szGroup[0] = '\0';
        for(count=0; count<length; count++) {
            ch = buffer[count];
    
            if (ch != '\r') {
    
                switch(state) {
                    case newline:
                        if (ch == '[') {
                            state = group;
                        }
                        else if (ch == '#') {
                            state = comment;
                        }
                        else if (ch != ' ' && ch != '\n') {
                            state = name;
                            szName[0] = ch;
                            szName[1] = '\0';
                            len = 1;
                        }
                        break;
    
                    case group:
                        if (ch == ']') {
                            // when a group is found, we need to then create a group object
                            _pGroupList = (DpIniFileGroup **) realloc(_pGroupList, sizeof(DpIniFileGroup *) * (_nGroups + 1));
                            _pGroupList[_nGroups] = new DpIniFileGroup;
                            _pGroupList[_nGroups]->SetGroup(szGroup);
                            _nGroups++;
                            state = comment;
    
                            szGroup[0] = '\0';
                            len = 0;
                        }
                        else if (ch == '#' || ch == '\n') {
                            state = fail;
                            count = length;
                        }
                        else {
                            ASSERT(len < 256);
                            szGroup[len+1] = 0;
                            szGroup[len] = ch;
                            len++;
                        }
    
                        break;
    
                    case name:
                        if (ch == '=') {
                            state = value;
                            szValue[0] = '\0';
                            len = 0;
                        }
                        else if (ch == '#' || ch == '\n') {
                            state = fail;
                            count = length;
                        }
                        else {
                            if (_nGroups <= 0) {
                                state = fail;
                                count = length;
                            }
                            else {
                                ASSERT(len < 256);
                                szName[len+1] = 0;
                                szName[len] = ch;
                                len++;
                            }
                        }
                        break;
    
                    case value:
                        if (ch == '\n' || ch == '#') {
                            if (ch == '#') {
                                state = comment;
                            }
                            else {
                                state = newline;
                            }
    
                            ASSERT(_nGroups > 0);
                            _pGroupList[_nGroups-1]->Add(szName, szValue);
    
                            szGroup[0] = '\0';
                            len = 0;
                        }
                        else {
                            ASSERT(len < 5000);
                            szValue[len+1] = 0;
                            szValue[len] = ch;
                            len++;
                        }
                        break;

                    case comment:
                        if (ch == '\n') {
                            state = newline;
                        }
                        break;
    
                    default:
                        break;
                }
            }
        }
    
        if (state != fail) {
            ok = true;
        }
    
        if (buffer != NULL) {
            free(buffer);
        }
    }
    
    return(ok);
}


//------------------------------------------------------------------------------
// CJW: Go thru the list of groups and find the one that this matches and
//      record the index.  If it is found, then return true, otherwise return
//      false.  if grp is NULL, then we will set the current group to the first
//      one in the file.
bool DpIniFile::SetGroup(char *grp)
{
    bool ok = false;
    int nCount;
    char *tmp;
    
    ASSERT(_nGroups > 0);
    ASSERT(_pGroupList != NULL);
    
    if (grp == NULL) {
        _nCurrentGroup = 0;
        ok = true;
    }
    else {
        for(nCount = 0; nCount < _nGroups; nCount++) {
            ASSERT(_pGroupList[nCount] != NULL);
            tmp = _pGroupList[nCount]->GroupName();
            ASSERT(tmp != NULL);
            if (strcmp(grp, tmp) == 0) {
                _nCurrentGroup = nCount;
                ok = true;
                nCount = _nGroups;
            }
        }
    }
    
    return(ok);
}


//------------------------------------------------------------------------------
bool DpIniFile::GetValue(char *name, int *value)
{
    bool ok = false;
    char *buffer;
    
    ASSERT(name != NULL);
    ASSERT(value != NULL);
    
    ok = GetValue(name, &buffer);
    if (ok == true) {
        if (buffer == NULL) {
            *value = 0;
        }
        else {
            *value = atoi(buffer);
            free(buffer);
        }
    }
    
    return(ok);
}

//------------------------------------------------------------------------------
bool DpIniFile::GetValue(char *name, char **value)
{
    bool ok = false;
    char *tmp;
    
    ASSERT(name != NULL);
    ASSERT(value != NULL);
    
    ASSERT(_nGroups > 0);
    ASSERT(_pGroupList != NULL);
    
    ASSERT(_nCurrentGroup < _nGroups);
    ASSERT(_nCurrentGroup >= 0);
    
    tmp = _pGroupList[_nCurrentGroup]->GetValue(name);
    if (tmp != NULL) {
        *value = (char *) malloc(strlen(tmp)+1);
        strcpy(*value, tmp);
        ok = true;
    }
    
    return(ok);
}



DpIniFileGroup::DpIniFileGroup()
{
    _szGroup = NULL;
    _nItems = 0;
    _pItemList = NULL;
}

DpIniFileGroup::~DpIniFileGroup()
{
    if (_szGroup != NULL) {
        free(_szGroup);
    }
    
    while(_nItems > 0) {
        ASSERT(_pItemList != NULL);
        _nItems--;
        if (_pItemList[_nItems].szName != NULL) {
            free(_pItemList[_nItems].szName);
        }
        if (_pItemList[_nItems].szValue != NULL) {
            free(_pItemList[_nItems].szValue);
        }
    }
    
    if (_pItemList != NULL) {
        free(_pItemList);
    }
}

void DpIniFileGroup::SetGroup(char *grp)
{
    ASSERT(grp != NULL);
    ASSERT(_szGroup == NULL);
    
    _szGroup = (char *) malloc(strlen(grp)+1);
    strcpy(_szGroup, grp);
}

void DpIniFileGroup::Add(char *name, char *value)
{
    ASSERT(name != NULL);
    ASSERT(value != NULL);
    
    if (_nItems == 0) {
        _pItemList = (struct DpIniFileItemData *) malloc(sizeof(struct DpIniFileItemData));
    }
    else {
        _pItemList = (struct DpIniFileItemData *) realloc(_pItemList, sizeof(struct DpIniFileItemData) * (_nItems+1));
    }
    
    if (_pItemList == NULL) {
        _nItems = 0;
    }
    else {
        _pItemList[_nItems].szName = (char *) malloc(strlen(name) + 1);
        strcpy(_pItemList[_nItems].szName, name);
    
        _pItemList[_nItems].szValue = (char *) malloc(strlen(value) + 1);
        strcpy(_pItemList[_nItems].szValue, value);
    
        _nItems ++;
    }
}

char *DpIniFileGroup::GroupName(void)
{
    ASSERT(_szGroup != NULL);
    return(_szGroup);
}

char *DpIniFileGroup::GetValue(char *name)
{
    char *value=NULL;
    char *tmp;
    int nCount;
    
    ASSERT(name != NULL);
    
    ASSERT(_nItems > 0);
    ASSERT(_pItemList != NULL);
    
    for(nCount=0; nCount<_nItems; nCount++) {
        tmp = _pItemList[nCount].szName;
        ASSERT(tmp != NULL);
        if (strcmp(name, tmp) == 0) {
            value = _pItemList[nCount].szValue;
            nCount = _nItems;
            ASSERT(value != NULL);
        }
    }
    
    return(value);
}




#endif




// #############################################################################
// #############################################################################
// #############################################################################
#ifndef _EXCLUDE_HTTPCLIENT

//------------------------------------------------------------------------------
// CJW: Constructor for the HttpClient class.
DpHttpClient::DpHttpClient()
{
    _szUrl = NULL;
    _szHeaders = NULL;
    _szHost[0] = '\0';
    _nPort = 80;
    _nCode = 0;
    _nLength = 0;
	_pBody = NULL;
}


//------------------------------------------------------------------------------
// CJW: Deconstructor.
DpHttpClient::~DpHttpClient()
{
    if (_szUrl != NULL) 	{ free(_szUrl); }
    if (_szHeaders != NULL)	{ free(_szHeaders); }
	if (_pBody != NULL)		{ free(_pBody); }
}


//------------------------------------------------------------------------------
// CJW: Set the URL that will be queried.
void DpHttpClient::SetURL(char *url)
{
	ASSERT(url != NULL);
	ASSERT(_szUrl == NULL);
	
	_szUrl = (char *) malloc(sizeof(char) * (strlen(url) + 1));
	strcpy(_szUrl, url);
}


//------------------------------------------------------------------------------
// CJW: Add a parameter to the URL that will be used.
void DpHttpClient::AddParam(char *name, char *value)
{
	bool bFirst = true;
	int i, len;
	char *encoded;
	
	ASSERT(name != NULL && value != NULL);
	ASSERT(_szUrl != NULL);
	
	len = strlen(_szUrl);
	for (i=0; i<len && bFirst == true; i++) {
		if (_szUrl[i] == '?') {
			bFirst = false;
		}
	}
	
	encoded = Encode(value);
	ASSERT(encoded != NULL);
	
	printf("Encoded value: original='%s', encoded='%s'\n", value, encoded);
	
	len += 1 + strlen(name) + 1 + strlen(encoded) + 1;
	_szUrl = (char *) realloc(_szUrl, len);
	if (bFirst == true) { strcat(_szUrl, "?"); }
	else 				{ strcat(_szUrl, "&"); }
	strcat(_szUrl, name);
	strcat(_szUrl, "=");
	strcat(_szUrl, encoded);
	
	free(encoded);
}

//------------------------------------------------------------------------------
// CJW: Add an integer value (convert to a string).
void DpHttpClient::AddParam(char *name, int value)
{
	char str[128];
	ASSERT(name != NULL);
	sprintf(str, "%d", value);
	AddParam(name, str);
}
		
		
//------------------------------------------------------------------------------
// CJW: Return the query string that has been generated.
char * DpHttpClient::GetURL(void)
{
	return(_szUrl);
}

//------------------------------------------------------------------------------
// CJW: This function is given a URL and it will connect to the HTTP server and
//      retrieve the contents of the page.
int DpHttpClient::Get(char *url)
{
    int state, tmp;
    char *ptr;

    ASSERT(_nCode == 0);
    ASSERT(_nLength == 0);

	if (url != NULL) {
		SetURL(url);
	}
	
    // make a copy of the url we are going to be using.
    ASSERT(_szUrl != NULL);

	// parse the URL to get the name and port.
	if (ParseUrl() != false) {
		
		// build our header data.
		BuildHeader();

		// connect to the name and port,
		ASSERT(_szHost != NULL);
		ASSERT(_nPort != 0);
		if (_xSocket.Connect(_szHost, _nPort) != false) {

			printf("Connected.\n");

			if (SendHeader() != false) {
				if (ReceiveHeader() == false) {
					printf("ReceiveHeader failed.\n");
				} 
				else {

					// parse the header to determine the result code.
					state = 0;  tmp = 0;
					ptr = _szHeaders;
					
					printf("Headers: %s\n", _szHeaders);
					
					while((_nCode == 0 || _nLength == 0) && *ptr != '\0') {
						if (*ptr != '\r') {

							switch(state) {
								case 0:
								case 4:
									if (*ptr == ' ') {
										state ++;
										tmp = 0;
									}
									break;

								case 1:
									if (*ptr >= '0' && *ptr <= '9') {
										tmp *= 10;
										tmp += (*ptr - '0');
									}
									else {
										_nCode = tmp;
										state ++;
									}
									break;

								case 2:
									if (*ptr == '\n') {
										state ++;
									}
									break;

								case 3:
									if (strncmp(ptr, "Content-Length:", strlen("Content-Length:")) == 0) {
										state++;
									}
									else {
										state--;
									}
									break;

								case 5:
									if (*ptr >= '0' && *ptr <= '9') {
										tmp *= 10;
										tmp += (*ptr - '0');
									}
									else {
										_nLength = tmp;
										state ++;
									}

									break;

								default:
									break;
							}
						}

						ptr++;
					}
				}
			}
			
		}
	}

    return(_nCode);
}

char * DpHttpClient::Encode(char *str)
{
	char *encoded = NULL;
	char *hex = "0123456789ABCDEF";
	int i, j, len;
	bool bConvert;
	
	ASSERT(str != NULL);
	len = strlen(str);
	ASSERT(len > 0);
	
	encoded = (char *) malloc((len*3)+1);
	for (i=0,j=0; i<len; i++) {
		
		if (str[i] >= 'A' && str[i] <= 'Z') 		{ bConvert = false; }
		else if (str[i] >= 'a' && str[i] <= 'z')	{ bConvert = false; }
		else if (str[i] >= '0' && str[i] <= '9')	{ bConvert = false; }
		else if (str[i] == '.' || str[i] == '-') 	{ bConvert = false; }
		else { 
			bConvert = true; 
		}
		
		if (bConvert == true) {
			encoded[j] = '%';
			encoded[j+1] = hex[str[i] / 16];
			encoded[j+2] = hex[str[i] % 16];
			j += 3;
		}
		else {
			encoded[j] = str[i];
			j++;
        }
	}
	
	ASSERT(j > 0);
	encoded[j] = '\0';
	j++;
	encoded = (char *) realloc(encoded, j);
	
	return(encoded);
}


//------------------------------------------------------------------------------
// CJW: Assuming that we have been given a URL, we will attempt to parse it to
//      determine the host and port that we need to connect to in order to get
//      this URL.
//
//  TODO:   This procedure could be improved quite a bit.
bool DpHttpClient::ParseUrl(void)
{
    bool ok = false;
    int nCount, nOuter;
    int nStatus = 0;
    int len;

    // First, find any spaces and replace them.  Th
    ASSERT(_szUrl != NULL);
	printf("ParseUrl: szUrl='%s'\n", _szUrl);

    // now we need to get the host and port out of it.
	len = strlen(_szUrl);
    nCount = 0;
    for (nOuter = 0; nOuter < len; nOuter++) {
        if (_szUrl[nOuter] == '/') {
            nStatus++;
            if (nStatus >= 3) {
                _szHost[nCount] = '\0';
                nOuter = len;
            }
        }
        else if (_szUrl[nOuter] == ':') {
            if (nStatus == 1) {
                nStatus ++;
                _nPort = 0;
            }
        }
        else {
            if (nStatus == 2) {
                _szHost[nCount] = _szUrl[nOuter];
                nCount++;
            }
            else if (nStatus == 3) {
                if (_nPort > 0)        { _nPort *= 10; }
                _nPort += (_szUrl[nOuter] - '0');
            }
        }
    }

    if (nStatus >= 3) {
        ok = true;
    }

    return(ok);
}


//------------------------------------------------------------------------------
// CJW: When we connect to the HTTP server, we then need to send some header
//      information.  These headers are built with this function and will be
//      sent to the server when a connection is made.
void DpHttpClient::BuildHeader(void)
{
    char *mask = "GET %s HTTP/1.0\r\nAccept: */*\r\nAccept-Language: en-us\r\nAccept-Encoding: gzip, deflate\r\nUser-Agent: Mozilla/4.0 (compatible; www.cjdj.org/products/devplus;)\r\nHost: %s\r\nCookie:Inform=1\r\n\r\n";

    ASSERT(_szHeaders == NULL);

    _szHeaders = (char *) malloc(strlen(mask) + strlen(_szUrl) + strlen(_szHost) + 2);
    ASSERT(_szHeaders != NULL);
    if (_szHeaders != NULL) {
        sprintf(_szHeaders, mask, _szUrl, _szHost);
    }
}


//------------------------------------------------------------------------------
// CJW: We have built our headers, and connected to the server, so now we need
//      to send them.
bool DpHttpClient::SendHeader(void)
{
    bool ok = false;
    int nSent=0,nNext=0;
    int nLength;
    int nMax = 100;

    ASSERT(_szHeaders != NULL);

    nLength = strlen(_szHeaders);
    if (nLength > 0) {

        while(nNext < nLength) {
            nSent = _xSocket.Send(&_szHeaders[nNext], nLength-nNext);
            if (nSent < 0) {
                // the connection was closed.
                nSent = -1;
            }
            else if (nSent == 0) {
                Sleep();
                nMax --;
                if (nMax == 0) {
                    nSent = -1;
                }
            }
            else {
                nNext += nSent;
            }
        }
    }

    if (nSent > 0) {
        ASSERT(nSent == nLength);
        ok = true;
    }
    
    return(ok);
}


//------------------------------------------------------------------------------
// CJW: We will provide this Sleep function, even though it is also available in 
// 		ThreadBase.  This is because this class can often be included in 
// 		applications that are single threaded.
void DpHttpClient::Sleep(void) 
{
#ifndef _EXCLUDE_THREADBASE
    DpThreadBase::Sleep(50);        // sleep 50 miliseconds...
#else
    sleep(1);       // or sleep 1 second...
#endif

}

//------------------------------------------------------------------------------
// CJW: Receive the data from the webpage until we have enough of the header 
//      that we can parse everything.  We will not pull all the data off the
//      socket yet.  But we will pull data into our data queue so that it is
//      easy to get the headers out.   We will wipe out our pre-build headers,
//      and use that pointer to hold the actual header data.
bool DpHttpClient::ReceiveHeader(void)
{
    bool ok = false;
    int nResult=0;
    int length=0;
    int nMax = 100;
    char pBuffer[DP_MAX_PACKET_SIZE];
    int i, state=0;

    ASSERT(DP_MAX_PACKET_SIZE > 0);
	
    ASSERT(_szHeaders != NULL);
    free(_szHeaders);
    _szHeaders = NULL;
    
    while(nResult != -1) {

        nResult = _xSocket.Receive(pBuffer, DP_MAX_PACKET_SIZE);
        if (nResult < 0) {
            // the connection was closed.
            ASSERT(nResult == -1);
        }
        else if (nResult > 0) {
            for(i=0; i<nResult; i++) {
				
                if (pBuffer[i] != '\n') {

                	if (pBuffer[i] == '\r') {
// 						printf("pBuffer[%d] == %d '\r', state==%d\n", i, pBuffer[i], state);

						switch (state) {
							case 0:	
								state = 1;
								break;
								
							case 1:
								ASSERT(length >= 0);
								ASSERT(i >= 0);
								ASSERT((length+i+1) > 0);
								
								_szHeaders = (char *) realloc(_szHeaders, length+i+1);
								ASSERT(_szHeaders);
								memcpy(&_szHeaders[length], pBuffer, i);
								_szHeaders[length+i] = '\0';
								
								state = 2;
								
								while (i < nResult && (pBuffer[i] == '\r' || pBuffer[i] == '\n' )) {
									i++;
								}
								
								if (i < nResult) {
									_xQueue.Add(&pBuffer[i], nResult - i);
									i = nResult;
								}
								nResult = -1;
								ok = true;
								break;
						}
					}
					else if (state > 0) {
// 						printf("pBuffer[%d] == %d '%c', state==%d\n", i, pBuffer[i], pBuffer[i], state);
						state = 0;
					}
					else {
// 						printf("pBuffer[%d] == %d '%c', state==%d\n", i, pBuffer[i], pBuffer[i], state);
					}
				}
            }
    
            if (state < 2) {
                _szHeaders = (char *) realloc(_szHeaders, length+nResult+1);
                ASSERT(_szHeaders);
                memcpy(&_szHeaders[length], pBuffer, nResult);
                _szHeaders[length+nResult] = '\0';

                length += nResult;
            }
			nMax = 100;
        }
        else {
            // we should idle a little bit to let it catch up.
            Sleep();
            nMax --;
            if (nMax == 0) {
                nResult = -1;
            }
        }
    }

    return(ok);
}


//------------------------------------------------------------------------------
// CJW: The headers have been received, so now we return the body of the result.  
// 		We are returning a dataqueue, because that is the most easiest way to 
// 		combine a chunk of data, and the length of it.  So basically, this 
// 		function will use the length that we already determined from the 
// 		headers, and receive that much data from the socket.  if the socket 
// 		closes before all the data has been received, then we will return with 
// 		what we've got.  The calling function will need to check that the header 
// 		length and the body length matches.
DpDataQueue * DpHttpClient::GetBody()
{
	bool bContinue = true;
	char pBuffer[DP_MAX_PACKET_SIZE];
	int i;
	
	while(bContinue != false) {
		i = _xSocket.Receive(pBuffer, DP_MAX_PACKET_SIZE);
		
		if (i < 0) 		{ bContinue = false; }
		else if (i > 0) { _xQueue.Add(pBuffer, i); }
		else 			{ Sleep(); }
		
		if (_xQueue.Length() >= _nLength) {
			bContinue = false;
		}
	}
	
	return(&_xQueue);
}



#endif


// #############################################################################
// #############################################################################
// #############################################################################


#ifndef _EXCLUDE_HTTPSERVER

//------------------------------------------------------------------------------
// CJW: Constructor.  Initialise anything...
DpHttpServer::DpHttpServer() 
{
	_Lock.Lock();
	_pList = NULL;
	_nItems = 0;
	_Lock.Unlock();
}

//------------------------------------------------------------------------------
// CJW: Decontructor.  Clean up anything that was created by this object.  This 
//		includes the list of connection objects 
DpHttpServer::~DpHttpServer() 
{
	_Lock.Lock();
	if (_pList != NULL) {
		ASSERT(_nItems >= 0);
		while (_nItems > 0) {
			_nItems --;
			if (_pList[_nItems] != NULL) {
				delete _pList[_nItems];
				_pList[_nItems] = NULL;
			}
		}
		free(_pList);
		_pList = NULL;
	}
	_Lock.Unlock();
}

//------------------------------------------------------------------------------
// CJW: When a new connection arrives, we will get this function firing.  We 
// 		need to create a new socket object to handle it.  Then we will manage 
// 		it.
void DpHttpServer::OnAccept(SOCKET nSocket)
{
	DpHttpServerConnection *pNew;
	
	ASSERT(nSocket != 0);
// 	printf("New socket received.\n");
	
	pNew = new DpHttpServerConnection;
	pNew->Accept(nSocket);

	AddNode(pNew);
}

void DpHttpServer::AddNode(DpHttpServerConnection *pNew)
{
	bool bDone = false;
	int i;
	
	ASSERT(pNew != NULL);
	
	_Lock.Lock();
	
	ASSERT((_pList == NULL && _nItems == 0) || (_pList != NULL && _nItems >= 0));
	
	i=0;
	while(i<_nItems && bDone == false) {
		
		if (_pList[i] == NULL) {
			_pList[i] = pNew;
			bDone = true;
		}
		i++;
	}
	
	if (bDone == false) {
		_pList = (DpHttpServerConnection **) realloc(_pList, sizeof(DpHttpServerConnection*)*(_nItems+1));
		_pList[_nItems] = pNew;
		_nItems++;
	}
	
	_Lock.Unlock();
}

//------------------------------------------------------------------------------
// CJW: Since we have a bunch of child threads that need to be managed, we need 
// 		to check them all frequently to see if there was any activity that needs 
// 		to be handled.
void DpHttpServer::OnIdle(void)
{
	char *szUrl, *szHeaders, *szData;
	DpCgiFormData *pForm;
	DpDataQueue *pQueue;
	int code, i;
	
	_Lock.Lock();
	// go thru the list of connections.
	ASSERT((_pList == NULL && _nItems == 0) || (_pList != NULL && _nItems >= 0));
	
	// Only one connection will be processed at a time.  We do this because we dont want to open a bunch of database handles and such.
	i=0;
	while(i<_nItems) {
		if (_pList[i] != NULL) {
			switch(_pList[i]->GetState()) {
				case DP_HTTP_STATE_READY:
					szUrl     = _pList[i]->GetUrl();
					szHeaders = _pList[i]->GetHeaders();
					szData    = _pList[i]->GetData();
					pForm     = _pList[i]->GetForm();
					
					ASSERT(szHeaders != NULL);
					ASSERT(pForm != NULL);
					
					pQueue = new DpDataQueue;
					code = OnPage(szUrl, szHeaders, szData, pForm, pQueue);
					_pList[i]->Reply(code, pQueue);
					break;
					
				case DP_HTTP_STATE_DELETE:
					delete _pList[i];
					_pList[i] = NULL;
					break;
					
				default:
					break;
			}
		}
		i++;
	}
	
	_Lock.Unlock();
}

// #############################################################################

//------------------------------------------------------------------------------
// CJW: Constructor.
DpHttpServerConnection::DpHttpServerConnection()
{
	_Lock.Lock();
	_nState = DP_HTTP_STATE_NEW;
	_pSocket = NULL;
	_szUrl = NULL;
	_szHeaders = NULL;
	_szData = NULL;
	_pForm = NULL;
	_nCode = 0;
	_pQueue = NULL;
	_Lock.Unlock();
}

//------------------------------------------------------------------------------
// CJW: Deconstructor.
DpHttpServerConnection::~DpHttpServerConnection()
{
	_Lock.Lock();
	if (_pSocket != NULL) {
		delete _pSocket;
		_pSocket = NULL;
	}
	
	if (_pQueue != NULL) {
		delete _pQueue;
		_pQueue = NULL;
	}
	
	if (_szHeaders != NULL) {
		free(_szHeaders);
		_szHeaders = NULL;
	}
	
	if (_szData != NULL) {
		free(_szData);
		_szData = NULL;
	}
	
	if (_pForm != NULL) {
		delete _pForm;
		_pForm = NULL;
	}
	
	_Lock.Unlock();
}

//------------------------------------------------------------------------------
// CJW: Accept the socket and create a socket object to handle it.
void DpHttpServerConnection::Accept(SOCKET nSocket)
{
	ASSERT(nSocket > 0);
	
	_Lock.Lock();
	ASSERT(_pSocket == NULL);
	ASSERT(_pQueue == NULL);
	
	_pSocket = new DpSocket;
	_pSocket->Accept(nSocket);
	
	_pQueue = new DpDataQueue;
	
	SetCycleTime(25);
	Start();
	
	_Lock.Unlock();
}

//------------------------------------------------------------------------------
// CJW: This function is executed on a timed basis inside a seperate thread.  We 
// 		will want to check the status and then try and retrieve all the data 
// 		from the socket connection.  All the information will be put in the 
// 		appropriate variables so that they can be passed back to the main server 
// 		object.  If we are waiting for a reply from the server, then we will not 
// 		do anything.
void DpHttpServerConnection::OnThreadRun()
{
	bool bLoop = true;
	
	_Lock.Lock();
	
	while(bLoop == true) {
		bLoop = false;
		
		switch(_nState) {
			case DP_HTTP_STATE_NEW:
				if (ReceiveHeaders() == true)
					bLoop = true;
				break;
				
			case DP_HTTP_STATE_REPLY:
				if (SendReply() == true) 
					bLoop = true;
				break;
				
			case DP_HTTP_STATE_READY:
			case DP_HTTP_STATE_DELETE:
			default:
				break;
		}
	}
	
	_Lock.Unlock();
}


//------------------------------------------------------------------------------
// CJW: When the derived class is done parsing the url request, we will be given 
// 		a DataQueue object which has all the data that we need to return to the 
// 		connection.   This function will simply store the dataqueue pointer 
// 		(that we are now responsible for), so that the thread can pick it up and 
// 		(start actually sending.
void DpHttpServerConnection::Reply(int nCode, DpDataQueue *pQueue)
{
	char s[1024];
	
	ASSERT(nCode >= 0 && pQueue != NULL);
	_Lock.Lock();
	ASSERT(_pQueue == NULL && _nState == DP_HTTP_STATE_READY && _nCode == 0);
	ASSERT(_pForm != NULL);
	if (nCode > 0) {
		_nState = DP_HTTP_STATE_REPLY;
		_pQueue = pQueue;
		_nCode = nCode;
		
		sprintf(s, "HTTP/1.1 %d OK\n", _nCode);
		_pQueue->Push(s, strlen(s));
		
		delete _pForm;  _pForm = NULL;
	}
	else {
		_nState = DP_HTTP_STATE_DELETE;
		delete pQueue;
	}
	_Lock.Unlock();
}


//------------------------------------------------------------------------------
// CJW: When the connection is established, we must first receive the headers.  
// 		When we are finished receiving the headers, we will check the header 
// 		values to determine if we also need to retrieve some data.  If there is 
// 		data, a Content-length header will tell us how much is there.  We will 
// 		get that too.
bool DpHttpServerConnection::ReceiveHeaders(void)
{
	bool bOK = true;
	char buffer[2048];
	char *ptr;
	int len, i, j, k;
	bool bFound = false;
	
	ASSERT(_pSocket != NULL && _pQueue != NULL);
	ASSERT(_szData == NULL);
	ASSERT(_pForm == NULL);
	ASSERT(_szUrl == NULL);
	
	// get data from the socket, and put it in the data queue.  
	len = _pSocket->Receive(buffer, 2048);
	if (len < 0) {
		// the socket was closed.  We just clean things up and then delete.
		_nState = DP_HTTP_STATE_DELETE;
		delete _pQueue;
		_pQueue = NULL;
		bOK = false;
	}
	else if (len == 0) {
		// If there is no more data, then we will check the dataqueue to see if the double-cr is found. If so, we will copy that much to the _szHeaders ptr.  
		len = _pQueue->Length();
		if (len > 0) {
			ptr = _pQueue->Pop(len);
			
			// The headers will then need to be parsed to find the important information we need to know.  
			for (i=0; i<len; i++) {
				if (ptr[i] != '\r') {
					
					if (ptr[i] == '\n') {
						if (bFound == true) {
							// ok, we found the cr-pair.
							
							_szUrl = (char *) malloc(i);
							
							k = 0;
							if (strncmp(ptr, "GET", 3) == 0) {
								j = 5;
								while(ptr[j] != ' ' && ptr[j] != '?') {
									_szUrl[k] = ptr[j];
									j++;  k++;
								}
								_szUrl[k] = '\0';
								_szUrl = (char *) realloc(_szUrl, k+1);
								_szData = (char *) malloc((i-k)+1);
								_szData[0] = '\0';
								
								if (ptr[j] == '?') {
									j++;
									k=0;
									while(ptr[j] != ' ') {
										_szData[k] = ptr[j];
										j++;  k++;
									}
									_szData[k] = '\0';
									_szData = (char *) realloc(_szData, k+1);
								}
							}
							else if (strncmp(ptr, "POST", 4) == 0) {
								j = 6;
								while(ptr[j] != ' ') {
									_szUrl[k] = ptr[j];
									j++;  k++;
								}
								_szUrl[k] = '\0';
								_szUrl = (char *) realloc(_szUrl, k+1);
								_szData = (char *) malloc((len-i)+1);
								_szData[0] = '\0';
								
								j=i+1;
								k=0;
								while(j < len) {
									if (ptr[j] != '\n' && ptr[j] != '\r') {
										_szData[k] = ptr[j];
										k++;
									}
									j++;  
								}
								_szData[k] = '\0';
								_szData = (char *) realloc(_szData, k+1);
							}
							
							
							
							
							_pForm = new DpCgiFormData;
							ASSERT(_szData != NULL);
							_pForm->ProcessData(_szData);
							
							delete _pQueue;		_pQueue = NULL;
							_szHeaders = ptr;
							_nState = DP_HTTP_STATE_READY;
							bOK = false;
							i = len;
						}
						else {
							bFound = true;
						}
					}
					else {
						if (bFound == true) {
							bFound = false;
						}
					}
				}
			}
			
			// Then we will get the rest of the data and put it in the _szData field.		
			
			// Once all the data is received and parsed, we need to create the FormData object.
		
			// when we have everything done, we will need to change the state to indicate that we are ready.
		}
	}
	else {
		_pQueue->Add(buffer, len);
		if (len < 2048) {
			bOK = false;
		}
	}
	
	return(bOK);
}

//------------------------------------------------------------------------------
// CJW: We have been given a reply to send back to the connected client.  So now 
// 		we need to send it.  As long as we have data and we can send it, we 
// 		will.  If the socket is full, but we still have more data, we will 
// 		return a true.  In fact, we will continue to return a true until either 
// 		the socket closes, or we have no more data to send.
bool DpHttpServerConnection::SendReply(void)
{
	bool bOK = true;
	int length, sent;
	char *ptr;
	
	ASSERT(_pQueue != NULL && _pSocket != NULL && _pForm == NULL);
	
	length = _pQueue->Length();
	if (length <= 0) {
		// we have no more data to send.
		delete _pSocket; _pSocket = NULL;
		delete _pQueue;  _pQueue = NULL;
		
		_nState = DP_HTTP_STATE_DELETE;
		bOK = false;
	}
	else {
		if (length > 1024) { length = 1024; }
		ptr = _pQueue->Pop(length);
		
		sent = _pSocket->Send(ptr, length);
		if (sent < 0) {
			delete _pSocket; _pSocket = NULL;
			delete _pQueue;  _pQueue = NULL;
		
			_nState = DP_HTTP_STATE_DELETE;
			bOK = false;
		}
		else if (sent < length) {
			// we couldnt send all the data at this time, so put the data back in the queue.
			_pQueue->Push(&ptr[sent], length-sent);
			
			// Then we will wait a little bit 
			_Lock.Unlock();
			bOK = false;
			Sleep(500);
			_Lock.Lock();
		}
		
		
		free(ptr);
	}
	
	
	
	return (bOK);
}



#endif


// #############################################################################
// #############################################################################
// #############################################################################


#ifndef _EXCLUDE_TEXTTOOLS

//------------------------------------------------------------------------------
// CJW: Constructor for the DpTextTools class.
DpTextTools::DpTextTools()
{
    _pData = NULL;
    _nItems = 0;

    _nCurrentLine = 0;
    _nCurrentChar = 0;

    _pWordArray = NULL;
}

//------------------------------------------------------------------------------
// CJW: Deconstructor.
DpTextTools::~DpTextTools()
{
    ClearWordArray();

    if (_pData != NULL) {
        ASSERT(_nItems != 0);

        while(_nItems > 0) {
            _nItems--;
            if (_pData[_nItems].data != NULL) {
                free(_pData[_nItems].data);
            }
        }

        free(_pData);
    }
    ASSERT(_nItems == 0);
}

//------------------------------------------------------------------------------
// CJW: Load the text from a file.   This function will get the data, and then
//      pass it to a ParseLines() function which will seperate the text into
//      individual lines.
//
//  TODO:   If the file is really big, we should probably process it in chunks.
bool DpTextTools::LoadFromFile(char *file)
{
    bool ok = false;
    FILE *fp;
    char *buffer = NULL;    // first we read the data into the buffer, then we will
    long length = 0;        // the number of chars in our buffer.
    int result;

    ASSERT(file != NULL);
    ASSERT(_pData == NULL);
    ASSERT(_nItems == 0);

    fp = fopen(file, "r");
    if (fp != NULL) {

        buffer = (char *) malloc(1024);
        ASSERT(buffer != NULL);

        result = fread(buffer, 1, 1024, fp);
        while (result > 0) {
            length += result;

            buffer = (char *) realloc(buffer, length + 1024);
            ASSERT(buffer != NULL);
            result = fread(&buffer[length], 1, 1024, fp);
        }

        fclose(fp);

        // Now we have the entire contents of the file in our buffer, we need 
        // to go thru it and add each line to an array.
        ok = Load(buffer);

        if (buffer != NULL) {
            free(buffer);
        }
    }

    return(ok);
}

//------------------------------------------------------------------------------
// CJW: Load the text into our line array.
bool DpTextTools::Load(char *buffer)
{
    bool ok = false;
    char *str;
    long count;
    char ch;
    int len;
    int length;

    ASSERT(buffer != NULL);

    // start the first line.
    _pData = (DpTextLine_t *) malloc(sizeof(DpTextLine_t));
    ASSERT(_pData != NULL);
    if (_pData != NULL) {
        str = (char *) malloc(TEXT_MAX_LINE_LENGTH);
        len = 0;

        length = strlen(buffer);
        for(count=0; count<length; count++) {
            ch = buffer[count];

            if (len >= TEXT_MAX_LINE_LENGTH) {
                str = (char *) realloc(str, len + 2);
            }

            if (ch == '\n') {
                str[len] = '\0';

                _pData[_nItems].data = (char *) realloc(str, len+1);
                _pData[_nItems].length = len;
                _nItems++;

                _pData = (DpTextLine_t *) realloc(_pData, sizeof(DpTextLine_t) * (_nItems+1));
                ASSERT(_pData != NULL);

                str = (char *) malloc(TEXT_MAX_LINE_LENGTH);
                len = 0;
            }
            else {
                str[len] = ch;
                len++;
            }
        }

        if (len > 0) {
            str[len] = '\0';
            _pData[_nItems].data = (char *) realloc(str, len+1);
            _pData[_nItems].length = len;
            _nItems++;
        }
        else {
            free(str);
            _nItems--;
            ASSERT(_nItems >= 0);
        }

        ok = true;
    }

    return(ok);
}



//------------------------------------------------------------------------------
// CJW: Given some text, we will find the line that BEGINS with this particular
//      text, starting from the current line.  If you wish to search for
//      particular text ANYWHERE in a line, then the general FindText function
//      would be more appropriate.  However, if you know the text you are
//      looking for is at the begining of the line, then this function will
//      perform much faster.
int DpTextTools::FindLine(char *txt)
{
    int nLine = -1;
    int count;
    int len;
    int glen;
    
    ASSERT(txt != NULL);
    ASSERT(_pData != NULL);

    glen = strlen(txt);

    count = _nCurrentLine;
    while(nLine == -1 && count < _nItems) {
        len = _pData[count].length;
        if (glen < len) { len = glen; }

        if (strncmp(txt, _pData[count].data, glen) == 0) {
            nLine = count;
            _nCurrentLine = nLine;
            _nCurrentChar = 0;
            count = _nItems;
        }
    
        count++;
    }

    return(nLine);
}

//------------------------------------------------------------------------------
// CJW: Move the 'cursor' to the begining of the next line.
int DpTextTools::MoveNextLine(void)
{
    int nLine;

    nLine = _nCurrentLine + 1;
    if (nLine < _nItems) {
        _nCurrentChar = 0;
        _nCurrentLine = nLine;
    }
    else {
        nLine = -1;
    }

    return(nLine);
}

//------------------------------------------------------------------------------
// CJW: This function is still quite simple, but is a little more complicated
//      than what we have done so far.  This one will delete all lines before
//      the current one.  This means, that we need to make sure that we dont
//      clobber ourselves if people start this function on the first line.   We
//      also need to be very careful about freeing all the memory that is no
//      longer needed.  To accomplish this, we will need several line counters,
//      and will need to process each line that we have.  some we will need to
//      move, others we will need to delete.
int DpTextTools::TrimBefore(void)
{
    int nLine = -1;
    int a, b, c;

    if (_nCurrentLine > 0) {

        a = 0;
        b = _nCurrentLine;

        for (c=0; c<_nItems; c++) {
            if (_pData[c].data != NULL) {
                free(_pData[c].data);
                _pData[c].data = NULL;
            }

            if (b < _nItems) {
                _pData[a].data = _pData[b].data;
                _pData[a].length = _pData[b].length;

                _pData[b].data = NULL;
                a++;
                b++;
            }
        }

        _nCurrentLine = 0;
        _nItems = a;
        nLine = 0;

        _pData = (DpTextLine_t *) realloc(_pData, sizeof(DpTextLine_t) * _nItems);
        ASSERT(_pData != NULL);
    }

    return(nLine);
}

//------------------------------------------------------------------------------
// CJW: Starting from the current line, delete everything to the end of the
//      list.  Move the current line marker to the previous line.
int DpTextTools::DeleteToEnd(void)
{
    int count;

    for (count=_nCurrentLine; count<_nItems; count++) {
        if (_pData[count].data != NULL) {
            free(_pData[count].data);
            _pData[count].data = NULL;
            _pData[count].length = 0;
        }
    }

    _nItems = _nCurrentLine;
    _pData = (DpTextLine_t *) realloc(_pData, sizeof(DpTextLine_t) * _nItems);
    ASSERT(_pData != NULL);

    if (_nCurrentLine > 0) {
        _nCurrentLine--;
    }

    return(_nCurrentLine);
}

//------------------------------------------------------------------------------
// CJW: Move the cursor to the first line in the array.  Also move to the char
//      of the line.
int DpTextTools::MoveFirstLine(void)
{
    _nCurrentLine = 0;
    _nCurrentChar = 0;
    return(0);
}

//------------------------------------------------------------------------------
// CJW: On the current line, search for a particular string, and insert a new
//      line into the text array whenever it is found.
int DpTextTools::InsertLineOnStr(char *str)
{
    char *text;     // our original line text.
    int length;     // the length of our original line.
    int a, b;       // general purpose counters/
    int len;        // length of our trigger string.
    int count;

    ASSERT(str != NULL);
    ASSERT(_nCurrentLine < _nItems);
    ASSERT(_pData != NULL);
    ASSERT(_nItems > 0);

    len = strlen(str);

    length = _pData[_nCurrentLine].length;
    text = (char *) malloc(length + 1);
    ASSERT(text != NULL);
    if (text != NULL) {
        strcpy(text, _pData[_nCurrentLine].data);
    
        count = 0;
        b = _pData[_nCurrentLine].length - len;
        for (a = _nCurrentChar; a<b; a++) {
            if (strncmp(str, &text[a], len) == 0) {
                // we found our string.. insert the new line, reset our counts, and then skip the word we found
                _nCurrentChar = count;
                InsertNewLine();
                a += len;
                count = len+1;
            }
            else {
                count++;
            }
        }

        free(text);
    }

    return(_nCurrentLine);
}

//------------------------------------------------------------------------------
// CJW: Insert a new line.  If there is text following cursor in the line, then
//      it will truncate the current one, and move the text to the newly created
//      new line.
int DpTextTools::InsertNewLine(void)
{
    int a;

    ASSERT(_nCurrentLine < _nItems);
    ASSERT(_pData != NULL);

    // increase the number of lines;
    _pData = (DpTextLine_t *) realloc(_pData, sizeof(DpTextLine_t) * (_nItems+1));
    ASSERT(_pData != NULL);

    // now move all the line data down by one.
    for(a=_nItems; a>(_nCurrentLine+1); a--) {
        _pData[a].data = _pData[a-1].data;
        _pData[a].length = _pData[a-1].length;
    }
    _nItems++;

    _pData[_nCurrentLine+1].data = (char *) malloc((_pData[_nCurrentLine].length - _nCurrentChar) + 2);
    ASSERT(_pData[_nCurrentLine+1].data != NULL);
    
    _pData[_nCurrentLine+1].length = _pData[_nCurrentLine].length - _nCurrentChar;
    strncpy(_pData[_nCurrentLine+1].data, &_pData[_nCurrentLine].data[_nCurrentChar], _pData[_nCurrentLine+1].length);
    _pData[_nCurrentLine+1].data[_pData[_nCurrentLine+1].length] = '\0';

    // now we need to trim the original and then move the current line and cursor to the new line.
    _pData[_nCurrentLine].data = (char *) realloc(_pData[_nCurrentLine].data, _nCurrentChar + 2);
    ASSERT(_pData[_nCurrentLine].data != NULL);

    _pData[_nCurrentLine].length = _nCurrentChar;
    _pData[_nCurrentLine].data[_nCurrentChar] = '\0';

    _nCurrentLine ++;
    _nCurrentChar = 0;

    return(_nCurrentLine);
}

//------------------------------------------------------------------------------
// CJW: On the current line, this function will remove all the HTML tags.   If
//      the spaces parameter is true, then it will ensure there is a space
//      between all text left over after html has been trimmed out.  For
//      example, with spaces set to true, "<b><i>title</i></b>fred" would
//      result in "title fred".  With spaces set to false, it would come out
//      as "titlefred".
void DpTextTools::RemoveHtmlOnLine(bool spaces)
{
    char *text;
    int length, len;
    int vv;
    int count;
    char ch;
    bool last = false;      // last real text had a space?

    ASSERT(_nCurrentLine < _nItems);
    ASSERT(_pData != NULL);

    length = 0;
    vv = 0;

    len = _pData[_nCurrentLine].length;
    text = (char *) malloc(len + 1);
    ASSERT(text != NULL);
    if (text != NULL) {

        for(count=0; count < len; count++) {
            ch = _pData[_nCurrentLine].data[count];

            if (ch == '<') {
                vv++;
            }
            else if (ch == '>') {
                vv--;
                if (vv == 0 && spaces==true) {
                    last = true;
                }
            }
            else {
                if (vv == 0) {
                    // do we add a space?
                    if (spaces == true) {
                        if (length > 0) {
                            if (last == true) {
                                if (text[length-1] != ' ') {
                                    text[length] = ' ';
                                    length++;
                                }
                            }
                        }
                        last = false;
                    }

                    text[length] = ch;
                    length++;
                }
            }
        }

        // realloc text.
        text[length] = '\0';
        text = (char *) realloc(text, length+1);

        // free original
        free(_pData[_nCurrentLine].data);

        // put text in current line.
        _pData[_nCurrentLine].data = text;
        _pData[_nCurrentLine].length = length;
    }
}

//------------------------------------------------------------------------------
// CJW: Return a char ptr array of the words that are in the line.  We will be
//      returning pointers to copies of the actual words, so modifying the
//      values inside this array will not change the actual text.  However, it
//      is important to note that the user is not responsible for releasing the
//      memory used by the elements in this array, but IS responsible for
//      releasing the array itself.  However, only one of these arrays can be
//      used at a time, because an internal pointer is used to maintain it, and
//      free the strings when this function either goes out of scope, or
//      another one of these function calls is made.
char **DpTextTools::GetWordArray(void)
{
    char **pta = NULL;
    int items = 0;
    int len;
    int count;
    bool next = true;   // true means that the next char is the begining of a word if it is not a space.
    char ch;

    ASSERT(_nCurrentLine < _nItems);
    ASSERT(_pData != NULL);

    // clear the word array if we already have one.
    ClearWordArray();

    // copy the current line to the word array pointer.
    len = _pData[_nCurrentLine].length;
    _pWordArray = (char *) malloc(len + 1);
    ASSERT(_pWordArray != NULL);
    if (_pWordArray != NULL) {
    
        strncpy(_pWordArray, _pData[_nCurrentLine].data, len+1);

        for(count=0; count<len; count++) {
            ch = _pWordArray[count];
            if (ch == ' ' || ch == '\t' || ch == ',') {
                _pWordArray[count] = '\0';
                next = true;
            }
            else {
                if (next == true) {
                    pta = (char **) realloc(pta, sizeof(char *) * (items + 1));
                    ASSERT(pta != NULL);
                    pta[items] = &_pWordArray[count];
                    items++;
                    next = false;
                }
            }
        }

        pta = (char **) realloc(pta, sizeof(char *) * (items + 1));
        ASSERT(pta != NULL);
        pta[items] = NULL;
        items++;
    }

    return(pta);
    
}

//------------------------------------------------------------------------------
// CJW: If a word array was generated on a line, this function will release all
//      the memory that was allocated to hold all the words.
void DpTextTools::ClearWordArray(void)
{
    if (_pWordArray != NULL) {
        free(_pWordArray);
        _pWordArray = NULL;
    }
}


//------------------------------------------------------------------------------
// CJW: Sometimes there are particular words that we want to delete on a line.
//      This function will do that.   When finished, the cursor will be moved to
//      the begining of the line.  Will return the number of times the word was
//      removed from the line.
int DpTextTools::DeleteFromLine(char *str)
{
    int words = 0;

    char *text;
    int length, len;
    int count;
    int slen;

    ASSERT(str != NULL);

    ASSERT(_nCurrentLine < _nItems);
    ASSERT(_pData != NULL);

    slen = strlen(str);
    ASSERT(slen > 0);

    length = 0;
    text = (char *) malloc(_pData[_nCurrentLine].length + 1);
    ASSERT(text != NULL);
    if (text != NULL) {

        len = _pData[_nCurrentLine].length - slen;
        for(count=0; count < len; count++) {
            if (strncmp(str, &_pData[_nCurrentLine].data[count], slen) == 0) {
                count += (slen-1);  // -1 because the for will increment one also.
                words++;
            }
            else {
                text[length] = _pData[_nCurrentLine].data[count];
                length++;
            }
        }

        // realloc text.
        text[length] = '\0';
        text = (char *) realloc(text, length+1);

        // free original
        free(_pData[_nCurrentLine].data);

        // put text in current line.
        _pData[_nCurrentLine].data = text;
        _pData[_nCurrentLine].length = length;
    }

    _nCurrentChar = 0;

    return(words);
}



#endif

// #############################################################################
// #############################################################################
// #############################################################################


#ifndef _EXCLUDE_WINSERVICE



//------------------------------------------------------------------------------
// CJW: Allocate our static data.
void *DpWinService::_pThis = NULL;



//------------------------------------------------------------------------------
// CJW: Constructor.  Initialise our structures.
DpWinService::DpWinService()
{
    _pThis = this;
    _szName = NULL;
}



//------------------------------------------------------------------------------
// CJW: Deconstructor.  Clean up.
DpWinService::~DpWinService()
{
}


//------------------------------------------------------------------------------
// CJW: Set the name of the service.  Required.
void DpWinService::SetServiceName(const char *name)
{
    ASSERT(name != NULL);
    ASSERT(_szName == NULL);

    _szName = name;
}


//------------------------------------------------------------------------------
// CJW: This function is where the service code is being done.  This function
//      should be over-ridden and handled in the child-class.
void DpWinService::Run(void)
{
    _xLock.Lock();
    while(_bRun != false) {
        _xLock.Unlock();
        Sleep(3000);        // sleep for 3 seconds.
        _xLock.Lock();
    }
    _xLock.Unlock();
}


//------------------------------------------------------------------------------
// CJW: Install the service.
bool DpWinService::Install(void)
{
    bool bOK = false;
    char szFilePath[_MAX_PATH];

    // Open the Service Control Manager
    SC_HANDLE hSCM = ::OpenSCManager(NULL, // local machine
                                     NULL, // ServicesActive database
                                     SC_MANAGER_ALL_ACCESS); // full access
    if (hSCM) {

        // Get the executable file path
        ::GetModuleFileName(NULL, szFilePath, sizeof(szFilePath));

        // Create the service
        SC_HANDLE hService = ::CreateService(hSCM,
                                             _szServiceName,
                                             _szServiceName,
                                             SERVICE_ALL_ACCESS,
                                             SERVICE_WIN32_OWN_PROCESS,
                                             SERVICE_DEMAND_START,        // start condition
                                             SERVICE_ERROR_NORMAL,
                                             szFilePath,
                                             NULL,
                                             NULL,
                                             NULL,
                                             NULL,
                                             NULL);

        if (hService) {
            bOK = true;
            ::CloseServiceHandle(hService);
        }

        ::CloseServiceHandle(hSCM);
    }

    return bOK;
}


#endif

// #############################################################################
// #############################################################################
// #############################################################################

#ifndef _EXCLUDE_STRING

DpString::DpString()
{
}
   
DpString::~DpString()
{
}         

#endif

// #############################################################################
// #############################################################################
// #############################################################################

#ifndef _EXCLUDE_ARGS

int DpArgs::_argc = 0;
char **DpArgs::_argv = NULL;

DpArgs::DpArgs()
{
}

DpArgs::~DpArgs()
{
}
		
void DpArgs::Process(int argc, char **argv)
{
	ASSERT(argc >= 0);
	ASSERT(argv != NULL);
	ASSERT(argv[0] != NULL);
	
	_argc = argc;
	_argv = argv;
}

#endif


// #############################################################################
// #############################################################################
// #############################################################################

#ifndef _EXCLUDE_MAIN

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>


#ifndef _EXCLUDE_LOCK
	DpLock DpMain::_Lock;
#endif
int  DpMain::_nInstances = 0;
bool DpMain::_bShutdown = false;
int  DpMain::_nReturn = 0;
DpMain **DpMain::_pObjList = NULL;
int  DpMain::_nObjCount = 0;
DWORD DpMain::_nLastWait = 0;

//------------------------------------------------------------------------------
// CJW: Initialise the Object.
DpMain::DpMain()
{
// 	printf("DpMain::DpMain() %x\n", this);

	#ifndef _EXCLUDE_LOCK
	_Lock.Lock();
	#endif
	
	ASSERT(_nInstances >= 0);
	_nInstances ++;
	ASSERT(_nInstances > 0);
	
	ASSERT((_nInstances == 1 && _nObjCount == 0 && _pObjList == NULL) || 
			(_nInstances > 1 && _nObjCount > 0  && _pObjList != NULL));
	_pObjList = (DpMain **) realloc(_pObjList, sizeof(DpMain*) * (_nObjCount+1));
	ASSERT(_pObjList != NULL);
	
	ASSERT(_nObjCount >= 0);
	_pObjList[_nObjCount] = this;
	_nObjID = _nObjCount;
	_nObjCount ++;
	ASSERT(_nObjCount > 0);
	
	_nLastWait = 0;
	_pTimers = NULL;
	_nTimers = 0;
	
	#ifndef _EXCLUDE_LOCK
	_Lock.Unlock();
	#endif
}


//-----------------------------------------------------------------------------
DpMain::~DpMain()
{
	#ifndef _EXCLUDE_LOCK
	_Lock.Lock();
	#endif
	
// 	printf("DpMain::~DpMain() - start. id=%d,%x=%x\n", _nObjID, _pObjList[_nObjID], this);
	
	ASSERT(_nInstances > 0);
	_nInstances --;
	ASSERT(_nInstances >= 0);
	
	ASSERT(_nObjID >= 0 && _nObjCount > 0 && _pObjList != NULL);
	ASSERT(_pObjList[_nObjID] == this);
	
	_pObjList[_nObjID] = NULL;
	if ((_nObjID+1) == _nObjCount) {
		_nObjCount = _nObjID;
	}
	
	if (_nInstances == 0) {
		_nObjID = 0;
		_nObjCount = 0;
		free (_pObjList);
		_pObjList = NULL;
	}
	
	ASSERT((_pTimers == NULL && _nTimers == 0) || (_pTimers != NULL && _nTimers >= 0));
	if (_pTimers != NULL) { free(_pTimers);	}
	_nLastWait = 0;
	_pTimers = NULL;
	_nTimers = 0;
	
	#ifndef _EXCLUDE_LOCK
	_Lock.Unlock();
	#endif

// 	printf("DpMain::~DpMain() - end\n");
}

//-----------------------------------------------------------------------------
// CJW: Since we will need to use random numbers from time to time in this 
// 		code, we need to seed it based on the current time.   This should be 
// 		non-predictive, but that might be a bit hard to time over an internet 
// 		connection.
void DpMain::InitRandom(void)
{
	struct timeval tv;
	struct timezone tz;
	unsigned int i;

	gettimeofday(&tv, &tz);
	i = (unsigned int) tv.tv_usec;
	srandom(i);
}



//-----------------------------------------------------------------------------
// CJW: Tell the application to shutdown.  Will probably do different things 
// 		depending on if we are multi-threaded or not.
void DpMain::Shutdown(int nReturn)
{
	ASSERT(_bShutdown == false);
	_bShutdown = true;
	_nReturn = nReturn;
}

//-----------------------------------------------------------------------------
// CJW: Set a timer that will trigger some functionality on a regular basis.
int DpMain::SetTimer(DWORD nMiliSecs)
{
	int nTimerID;
	
	ASSERT(nMiliSecs > 0);
	
	// first we see if there are any timers already.. if not, then we create the first one.
	if (_pTimers == NULL) {
		ASSERT(_nTimers == 0);
		_pTimers = (DpTimerStruct *) malloc(sizeof(DpTimerStruct));
		ASSERT(_pTimers != NULL);
		
		nTimerID = 1;
		_pTimers[0].nTimerID = nTimerID;
		_pTimers[0].nTime    = nMiliSecs;
		_pTimers[0].nLeft    = nMiliSecs;
		_nTimers = 1;
	}
	else {
		ASSERT(_pTimers == NULL);
		// we havent got the code here yet to handle more than one timer.
	}
	
	
	return(nTimerID);
}


//-----------------------------------------------------------------------------
void DpMain::Signal(int signo)
{
	int i;
	
#ifndef _EXCLUDE_LOCK
	_Lock.Lock();
#endif

	if (signo == SIGINT) {
		
		ASSERT(_bShutdown == false);
		for(i=0; i<_nObjCount; i++) {
			if (_pObjList[i] != NULL) {
				_pObjList[i]->OnCtrlBreak();
			}
		}
		_bShutdown = true;
	}
	else {
		ASSERT(0);
	}
	
#ifndef _EXCLUDE_LOCK
	_Lock.Unlock();
#endif
}


//-----------------------------------------------------------------------------
// When a signal arrives, we need to mark a global variable so that the threads 
// can know to shutdown.
void DpMain::HandleSig(int signo)
{
	DpMain xMain;
	xMain.Signal(signo);
}


//-----------------------------------------------------------------------------
int DpMain::ProcessAll(void)
{
	bool bStop = false;
	int i;
	DWORD nWait, nNext;
	int nReturn = 0;
	
// 	printf("DpMain::ProcessAll() - Start\n");
	
	// set up our signal handlers.
	signal(SIGINT, &DpMain::HandleSig);

	#ifndef _EXCLUDE_LOCK
	_Lock.Lock();
	#endif
	for (i=0; i<_nObjCount; i++) {
		if (_pObjList[i] != NULL) {
			_pObjList[i]->OnStartup();
		}
	}

	// if one of the tasks told us to shutdown, we shouldnt waste time by going thru the 1 second loop.
	if (_bShutdown == true) {
		bStop = true;
	}
	
	#ifndef _EXCLUDE_LOCK
	_Lock.Unlock();
	#endif
	
	// keep looping until until we are told to shutdown.
	while(bStop == false) {
						
		// we are going to sleep for 1 seconds.  However, when a signal 
		// comes in, the sleep will imediatly be broken and we can check 
		// to see if we are being told to shutdown.
		
		nWait = 1000;
		for (i=0; i<_nObjCount; i++) {
			if (_pObjList[i] != NULL) {
				nNext = _pObjList[i]->ProcessTimers(_nLastWait);
				if (nNext > 0 && nNext < nWait) {
					nWait = nNext;
				}
			}
		}
		
		Sleep(nWait);
		_nLastWait = nWait;
						
		#ifndef _EXCLUDE_LOCK
		_Lock.Lock();
		#endif
		
		if (_bShutdown == true) {
			bStop = true;
		}
		
		#ifndef _EXCLUDE_LOCK
		_Lock.Unlock();
		#endif
	}
	
	
	#ifndef _EXCLUDE_LOCK
	_Lock.Lock();
	#endif
	for (i=0; i<_nObjCount; i++) {
		if (_pObjList[i] != NULL) {
			_pObjList[i]->OnShutdown();
		}
	}
	#ifndef _EXCLUDE_LOCK
	_Lock.Unlock();
	#endif
	
// 	printf("DpMain::ProcessAll() - Exit\n");
	
	return(nReturn);
}


//-----------------------------------------------------------------------------
// CJW: This function is called for each instance of DpMain so that any timers 
// 		that are set are returned.  If we actually have timers, then we will 
// 		keep track of which ones we need to process.  We are given an argument 
// 		that tells us how much time has passed since the last time this 
// 		function was called here.  We will return the number of miliseconds 
// 		until the next timer expires that we need to process.  If we dont have 
// 		any timers, then we will return 0.
DWORD DpMain::ProcessTimers(DWORD nLast)
{
	int i;
	DWORD nNext = 0;
	
	if (_pTimers != NULL) {
		for (i=0; i<_nTimers; i++) {	
			if (_pTimers[i].nTimerID > 0) {
				ASSERT(_pTimers[i].nTime > 0 && _pTimers[i].nLeft > 0);
				if (_pTimers[i].nLeft <= nLast) {
					OnTimer(_pTimers[i].nTimerID);
					_pTimers[i].nLeft = _pTimers[i].nTime;
				}
				else {
					_pTimers[i].nLeft -= nLast;
				}
				
				ASSERT(_pTimers[i].nLeft > 0);
				if (nNext == 0 || _pTimers[i].nLeft < nNext) {
					nNext = _pTimers[i].nLeft;
				}
			}
		}
	}
	
	return(nNext);
}


//-----------------------------------------------------------------------------
// CJW: This virtual function will be called when the main object starts up.
void DpMain::OnStartup(void)
{
// 	printf("DpMain::OnStartup()\n");
}

//-----------------------------------------------------------------------------
// CJW: This virtual function will be called when the main object is shutting 
// 		down.
void DpMain::OnShutdown(void) 
{
// 	printf("DpMain::OnShutdown()\n");
}

//-----------------------------------------------------------------------------
// CJW: process the Ctrl-Break that we received.
void DpMain::OnCtrlBreak(void)
{
	// *** Here we need to actually set things in motion so that we quit.
	#ifndef _EXCLUDE_LOCK
	_Lock.Lock();
	#endif

// 	printf("DpMain::OnCtrlBreak()\n");
	
	#ifndef _EXCLUDE_LOCK
	_Lock.Unlock();
	#endif
}

void DpMain::OnTimer(int nTimerID)
{
// 	printf("DpMain::OnTimer(%d)\n", nTimerID);
	// Why set a timer if your not going to process it?
	ASSERT(0);
}


//------------------------------------------------------------------------------
// CJW: This is the main function that starts up everything. We first create the 
// 		initial instance of our base application object.  It is used to 
// 		initialise things.
int main(int argc, char **argv)
{
	DpArgs *pArgs;
	DpMain *pMain;
	int nReturn = 0;
	
// 	printf("Main - Start\n");
	
	pArgs = new DpArgs;
	ASSERT(pArgs != NULL);
	pArgs->Process(argc, argv);
	delete pArgs;
	
	pMain = new DpMain;
	ASSERT(pMain != NULL);
	nReturn = pMain->ProcessAll();
	delete pMain;
	
// 	printf("Main - Exit\n");
	return(nReturn);
}


#endif

// #############################################################################
// #############################################################################
// #############################################################################


//-----------------------------------------------------------------------------
// CJW: Constructor.  We must initialise our data first.   Since we will be 
//		using an array of form data, we need to start if off in a cleared 
//		state.
DpCgiFormData::DpCgiFormData()
{
	_pItemList = NULL;
	_nItems = 0;
}





//-----------------------------------------------------------------------------
// CJW:	Deconstructor.  If we formulated any data, or reserved any resources, we need to clean them up.  Specifically, the 
DpCgiFormData::~DpCgiFormData()
{
	if(_pItemList != NULL) {
		while(_nItems > 0) {
			_nItems--;
			if (_pItemList[_nItems].name)  free(_pItemList[_nItems].name);
			if (_pItemList[_nItems].value) free(_pItemList[_nItems].value);
		}

		free(_pItemList);
		_pItemList = NULL;
		_nItems = 0;
	}
}


//-----------------------------------------------------------------------------
// CJW: Process the environment and parameters for form data that was passed to 
//		the script.   Keep in mind that there might be some binary files that 
//		were passed.  These will need to be stored in this object some how.
//
//		The form data will be stored in an array that is used as a hash.  All 
//		references to the form data will likely be thru a named entity.  These 
//		can be accessed by using an [] operator.
void DpCgiFormData::Process(void)
{
	char *method;
	char *content;
	int index;		// index into our current field we are populating.
	char *query = NULL;
	int tmp=0;
	
	method = getenv("REQUEST_METHOD");
	if(method != NULL) {
		if(strcmp(method, "POST")  == 0) {
			content = getenv ("CONTENT_LENGTH");
			if (content) {
				index = atoi (content);
				query = (char *) malloc (index+1);
				query[0] = '\0';
				tmp = 0;
				while(tmp < index) {
					query[tmp++] = fgetc(stdin);
				}
				query[tmp++] = '\0';
			}
		}
		else {
			// Get the query string from the environment variable. 
			query = getenv("QUERY_STRING");
		}
	}

	ProcessData(query);

	// free the query string if we allocated memory for it.
	if (tmp > 0) {
		free(query);
	}
}

//-----------------------------------------------------------------------------
// CJW: This function will take the string and break apart all the form data 
// 		into name/value pairs.
void DpCgiFormData::ProcessData(char *query)
{
	int index;		// index into our current field we are populating.
	char *key;
	char *value;
	int hex;
	enum {
		start,
		inkey,
		assign,
		invalue,
	} status = start;
	int n;
	int len;
	
	ASSERT(query != NULL);
	
	// now we need to go thru that query string, and pull out the data pairs.
	if (query != NULL) {
		len = strlen(query);
		for(n=0; n<len; n++) {
			
			switch(status) {
				case start:
					key = (char*) malloc(255);
					status = inkey;
					index = 0;
					// nobreak

				case inkey:
					if(query[n] == '=') {
						key[index] = '\0';
						index++;
						key = (char *) realloc(key, index);
						status = assign;
					}
					else {
						// ** need to parse special characters.
						if(query[n] == '+') {
							key[index] = ' ';
						}
						else if(query[n] == '%') {
							hex = 0;
							hex = 0;
							hex = GetHex(query[n+1]);
							hex = hex * 16;
							hex += GetHex(query[n+2]);
							key[index] = hex;
							n+=2;
						}
						else {
							key[index] = query[n];
						}
						index++;
						if(index >= 255) {
							key = (char *) realloc(key, index+2);
						}
					}
					break;

				case assign:
					value = (char *) malloc(255);
					index = 0;
					status = invalue;
					// nobreak

				case invalue:
					if(query[n] == '&') {
						value[index] = '\0';
						index++;
						value = (char *) realloc(value, index);
			
						AddItem(key, value);
						status = start;
					}
					else {
						if(query[n] == '+') {
							value[index] = ' ';
						}
						else if(query[n] == '%') {
							hex = 0;
							hex = GetHex(query[n+1]);
							hex = hex * 16;
							hex += GetHex(query[n+2]);
							value[index] = hex;
							n+=2;
						}
						else {
							value[index] = query[n];
						}

						index++;
						if(index >= 255) {
							value = (char *) realloc(value, index+2);
						}
					}
					break;
					
				default:
					break;
			}
		}	

		if (status == invalue) {
			value[index] = '\0';
			index++;
			value = (char *) realloc(value, index);

			AddItem(key, value);
		}
	}	
}


//-----------------------------------------------------------------------------
// CJW: Add a form element to the form list.  We will simply add the pointers, 
//		so we need to make sure that the data is in heap memory and is not 
//		going to be freed.  We will free the memory when this object goes out 
//		of scope.
void DpCgiFormData::AddItem(char *key, char *value)
{
	ASSERT(key != NULL && value != NULL);
	ASSERT((_pItemList != NULL && _nItems > 0) || (_pItemList == NULL && _nItems == 0))
	
	_pItemList = (_DpCgiFormItem *) realloc(_pItemList, sizeof(_DpCgiFormItem)*(_nItems+1));
	if (_pItemList) {
		_pItemList[_nItems].name = key;
		_pItemList[_nItems].value = value;
		_nItems++;
	}
	else {
		_nItems = 0;
	}
}


//-----------------------------------------------------------------------------
// return the value of a passed parameter.
char * DpCgiFormData::Get(char *key)
{
	char *val = NULL;
	unsigned int n;
	
	ASSERT(key != NULL);
	ASSERT((_pItemList != NULL && _nItems > 0) || (_pItemList == NULL && _nItems == 0));

	for(n=0; n<_nItems; n++) {
		if(strcmp(_pItemList[n].name, key) == 0) {
			val = _pItemList[n].value;
			n=_nItems;
		}
	}
	return(val);
}


//-----------------------------------------------------------------------------
// using the [] operator... return the value of a passed parameter.
char * DpCgiFormData::operator()(char* key)
{
	ASSERT(key != NULL);
	return(Get(key));
}



//-----------------------------------------------------------------------------
// CJW: passing in a character which is either 0-9, a-f or A-F, we will return 
//		the numberical value of the character entered.
unsigned int DpCgiFormData::GetHex(char ch)
{
	unsigned int val;

	if (ch >= '0' && ch <= '9')
		val = ch - '0';
	else if (ch >= 'A' && ch <= 'F')
		val = 10 + (ch - 'A');
	else if (ch >= 'a' && ch <= 'f')
		val = 10 + (ch - 'a');

	return(val);
}


// ##########

//-----------------------------------------------------------------------------
// CJW: Constructor.  Set our flags and other variables that are used to keep 
// 		track of the state of things and help trap programmer errors.
DpCgi::DpCgi()
{
	_bSetContentType = false;
}

//-----------------------------------------------------------------------------
// CJW: Deconstructor.  Clean up anything we created along the way.
DpCgi::~DpCgi()
{
}

//-----------------------------------------------------------------------------
// CJW: When providing output as a CGI, we must tell the receiving system what 
// kind of data it is that we are returning.  Normally you use 'text/html' to 
// indicate that the result is html.
void DpCgi::ContentType(char *tt) 
{
	ASSERT(tt != NULL);
	ASSERT(_bSetContentType == false);
	printf("Content-type: %s\n\n", tt);
	_bSetContentType = true;
}


// #############################################################################
// #############################################################################
// #############################################################################


#ifndef _EXCLUDE_DATA_SERVER


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



#endif



// #############################################################################
// #############################################################################
// #############################################################################

#ifndef _EXCLUDE_BASE64

char DpBase64::_szCodes[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
unsigned char DpBase64::_szReverse[128];
bool DpBase64::_bInit = false;


//-----------------------------------------------------------------------------
// CJW: Constructor.  Initialise the variables and states.
DpBase64::DpBase64()
{
	int i;
	
	_pEncoded = NULL;
	_pDecoded = NULL;
	
	// This information should be statically assigned as a const, but since I 
	// cant be bothered to build the array manually right now, I've written 
	// this code to do it instead.
	if (_bInit == false) {
		for (i=0; i<128; i++) { _szReverse[i] = 0; }
		for (i=0; i<64; i++)  { 
// 			printf("'%c'==%d\n", _szCodes[i], _szCodes[i]);
// 			printf("_szReverse[(unsigned int) %d] = %d\n", _szCodes[i], i);
			_szReverse[(unsigned int) _szCodes[i]] = i; 
		}
		_bInit = true;
	}
}

//-----------------------------------------------------------------------------
// CJW: Deconstructor.  Clean up the resources that this object uses.
DpBase64::~DpBase64()
{
	Clear();
}


//-----------------------------------------------------------------------------
// CJW: Clear the strings ready for another operation, or to close down the 
// 		object.
void DpBase64::Clear(void)
{
	if (_pEncoded != NULL) {
		free(_pEncoded);
		_pEncoded = NULL;
	}
	
	if (_pDecoded != NULL) {
		free(_pDecoded);
		_pDecoded = NULL;
	}
	
}

//-----------------------------------------------------------------------------
// CJW: This function should only be used if you are sure that the string 
//		pointer passed in is definately a null-terminated string.
char * DpBase64::EncodeStr(char *str)
{
	ASSERT(str != NULL);
	
	return(Encode((unsigned char *)str, strlen(str)));
}


//-----------------------------------------------------------------------------
// CJW: Encode a set of data into a base64 string.
char * DpBase64::Encode(unsigned char *str, int length)
{
	int i,j, left;
	
	ASSERT(str != NULL && length > 0);
	
	Clear();

	ASSERT(_pDecoded == NULL);
	ASSERT(_pEncoded == NULL);
	
	_pEncoded = (char *) malloc((length / 3 * 4 ) + 5);
	
	for(i=0,j=0; i<length; i+=3, j+=4) {
		left = length-i;
		if (left > 3) { left = 3; }
		EncodeBlock(&str[i], &_pEncoded[j], left);
	}
	_pEncoded[j] = '\0';
	
	return(_pEncoded);
}


//-----------------------------------------------------------------------------
// CJW: Encode a 3 byte block into a 4 byte block.
void DpBase64::EncodeBlock(unsigned char *in, char *out, int len) 
{
	ASSERT(in != NULL && out != NULL && len > 0);
	
	out[0] = _szCodes[ in[0] >> 2 ];
	out[1] = _szCodes[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ];
	if (len > 1) { out[2] = _szCodes[ ((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6) ] ; }
	else 		 { out[2] = '='; }
	if (len > 2) { out[3] = _szCodes[ in[2] & 0x3f ]; }
	else 		 { out[3] = '='; }
}


//-----------------------------------------------------------------------------
// CJW: Decode a 4 byte base64 block into a 3 byte binary block.
unsigned char * DpBase64::Decode(char *str, int *length)
{
	int len;
	int i,j, used;
	
	ASSERT(str != NULL && length != NULL);
	
	Clear();

	ASSERT(_pDecoded == NULL);
	ASSERT(_pEncoded == NULL);
	
	len = strlen(str);
	ASSERT(len > 0);
	_pDecoded = (unsigned char *) malloc(len+1);
	
	length[0] = 0;
	for(i=0,j=0; j<len; i+=3, j+=4) {
		while(str[j] == '\n' || str[j] == '\r') {
			j++;
		}
		ASSERT(j < len);
		DecodeBlock(&str[j], &_pDecoded[i], &used);
		ASSERT(used > 0 && used <= 3);
		length[0] += used;
		ASSERT(length[0] < len);
	}
	_pDecoded[length[0]] = '\0';
	
	return(_pDecoded);
	
}


//-----------------------------------------------------------------------------
// CJW: Decode a 4 byte base64 block into a 3 byte binary block.
void DpBase64::DecodeBlock(char *in, unsigned char *out, int *len) 
{
	unsigned char ch;
	
	ASSERT(in != NULL && out != NULL && len != NULL);
	ASSERT(in[0] != '=' && in[0] != '\0');
	ASSERT(in[1] != '=' && in[1] != '\0');

	len[0] = 1;
	
// 	printf("in[0] == '%c' (%d)\n", in[0], in[0]);
// 	printf("_szReverse[in[0]] == (%d)\n", _szReverse[in[0]]);
	
	out[0] = (_szReverse[(int)in[0]]) << 2;
// 	printf("out[0] == '%c' (%d)\n", out[0], out[0]);
	
	ch = _szReverse[(int)in[1]];
// 	printf("ch = '%c' (%d)\n", ch, ch);
	out[0] |= ((ch >> 4) & 0x03);
// 	printf("ch>>6 = '%c' (%d)\n", ch>>4, ch>>4);
// 	printf("out[0] == '%c' (%d)\n", out[0], out[0]);
	if (in[2] != '=') {
		len[0] ++;
		out[1] = ((ch & 0x0f) << 4);
		ch = _szReverse[(int)in[2]];
		out[1] |= (ch >> 2);
		
		if (in[3] != '=') {
			len[0] ++;
			out[2] = (ch & 0x03) << 6;
			out[2] |= _szReverse[(int)in[3]];
		}
	}
}



#endif

// #############################################################################
// #############################################################################
// #############################################################################
