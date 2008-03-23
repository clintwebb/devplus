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

#include <DpThreadObject.h>

#ifdef __GNUC__
	#include <pthread.h>
#else
	#include <process.h>
#endif
#include <time.h>



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

