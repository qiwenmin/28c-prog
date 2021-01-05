#include <Arduino.h>
#include "cmd.h"
#include "cli.h"
#include "prog.h"
#include "version.h"

#define CMD_OK "OK"

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

void cmdWrite(const char *cmdline, command_session *session) {
    // TODO parse arguments
    cli_print("Not implemented\n");
}

void cmdWriting(const char *cmdline, command_session *session) {
    // TODO parse arguments
    cli_print("Not implemented\n");
}

void cmdErase(const char *cmdline, command_session *session) {
    // TODO parse arguments
    cli_print("Not implemented\n");
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

void cli_lauch_cmd(const char *cmdline, void *session) {

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
                default:
                    cli_print("Unknown command. Type 'h' for help.\n");
                    break;
            }
            break;
        default:
            break;
    }
}
