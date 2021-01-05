#pragma once

#include <stdint.h>

typedef enum {
    css_normal = 0,
    css_writing
} command_session_state;

typedef struct {
    uint16_t current_address;
    command_session_state state;
} command_session;
