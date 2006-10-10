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
 
/*
    Description:
        DevPlus is a bunch of classes that maintain a sensible interface as 
        closely as possible throughout the various classes and functions.  It 
        is designed to be a powerful substitute or enhancement to the various 
        incompatible methods within MFC and other libraries.  

        DevPlus is intended to be compiled into the application, rather than 
        linked in.  This has some added advantages.  Of course, if you wanted 
        you could create a library out of it and link it in however you want.

        DevPlus is provided as Source Code, but that does not mean that it is 
        without limitations.  DevPlus can only be used in accordance with the 
        licence you choose.
        
    Versions
        See the ChangeLog file for a description of all versions and changes.

    Excludes
    	_EXCLUDE_BASE64
    	_EXCLUDE_CGI
    	_EXCLUDE_DATAQUEUE
        _EXCLUDE_DB
        _EXCLUDE_DB_MYSQL
        _EXCLUDE_DB_ODBC
        _EXCLUDE_DB_SQLITE3
        _EXCLUDE_HTTPCLIENT
        _EXCLUDE_HTTPSERVER
        _EXCLUDE_INI
        _EXCLUDE_LOCK
        _EXCLUDE_LOGGER
        _EXCLUDE_READWRITELOCK
        _EXCLUDE_SERVERINTERFACE
        _EXCLUDE_SETTINGS
        _EXCLUDE_SOCKET
        _EXCLUDE_TEXTTOOLS
        _EXCLUDE_THREADBASE
        _EXCLUDE_WINSERVICE
        _EXCLUDE_MAIN
        
  ------------------------------------------------------------------------------
*/

#ifndef __DEVPLUS_H
#define __DEVPLUS_H

//-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/
#define VERSION_DEVPLUS_THIS 147
//-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/-/



#ifndef VERSION_DEVPLUS
    #define VERSION_DEVPLUS VERSION_DEVPLUS_THIS
#else
    #if (VERSION_DEVPLUS > VERSION_DEVPLUS_THIS)
        #error The VERSION_DEVPLUS value specified is greater than the version this file supports.
    #endif
#endif

#if (VERSION_DEVPLUS <= 147) 
    // Remove the objects that are not yet ready, but still in the code.
	#define _EXCLUDE_DB_ODBC
	#define _EXCLUDE_LOGGER
	#define _EXCLUDE_STRING
#endif

#if (VERSION_DEVPLUS < 140)
    #define _EXCLUDE_MAIN
	#define _EXCLUDE_DATA_SERVER
#endif



#if (VERSION_DEVPLUS < 140)
    #error DevPlus versions below 110 are not compatible.
#endif


//------------------------------------------------------------------------------
// CJW: If DP_SINGLETHREAD is defined, then all threading will be removed, and
//      some functionality will be tuned a little better for a single-threaded
//      application. If we find any usable define that can indicate that they
//      want a single-threaded application then we should use that also.
#ifdef DP_SINGLETHREAD 
	#define _EXCLUDE_LOCK
    #define _EXCLUDE_READWRITELOCK
    #define _EXCLUDE_THREADOBJECT
	#define _EXCLUDE_DATA_SERVER
	#define _EXCLUDE_SERVERINTERFACE
#endif


//------------------------------------------------------------------------------
// If this is not a windows compile, then disable any classes that are windows 
// specific
#ifdef _WIN32 
    #define __WIN32__
#endif
#ifndef __WIN32__
    #ifndef _EXCLUDE_WINSERVICE
        #define _EXCLUDE_WINSERVICE
    #endif
#endif


//------------------------------------------------------------------------------
// If the socket code is excluded, then DpServerInterface must be excluded also.
#ifdef _EXCLUDE_SOCKET
    #define _EXCLUDE_SERVERINTERFACE
    #define _EXCLUDE_HTTPCLIENT
#else
    #ifdef _MSC_VER 
        #ifndef _WINSOCKAPI_
            #ifdef __AFXMT_H__
                #error You have specifically included <afxmt.h> which will interfere.  You do not need it if you are using DpThreadObject for controlling your threads.
            #else
                #include <Winsock2.h>
            #endif
        #endif
    #endif
    #ifdef __DMC__
        #include <Winsock2.h>
    #endif
