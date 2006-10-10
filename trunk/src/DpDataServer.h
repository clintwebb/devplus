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
 

#ifndef __DP_DATASERVER_H
#define __DP_DATASERVER_H

#include <DevPlus.h>
#include <DpSqlite3.h>
#include <DpThreadObject.h>


struct DpRequestResult 
{ 
	char *szQuery;
	DpSqlite3Result *pResult;
	bool bDone;
	bool bReturn;
};


class DpDataServer : public DpThreadObject 
{
	private:
		DpSqlite3 *_pDB;
		DpLock _Lock;
		DpLock _EndLock;
		DpRequestResult *_pList;
		int _nItems;
		char *_szFilename;
		enum {
			eOpen,
			ePending,
			eLocked,
			eCommit,
			eCleared
		} _nLocked;
		
	public:
		DpDataServer(); 
		virtual ~DpDataServer();
		
		bool Load(char *szFile);
		DpSqlite3Result * Execute(char *szQuery, ...);
		void ExecuteNR(char *szQuery, ...);
		
		void Begin();
		void Commit();
		
	protected:
		virtual void OnThreadRun(void);
		
	private:
		
};



#endif
