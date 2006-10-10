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
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>

#include <DpMain.h>





DpLock DpMain::_Lock;
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

	_Lock.Lock();
	
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
	
	_Lock.Unlock();
}


//-----------------------------------------------------------------------------
DpMain::~DpMain()
{
	_Lock.Lock();
	
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
	
	_Lock.Unlock();

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
	
	_Lock.Lock();

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
	
	_Lock.Unlock();
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

	_Lock.Lock();
	for (i=0; i<_nObjCount; i++) {
		if (_pObjList[i] != NULL) {
			_pObjList[i]->OnStartup();
		}
	}

	// if one of the tasks told us to shutdown, we shouldnt waste time by going thru the 1 second loop.
	if (_bShutdown == true) {
		bStop = true;
	}
	
	_Lock.Unlock();
	
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
						
		_Lock.Lock();
		
		if (_bShutdown == true) {
			bStop = true;
		}
		
		_Lock.Unlock();
	}
	
	
	_Lock.Lock();
	for (i=0; i<_nObjCount; i++) {
		if (_pObjList[i] != NULL) {
			_pObjList[i]->OnShutdown();
		}
	}
	_Lock.Unlock();
	
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
	_Lock.Lock();

// 	printf("DpMain::OnCtrlBreak()\n");
	
	_Lock.Unlock();
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




