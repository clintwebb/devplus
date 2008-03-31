# makefile

all: test-httpserver test-sqlite test-httpclient test-pipe test-socketex test-timestream

LIBS=-lpthread -ldevplus-thread -ldevplus-main -ldevplus 


test-httpserver: httpserver.cpp 
	g++ -o test-httpserver httpserver.cpp $(LIBS)

test-httpclient: httpclient.cpp
	g++ -o test-httpclient httpclient.cpp $(LIBS)

test-sqlite: sqlite.cpp
	g++ -o test-sqlite sqlite.cpp $(LIBS) -lsqlite3 -ldevplus-sqlite

test-pipe: pipe.cpp
	g++ -o test-pipe pipe.cpp $(LIBS)

test-socketex: socketex.cpp
	g++ -o $@ socketex.cpp $(LIBS)

test-timestream: timestream.cpp
	g++ -o $@ timestream.cpp $(LIBS)



clean:
	@-rm *.o
	@-rm test-timestream
	@-rm test-socketex

