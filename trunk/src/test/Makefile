# makefile

DEVPLUS=-I. -I.. 

all: test-httpserver test-sqlite test-httpclient

LIBS=-lpthread -lsqlite3 -ldevplus -ldevplus-thread -ldevplus-sqlite


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
