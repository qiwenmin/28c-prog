#include "cli.h"

#define CLI_PROMPT "> "

void cli_init_session(void *session);
void cli_lauch_cmd(const char *cmdline, void *session);

void cli_init(cli_context *ctx, void *session) {
    ctx->cmdline_used = 0;
    ctx->_lastIsEsc = false;
    ctx->session = session;

    cli_init_session(session);

    cli_print("\n" CLI_PROMPT);
}

void cli_process(cli_context *ctx) {
    auto ch = cli_getch();

    if (ch == -1) return;

    if (ctx->_lastIsEsc) {
        if (ch == '[') {
            // CSI - read until 0x40-0x7E
            do {
                while ((ch = cli_getch()) == -1) ;
            } while (ch < 0x40 || ch > 0x7e);
        }

        ctx->_lastIsEsc = false;
        return;
    }

    switch (ch) {
        case '\r':
            break; // ignore
        case '\n':
            if (ctx->cmdline_used > 0) {
                ctx->cmdline[ctx->cmdline_used] = 0;
                // Get a command line
                cli_print("\n");

                cli_lauch_cmd(ctx->cmdline, ctx->session);

                ctx->cmdline_used = 0;
                cli_print(CLI_PROMPT);
            } else {
                cli_print("\n" CLI_PROMPT);
            }
            break;
        case 27: // ESC
            ctx->_lastIsEsc = true;
            break;
        case '\b':
        case 0x7f:
            if (ctx->cmdline_used > 0) {
                ctx->cmdline_used --;

                cli_print("\b \b");
            }
            break;
        case '\t':
            ch = ' ';
        default:
            if ((ch >= 0x20) && (ch <= 0x7e) && (ctx->cmdline_used < (COMMAND_LINE_MAX_SIZE - 1))) {
                ctx->cmdline[ctx->cmdline_used++] = ch;
                cli_print(ch);
            }
            break;
    }
}
