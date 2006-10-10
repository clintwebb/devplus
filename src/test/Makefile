# makefile

DEVPLUS=-I. -I.. -D_EXCLUDE_INI -D_EXCLUDE_TEXTTOOLS -D_EXCLUDE_DB_MYSQL 
# DEVPLUS=-I. -D_EXCLUDE_DATAQUEUE -D_EXCLUDE_DB_MYSQL -D_EXCLUDE_INI -DDP_SINGLETHREAD -D_EXCLUDE_SOCKET -D_EXCLUDE_TEXTTOOLS

all: test-httpserver test-sqlite test-httpclient

LIBS=-lpthread -lsqlite3


test-httpserver: httpserver.o DevPlus.o
	g++ -o test-httpserver httpserver.o DevPlus.o $(LIBS) $(DEVPLUS)

test-httpclient: httpclient.o DevPlus.o
	g++ -o test-httpclient httpclient.o DevPlus.o $(LIBS) $(DEVPLUS)

test-sqlite: sqlite.o DevPlus.o
	g++ -o test-sqlite sqlite.o DevPlus.o $(LIBS) $(DEVPLUS)


httpserver.o: httpserver.cpp ../DevPlus.h
	g++ -c -o httpserver.o httpserver.cpp $(DEVPLUS)

httpclient.o: httpclient.cpp ../DevPlus.h
	g++ -c -o httpclient.o httpclient.cpp $(DEVPLUS)

sqlite.o: sqlite.cpp ../DevPlus.h
	g++ -c -o sqlite.o sqlite.cpp $(DEVPLUS)


DevPlus.o: ../DevPlus.cpp ../DevPlus.h
	g++ -c -o DevPlus.o ../DevPlus.cpp $(DEVPLUS)



clean:
	@-rm *.o
