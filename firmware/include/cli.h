#pragma once

#include <stdint.h>
#include <WString.h>

#ifdef ARDUINO_AVR_ATmega8
#define COMMAND_LINE_MAX_SIZE 256
#else
#define COMMAND_LINE_MAX_SIZE 512
#endif

typedef struct {
    char cmdline[COMMAND_LINE_MAX_SIZE];
    uint16_t cmdline_used;
    bool _lastIsEsc;
    void *session;
} cli_context;

void cli_init(cli_context *ctx, void *session);
void cli_process(cli_context *ctx);

int cli_getch();
void cli_print(char ch);
void cli_print(const char *str);
void cli_print(const __FlashStringHelper *str);
