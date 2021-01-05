# Author: Ryan Steinwert
# Makefile for CSDB test suite

HEADERS = ../CSDB/CSDB.h ../CSDB/CSDBAccessManager.h ../CSDB/CSDBRuleManager.h ../CSDB/CollectionTree.h ../CSDB/Item.h
SOURCES = $(HEADERS:.h=.cpp) main.cpp

OBJECTS = CSDB.o CSDBAccessManager.o CSDBRuleManager.o CollectionTree.o Item.o main.o
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
