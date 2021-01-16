#include <Arduino.h>
#include "cmd.h"
#include "cli.h"
#include "prog.h"
#include "version.h"

#define CMD_OK "OK"
#define TOK_SEP " ,\t"

#define MAX_ADDRESS "0x7FFFF"
const uint16_t max_rom_size = 0x8000;

static void printUnknownCommand() {
    cli_print(F("Unknown command. Type 'h' for help.\n"));
}

static void printWrongArgument() {
    cli_print(F("Invalid argument(s). Type 'h' for help.\n"));
}

static void printWrongAddress() {
    cli_print(F("Invalid address. The address should between 0x0000 and " MAX_ADDRESS ".\n"));
}

static bool dec_to_uint16(const char *dec, uint16_t *u16) {
    *u16 = 0;
    for (const char *p = dec; *p != 0; p++) {
        *u16 *= 10;
        if (*p >= '0' && *p <= '9') {
            *u16 += (*p - '0');
        } else {
            return false;
        }
    }

    return true;
}

static bool oct_to_uint16(const char *oct, uint16_t *u16) {
    *u16 = 0;
    for (const char *p = oct; *p != 0; p++) {
        *u16 *= 8;
        if (*p >= '0' && *p <= '7') {
            *u16 += (*p - '0');
        } else {
            return false;
        }
    }

    return true;
}

static bool bin_to_uint16(const char *bin, uint16_t *u16) {
    *u16 = 0;
    for (const char *p = bin; *p != 0; p++) {
        *u16 <<= 1;
        if (*p >= '0' && *p <= '1') {
            *u16 += (*p - '0');
        } else {
            return false;
        }
    }

    return true;
}

static bool hex_to_uint16(const char *hex, uint16_t *u16) {
    *u16 = 0;
    for (const char *p = hex; *p != 0; p++) {
        *u16 <<= 4;
        char ch = toupper(*p);
        if (ch >= '0' && ch <= '9') {
            *u16 += (ch - '0');
        } else if (ch >= 'A' && ch <= 'F') {
            *u16 += (ch - 'A' + 10);
        } else {
            return false;
        }
    }

    return true;
}

static bool str_to_uint16(const char *str, uint16_t *u16) {
    if (str[0] == '0') {
        if (toupper(str[1]) == 'X') {
            // Hex
            return hex_to_uint16(&str[2], u16);
        } else if (toupper(str[1]) == 'B') {
            // Bin
            return bin_to_uint16(&str[2], u16);
        } else {
            // Oct
            return oct_to_uint16(&str[1], u16);
        }
    } else {
        // Dec
        return dec_to_uint16(str, u16);
    }
}

static char _num_str_buf[6];

static char _b4_to_hexchar(uint8_t b4) {
    return b4 < 10 ? '0' + b4 : 'A' - 10 + b4;
}

static char *uint16_to_hex4(uint16_t u16) {
    _num_str_buf[0] = _b4_to_hexchar((u16 & 0xF000) >> 12);
    _num_str_buf[1] = _b4_to_hexchar((u16 & 0x0F00) >> 8);
    _num_str_buf[2] = _b4_to_hexchar((u16 & 0x00F0) >> 4);
    _num_str_buf[3] = _b4_to_hexchar((u16 & 0x000F));
    _num_str_buf[4] = 0;

    return _num_str_buf;
}

static char *uint8_to_hex2(uint8_t u8) {
    _num_str_buf[0] = _b4_to_hexchar((u8 & 0xF0) >> 4);
    _num_str_buf[1] = _b4_to_hexchar((u8 & 0x0F));
    _num_str_buf[2] = 0;

    return _num_str_buf;
}

static char *uint16_to_dec(uint16_t u16) {
    _num_str_buf[5] = 0;
    char *p = &_num_str_buf[5];

    do {
        *(--p) = _b4_to_hexchar(u16 % 10);
        u16 /= 10;
    } while (u16 > 0);

    return p;
}

static void cmdHelp(command_session *session) {
    auto p = strtok(0, TOK_SEP);

    if (p != 0) {
        printWrongArgument();
        return;
    }

    cli_print(
        F("h - help\n"
        "r [start_address] [count] - read ROM\n"
        "                  <enter> - read ROM\n"
        "w [start_address] - write ROM\n"
        "e [length] - erase ROM\n"
        "l - enable SDP (lock)\n"
        "u - disable SDP (unlock)\n"
        "b - switch to binary mode\n"
        "v - print version\n"
        ));
}

static void cmdRead(command_session *session) {
    // address
    uint16_t addr = session->current_address;
    auto p = strtok(0, TOK_SEP);

    if (p != 0) {
        if (!str_to_uint16(p, &addr)) {
            printWrongArgument();
            return;
        }

        if (addr >= max_rom_size) {
            printWrongAddress();
            return;
        }

        p = strtok(0, TOK_SEP); // next arg
    }

    // len
    uint16_t len = min(0x0100 - (addr & 0x00ff), max_rom_size - addr);
    if (p != 0) {
        if (!str_to_uint16(p, &len)) {
            printWrongArgument();
            return;
        }

        len = min(len, max_rom_size - addr);
        p = strtok(0, TOK_SEP);
    }

    if (p != 0) {
        printWrongArgument();
        return;
    }

    progBeginRead();
    for (uint16_t i = addr; i < addr + len; i++) {
        if ((i & 0x000f) == 0x0000 || i == addr) {
            cli_print(uint16_to_hex4(i & 0xfff0));
            cli_print(": ");

            for (uint16_t j = 0; j < (i & 0x000f); j++) {
                cli_print(".. ");
            }
        }

        cli_print(uint8_to_hex2(progReadByte(i)));
        cli_print(" ");

        if ((i & 0x000f) == 0x000f || i == (addr + len - 1)) {
            cli_print("\n");
        }
    }

    progEndRead();

    session->current_address = addr + len;
}

