#include <Arduino.h>

// Programmer
#define PROG_DS A0
#define PROG_LATCH A1
#define PROG_CLOCK A2
#define PROG_CS A3
#define PROG_OE A4
#define PROG_WE A5

#define PROG_EEPROM_D0 2
#define PROG_EEPROM_D7 9

#define PROG_PROG_LED 12
#define PROG_READ_LED 13

void progSetAddress(uint16_t address)
{
    shiftOut(PROG_DS, PROG_CLOCK, MSBFIRST, address >> 8);
    shiftOut(PROG_DS, PROG_CLOCK, MSBFIRST, address & 0xFF);

    digitalWrite(PROG_LATCH, LOW);
    digitalWrite(PROG_LATCH, HIGH);
    digitalWrite(PROG_LATCH, LOW);
}

void progSetDataOutput()
{
    for (int pin = PROG_EEPROM_D0; pin <= PROG_EEPROM_D7; pin++)
    {
        pinMode(pin, OUTPUT);
    }
}

void progSetDataInput()
{
    for (int pin = PROG_EEPROM_D0; pin <= PROG_EEPROM_D7; pin++)
    {
        pinMode(pin, INPUT);
    }
}

void progBeginWrite()
{
    digitalWrite(PROG_PROG_LED, HIGH);
    progSetDataOutput();
    digitalWrite(PROG_OE, HIGH);
}

void progEndWrite()
{
    delay(10);
    digitalWrite(PROG_OE, LOW);
    progSetDataInput();
    digitalWrite(PROG_PROG_LED, LOW);
}

void progWriteByte(uint16_t address, byte d)
{
    progSetAddress(address);

    for (int pin = PROG_EEPROM_D0; pin <= PROG_EEPROM_D7; pin++)
    {
        digitalWrite(pin, d & 0x01);
        d >>= 1;
    }

    digitalWrite(PROG_WE, LOW);
    delayMicroseconds(1);
    digitalWrite(PROG_WE, HIGH);
    delayMicroseconds(1);
}

void progWriteBytes(uint16_t address, byte *buf, uint16_t len)
{
    for (uint16_t addr = address; addr < address + len; addr++)
    {
        if ((addr & 0x3f) == 0)
        {
            delay(10); // wait page write
        }

        progWriteByte(addr, buf[addr - address]);
    }

    if ((len & 0x3f) == 0)
    {
        delay(10); // wait page write
    }
}

void progErase(uint16_t len)
{
    for (uint16_t addr = 0; addr < len; addr++)
    {
        if ((addr & 0x3f) == 0)
        {
            delay(10); // wait page write
        }

        progWriteByte(addr, 0xff);
    }

    if ((len & 0x3f) == 0)
    {
        delay(10); // wait page write
    }
}

void progBeginRead()
{
    digitalWrite(PROG_READ_LED, HIGH);
}

void progEndRead()
{
    digitalWrite(PROG_READ_LED, LOW);
}

byte progReadByte(uint16_t address)
{
    progSetAddress(address);

    byte d = 0;
    for (int pin = PROG_EEPROM_D7; pin >= PROG_EEPROM_D0; pin--)
    {
        d <<= 1;
        d |= digitalRead(pin) == 0 ? 0 : 1;
    }

    return d;
}

void progDisableSDP()
{
    progWriteByte(0x5555, 0xaa);
    progWriteByte(0x2aaa, 0x55);
    progWriteByte(0x5555, 0x80);
    progWriteByte(0x5555, 0xaa);
    progWriteByte(0x2aaa, 0x55);
    progWriteByte(0x5555, 0x20);
}

void progEnableSDP()
{
    progWriteByte(0x5555, 0xaa);
    progWriteByte(0x2aaa, 0x55);
    progWriteByte(0x5555, 0xa0);
}

