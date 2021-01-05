#pragma once

#include <stdint.h>

#define COMMAND_LINE_MAX_SIZE 512

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
