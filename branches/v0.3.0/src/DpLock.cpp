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


#include <DpLock.h>


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