void progInit()
{
    digitalWrite(PROG_PROG_LED, LOW);
    pinMode(PROG_PROG_LED, OUTPUT);

    digitalWrite(PROG_READ_LED, LOW);
    pinMode(PROG_READ_LED, OUTPUT);

    digitalWrite(PROG_WE, HIGH); // WE diabled
    pinMode(PROG_WE, OUTPUT);

    digitalWrite(PROG_CS, LOW); // CS enabled
    pinMode(PROG_CS, OUTPUT);

    digitalWrite(PROG_OE, LOW); // OE enabled
    pinMode(PROG_OE, OUTPUT);

    digitalWrite(PROG_LATCH, LOW);
    pinMode(PROG_LATCH, OUTPUT);

    digitalWrite(PROG_DS, LOW);
    pinMode(PROG_DS, OUTPUT);

    digitalWrite(PROG_CLOCK, LOW);
    pinMode(PROG_CLOCK, OUTPUT);

    progSetDataInput();
}

// cli io
static int cli_getch() {
    return Serial.read();
}

static void cli_print(char ch) {
    Serial.print(ch);
}

static void cli_print(const char *str) {
    Serial.print(str);
}

// Commands
void cmdHelp() {
    cli_print(
        "h - help\n"
        "r [start_address] [count] - read memory\n"
        "w [start_address] - write memory\n"
        "l - enable SDP (lock)\n"
        "u - disable SDP (unlock)\n"
        "b - switch to binary mode\n"
        "v - print version\n"
        );
}

void cmdRead() {
    char buf[8];

    progBeginRead();
    for (int i = 0; i < 256; i++)
    {
        if ((i & 0x0f) == 0x00) {
            sprintf(buf, "%04X: ", i);
            Serial.print(buf);
        }

        sprintf(buf, "%02X ", progReadByte(i));
        Serial.print(buf);

        if ((i & 0x0f) == 0x0f)
        {
            Serial.print("\n");
        }
    }

    progEndRead();
}

// Command line
#define CLI_PROMPT "> "
#define CLI_LF_PROMPT "\n> "

const uint16_t command_line_max_size = 256;

typedef struct {
    uint16_t current_address;
    char cmdline[command_line_max_size];
    uint16_t cmdline_used;
    bool lastIsEsc;
} cli_context;

void cli_init(cli_context *ctx) {
    ctx->current_address = 0x0000;
    ctx->cmdline_used = 0;
    ctx->lastIsEsc = false;

    cli_print(CLI_LF_PROMPT);
}

void cli_lauch_cmd(cli_context *ctx) {
    if (ctx->cmdline[0] == 'r') {
        cmdRead();
    } else if (ctx->cmdline[0] == 'h') {
        cmdHelp();
    }
}

void cli_process(cli_context *ctx) {
    auto ch = cli_getch();

    if (ch == -1) return;

    if (ctx->lastIsEsc) {
        if (ch == '[') {
            // CSI - read until 0x40-0x7E
            do {
                while ((ch = cli_getch()) == -1) ;
            } while (ch < 0x40 || ch > 0x7e);
        }

        ctx->lastIsEsc = false;
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

                cli_lauch_cmd(ctx);

                ctx->cmdline_used = 0;
                cli_print(CLI_PROMPT);
            } else {
                cli_print(CLI_LF_PROMPT);
            }
            break;
        case '\b':
            if (ctx->cmdline_used > 0) {
                ctx->cmdline_used --;

                cli_print("\b \b");
            }
            break;
        case 27: // ESC
            ctx->lastIsEsc = true;
            break;
        default:
            if ((ch >= 0x20) && (ch <= 0x7e) && (ctx->cmdline_used < (command_line_max_size - 1))) {
                ctx->cmdline[ctx->cmdline_used++] = ch;
                cli_print(ch);
            }
            break;
    }
}

static cli_context ctx;

void setup()
{
    progInit();
    Serial.begin(38400);

    /*progBeginWrite();
    progWriteByte(0x01FF, 0xFF);
    progEndWrite();*/

    cli_init(&ctx);
}

void loop()
{
    while (Serial.available()) {
        cli_process(&ctx);
    }
}
