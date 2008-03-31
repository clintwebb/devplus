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
 

#ifndef __DP_SERVERINTERFACE_H
#define __DP_SERVERINTERFACE_H

#include <DevPlus.h>
#include <DpSocket.h>
#include <DpThreadObject.h>

//------------------------------------------------------------------------------
// DpServerInterface: interface for the DpServerInterface class.
//------------------------------------------------------------------------------
//  
//      This object will be the interface to the server that the clients 
//      connect to.  The server then maintains a list of clients that are 
//      currently connected.  
//
//      Does not contain a static data store, so can be instantiated multiple 
//      times for different ports.  Will create the same client threads 
//      however.
//
//      Intended to be used as a base class for an object that will actually 
//      process the connection.  In fact, it must be derived and cannot be 
//      instantiated by itself.
//
//------------------------------------------------------------------------------


class DpServerInterface : public DpThreadObject
{

	public:
		DpServerInterface();                    	// Constructor.
		virtual ~DpServerInterface();           	// Deconstructor.
	
	public:
		virtual void OnAccept(SOCKET nSocket) = 0;  // Function that will handle the new connection received 
													// on the socket.   Must be resolved for class to be
													// instantiated.

		virtual void OnIdle(void);                  // Called on each cycle when nothing was accepted.
	
	public:
		virtual bool Listen(int nPort);             // Start listening on a socket.
		
	protected:
		virtual void OnThreadRun(void); 		   // Worker thread that processes the incoming connections.
		virtual bool OnObjectDelete(DpThreadObject *pObject);
		virtual void OnAcceptFail(void);

	protected:
		virtual void AddObject(DpThreadObject *pObject);
		virtual void CheckList(void);
		
	
	public:
		virtual int ItemCount()
		{	
			int i;
			Lock();
			i = _nItems;
			Unlock();
			return(i);
		}
	
	private:
		DpSocket        _xSocket;
		DpThreadObject **_pList;
		int              _nItems;
		
};


#endif
