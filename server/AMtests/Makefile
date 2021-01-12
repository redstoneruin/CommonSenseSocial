# Author: Ryan Steinwert
# Makefile for session manager test suite

HEADERS = ../AccountManager.h
SOURCES = $(HEADERS:.h=.cpp) main.cpp

OBJECTS = AccountManager.o main.o
DEPS = $(OBJECTS:.o=.d)
TARGET = AMtests

COMPILE = clang++ -std=gnu++2a -I../lib/openssl/include -Wall -Wextra -Wpedantic -Wshadow -g -Og -c
LINK = clang++ -lssl -lcrypto -fstack-protector -m64 -o

COPYCOMMON = cp ../../common/* ..



all : $(TARGET)

$(TARGET) : $(OBJECTS)
	$(LINK) $(TARGET) $(OBJECTS)

$(OBJECTS) : $(SOURCES)
	$(COPYCOMMON)
	$(COMPILE) $(SOURCES)


spotless : clean
	rm -f $(TARGET)

clean :
	rm -f $(OBJECTS) $(DEPS)