#endif

//------------------------------------------------------------------------------
// If we exclude the DpArgs object, then we must also exclude the DpMain.
#ifdef _EXCLUDE_ARGS
	#define _EXCLUDE_MAIN
#endif


//------------------------------------------------------------------------------
// If threadbase is excluded, then we have some other classes that will need to 
//  be excluded also.
#ifdef _EXCLUDE_THREADBASE
    #define _EXCLUDE_LOCK
    #define _EXCLUDE_THREADOBJECT
#endif

//------------------------------------------------------------------------------
// The DpHttpClient object uses the DataQueue object to pass the results of the 
// page request, so if it is excluded, then we cannot have a DpHttpClient 
// object.
#ifdef _EXCLUDE_DATAQUEUE
	#define _EXCLUDE_HTTPCLIENT
#endif

#ifdef _EXCLUDE_LOCK
	#define _EXCLUDE_READWRITELOCK
	#define _EXCLUDE_SETTINGS
	#define _EXCLUDE_THREADOBJECT
	#define _EXCLUDE_WINSERVICE
#endif

//------------------------------------------------------------------------------
// if we are excluding the ReadWriteLock functionality, we also need to...
#ifdef _EXCLUDE_READWRITELOCK
#endif


//------------------------------------------------------------------------------
// if we are excluding ThreadObject functionality...
#ifdef _EXCLUDE_THREADOBJECT
//     #define _EXCLUDE_SERVERINTERFACE
#endif


//------------------------------------------------------------------------------
// Http server object requires threads, so if we are excluding threads, then we 
// should also exclude the httpserver.
#ifdef _EXCLUDE_THREADOBJECT
	#define _EXCLUDE_HTTPSERVER
#endif

//------------------------------------------------------------------------------
// Double check our dependancies.  A user should not see this unless changes 
// have been made which break the chain of dependancy amoung the excluded 
// objects.
#ifndef _EXCLUDE_SERVERINTERFACE
	#ifndef DP_SINGLETHREAD
		#ifdef _EXCLUDE_THREADOBJECT
			#error DpServerInterface requires DpThreadObject which has been excluded.  
		#endif
		#ifdef _EXCLUDE_LOCK
			#error DpServerInterface requires DpLock which has been excluded.  
		#endif
	#endif
#endif


#ifdef _EXCLUDE_DB
	#define _EXCLUDE_DB_MYSQL
	#define _EXCLUDE_DB_MYSQL
	#define _EXCLUDE_DB_SQLITE3
#endif


#ifdef _EXCLUDE_DB_SQLITE3
	#define _EXCLUDE_DATA_SERVER
#endif


//------------------------------------------------------------------------------
// CJW: Include the required headers for each module as needed.
#ifndef _EXCLUDE_THREADBASE
    #ifdef __GNUC__
        #include <pthread.h>
    #endif
#endif




#ifndef _EXCLUDE_DB
    #ifndef _EXCLUDE_DB_ODBC
        #if (ODBCVER < 0x351)
            #error You are using an old version of ODBC headers that is not supported.
        #endif
        #ifdef __SQLTYPES
            #error __SQLTYPES already declared
        #else
            #include <sql.h>
        #endif
    
        #ifndef SQLHANDLE
            #error SQLHANDLE needs to be defined.  
            #error If you have no need for Database classes, define _EXCLUDE_DB
        #endif
    #endif
#endif

#ifndef _EXCLUDE_READWRITELOCK
    #ifdef __GNUC__
        #include <pthread.h>
    #endif
#endif
//------------------------------------------------------------------------------




//------------------------------------------------------------------------------
// Provide our assertion mapping function.  This would also depend eventually on 
// what compiler we are using to compile with.  VC++ uses a graphical assertion, 
// where DigitalMars provides an application stop assertion.
#ifndef ASSERT 
	#include <assert.h>
	#define ASSERT(x) assert(x);
#endif



// #############################################################################
// #############################################################################
// #############################################################################


//------------------------------------------------------------------------------
// DpString
//
//  This string object is used to encapsulate string handling functions.   It is
//  a simple string manipulation object.  It is not intended to be complicated,
//  or super intelligent.  It is intended to be intuitive and safe.

