LIBS=libdevplus.a libdevplus-thread.a libdevplus-main.a libdevplus-sqlite.a libdevplus-mysql.a 


all: $(LIBS)


H_DevPlus=DevPlus.h

H_DataQueue=DpDataQueue.h $(H_DevPlus)
H_String=DpString.h $(H_DevPlus)
H_Base64=DpBase64.h $(H_DevPlus)
H_IniFile=DpIniFile.h $(H_DevPlus)
H_Socket=DpSocket.h $(H_DevPlus)
H_TextTools=DpTextTools.h $(H_DevPlus)

H_Cgi=DpCgi.h $(H_DevPlus)
H_HttpClient=DpHttpClient.h $(H_Socket) $(H_DataQueue) $(H_DevPlus)
H_ThreadBase=DpThreadBase.h $(H_DevPlus)
H_Lock=DpLock.h $(H_ThreadBase) $(H_DevPlus)
H_Settings=DpSettings.h $(H_Lock) $(H_DevPlus)
H_ThreadObject=DpThreadObject.h $(H_Lock) $(H_DevPlus)
H_ServerInterface=DpServerInterface.h $(H_Socket) $(H_ThreadObject) $(H_DevPlus)
H_HttpServer=DpHttpServer.h $(H_ServerInterface) $(H_Socket) $(H_Lock) $(H_Cgi) $(H_DataQueue) $(H_DevPlus)
H_Logger=DpLogger.h $(H_DevPlus)
H_PipeCommand=DpPipeCommand.h $(H_ThreadObject) $(H_DevPlus) 
H_ReadWriteLock=DpReadWriteLock.h $(H_Lock) $(H_DevPlus)


H_Args=DpArgs.h $(H_DevPlus)
H_Main=DpMain.h $(H_Lock) $(H_DevPlus)

H_Sqlite3=DpSqlite3.h $(H_DevPlus)
H_DataServer=DpDataServer.h $(H_Sqlite3) $(H_ThreadObject) $(H_Lock) $(H_DevPlus)

H_MySql=DpMySql.h $(H_Lock) $(H_DevPlus)

libdevplus.a: DpDataQueue.o DpString.o DpBase64.o DpIniFile.o DpSocket.o DpTextTools.o 
	@-rm $@>/dev/null
	ar -r $@
	ar -r $@ $^

libdevplus-thread.a: DpCgi.o DpHttpClient.o DpThreadBase.o DpLock.o DpSettings.o DpThreadObject.o DpServerInterface.o DpHttpServer.o DpLogger.o DpPipeCommand.o DpReadWriteLock.o 
	@-rm $@>/dev/null
	ar -r $@
	ar -r $@ $^


libdevplus-main.a: DpArgs.o DpMain.o 
	@-rm $@>/dev/null
	ar -r $@
	ar -r $@ $^


libdevplus-sqlite.a: DpSqlite3.o DpDataServer.o  
	@-rm $@>/dev/null
	ar -r $@
	ar -r $@ $^


libdevplus-mysql.a: DpMySql.o 
	@-rm $@>/dev/null
	ar -r $@
	ar -r $@ $^


DpArgs.o: DpArgs.cpp $(H_Args)
	gcc -c DpArgs.cpp -I. -o $@

DpBase64.o: DpBase64.cpp $(H_Base64)
	gcc -c DpBase64.cpp -I. -o $@

DpCgi.o: DpCgi.cpp $(H_Cgi)
	gcc -c DpCgi.cpp -I. -o $@

DpDataQueue.o: DpDataQueue.cpp $(H_DataQueue)
	gcc -c DpDataQueue.cpp -I. -o $@

DpDataServer.o: DpDataServer.cpp $(H_DataServer)
	gcc -c DpDataServer.cpp -I. -o $@

DpHttpClient.o: DpHttpClient.cpp $(H_HttpClient)
	gcc -c DpHttpClient.cpp -I. -o $@

DpHttpServer.o: DpHttpServer.cpp $(H_HttpServer)
	gcc -c DpHttpServer.cpp -I. -o $@

