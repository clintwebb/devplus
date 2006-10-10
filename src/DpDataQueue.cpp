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

#include <DpDataQueue.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>


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