#ifndef _EXCLUDE_STRING

class DpString
{
    public:
        DpString();
        virtual ~DpString();
        
    protected:
    private:               
};   


#endif


// #############################################################################
// #############################################################################
// #############################################################################


//------------------------------------------------------------------------------
// DpThreadBase
//
//  This class contains some common functionality to any class that required 
//  some thread operations.   

#ifndef _EXCLUDE_THREADBASE

#ifndef DWORD
    #define DWORD unsigned long
#endif


class DpThreadBase 
{
    public:
        DpThreadBase();
        virtual ~DpThreadBase();
        
    protected:
        #ifdef __GNUC__
            void Sleep(DWORD dTime);
        #else
            void Sleep(DWORD dTime) { ::Sleep(dTime); }
        #endif

    
    private:
};

#endif



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

#ifndef _EXCLUDE_LOCK

#ifdef _EXCLUDE_THREADBASE
#error DpLock requires DpThreadBase
#endif

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

#ifndef _EXCLUDE_READWRITELOCK

#ifdef _EXCLUDE_LOCK
#error DpReadWriteLock requires DpLock
#endif

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

#endif





//-----------------------------------------------------------------------------
// DpSocket:    Socket class.
//-----------------------------------------------------------------------------
//  Developer:  Clint Webb (dev@cjdj.org)
//  
//      Socket functionality wrapper.  Basic functionality for connecting, 
//      sending and receiving data over a socket.   Not all socket features 
//      should be implemented.  Interface should be simple and used from a 
//      functional perspective rather than a direct mapping to the api.   
//      Basically, connect, send, receive and close are the public functions.
//
//-----------------------------------------------------------------------------

#ifndef _EXCLUDE_SOCKET

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
        bool Connect(char *szHost, int nPort);  
		
		// send data over the socket, return the number of chars sent, -1 if 
		// error (which could mean WOULDBLOCK), 0 if connection was closed.
        int Send(char *data, int len);
		
		// receive data on the socket.  return the number of chars received, 
		// -1 if error (which could mean WOULDBLOCK), 0 if connection was 
		// -closed.
        int Receive(char *data, int len);       
		
		virtual bool Listen(int nPort);
        
#ifndef _EXCLUDE_STRING
        bool Connect(DpString &szHost, int nPort);
        int Send(DpString &string);
        int Receive(DpString &string);
#endif      
        
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
		int Resolve(const char szAddr[], int iPort, struct sockaddr_in *pSin, char *szType="A");      
		
};

#endif





//-----------------------------------------------------------------------------
// DataQueue.h: interface for the CDataQueue class.
//-----------------------------------------------------------------------------
//  Developer:  Clint Webb (digger_clint@cjdj.org)
//  Created:    September 5, 2003
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
#ifndef _EXCLUDE_DATAQUEUE
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





    



    
    

//------------------------------------------------------------------------------
//  DpThreadObject
//
//  This object would allow a program to derive a class and it would become a 
//  thread-running class.  The derived class would require several virtual 
//  functions to be used.  These would be used to Start and Stop the thread, 
//  as well as a function that will be Run every cycle.
//
//  This object could also be used by itself so that threads can be started and controlled with a simple interface.  
//
//  For example, this class can be used in teh following examples.
//
//    Example 1:
//      class CSample1 : public CThreadObject 
//      {
//          public:
//              virtual bool ThreadRun(void);
//      };
//
//    Example 2:
//      class CSample2 
//      {
//          private:
//              CThreadObject xThreadControl;
//      };
//
//  The first example would basically make each instantiated object a thread, 
//  and will execute the ThreadRun function constantly.   The second example 
//  allows for a thread to be created and then controlled.  When the object is 
//  deleted, the thread would be informed to exit, etc.  Both samples have their 
//  benefits.
//
//------------------------------------------------------------------------------

#ifndef _EXCLUDE_THREADOBJECT

#ifdef _EXCLUDE_LOCK
#error DpThreadObject requires DpLock
#endif

// Set some defaults.  Do not modify, use the functions to set.
// default cycle time of 250 mili-seconds.
#define DP_DEFAULT_CYCLE_TIME 250   

