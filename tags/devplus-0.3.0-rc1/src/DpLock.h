//------------------------------------------------------------------------------
//  CJDJ Creations
//  DevPlus C++ Library.
//
//  Version:    0.1.47
//  Date:       **current**
//  
/***************************************************************************
 *   Copyright (C) 2003-2005 by Clinton Webb,,,                            *
 *   devplus@cjdj.org                                                      *
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
      **    http://cjdj.org/products/devplus/                       **
      **                                                            **
      ****************************************************************
*/ 
 

#ifndef __DP_LOCK_H
#define __DP_LOCK_H

#include <DevPlus.h>
#include <DpThreadBase.h>

//------------------------------------------------------------------------------
// DpLock
//
//  This class can be used to implement mutex locks. This is very important for 
//  multi-threaded applications.
//
//	If you want to use a more advanced locking mechanism that allows for 
//	multiple-readers/single-writer access, then use the DpReadWriteLock object 
//	instead.
//
//------------------------------------------------------------------------------


class DpLock : public DpThreadBase
{
	public:
		virtual void Lock();
		virtual void Unlock();

	public:
		DpLock();
		virtual ~DpLock();

	private:
		int _nWriteCount;
    
#ifdef __GNUC__
        pthread_mutex_t _csLock;
#else
        CRITICAL_SECTION _csLock;
#endif
};



#endif
