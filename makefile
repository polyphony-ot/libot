CC=clang
CFLAGS=-std=c99 -Wall -funsigned-char -pedantic -fpic
AR=ar
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
BIN=bin
LIB=libot.a
SONAME=libot.so.0
SOSUFFIX=.0.1
EXESUFFIX=
TESTRUNNER=

ifndef OS
OS:=$(shell uname)
endif

ifdef COVERAGE
CFLAGS += -coverage
endif

ifdef LLVM
EMITLLVM=-emit-llvm
LLC=llc
LLVMLINK=llvm-link
endif

all: debug release test

debug: $(SOURCES)
	$(CC) $(CFLAGS) $(EMITLLVM) -c -g -Icjson $(SOURCES)
	mkdir -p $(BIN)/$@
ifdef LLVM
	$(LLVMLINK) -o $(BIN)/$@/libot.bc *.bc
	$(LLC) -filetype=obj -o libot.o $(BIN)/$@/libot.bc
	rm *.bc
endif
	$(AR) rs $(BIN)/$@/$(LIB) *.o
ifeq ($(OS), Darwin)
	$(CC) $(CFLAGS) -g -shared -Wl,-install_name,$(SONAME) -o $(BIN)/$@/$(SONAME)$(SOSUFFIX) *.o
endif
	rm *.o

release: $(SOURCES)
	$(CC) $(CFLAGS) $(EMITLLVM) -DNDEBUG -c -O3 -Icjson $(SOURCES)
	mkdir -p $(BIN)/$@
ifdef LLVM
	$(LLVMLINK) -o $(BIN)/$@/libot.bc *.bc
	$(LLC) -filetype=obj -O3 -o libot.o $(BIN)/$@/libot.bc
	rm *.bc
endif
	$(AR) rs $(BIN)/$@/$(LIB) *.o
ifeq ($(OS), Darwin)
	$(CC) $(CFLAGS) -shared -Wl,-install_name,$(SONAME) -o $(BIN)/$@/$(SONAME)$(SOSUFFIX) *.o
endif
	rm *.o

test: debug test/libot_test.c
	$(CC) $(CFLAGS) -g -Icjson -o "$(BIN)/debug/test$(EXESUFFIX)" test/libot_test.c $(BIN)/debug/$(LIB)
	$(TESTRUNNER) $(BIN)/debug/test$(EXESUFFIX)
ifdef COVERAGE
	@echo "Code coverage is temporarily disabled due to an incompatibility between lcov and Apple's (buggy) version of gcov."
	rm *.gcno *.gcda
endif

clean:
	rm -rf $(BIN) *.gcno *.gcda *.o *.bc
