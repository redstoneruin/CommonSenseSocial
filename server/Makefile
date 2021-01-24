# Makefile for Common Sense Social server

HEADERS		= CSServer.h SessionManager.h AccountManager.h CSDB/CSDBAccessManager.h CSDB/CSDB.h CSDB/CollectionTree.h CSDB/Item.h CSDB/CSDBRuleManager.h
SOURCES		= $(HEADERS:.h=.cpp) main.cpp

OBJECTS 	= main.o CSServer.o SessionManager.o AccountManager.o CSDBAccessManager.o CSDB.o CollectionTree.o CSDBRuleManager.o Item.o
TARGET		= csServer

COMPILE 	= clang++ -std=gnu++2a -I../lib/openssl/include -Wall -Wextra -Wpedantic -Wshadow -g -Og -c
LINK 		= clang++ -lssl -lcrypto -fstack-protector -m64 -pthread -o

COPYCOMMON 	= cp ../common/* .

REMOVE		= rm -f

all : $(TARGET)

$(TARGET) : $(OBJECTS)
	$(LINK) $(TARGET) $(OBJECTS)

$(OBJECTS) : $(SOURCES)
	$(COPYCOMMON)
	$(COMPILE) $(SOURCES)

clean:
	$(REMOVE) $(OBJECTS)

spotless: clean
	$(REMOVE) $(TARGET)

format:
	clang-format -i $(SOURCES) $(INCLUDES)

-include $(DEPS)

.PHONY: all clean format spotless
