#include <Arduino.h>
#include "prog.h"

// Programmer
#define PROG_DS A0
#define PROG_LATCH A1
#define PROG_CLOCK A2
#define PROG_CS A3
#define PROG_OE A4
#define PROG_WE A5

#define PROG_EEPROM_D0 2
#define PROG_EEPROM_D7 9

#define PROG_PROG_LED 12
#define PROG_READ_LED 13

#define WAIT_PAGE_WRITE delay(10)

static void progSetAddress(uint16_t address) {
    shiftOut(PROG_DS, PROG_CLOCK, MSBFIRST, address >> 8);
    shiftOut(PROG_DS, PROG_CLOCK, MSBFIRST, address & 0xFF);

    digitalWrite(PROG_LATCH, LOW);
    digitalWrite(PROG_LATCH, HIGH);
    digitalWrite(PROG_LATCH, LOW);
}

static void progSetDataOutput() {
    for (int pin = PROG_EEPROM_D0; pin <= PROG_EEPROM_D7; pin++) {
        pinMode(pin, OUTPUT);
    }
}

static void progSetDataInput() {
    for (int pin = PROG_EEPROM_D0; pin <= PROG_EEPROM_D7; pin++) {
        pinMode(pin, INPUT);
    }
}

void progBeginWrite() {
    digitalWrite(PROG_PROG_LED, HIGH);
    progSetDataOutput();
    digitalWrite(PROG_OE, HIGH);
}

void progEndWrite() {
    WAIT_PAGE_WRITE;
    digitalWrite(PROG_OE, LOW);
    progSetDataInput();
    digitalWrite(PROG_PROG_LED, LOW);
}

void progWriteByte(uint16_t address, uint8_t d) {
    progSetAddress(address);

    for (int pin = PROG_EEPROM_D0; pin <= PROG_EEPROM_D7; pin++) {
        digitalWrite(pin, d & 0x01);
        d >>= 1;
    }

    digitalWrite(PROG_WE, LOW);
    delayMicroseconds(1);
    digitalWrite(PROG_WE, HIGH);
    delayMicroseconds(1);
}

void progWriteBytes(uint16_t address, uint8_t *buf, uint16_t len) {
    for (uint16_t addr = address; addr < address + len; addr++) {
        if ((addr & 0x3f) == 0) {
            WAIT_PAGE_WRITE;
        }

        progWriteByte(addr, buf[addr - address]);
    }

    if ((len & 0x3f) == 0) {
        WAIT_PAGE_WRITE;
    }
}

void progErase(uint16_t len) {
    for (uint16_t addr = 0; addr < len; addr++) {
        if ((addr & 0x3f) == 0) {
            WAIT_PAGE_WRITE;
        }

        progWriteByte(addr, 0xff);
    }

    if ((len & 0x3f) == 0) {
        WAIT_PAGE_WRITE;
    }
}

void progBeginRead() {
    digitalWrite(PROG_READ_LED, HIGH);
}

void progEndRead() {
    digitalWrite(PROG_READ_LED, LOW);
}

uint8_t progReadByte(uint16_t address) {
    progSetAddress(address);

    uint8_t d = 0;
    for (int pin = PROG_EEPROM_D7; pin >= PROG_EEPROM_D0; pin--) {
        d <<= 1;
        d |= digitalRead(pin) == 0 ? 0 : 1;
    }

    return d;
}

void progDisableSDP() {
    progWriteByte(0x5555, 0xaa);
    progWriteByte(0x2aaa, 0x55);
    progWriteByte(0x5555, 0x80);
    progWriteByte(0x5555, 0xaa);
    progWriteByte(0x2aaa, 0x55);
    progWriteByte(0x5555, 0x20);
}

void progEnableSDP() {
    progWriteByte(0x5555, 0xaa);
    progWriteByte(0x2aaa, 0x55);
    progWriteByte(0x5555, 0xa0);
}

void progInit() {
    digitalWrite(PROG_PROG_LED, LOW);
    pinMode(PROG_PROG_LED, OUTPUT);

    digitalWrite(PROG_READ_LED, LOW);
    pinMode(PROG_READ_LED, OUTPUT);

    digitalWrite(PROG_WE, HIGH); // WE diabled
    pinMode(PROG_WE, OUTPUT);

    digitalWrite(PROG_CS, LOW); // CS enabled
    pinMode(PROG_CS, OUTPUT);

    digitalWrite(PROG_OE, LOW); // OE enabled
    pinMode(PROG_OE, OUTPUT);

    digitalWrite(PROG_LATCH, LOW);
    pinMode(PROG_LATCH, OUTPUT);

    digitalWrite(PROG_DS, LOW);
    pinMode(PROG_DS, OUTPUT);

    digitalWrite(PROG_CLOCK, LOW);
    pinMode(PROG_CLOCK, OUTPUT);

    progSetDataInput();
}