class DpThreadObject : public DpLock
{
    public:
        DpThreadObject();
        virtual ~DpThreadObject();
        
        void SetCycleTime(DWORD dTime=DP_DEFAULT_CYCLE_TIME);
        void SetStackSize(unsigned nStackSize);
        void Start(void (*fn)(void *), void *pParam);
        
        void Stop(void);
        void WaitForThread(void);
        
		bool IsDone(void) {
			bool b = false;
			Lock();
			if (_eToStatus == dptDone || _eToStatus == dptStopped) {
				b = true;
			}
			Unlock();
			return (b);
		}
		
    protected:
        void Start(void);

        virtual void OnThreadStart(void);
        virtual void OnThreadStop(void);
        virtual void OnThreadRun(void);
        
    private:
    #ifdef __GNUC__
        static void *ThreadProcessFunction(void *pParam);
    #else
        static void ThreadProcessFunction(void *pParam);
    #endif
        void ProcessThread(void);
        void ProcessThreadCycle(void);

    private:
    #ifdef __GNUC__
        pthread_t _thread;
        pthread_attr_t _attr;
    #else
        HANDLE _hToHandle;
    #endif
        DpLock _xToLock;
        DWORD _dToCycleTime;
        
        void (*_pToFunction)(void *);
        void *_pToParam;
        
    #ifndef __GNUC__
        unsigned _nToStackSize;
    #endif
        
        enum {
			dptRunOnce,
			dptCycle
        } _eToMode;
        
        enum {
            dptUnknown,
			dptPending,
			dptStarting,
			dptStarted,
			dptRunning,
			dptWaiting,
			dptClosing,
			dptStopped,
			dptDone
        } _eToStatus;
        
        enum {
			dptControlRun,
			dptControlStop
        } _eToControl;
};

#endif

//------------------------------------------------------------------------------
// DpServerInterface: interface for the DpServerInterface class.
//------------------------------------------------------------------------------
//  Developer:  Clint Webb (cjw_dev@cjdj.org)
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

#ifndef _EXCLUDE_SERVERINTERFACE

class DpServerInterface : public DpThreadObject
{

	public:
		DpServerInterface();                    	// Constructor.
		virtual ~DpServerInterface();           	// Deconstructor.
	
	public:
		virtual void OnAccept(SOCKET nSocket) = 0;  // Function that will handle the new connection received 
													// on the socket.   Must be resolved for class to be
													// instantiated.

#ifndef DP_SINGLETHREAD
		virtual void OnIdle(void);                  // Called on each cycle when nothing was accepted.
#endif
	
	public:
		virtual bool Listen(int nPort);             // Start listening on a socket.
		
#ifndef DP_SINGLETHREAD		
	protected:
		virtual void OnThreadRun(void); 		   // Worker thread that processes the incoming connections.

	protected:
		virtual void AddObject(DpThreadObject *pObject);
		virtual void CheckList(void);
	
	private:
		DpSocket        _xSocket;
		DpThreadObject **_pList;
		int              _nItems;
		
#endif // DP_SINGLETHREAD
};

#endif

//------------------------------------------------------------------------------
// CBaseSqlDB
//------------------------------------------------------------------------------
//  Developer:  Clint Webb (cjw_dev@cjdj.org)
//  Created:    2004-02-10
//  
//      This is the base class for Database objects.  This is being created in 
//      order to join the functionality of the ODBC based class, and the MySQL 
//      Api based class into a common base hierarchy.
//
//------------------------------------------------------------------------------

#ifndef _EXCLUDE_DB





#ifndef _EXCLUDE_DB_SQLITE3

#include <sqlite3.h>

class DpSqlite3Result
{
	public:
		DpSqlite3Result();
		virtual ~DpSqlite3Result();

		void SetResult(int rows, int cols, char **results, char *szErr, int insert);
		bool NextRow();
		
		int GetInt(char *name);
		char *GetStr(char *name);
		
		int GetInsertID(void);
		
	private:
		int    _nRow;
		int    _nColumns;
		int    _nRows;
		char **_szResult;
		char  *_szErr;
		int    _nInsertID;
};


