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
#include <unistd.h>

#include <DpHttpClient.h>



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
	// TODO: This should be optimised more.
    sleep(1);       // or sleep 1 second...
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




