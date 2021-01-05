#pragma once

#include <stdint.h>

void progInit();

void progBeginRead();
void progEndRead();

uint8_t progReadByte(uint16_t address);

void progBeginWrite();
void progEndWrite();

void progWriteByte(uint16_t address, uint8_t d);
void progWriteBytes(uint16_t address, uint8_t *buf, uint16_t len);
void progErase(uint16_t len);

void progDisableSDP();
void progEnableSDP();