class DpSqlite3
{
	public:
		DpSqlite3();
		virtual ~DpSqlite3();

		bool Open(char *szFilename);
		void ExecuteNR(char *query, ...);
		DpSqlite3Result * Execute(char *query, ...);
		DpSqlite3Result * ExecuteStr(char *query);
		
		int LastResultCode(void) { return _nLastResultCode; }
		
		void Begin(bool bNow=false);
		void Commit();

	protected:

	private:
		sqlite3 *_pDb;
		int _nLastResultCode;
	
};

#endif

#ifndef _EXCLUDE_DB_MYSQL

#include <stdlib.h>

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
#ifndef _EXCLUDE_LOCK
        DpLock _xLock;
#endif
};



#include <mysql/mysql.h>

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


#ifndef _EXCLUDE_DB_ODBC


struct _SQL_ColumnData {
    SQLSMALLINT  ColumnNumber;
    SQLCHAR     *ColumnName;
    SQLSMALLINT  DataType;
    SQLUINTEGER  ColumnSize;
    SQLSMALLINT  DecimalDigits;
    SQLSMALLINT  Nullable;
};


//-----------------------------------------------------------------------------
// CJW: Result Set from an SQL query.   Every Executed statement should return 
//      a result object which has all the information about the result.
//      When the statement handle is passed to this object, it gets information 
//      about the rows and items.  
class CSqlResult : public CBaseSqlResult
{

public:
    char * LastState();
    char * LastText();
    SQLHANDLE GetHandle();
    SQLRETURN Status();
    bool GetData(const char *field, char **value);
    bool GetData(const char *field, int *value);

    bool NextRow();
    unsigned int Items();
    void Process(SQLRETURN ret, char *state=NULL, char *text=NULL);

    CSqlResult(void *pParent);
    virtual ~CSqlResult();

private:
    char * _lastErrorText;
    char * _lastState;
    unsigned int _nCurrentRow;         // 1 based... 1 is first row.
    _SQL_ColumnData *_pColumnData;
    SQLSMALLINT _nColumns;
    void * _pParent;
    SQLHANDLE _hStatement;
    SQLRETURN _lastReturn;
};


// CJW: Database object.
class CSqlDB : public CBaseSqlDB
{
public:
    bool Commit();
    void DeleteResult(CSqlResult *ptr);
    SQLHANDLE GetHandle();
    bool Connected();
    CSqlResult * Execute(char *query, ...);
    CSqlResult * ExecuteStr(char *query);
    void Close();
    virtual bool Connect(char *source, char *user, char *pass);
    CSqlDB();
    virtual ~CSqlDB();


private:
    CSqlResult * NewResult();

private:
    bool _bConnected;
    SQLHANDLE _hEnv;
    SQLHANDLE _hConnection;
    CSqlResult **_pResultList;
    unsigned int _nItems;
};


#endif // not _EXCLUDE_DB_ODBC
#endif // not _EXCLUDE_DB
//------------------------------------------------------------------------------



// ############################################################################
// ############################################################################
// ############################################################################


#ifndef _EXCLUDE_DATA_SERVER

#ifdef _EXCLUDE_DB
#error Database support is required.
#endif

#ifdef _EXCLUDE_DB_SQLITE3
#error SQLITE support is required
#endif

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


// ############################################################################
// ############################################################################
// ############################################################################


//------------------------------------------------------------------------------
// DpSettings - generic global settings holder.  Useful for accessing settings
// information from multiple threads.  Not really designed to transfer transient
// information from one thread to another.
//------------------------------------------------------------------------------
#ifndef _EXCLUDE_SETTINGS

enum SettingType {
    TYPE_EMPTY,
    TYPE_INTEGER,
    TYPE_STRING,
    TYPE_CHAR,
    TYPE_UNSIGNED,
    TYPE_PTR
};


union SettingsUnion {
    void *pointer;
    char *string;
    int  integer;
    unsigned int uint;
    char ch;
};


struct SettingsData {
    enum SettingType eType;
    union SettingsUnion *pData;
    int nItems;
    char *szName;
};  


