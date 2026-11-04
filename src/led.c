#include <stdio.h>
#include <unistd.h>
#include <pigpio.h>
#define PIN 17

int main(int argc, char *argv[]) {
    if (gpioInitialise() < 0) {
        fprintf(stderr, "Failed to initialize GPIO\n");
        return 1;
    }

    gpioSetMode(PIN, PI_OUTPUT);

    gpioWrite(PIN, 1); // Turn LED on
    sleep(1);
    gpioWrite(PIN, 0); // Turn LED off
    sleep(1);

    gpioTerminate();
    return 0;
}