all: libdevplus-thread.a

libdevplus-thread.a: DevPlus.o
	ar -r libdevplus-thread.a
	ar -r libdevplus-thread.a DevPlus.o


DevPlus.o: DevPlus.cpp DevPlus.h
	gcc -c DevPlus.cpp -I. -o DevPlus.o


clean:
	@-rm DevPlus.o
	@-rm libdevplus-thread.a


install: libdevplus-thread.a DevPlus.cpp DevPlus.h COPYING AUTHORS docs/DevPlus-DevGuide.pdf
	-mkdir /usr/src/DevPlus
	-mkdir /usr/src/DevPlus/docs
	mv libdevplus-thread.a /usr/lib
	cp DevPlus.h /usr/include
	cp DevPlus.h /usr/src/DevPlus
	cp DevPlus.cpp /usr/src/DevPlus
	cp COPYING /usr/src/DevPlus
	cp AUTHORS /usr/src/DevPlus
	cp docs/DevPlus-DevGuide.pdf /usr/src/DevPlus/docs

remove:
	@-rm /usr/src/DevPlus/docs/DevPlus-DevGuide.pdf
	@-rm /usr/src/DevPlus/AUTHORS
	@-rm /usr/src/DevPlus/COPYING
	@-rm /usr/src/DevPlus/DevPlus.h
	@-rm /usr/src/DevPlus/DevPlus.cpp
	@-rm /usr/include/DevPlus.h
	@-rm /usr/lib/libdevplus-thread.a
	@-rmdir /usr/src/DevPlus/docs
	@-rmdir /usr/src/DevPlus




