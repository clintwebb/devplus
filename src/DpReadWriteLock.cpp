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

#include <signal.h>

#include <DpReadWriteLock.h>


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



