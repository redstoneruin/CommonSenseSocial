# Makefile for Common Sense Social server

SOURCES		= main.cpp
HEADERS		= definitions.h

OBJECTS 	= $(SOURCES:.cpp=.o)
DEPS		= $(SOURCES:.cpp=.d)
TARGET		= csServerTestSuite

COMPILE 	= clang++ -std=gnu++2a -Wall -Wextra -Wpedantic -Wshadow -g -Og -c
LINK 		= clang++ -pthread -o

COPYCOMMON	= cp ../common/* .

REMOVE		= rm -f

all : $(TARGET)

$(TARGET) : $(OBJECTS)
	$(LINK) $(TARGET) $(OBJECTS)

$(OBJECTS) : $(SOURCES)
	$(COPYCOMMON)
	$(COMPILE) $(SOURCES)

clean:
	$(REMOVE) $(OBJECTS) $(DEPS)

spotless: clean
	$(REMOVE) $(TARGET)

format:
	clang-format -i $(SOURCES) $(INCLUDES)

-include $(DEPS)

.PHONY: all clean format spotless
