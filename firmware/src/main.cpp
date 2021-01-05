#include <Arduino.h>
#include "cli.h"
#include "cmd.h"

// cli io
int cli_getch() {
    return Serial.read();
}

void cli_print(char ch) {
    Serial.print(ch);
}

void cli_print(const char *str) {
    Serial.print(str);
}

static cli_context ctx;
static command_session session;

void setup() {
    Serial.begin(38400);

    cli_init(&ctx, &session);
}

void loop() {
    while (Serial.available()) {
        cli_process(&ctx);
    }
}
