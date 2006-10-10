//------------------------------------------------------------------------------
//  CJDJ Creations
//  DevPlus C++ Library.
//  
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
 
 
/*
      ****************************************************************
      **                                                            **
      **  NOTE                                                      **
      **  ----                                                      **
      **                                                            **
      **  A commercial licence of this library is available if the  **
      **  GPL licence is not suitable for your task.                **
      **                                                            **
      **  This means that if you are developing a project and       **
      **  intend to release it under the GPL or equivilent , you do **
      **  NOT need to purchase a commercial licence.                **
      **                                                            **
      **  But if you do not intend to distribute your product under **
      **  the GPL, you are required to purchase a commercial        **
      **  licence.  This licence allows you to use this DevPlus     **
      **  library without providing the source.                     **
      **                                                            **
      **  You can purchase a commercial licence at:                 **
      **    http://hyper-active.com.au/products/devplus/            **
      **                                                            **
      ****************************************************************
*/ 
 

#ifndef __DP_THREADOBJECT_H
#define __DP_THREADOBJECT_H

#include <DevPlus.h>
#include <DpLock.h>


//------------------------------------------------------------------------------
//  DpThreadObject
//
//  This object would allow a program to derive a class and it would become a 
//  thread-running class.  The derived class would require several virtual 
//  functions to be used.  These would be used to Start and Stop the thread, 
//  as well as a function that will be Run every cycle.
//
//  This object could also be used by itself so that threads can be started and controlled with a simple interface.  
//
//  For example, this class can be used in teh following examples.
//
//    Example 1:
//      class CSample1 : public CThreadObject 
//      {
//          public:
//              virtual bool ThreadRun(void);
//      };
//
//    Example 2:
//      class CSample2 
//      {
//          private:
//              CThreadObject xThreadControl;
//      };
//
//  The first example would basically make each instantiated object a thread, 
//  and will execute the ThreadRun function constantly.   The second example 
//  allows for a thread to be created and then controlled.  When the object is 
//  deleted, the thread would be informed to exit, etc.  Both samples have their 
//  benefits.
//
//------------------------------------------------------------------------------


// Set some defaults.  Do not modify, use the functions to set.
// default cycle time of 250 mili-seconds.
#define DP_DEFAULT_CYCLE_TIME 250   

class DpThreadObject : public DpLock
{
    public:
        DpThreadObject();
        virtual ~DpThreadObject();
        
        void SetCycleTime(DWORD dTime=DP_DEFAULT_CYCLE_TIME);
        void SetStackSize(unsigned nStackSize);
        void Start(void (*fn)(void *), void *pParam);
        
        void Stop(void);
        void WaitForThread(void);
        
		bool IsDone(void) {
			bool b = false;
			Lock();
			if (_eToStatus == dptDone || _eToStatus == dptStopped) {
				b = true;
			}
			Unlock();
			return (b);
		}
		
    protected:
        void Start(void);

        virtual void OnThreadStart(void);
        virtual void OnThreadStop(void);
        virtual void OnThreadRun(void);
        
    private:
    #ifdef __GNUC__
        static void *ThreadProcessFunction(void *pParam);
    #else
        static void ThreadProcessFunction(void *pParam);
    #endif
        void ProcessThread(void);
        void ProcessThreadCycle(void);

    private:
    #ifdef __GNUC__
        pthread_t _thread;
        pthread_attr_t _attr;
    #else
        HANDLE _hToHandle;
    #endif
        DpLock _xToLock;
        DWORD _dToCycleTime;
        
        void (*_pToFunction)(void *);
        void *_pToParam;
        
    #ifndef __GNUC__
        unsigned _nToStackSize;
    #endif
        
        enum {
			dptRunOnce,
			dptCycle
        } _eToMode;
        
        enum {
            dptUnknown,
			dptPending,
			dptStarting,
			dptStarted,
			dptRunning,
			dptWaiting,
			dptClosing,
			dptStopped,
			dptDone
        } _eToStatus;
        
        enum {
			dptControlRun,
			dptControlStop
        } _eToControl;
};


#endif
