//------------------------------------------------------------------------------
//  CJDJ Creations
//  DevPlus C++ Library.
//  
/***************************************************************************
 *   Copyright (C) 2006-2008 by Hyper-Active Systems,,,                    *
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
 

#ifndef __DP_SOCKET_H
#define __DP_SOCKET_H

#include <DevPlus.h>

//-----------------------------------------------------------------------------
// DpSocket:    Socket class.
//-----------------------------------------------------------------------------
//  
//      Socket functionality wrapper.  Basic functionality for connecting, 
//      sending and receiving data over a socket.   Not all socket features 
//      should be implemented.  Interface should be simple and used from a 
//      functional perspective rather than a direct mapping to the api.   
//      Basically, connect, send, receive and close are the public functions.
//
//-----------------------------------------------------------------------------

#ifndef SOCKET
	#define SOCKET int
#endif

#ifndef INVALID_SOCKET
	#define INVALID_SOCKET -1
#endif

class DpSocket
{
	
	protected:
		// Socket handle.
		SOCKET _nSocket;                       
	
	public:
		DpSocket();                             // Constructor.
		virtual ~DpSocket();                    // Deconstructor.

		// Initialize the socket system.  not needed in linux, but necessary in windows.
		void Init(void);

		// Used for listening sockets, check to see if a new connection is 
		// waiting, if so accept it and return the socket.
		SOCKET Accept();                        
		
		// Accept and use the socket indicated.
		virtual void Accept(SOCKET nSocket);    
		
		// detach a socket, remove it from this object, doesn't do any 
		// operations on the socket.
		SOCKET Detach();                        

		// connect to the specified host and port, returning true if 
		// connected.
		bool Connect(const char *szHost, int nPort);  
		
		// send data over the socket, return the number of chars sent, -1 if 
		// error (which could mean WOULDBLOCK), 0 if connection was closed.
		int Send(char *data, int len);
		
		// receive data on the socket.  return the number of chars received, 
		// -1 if error (which could mean WOULDBLOCK), 0 if connection was 
		// -closed.
		int Receive(char *data, int len);       
		
		virtual bool Listen(int nPort);
        
		virtual void GetPeerName(char *pStr, int nMax);
        
		// Close the connection.
		void Close();                           
		void Disconnect() { Close(); }
		
		// return true if we have a socket handle.
		bool IsConnected();                     

		// set the socket to Non blocking mode, which means that when 
		// receiving or sending data, the function will not wait until it is 
		// able to send or receive data.  If there is nothing to receive it 
		// will continue on with a -1.
		void SetNonBlocking();

	private:
		
		// resolve a name (or ip) and port into a socket structure that can be 
		// used to connect to the remote server.
		int Resolve(const char *szAddr, int iPort, struct sockaddr_in *pSin, char *szType="A");      
		
};


#endif
