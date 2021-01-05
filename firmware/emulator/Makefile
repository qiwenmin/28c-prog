.PHONY: all clean

headers := $(wildcard ../include/*.h)
sources := $(wildcard ../src/*.cpp)

all: emulator

emulator: Arduino.cpp Arduino.h $(headers) $(sources)
	g++ -Wall -Werror -std=c++11 -I../include -I. -o emulator Arduino.cpp $(sources)

clean:
	rm -f emulator
