# Makefile for Common Sense Social server

SOURCES		= main.cpp CSServer.cpp tls/ServerCredentials.cpp tls/Callbacks.cpp
HEADERS		= CSServer.h definitions.h tls/ServerCredentials.h tls/Callbacks.h

OBJECTS 	= main.o CSServer.o ServerCredentials.o Callbacks.o
TARGET		= csServer

COMPILE 	= clang++ -std=gnu++2a -I../lib/botan/build/include -Wall -Wextra -Wpedantic -Wshadow -g -Og -c
LINK 		= clang++ -L../lib/botan -lbotan-3 -fstack-protector -m64 -pthread -o

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
