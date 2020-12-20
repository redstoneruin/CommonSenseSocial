# Makefile for Common Sense Social server

SOURCES		= main.cpp CSServer.cpp
HEADERS		= CSServer.h definitions.h

OBJECTS 	= main.o CSServer.o
TARGET		= csServer

COMPILE 	= clang++ -std=gnu++2a -I../lib/openssl/include -Wall -Wextra -Wpedantic -Wshadow -g -Og -c
LINK 		= clang++ -L../lib/openssl -lssl -lcrypto -fstack-protector -m64 -pthread -o

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
