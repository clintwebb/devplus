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
 

#ifndef __DP_READWRITELOCK_H
#define __DP_READWRITELOCK_H

#include <DevPlus.h>
#include <DpLock.h>

//------------------------------------------------------------------------------
// DpReadWriteLock
//
//  This class can be used to implement read/write locks. If you want to lock
//  an object for reading where multiple threads can read from the object.
//  When a thread needs to modify the object, then it will do a write lock
//  which will wait until all the current read objects have finished 
//  processing and then it will actually lock the object. All new read
//  objects will not begin processing until the write lock has been completed.
//
//  The read locks do not actually prevent anything from writing to the
//  project.  You need to manually determine if your function will be writing
//  to the object.
//
//  Notes:  
//      #1  Due to the potential for difficult errors being found, it is 
//          important that the functionality in this class be totally perfect, 
//          with zero defects.  To ensure this, an entire suite of tests are 
//          performed from the Tester to ensure that it works as it should.  Any 
//          changes made to this script need to pass all the critical tests.
//
//------------------------------------------------------------------------------


class DpReadWriteLock : public DpLock
{
    public:
        virtual void ReadLock();
        virtual void WriteLock();
        virtual void Unlock();
        
        virtual void WriteUnlock()      { Unlock(); }
        virtual void ReadUnlock()       { Unlock(); }
        
        virtual void Lock()             { WriteLock(); }    

    public:
        DpReadWriteLock();
        virtual ~DpReadWriteLock();

	protected:
		void DataLock(void);
		void DataUnlock(void);
		
    private:
        int  _nReadCount;
        int  _nWriteCount;
    
#ifdef __GNUC__
        pthread_mutex_t _csLockData;
#else
        CRITICAL_SECTION _csLockData;
#endif
};




#endif  // __DP_READWRITELOCK_H
