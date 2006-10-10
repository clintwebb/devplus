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
 

#ifndef __DP_DATAQUEUE_H
#define __DP_DATAQUEUE_H

#include <DevPlus.h>

//-----------------------------------------------------------------------------
// DataQueue.h: interface for the CDataQueue class.
//-----------------------------------------------------------------------------
//  
//      This is a simple implementation of a FIFO queue.  It uses single-char 
//      elements and allows you to add, view and remove data from the queue.  
//      To edit data in the queue, you need to get the data, copy it somewhere, 
//      edit it, clear the queue and then add the data back.
//
//      Because of the way this class is built, it can be used for both stack 
//      and FIFO (first in, first out) queues.  So if you are using it as a FIFO 
//      queue, make sure to use Add/Pop functions.  For a stack, use Push/Pop.  
//      Operations can be used interminably.  That means that if you pop data 
//      off, and want it put back on, you can push it.
//
//-----------------------------------------------------------------------------
class DpDataQueue  
{
    public:
        DpDataQueue();
        virtual ~DpDataQueue();

        char * Pop(int nLength);
		
        void Push(char *data, int len);     // inserts chars to the top of the queue.
        void Push(char ch);                 // inserts a single char to the top of the queue.
        void Add(char *data, int len);      // add chars to the bottom of the queue.

        int Length();                       // returns number of chars in the queue.
        void Remove(int cnt);               // remove 'cnt' number of elements from the top of the queue (presumably because you processed them with the pointer returned from Data()
        void Clear();                       // clear all the data in the queue... frees the memory and resets the lengths.
        bool IsEmpty();                     // returns true if the queue is empty.
        
        int FindChar(char ch);

        int Print(char *fmt, ...);
        char *GetLine(void);

    private:
        int _nLength;                      // number of chars in the queue.
        char * _pBuffer;                   // chars in the queue
};


#endif