static void cmdWrite(command_session *session) {
    // address
    uint16_t addr = session->current_address;
    auto p = strtok(0, TOK_SEP);

    if (p != 0) {
        if (!str_to_uint16(p, &addr)) {
            printWrongArgument();
            return;
        }

        if (addr >= max_rom_size) {
            printWrongAddress();
            return;
        }

        p = strtok(0, TOK_SEP); // next arg
    }

    if (p != 0) {
        printWrongArgument();
        return;
    }

    session->current_address = addr;

    cli_print("w ");
    cli_print(uint16_to_hex4(addr));
    cli_print(" ");

    session->state = css_writing;
}

static void cmdWriting(char *commandline, command_session *session) {
    if (commandline[0] == 0) {
        session->state = css_normal;
        return;
    }

    uint8_t buf[COMMAND_LINE_MAX_SIZE / 2];
    uint16_t idx = 0;
    for (auto p = strtok(commandline, TOK_SEP);
        p != 0 && idx < COMMAND_LINE_MAX_SIZE / 2 && idx + session->current_address < max_rom_size;
        p = strtok(0, TOK_SEP)) {
        uint16_t data;
        if ((!hex_to_uint16(p, &data)) || (data > 0x00ff)) {
            cli_print(F("Invalid hex number sequence. Please input hex numbers (without leading '0x', between 00 and FF) in writing mode.\n"));
            idx = 0;
            break;
        }

        buf[idx++] = data & 0x00ff;
    }

    if (idx > 0) {
        progBeginWrite();
        progWriteBytes(session->current_address, buf, idx);
        progEndWrite();

        session->current_address += idx;
    }

    cli_print("w ");
    cli_print(uint16_to_hex4(session->current_address));
    cli_print(" ");
}

static void cmdErase(command_session *session) {
    // len
    uint16_t len = max_rom_size;
    auto p = strtok(0, TOK_SEP);
    if (p != 0) {
        if (!str_to_uint16(p, &len)) {
            printWrongArgument();
            return;
        }

        len = min(len, max_rom_size);
        p = strtok(0, TOK_SEP);
    }

    if (p != 0) {
        printWrongArgument();
        return;
    }

    cli_print(F("Erasing...\n"));

    progBeginWrite();
    progErase(len);
    progEndWrite();

    session->current_address = 0;

    cli_print("Erased ");
    cli_print(uint16_to_hex4(len));
    cli_print(" (");
    cli_print(uint16_to_dec(len));
    cli_print(") bytes.\n");
}

static void cmdEnableSDP(command_session *session) {
    auto p = strtok(0, TOK_SEP);
    if (p != 0) {
        printWrongArgument();
        return;
    }

    progBeginWrite();
    progEnableSDP();
    progEndWrite();

    cli_print(CMD_OK "\n");
}

static void cmdDisableSDP(command_session *session) {
    auto p = strtok(0, TOK_SEP);
    if (p != 0) {
        printWrongArgument();
        return;
    }

    progBeginWrite();
    progDisableSDP();
    progEndWrite();

    cli_print(CMD_OK "\n");
}

static void cmdSwitchToBinaryMode(command_session *session) {
    cli_print(F("Not implemented\n"));
}

static void cmdVersion(command_session *session) {
    auto p = strtok(0, TOK_SEP);
    if (p != 0) {
        printWrongArgument();
        return;
    }

    cli_print(VERSION_STRING
        "\n");
}

static void dispatchCmd(char *cmdline, command_session *session) {
    auto p = strtok(cmdline, TOK_SEP);

    if (p == 0 || strcasecmp(p, "r") == 0) {
        cmdRead(session);
    } else if (strcasecmp(p, "w") == 0) {
        cmdWrite(session);
    } else if (strcasecmp(p, "e") == 0) {
        cmdErase(session);
    } else if (strcasecmp(p, "l") == 0) {
        cmdEnableSDP(session);
    } else if (strcasecmp(p, "u") == 0) {
        cmdDisableSDP(session);
    } else if (strcasecmp(p, "b") == 0) {
        cmdSwitchToBinaryMode(session);
    } else if (strcasecmp(p, "v") == 0) {
        cmdVersion(session);
    } else if (strcasecmp(p, "h") == 0) {
        cmdHelp(session);
    } else {
        printUnknownCommand();
    }
}

void cli_init_session(void *session) {
    if (session) {
        auto *sp = (command_session *)session;
        sp->current_address = 0;
        sp->state = css_normal;
    }

    progInit();
}

void cli_lauch_cmd(char *cmdline, void *session) {

    auto sp = (command_session *)session;

    switch (sp->state) {
        case css_writing:
            cmdWriting(cmdline, sp);
            break;
        case css_normal:
            dispatchCmd(cmdline, sp);
            break;
        default:
            break;
    }
}
