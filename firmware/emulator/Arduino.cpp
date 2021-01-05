#include <Arduino.h>
#include <time.h>
#include <errno.h>
#include <termios.h>

static void init_console() {
    struct termios TermConf;
    tcgetattr(0, &TermConf);
    TermConf.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(0, TCSANOW, &TermConf);
}

void setup();
void loop();

int main() {
    init_console();

    setup();

    for (;;) {
        loop();
        delayMicroseconds(500);
    }
}

void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val) {
}

void pinMode(uint8_t pin, uint8_t mode) {
}

int digitalRead(uint8_t pin) {
    return 0xFF;
}

void digitalWrite(uint8_t pin, uint8_t val) {
}

void delay(unsigned long ms) {
    struct timespec ts;
    int res;

    if (ms < 0)
    {
        errno = EINVAL;
        return;
    }

    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);
}

void delayMicroseconds(unsigned int us) {
    struct timespec ts;
    int res;

    if (us < 0)
    {
        errno = EINVAL;
        return;
    }

    ts.tv_sec = us / 1000000;
    ts.tv_nsec = (us % 1000000) * 1000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);
}

Serial_ Serial;