DpIniFile.o: DpIniFile.cpp $(H_IniFile)
	gcc -c DpIniFile.cpp -I. -o $@

DpLock.o: DpLock.cpp $(H_Lock)
	gcc -c DpLock.cpp -I. -o $@

DpLogger.o: DpLogger.cpp $(H_Logger)
	gcc -c DpLogger.cpp -I. -o $@

DpMain.o: DpMain.cpp $(H_Main) $(H_Args)
	gcc -c DpMain.cpp -I. -o $@

DpMySql.o: DpMySql.cpp $(H_MySql)
	gcc -c DpMySql.cpp -I. -o $@

DpPipeCommand.o: DpPipeCommand.cpp $(H_PipeCommand)
	gcc -c DpPipeCommand.cpp -I. -o $@

DpReadWriteLock.o: DpReadWriteLock.cpp $(H_ReadWriteLock)
	gcc -c DpReadWriteLock.cpp -I. -o $@

DpServerInterface.o: DpServerInterface.cpp $(H_ServerInterface)
	gcc -c DpServerInterface.cpp -I. -o $@

DpSettings.o: DpSettings.cpp $(H_Settings)
	gcc -c DpSettings.cpp -I. -o $@

DpSocket.o: DpSocket.cpp $(H_Socket)
	gcc -c DpSocket.cpp -I. -o $@

DpSqlite3.o: DpSqlite3.cpp $(H_Sqlite3)
	gcc -c DpSqlite3.cpp -I. -o $@

DpString.o: DpString.cpp $(H_String)
	gcc -c DpString.cpp -I. -o $@

DpTextTools.o: DpTextTools.cpp $(H_TextTools)
	gcc -c DpTextTools.cpp -I. -o $@

DpThreadBase.o: DpThreadBase.cpp $(H_ThreadBase)
	gcc -c DpThreadBase.cpp -I. -o $@

DpThreadObject.o: DpThreadObject.cpp $(H_ThreadObject)
	gcc -c DpThreadObject.cpp -I. -o $@


clean:
	@-rm *.o
	@-rm libdevplus*.a


install: $(LIBS) COPYING AUTHORS docs/DevPlus-DevGuide.pdf
	-mkdir $(PREFIX)/usr/src/DevPlus
	-mkdir $(PREFIX)/usr/src/DevPlus/docs
	cp libdevplus.a $(PREFIX)/usr/lib
	cp libdevplus-thread.a $(PREFIX)/usr/lib
	cp libdevplus-main.a $(PREFIX)/usr/lib
	cp libdevplus-sqlite.a $(PREFIX)/usr/lib
	cp libdevplus-mysql.a $(PREFIX)/usr/lib
	cp *.h $(PREFIX)/usr/include
	cp *.h $(PREFIX)/usr/src/DevPlus
	cp *.cpp $(PREFIX)/usr/src/DevPlus
	cp COPYING $(PREFIX)/usr/src/DevPlus
	cp AUTHORS $(PREFIX)/usr/src/DevPlus
	cp docs/DevPlus-DevGuide.pdf $(PREFIX)/usr/src/DevPlus/docs

remove:
	@-rm $(PREFIX)/usr/src/DevPlus/docs/DevPlus-DevGuide.pdf
	@-rm $(PREFIX)/usr/src/DevPlus/AUTHORS
	@-rm $(PREFIX)/usr/src/DevPlus/COPYING
	@-rm $(PREFIX)/usr/src/DevPlus/DevPlus.h
	@-rm $(PREFIX)/usr/src/DevPlus/DevPlus.cpp
	@-rm $(PREFIX)/usr/include/DevPlus.h
	@-rm $(PREFIX)/usr/lib/libdevplus-thread.a
	@-rm $(PREFIX)/usr/lib/libdevplus-mysql.a
	@-rmdir $(PREFIX)/usr/src/DevPlus/docs
	@-rmdir $(PREFIX)/usr/src/DevPlus




