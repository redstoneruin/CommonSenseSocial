# Author: Ryan Steinwert
# Makefile for CSDB test suite

SOURCES = ../CSDB/CSDB.cpp ../CSDB/CSDBAccessManager.cpp main.cpp
HEADERS = ../CSDB/CSDB.h ../CSDB/CSDBAccessManager.h

OBJECTS = CSDB.o CSDBAccessManager.o main.o
DEPS = $(OBJECTS:.o=.d)
TARGET = CSDBtest

COMPILE = clang++ -std=gnu++2a -Wall -Wextra -Wpedantic -Wshadow -g -Og -c
LINK = clang++ -fstack-protector -m64 -o

COPYCOMMON = cp ../../common/* ..

DEFAULTDBFOLDER = db



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

removedb:
	rm -rf $(DEFAULTDBFOLDER)
