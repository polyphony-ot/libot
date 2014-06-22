# Makefile for building libot.
#
# All build artifacts will be placed somewhere within the "bin" directory. The
# main debug and release targets build a statically linked archive. A shared
# library is also built if your OS is supported (currently only Darwin).
#
# Main targets:
# 	debug - performs a debug build with symbols and no optimizations.
# 	release - performs a release build with all optimizations enabled.
# 	test - performs a debug build and then runs all unit tests against it.
# 	clean

CC=clang
CFLAGS=-std=c99 -Wall -Wextra -funsigned-char -pedantic -fpic
AR=ar

# List of all the sources belonging to the core library (no tests).
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
	sha1.c \
	doc.c \
	cjson/cjson.c

# List of sources for test scenarios.
SCENARIOS=$(wildcard test/scenarios/*.c)

# Output directory where binaries and build artifacts will be placed.
BIN=bin

# Filename to use for the statically linked archive.
LIB=libot.a

# Soname to use for the shared library. Note that soname is the Unix term, but
# this variable applies to other OSes as well (e.g., on Darwin it's called
# "install_name").
SONAME=libot.so.0

# Suffix to append to SONAME to create the shared library's filename.
SOSUFFIX=.0.1

# Optional variables that can be set when running tests. EXESUFFIX is a suffix
# to append to any executables (you may want to set this to ".exe" on Windows).
# TESTRUNNER is a command to use when running tests. For example, if
# TESTRUNNER=time then tests will be run as "time bin/debug/test".
EXESUFFIX=
TESTRUNNER=

# The OS variable is used when figuring out how to build shared libraries. If OS
# isn't explicitly set, then we try to set it using uname. If the OS is unknown,
# then a shared library will not be built.
ifndef OS
OS:=$(shell uname)
endif

# COVERAGE can be set to enable code coverage profiling with gcov.
ifdef COVERAGE
CFLAGS += -coverage
endif

all: debug release test

# Debug targets #

$(BIN)/debug/$(LIB): $(SOURCES)
	$(CC) $(CFLAGS) -c -g -Icjson $(SOURCES)
	mkdir -p $(BIN)/debug
	$(AR) rs $(BIN)/debug/$(LIB) *.o
ifeq ($(OS), Darwin)
	$(CC) $(CFLAGS) -g -shared -Wl,-install_name,$(SONAME) \
	-o $(BIN)/debug/$(SONAME)$(SOSUFFIX) *.o
endif
	rm *.o

debug: $(BIN)/debug/$(LIB)

# Release targets #

$(BIN)/release/$(LIB): $(SOURCES)
	$(CC) $(CFLAGS) -DNDEBUG -c -O3 -Icjson $(SOURCES)
	mkdir -p $(BIN)/release
	$(AR) rs $(BIN)/release/$(LIB) *.o
ifeq ($(OS), Darwin)
	$(CC) $(CFLAGS) -shared -Wl,-install_name,$(SONAME) \
	-o $(BIN)/release/$(SONAME)$(SOSUFFIX) *.o
endif
	rm *.o

release: $(BIN)/release/$(LIB)

# Test targets #

$(BIN)/debug/scenarios$(EXESUFFIX): $(BIN)/debug/$(LIB) $(SCENARIOS) \
	test/scenarios/scenario.h
	$(CC) $(CFLAGS) -g -Icjson -o "$(BIN)/debug/scenarios$(EXESUFFIX)" \
	$(BIN)/debug/$(LIB) $(SCENARIOS)

$(BIN)/debug/test$(EXESUFFIX): $(BIN)/debug/$(LIB) test/libot_test.c
	$(CC) $(CFLAGS) -g -Icjson -o "$(BIN)/debug/test$(EXESUFFIX)" \
	test/libot_test.c $(BIN)/debug/$(LIB)

test: $(BIN)/debug/test$(EXESUFFIX) $(BIN)/debug/scenarios$(EXESUFFIX)
	$(TESTRUNNER) $(BIN)/debug/test$(EXESUFFIX)
	$(TESTRUNNER) $(BIN)/debug/scenarios$(EXESUFFIX)
ifdef COVERAGE
	@echo "Code coverage is temporarily disabled due to an incompatibility \
	between lcov and Apple's (buggy) version of gcov."
	rm *.gcno *.gcda
endif

# Misc. targets #

clean:
	rm -rf $(BIN) *.gcno *.gcda *.o *.bc
