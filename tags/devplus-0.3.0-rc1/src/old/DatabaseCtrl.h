// DatabaseCtrl.h: interface for the CDatabaseCtrl class.
//
// This object uses static data members so that it can be instantiated 
// and used in mutlipe threads and locations while maintaining a 
// single database connection.  If the connection is busy then it will 
// remain locked until the database call is completed.  Therefore it 
// is important to release the database object as soon as it is no 
// longer needed.
//
// Each database object is expected to handle only ONE query or 
// interaction at a time.  Although multiple CDatabaseCtrl objects can 
// be instantiated within a thread.  Each will maintain its own 
// current record set.  Each individual database operation will be 
// encased in a lock though so that only one database operation is 
// being performed at a time.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DATABASECTRL_H__04C8B578_5738_4C2D_964A_388019139F08__INCLUDED_)
#define AFX_DATABASECTRL_H__04C8B578_5738_4C2D_964A_388019139F08__INCLUDED_

//#include <winsock2.h>
#include <afxmt.h>
#include "include/mysql.h"


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CDatabaseCtrl  
{

private:
	static CCriticalSection xLock;
	static CCriticalSection xServerLock;
	static MYSQL *hSql;


// Internal data for this instance.  These are not shared among other objects of this type.
private:
	my_ulonglong nInsertID;
	bool bTablesLocked;
	MYSQL_RES *result;
	MYSQL_ROW row;
	MYSQL_FIELD *fieldList;
	unsigned int nFields;



public:
	bool Use(char *db);
	char * Quote(char *str, unsigned int length);
	my_ulonglong GetInsertID();
	bool UnlockTables();
	bool LockTable(char *table);
	bool GetData(char *field, unsigned int *value);
	bool GetData(char *field, short *value);
	bool GetData(char *field, int *value);
	bool GetData(char *field, unsigned long *value);
	bool GetData(char *field, char *value, int buflen);
	bool GetData(char *field, char **value);
	bool NextRow(void);
	bool Execute(char *query, ...);
	bool Disconnect();
	bool Connect(char *server, char *user, char *password, char *db=NULL);
	CDatabaseCtrl();
	virtual ~CDatabaseCtrl();

};

#endif // !defined(AFX_DATABASECTRL_H__04C8B578_5738_4C2D_964A_388019139F08__INCLUDED_)
