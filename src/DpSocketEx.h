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
 

#ifndef __DP_SOCKETEX_H
#define __DP_SOCKETEX_H

#include <DevPlus.h>
#include <DpSocket.h>
#include <DpThreadObject.h>

//-----------------------------------------------------------------------------
// DpSocketEx:    Socket class.
//-----------------------------------------------------------------------------
//  
//	This class joins properties of DpSocket and DpThreadObject so that when the 
//	socket is connected a seperate thread is maintained that willcall virtual 
//	functions when data has been received, or sent, or the connection closed, 
//	etc.
//
//
//	** Just to get it working, we will use a non-blocking loop with a sleep.  
//
//-----------------------------------------------------------------------------


class DpSocketEx : public DpThreadObject
{
	protected:
		DpSocket *_pSocket;
		char     *_pBuffer;
		char 	 *_pReadBuffer;
		int       _nBufferLength;
		bool      _bWait;
		DpLock    _MainLock, _ExternalLock, _BufferLock;
		
		bool	  _bClosed;
	
	
    public:
        DpSocketEx();                             // Constructor.
        virtual ~DpSocketEx();                    // Deconstructor.

		bool Connect(char *szHost, int nPort);
		bool IsClosed(void);
		void Accept(SOCKET nSocket);
        virtual void GetPeerName(char *pStr, int nMax);
        void Close(void);

	protected:
		virtual void OnThreadStart(void);
		virtual void OnThreadRun(void);
	
		virtual int OnReceive(char *pData, int nLength) = 0;
		virtual void OnIdle(void);
		virtual void OnClosed(void);
		virtual void OnStalled(char *pData, int nLength);
		
		virtual int Send(char *pData, int nLength);

    
    private:
		
};


#endif
