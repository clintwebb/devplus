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
 

#ifndef __DP_MAIN_H
#define __DP_MAIN_H

#include <DevPlus.h>
#include <DpArgs.h>
#include <DpLock.h>

//------------------------------------------------------------------------------
// DpMain can be used to provide a common structure to applications.  It 
// provides a method to start and stop daemons in a controlled manner, and also 
// provides support for run-thru applications.   


#ifndef DWORD
	#define DWORD unsigned long
#endif


struct DpTimerStruct 
{
	int nTimerID;	// ID of the timer.  0 indicates timer has been stopped.
	DWORD nTime;	// in miliseconds.  0 indicates timer has been stopped.
	DWORD nLeft;	// time in miliseconds that is left before this timer is triggered again.
};

class DpMain : public DpThreadBase 
{
    // Methods, functions.
    public:
        DpMain();
        virtual ~DpMain();

		static void HandleSig(int signo);
		int ProcessAll(void);
		void Shutdown(int nReturn=0);
		int SetTimer(DWORD nMiliSecs);
		DWORD ProcessTimers(DWORD nTime);
		
    protected:
		virtual void OnStartup(void);
		virtual void OnShutdown(void);
		virtual void OnCtrlBreak(void);
		virtual void OnTimer(int nTimerID);
		
		virtual void InitRandom(void);


    private:
		void Signal(int signo);
		
		
    // Data, internal objects.
    public:
    protected:
    private:
		
		static int _nInstances;
		static int _nReturn;
		static bool _bShutdown;
		static DpMain **_pObjList;
		static int _nObjCount;
		int _nObjID;
		
		struct DpTimerStruct *_pTimers;
		int _nTimers;
		static DWORD _nLastWait;
				
		static DpLock _Lock;
		
};



#endif
