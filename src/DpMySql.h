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
 

#ifndef __DP_MYSQL_H
#define __DP_MYSQL_H

#include <stdlib.h>
#include <mysql/mysql.h>

#include <DevPlus.h>
#include <DpLock.h>


class DpSqlDB
{
	public:
		DpSqlDB();
		virtual ~DpSqlDB();
        
		virtual bool Connect(char *server, char *user, char *password, char *db=NULL) = 0;
		virtual bool Use(char *db) = 0;
		virtual char * Quote(char *str, unsigned int length) = 0;
		virtual bool Execute(char *query, ...) = 0;
		virtual bool ExecuteStr(char *query) = 0;

		virtual void ClientLock(void);
		virtual void ClientUnlock(void);
        
	protected:
		virtual void WriteLock(void);
		virtual void ReadLock(void);
		virtual void Unlock(void);

	private:
        DpLock _xLock;
};




class DpMySqlDB : public DpSqlDB
{
    public:
        DpMySqlDB();
        virtual ~DpMySqlDB();

        DpMySqlDB* Spawn();
        void Unspawn();
        void AttachParent(DpMySqlDB *pParent);
        
        bool Connect(char *server, char *user, char *password, char *db=NULL);
        bool Use(char *db);
        char * Quote(char *str, unsigned int length=0);
        bool Execute(char *query, ...);
        bool ExecuteStr(char *query);
        bool NextRow(void);
        bool GetData(char *field, unsigned int *value);
        bool GetData(char *field, int *value);
        bool GetData(char *field, char *value, int buflen);
        void LockTables(char *szTables);
        void UnlockTables(void);

        my_ulonglong GetInsertID();

        MYSQL * GetHandle(void);
        
    protected:
        virtual void ClientLock(void);
        virtual void ClientUnlock(void);
        
    private:
        MYSQL *_hSql;
        DpMySqlDB *_pParent;
        int _nChildren;
        my_ulonglong _nInsertID;
        MYSQL_RES *_pResult;
        unsigned int _nFields;
        MYSQL_FIELD *_pFieldList;
        unsigned int _nLastErr;
        MYSQL_ROW _Row;
        bool _bTableLock;
};


#endif
