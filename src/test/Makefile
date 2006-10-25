# makefile

all: test-httpserver test-sqlite test-httpclient test-pipe

LIBS=-lpthread -ldevplus -ldevplus-thread -ldevplus-main


test-httpserver: httpserver.cpp 
	g++ -o test-httpserver httpserver.cpp $(LIBS)

test-httpclient: httpclient.cpp
	g++ -o test-httpclient httpclient.cpp $(LIBS)

test-sqlite: sqlite.cpp
	g++ -o test-sqlite sqlite.cpp $(LIBS) -lsqlite3 -ldevplus-sqlite

test-pipe: pipe.cpp
	g++ -o test-pipe pipe.cpp $(LIBS)



clean:
	@-rm *.o
