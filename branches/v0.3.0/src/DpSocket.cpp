//-----------------------------------------------------------------------------
// DevPlus.   
//-----------------------------------------------------------------------------


/***************************************************************************
 *   Copyright (C) 2006-2008 by Hyper-Active Systems,,,                    *
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
// CJW: Include the required header files for each class that is not excluded.

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

//-----------------------------------------------------------------------------

#include <DpSocket.h>




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
bool DpSocket::Connect(const char *szHost, int nPort)
{
    bool bConnected=false;
    struct sockaddr_in sin;
    int on = 1;

    ASSERT(szHost != NULL);
    ASSERT(nPort > 0);
    ASSERT(_nSocket == 0);

    if (Resolve(szHost,nPort,&sin) >= 0) {
        // CJW: Create the socket
        _nSocket = socket(AF_INET,SOCK_STREAM,0);
        if (_nSocket >= 0) {
        
       		// set the socket options so that we can avoid the TIME_WAIT annoying problem.
		    if ( setsockopt(_nSocket, SOL_SOCKET, SO_REUSEADDR, (const char*) &on, sizeof (on)) == -1) {
                Close();
                bConnected = false;
            }
            else {
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
        	Close();
            ASSERT(_nSocket == 0);
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
        nResult = send(_nSocket, data, len, MSG_NOSIGNAL);
        if (nResult == 0) {
//          	printf("DpSocket::Send() - send returned a 0\n");
        	Close();
            ASSERT(_nSocket == 0);
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

#include <stdio.h>

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
    int i;
    
    ASSERT(pStr != NULL);
    ASSERT(nMax > 0);
    
    ASSERT(_nSocket > 0);
    
    namelen = sizeof(name);
    nResult = getpeername(_nSocket, &name, &namelen);
    
    sprintf(pStr, "%d.%d.%d.%d", (unsigned int) name.sa_data[2], (unsigned int) name.sa_data[3], (unsigned int) name.sa_data[4], (unsigned int) name.sa_data[5]);
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
	int on = 1;

    // will not work with a 0 or a negative number
	ASSERT(nPort > 0);
	ASSERT(_nSocket == 0);

    // CJW: Create the socket place holder
	_nSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_nSocket > 0) {
	
	    if ( setsockopt(_nSocket, SOL_SOCKET, SO_REUSEADDR, (const char*) &on, sizeof (on)) == -1) {
			#ifdef __GNUC__
		        close(_nSocket);
			#else
            	closesocket(_nSocket);
			#endif
            _nSocket = 0;
		}
		else {
	
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
	}
    
	return (bReturn);
}