class DpSettings : public DpLock
{
    public:
        DpSettings();
        virtual ~DpSettings();
        
        bool Set(char *name, char *value, int index=0);
        bool Set(char *name, int   value, int index=0);
        
        bool Get(char *name, char  *value, int index=0);
        bool Get(char *name, char **value, int index=0);
        bool Get(char *name, int   *value, int index=0);
        
    protected:
    
    private:
        static int _nInstances;
        static int _nItems;
        static SettingsData *_pDataList;
        
        int FindName(char *name);
        int CreateName(char *name);
        bool VerifyIndex(int entry, int index);
        void CreateIndex(int entry, int index);

};
#endif
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
// DpLogger - This class provides a comprehensive method to log test and result 
//            data to log files, consoles, etc.
//------------------------------------------------------------------------------
#ifndef _EXCLUDE_LOGGER

#ifdef _EXCLUDE_THREADOBJECT
class DpLogger 
#else
class DpLogger : public DpThreadObject
#endif
{
    public:
        DpLogger();
        virtual ~DpLogger();
        
        CreateConsole(BYTE mask);
        
    protected:
    private:
    
};

#endif
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
// DpIniFile - This class provides an easy to use interface used to get 
//             configuration and values from an INI style file.
//------------------------------------------------------------------------------
#ifndef _EXCLUDE_INI


struct DpIniFileItemData
{
    char *szName;
    char *szValue;
};

class DpIniFileGroup 
{
    public:
        DpIniFileGroup();
        virtual ~DpIniFileGroup();
        
        void SetGroup(char *str);
        void Add(char *name, char *value);
        char *GroupName(void);
        char *GetValue(char *name);
        
    protected:
    private:
        char *_szGroup;
        int _nItems;
        struct DpIniFileItemData *_pItemList;
};


class DpIniFile 
{
    public:
        DpIniFile();
        virtual ~DpIniFile();
        
        bool Load(char *path);
        bool SetGroup(char *grp);
        bool GetValue(char *name, int   *value);
        bool GetValue(char *name, char **value);
        
    protected:
    private:
        DpIniFileGroup **_pGroupList;
        int _nGroups;
        int _nCurrentGroup;
};

#endif
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
#ifndef _EXCLUDE_WINSERVICE

#ifdef _EXCLUDE_LOCK
    #error DpWinService requires DpLock
#endif

class DpWinService 
{
    public:
        DpWinService();
        ~DpWinService();

        bool Start();
        bool Stop();

        virtual bool Install();


    protected:
        void SetServiceName(const char *name);

        virtual void Run(void);


    private:
        char *_szServiceName;
        DpLock _xLock;

};

#endif

//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
// CJW: This class provides and easy to use and intuitive HTTP interface client 
//      object.
//------------------------------------------------------------------------------
#ifndef _EXCLUDE_HTTPCLIENT

#define DP_MAX_PACKET_SIZE 4096
#define DP_MAX_HOST_LEN 255

#ifndef _EXCLUDE_THREADBASE
class DpHttpClient : public DpThreadBase
#else
class DpHttpClient
#endif
{
    // functions, and methods.
    public:
        DpHttpClient();
        virtual ~DpHttpClient();

		void SetURL(char *url);
// 		void SetMethod(char *method);
		void AddParam(char *name, char *value);
		void AddParam(char *name, int value);
		char * GetURL(void);
				
        int Get(char *url=NULL);
        int GetLength(void) { return(_nLength); }
        int GetCode(void)   { return(_nCode);   }
		DpDataQueue * GetBody();
		
		char * Encode(char *str);

    protected:

    private:
        bool ParseUrl(void);
        void BuildHeader(void);
        bool SendHeader(void);
        bool ReceiveHeader(void); 
        void Sleep(void);

    // data and objects.
    public:

    protected:

    private:
        DpSocket _xSocket;
        DpDataQueue _xQueue;
        char *_szUrl;
        char *_szHeaders;
		char *_pBody;
        char  _szHost[DP_MAX_HOST_LEN+1];
		int   _nPort;
		int   _nCode, _nLength;
};

#endif
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
#ifndef _EXCLUDE_CGI

struct _DpCgiFormItem 
{
	char *name;
	char *value;
};

