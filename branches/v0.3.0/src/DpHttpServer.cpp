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
#include <stdlib.h>
#include <string.h>

#include <DpHttpServer.h>


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



