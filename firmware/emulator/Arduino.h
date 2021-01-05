#pragma once

#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h>

#define min(a,b) ((a)<(b)?(a):(b))

#define A0 20
#define A1 21
#define A2 22
#define A3 23
#define A4 24
#define A5 25

#define MSBFIRST 1

#define LOW 0
#define HIGH 1

#define INPUT 0
#define OUTPUT 1

void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val);

void pinMode(uint8_t pin, uint8_t mode);

int digitalRead(uint8_t pin);
void digitalWrite(uint8_t pin, uint8_t val);

void delay(unsigned long ms);
void delayMicroseconds(unsigned int us);

class Serial_ {
public:
    void begin(unsigned long baud) {};

    bool available() {
        return true;
    };

    int read() {
        return getchar();
    };

    size_t print(char ch) {
        auto result = putchar(ch);
        fflush(stdout);
        return result;
    };

    size_t print(const char *str) {
        return printf("%s", str);
    };
};

extern Serial_ Serial;