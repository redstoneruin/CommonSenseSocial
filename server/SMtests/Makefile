# Author: Ryan Steinwert
# Makefile for session manager test suite

HEADERS = ../SessionManager.h
SOURCES = $(HEADERS:.h=.cpp) main.cpp

OBJECTS = SessionManager.o main.o
DEPS = $(OBJECTS:.o=.d)
TARGET = SMtests

COMPILE = clang++ -std=gnu++2a -Wall -Wextra -Wpedantic -Wshadow -g -Og -c
LINK = clang++ -fstack-protector -m64 -o

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
