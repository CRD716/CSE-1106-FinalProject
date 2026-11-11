/*
 * pigpio.h - FAKE version of the pigpio library, used only for testing.
 *
 * It lets Tracker.c be tested on any computer without real buttons or an LED.
 * Instead of turning on a real LED, gpioWrite() just saves the value in fakeLed
 * so the tests can check it.
 */

#define PI_INPUT 0
#define PI_OUTPUT 1
#define PI_PUD_UP 2

int fakeLed = 0;

int gpioInitialise(void) { return 0; }
void gpioTerminate(void) { }
int gpioSetSignalFunc(unsigned signum, void (*function)(int)) { return 0; }
int gpioSetMode(unsigned pin, unsigned mode) { return 0; }
int gpioSetPullUpDown(unsigned pin, unsigned pud) { return 0; }
int gpioRead(unsigned pin) { return 1; }
unsigned gpioDelay(unsigned micros) { return 0; }

int gpioWrite(unsigned pin, unsigned level)
{
    fakeLed = level;
    return 0;
}
