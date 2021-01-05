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
    cli_print("Unknown command. Type 'h' for help.\n");
}

static void printWrongArgument() {
    cli_print("Invalid argument(s). Type 'h' for help.\n");
}

static void printWrongAddress() {
    cli_print("Invalid address. The address should between " MAX_ADDRESS ".\n");
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

static void cmdHelp(command_session *session) {
    auto p = strtok(0, TOK_SEP);

    if (p != 0) {
        printWrongArgument();
        return;
    }

    cli_print(
        "h - help\n"
        "r [start_address] [count] - read ROM\n"
        "                  <enter> - read ROM\n"
        "w [start_address] - write ROM\n"
        "e [length] - erase ROM\n"
        "l - enable SDP (lock)\n"
        "u - disable SDP (unlock)\n"
        "b - switch to binary mode\n"
        "v - print version\n"
        );
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
    uint16_t len = min(256, max_rom_size - addr);
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

    char buf[8];

    progBeginRead();
    for (uint16_t i = addr; i < addr + len; i++) {
        if ((i & 0x000f) == 0x0000 || i == addr) {
            sprintf(buf, "%04X: ", i & 0xfff0);
            cli_print(buf);

            for (uint16_t j = 0; j < (i & 0x000f); j++) {
                cli_print("   ");
            }
        }

        sprintf(buf, "%02X ", progReadByte(i));
        cli_print(buf);

        if ((i & 0x000f) == 0x000f || i == (addr + len - 1)) {
            cli_print("\n");
        }
    }

    progEndRead();

    session->current_address = addr + len;
}

static void cmdWrite(command_session *session) {
    // TODO parse arguments

    uint8_t data[256];
    for (int i = 0; i < 256; i ++) {
        data[i] = i & 0x00ff;
    }
    progBeginWrite();
    progWriteBytes(0x0000, data, 256);
    progEndWrite();
    cli_print("Write first 256 bytes.\n");
}

static void cmdWriting(char *commandline, command_session *session) {
    // TODO parse arguments
    cli_print("Not implemented\n");
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

    cli_print("Erasing...\n");

    progBeginWrite();
    progErase(len);
    progEndWrite();
    char msg[32];
    sprintf(msg, "Erased 0x%04X (%u) bytes.\n", len, len);
    cli_print(msg);
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
    cli_print("Not implemented\n");
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
