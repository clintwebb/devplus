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
 

#ifndef __DP_HTTPSERVER_H
#define __DP_HTTPSERVER_H

#include <DevPlus.h>
#include <DpServerInterface.h>
#include <DpSocket.h>
#include <DpLock.h>
#include <DpCgi.h>
#include <DpDataQueue.h>

//------------------------------------------------------------------------------
// Start a server listening and responding to either page requests, or respond 
// directly to requests.

#define DP_HTTP_STATE_NEW		0
#define DP_HTTP_STATE_READY		1
#define DP_HTTP_STATE_DELETE	2
#define DP_HTTP_STATE_REPLY		3

class DpHttpServerConnection : public DpThreadObject
{
	private:
		int _nState;
		DpSocket *_pSocket;
		DpLock _Lock;
		char *_szUrl;
		char *_szHeaders;
		char *_szData;
		
		DpCgiFormData *_pForm;
		int _nCode;
		DpDataQueue *_pQueue;
		
		bool ReceiveHeaders(void);
		bool SendReply(void);
	
		
	protected:
		virtual void OnThreadRun();
		
	public:
		DpHttpServerConnection();
		virtual ~DpHttpServerConnection();
		
		void Accept(SOCKET nSocket);
		void Reply(int nCode, DpDataQueue *pQueue);
		
		int GetState(void) {
			int i;	
			_Lock.Lock();
			i = _nState;
			_Lock.Unlock();
			return(i);
		}
		
		char *GetUrl(void)
		{
			char *s;
			_Lock.Lock();
			s = _szUrl;
			_Lock.Unlock();
			return(s);
		}

		char *GetHeaders(void)
		{
			char *s;
			_Lock.Lock();
			s = _szHeaders;
			_Lock.Unlock();
			return(s);
		}

		char *GetData(void)
		{
			char *s;
			_Lock.Lock();
			s = _szData;
			_Lock.Unlock();
			return(s);
		}

		DpCgiFormData *GetForm(void)
		{
			DpCgiFormData *s;
			_Lock.Lock();
			s = _pForm;
			_Lock.Unlock();
			return(s);
		}

};


class DpHttpServer : public DpServerInterface
{
	private:
		DpHttpServerConnection **_pList;
		int _nItems;
		DpLock _Lock;
		
		void AddNode(DpHttpServerConnection *pNew);
		
	public:
		DpHttpServer();
		virtual ~DpHttpServer();
		
	protected:
		virtual void OnAccept(SOCKET nSocket);
		virtual int OnPage(char *szUrl, char *szHeaders, char *szData, DpCgiFormData *pForm, DpDataQueue *pQueue) = 0;
		virtual void OnIdle(void);

		
};




#endif
