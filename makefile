CC=clang
CFLAGS=-std=c99 -Wall -funsigned-char -pedantic
SOURCES=\
	array.c \
	client.c \
	compose.c \
	hex.c \
	ot.c \
	otdecode.c \
	otencode.c \
	server.c \
	xform.c \
	cjson/cjson.c
OBJDIR=obj
BIN=bin
LIB=libot.a

ifdef COVERAGE
CFLAGS += -coverage
endif

all: clean debug release test

debug: $(SOURCES)
	$(CC) $(CFLAGS) -c -g -O0 -Icjson $(SOURCES)
	mkdir -p $(BIN)/debug
	ar rs $(BIN)/debug/$(LIB) *.o
	rm *.o

release: $(SOURCES)
	$(CC) $(CFLAGS) -DNDEBUG -c -O3 -Icjson $(SOURCES)
	mkdir -p $(BIN)/release
	ar rs $(BIN)/release/$(LIB) *.o
	rm *.o

test: debug test/libot_test.c
	$(CC) $(CFLAGS) -g -O0 -Icjson -o "$(BIN)/runtests" test/libot_test.c $(BIN)/debug/$(LIB)
	$(BIN)/runtests
ifdef COVERAGE
	lcov --capture --directory . --output-file $(BIN)/coverage.info --rc lcov_branch_coverage=1
	genhtml $(BIN)/coverage.info --output-directory $(BIN)/coverage --function-coverage --branch-coverage
	rm *.gcno *.gcda
endif

clean:
	rm -rf $(BIN) *.gcno *.gcda *.o
