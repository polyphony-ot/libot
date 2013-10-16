CC=clang
CFLAGS=-std=c99 -Wall -funsigned-char -pedantic
SOURCES=array.c hex.c ot.c otdecode.c otencode.c cjson/cjson.c
OBJDIR=obj
BIN=bin
LIB=libot.a

ifdef COVERAGE
CFLAGS += -coverage
endif

debug: $(SOURCES)
	$(CC) $(CFLAGS) -c -g -O0 -Icjson $(SOURCES)
	mkdir -p $(BIN)
	ar rs bin/$(LIB) *.o
	rm *.o

release: $(SOURCES) clean
	$(CC) $(CFLAGS) -c -O4 -Icjson $(SOURCES)
	mkdir -p $(BIN)
	ar rs bin/$(LIB) *.o
	rm *.o

test: debug test/libot_test.c
	$(CC) $(CFLAGS) -g -O0 -Icjson -o "$(BIN)/runtests" test/libot_test.c $(BIN)/$(LIB)
	$(BIN)/runtests

clean:
	rm -rf $(BIN) *.gcno *.gcda *.o