class DpCgiFormData
{
	public:
		char * operator()(char* key);
		char * Get(char *key);

	public:
		DpCgiFormData();
		virtual ~DpCgiFormData();
		
		void Process(void);
		void ProcessData(char *query);
		
	private:
		unsigned int GetHex(char ch);
		void AddItem(char *key, char *value);
		
	private:
		struct _DpCgiFormItem *_pItemList;
		unsigned int _nItems;
};

class DpCgi
{
	public:
		DpCgi();
		virtual ~DpCgi();
		
	public:
		void ContentType(char *tt="text/html");
		
	private:
		bool _bSetContentType;
};


#endif
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
// Start a server listening and responding to either page requests, or respond 
// directly to requests.
#ifndef _EXCLUDE_HTTPSERVER

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
//------------------------------------------------------------------------------




//------------------------------------------------------------------------------
#ifndef _EXCLUDE_TEXTTOOLS

#define TEXT_MAX_LINE_LENGTH    4096

struct DpTextLine_t {
    char *data;
    int length;
};


class DpTextTools 
{
    // Methods, functions.
    public:
        DpTextTools();
        ~DpTextTools();

        bool Load(char *buffer);
        bool LoadFromFile(char *file);

        int FindLine(char *txt);
        int MoveNextLine(void);
        int TrimBefore(void);
        int DeleteToEnd(void);
        int MoveFirstLine(void);
        int InsertLineOnStr(char *str);
        int InsertNewLine(void);
        void RemoveHtmlOnLine(bool spaces);
        int DeleteFromLine(char *str);

        char **GetWordArray(void);

    protected:

    private:
        void ClearWordArray(void);


    // Data, internal objects.
    public:
    protected:
    private:
        DpTextLine_t *_pData;
        int           _nItems;
        char         *_pWordArray;
        int           _nCurrentLine;
        int           _nCurrentChar;
};

#endif
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
#ifndef _EXCLUDE_ARGS

class DpArgs 
{
	public:
		DpArgs();
		virtual ~DpArgs();
		
		void Process(int argc, char **argv);
		
	protected:
		static int _argc;
		static char **_argv;
};

#endif



//------------------------------------------------------------------------------
// DpMain can be used to provide a common structure to applications.  It 
// provides a method to start and stop daemons in a controlled manner, and also 
// provides support for run-thru applications.   
#ifndef _EXCLUDE_MAIN

#ifdef _EXCLUDE_ARGS
#error DpMain requires DpArgs
#endif

#ifndef DWORD
#define DWORD unsigned long
#endif


struct DpTimerStruct 
{
	int nTimerID;	// ID of the timer.  0 indicates timer has been stopped.
	DWORD nTime;	// in miliseconds.  0 indicates timer has been stopped.
	DWORD nLeft;	// time in miliseconds that is left before this timer is triggered again.
};

#ifndef _EXCLUDE_THREADBASE
class DpMain : public DpThreadBase 
#else
class DpMain
#endif
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
				
		#ifndef _EXCLUDE_LOCK
		static DpLock _Lock;
		#endif
		
};

#endif
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// Sometimes we need to convert text to and from base64.  
#ifndef _EXCLUDE_BASE64

class DpBase64 
{
	private:
		static char _szCodes[65];
		static unsigned char _szReverse[128];
		static bool _bInit;
		char *_pEncoded;
		unsigned char *_pDecoded;
	protected:
	public:
		
	public:
		DpBase64();
		virtual ~DpBase64();
		
		unsigned char * Decode(char *str, int *length);
		char * Encode(unsigned char *str, int length);
		char * EncodeStr(char *str);

		
	protected:
	private:
		void Clear(void);
		void EncodeBlock(unsigned char *in, char *out, int len);
		void DecodeBlock(char *in, unsigned char *out, int *len);
};

#endif
//------------------------------------------------------------------------------




//------------------------------------------------------------------------------
// This object is used to   
#ifndef _EXCLUDE_PIPECOMMAND

class DpPipeCommand 
{
	private:
	protected:
	public:
		
};

#endif	// _EXCLUDE_PIPECOMMAND




//------------------------------------------------------------------------------
#endif
