#include <Arduino.h>
#include "cmd.h"
#include "cli.h"
#include "prog.h"
#include "version.h"

#define CMD_OK "OK"
#define TOK_SEP " ,\t"

const uint16_t max_rom_size = 0x8000;

static void printUnknownCommand() {
    cli_print("Unknown command. Type 'h' for help.\n");
}

static void printWrongArgument() {
    cli_print("Invalid argument(s). Type 'h' for help.\n");
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

void cmdHelp() {
    cli_print(
        "h - help\n"
        "r [start_address] [count] - read ROM\n"
        "w [start_address] - write ROM\n"
        "e [length] - erase ROM\n"
        "l - enable SDP (lock)\n"
        "u - disable SDP (unlock)\n"
        "b - switch to binary mode\n"
        "v - print version\n"
        );
}

void cmdRead(const char *cmdline, command_session *session) {
    // TODO parse arguments
    char buf[8];

    progBeginRead();
    for (int i = 0; i < 256; i++) {
        if ((i & 0x0f) == 0x00) {
            sprintf(buf, "%04X: ", i);
            cli_print(buf);
        }

        sprintf(buf, "%02X ", progReadByte(i));
        cli_print(buf);

        if ((i & 0x0f) == 0x0f) {
            cli_print("\n");
        }
    }

    progEndRead();
}

void cmdWrite(char *cmdline, command_session *session) {
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

void cmdWriting(char *cmdline, command_session *session) {
    // TODO parse arguments
    cli_print("Not implemented\n");
}

void cmdErase(char *cmdline, command_session *session) {
    // cmd
    auto p = strtok(cmdline, TOK_SEP);
    if (p == 0 || strcasecmp(p, "e") != 0) {
        printUnknownCommand();
        return;
    }

    // len
    uint16_t len = max_rom_size;
    p = strtok(0, TOK_SEP);
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

void cmdEnableSDP() {
    progBeginWrite();
    progEnableSDP();
    progEndWrite();

    cli_print(CMD_OK "\n");
}

void cmdDisableSDP() {
    progBeginWrite();
    progDisableSDP();
    progEndWrite();

    cli_print(CMD_OK "\n");
}

void cmdSwitchToBinaryMode(command_session *session) {
    cli_print("Not implemented\n");
}

void cmdVersion() {
    cli_print(VERSION_STRING
        "\n");
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
            switch (toupper(cmdline[0])) {
                case 'H':
                    cmdHelp();
                    break;
                case 'R':
                    cmdRead(cmdline, sp);
                    break;
                case 'W':
                    cmdWrite(cmdline, sp);
                    break;
                case 'E':
                    cmdErase(cmdline, sp);
                    break;
                case 'L':
                    cmdEnableSDP();
                    break;
                case 'U':
                    cmdDisableSDP();
                    break;
                case 'B':
                    cmdSwitchToBinaryMode(sp);
                    break;
                case 'V':
                    cmdVersion();
                    break;
                case 0:
                    break;
                default:
                    printUnknownCommand();
                    break;
            }
            break;
        default:
            break;
    }
}
