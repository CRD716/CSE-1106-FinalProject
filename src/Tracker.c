/*
 * Tracker.c - Smart Study Room Occupancy Tracker
 *
 * Hardware (Raspberry Pi 4B, BCM pin numbers):
 *   ENTER button : GPIO 23 -> button -> GND  (internal pull-up used)
 *   EXIT button  : GPIO 24 -> button -> GND  (internal pull-up used)
 *   Status LED   : GPIO 17 -> resistor -> LED -> GND
 *
 * LED meaning:  OFF = room has space,  ON = room is FULL
 *
 * Log file lines look like:  2026-11-11 18:36:01,ENTER,3,5
 *                            (time, event, occupancy, capacity)
 *
 * Build:  gcc -Wall -o tracker src/Tracker.c -lpigpio -lrt -lpthread
 * Run:    sudo ./tracker          (uses default capacity of 5)
 *         sudo ./tracker 8        (sets maximum capacity to 8)
 *
 * Press Ctrl+C to quit.
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <time.h>
#include <pigpio.h>

#define ENTER_PIN 23
#define EXIT_PIN 24
#define LED_PIN 17

int occupancy = 0;
int maxCapacity = 5;
int running = 1;
const char *logFileName = "data/entrylog.csv";

/* Called when the user presses Ctrl+C */
void stopProgram(int signum)
{
    running = 0;
}

/* Add one line to the log file */
void logEvent(const char *event)
{
    FILE *file = fopen(logFileName, "a");
    if (file == NULL) {
        printf("Warning: could not open log file %s\n", logFileName);
        return;
    }

    time_t now = time(NULL);
    char timeText[30];
    strftime(timeText, sizeof(timeText), "%Y-%m-%d %H:%M:%S", localtime(&now));

    fprintf(file, "%s,%s,%d,%d\n", timeText, event, occupancy, maxCapacity);
    fclose(file);
}

/* Update the LED and print the current status to the terminal */
void showStatus(void)
{
    int full = (occupancy >= maxCapacity);

    gpioWrite(LED_PIN, full);
    printf("Occupancy: %d / %d  -  Status: %s\n",
           occupancy, maxCapacity, full ? "FULL" : "AVAILABLE");
}

/* Someone pressed the ENTER button */
void personEnters(void)
{
    if (occupancy >= maxCapacity) {
        printf("Room is full! Entry not counted.\n");
        logEvent("ENTRY_DENIED_FULL");
    } else {
        occupancy++;
        logEvent("ENTER");
    }
    showStatus();
}

/* Someone pressed the EXIT button */
void personExits(void)
{
    if (occupancy <= 0) {
        printf("Room is already empty! Exit not counted.\n");
        logEvent("EXIT_IGNORED_EMPTY");
    } else {
        occupancy--;
        logEvent("EXIT");
    }
    showStatus();
}

/* Change the maximum capacity. Returns 1 if it worked, 0 if the value is invalid. */
int setCapacity(int newCapacity)
{
    if (newCapacity <= 0) {
        printf("Invalid capacity %d, keeping %d\n", newCapacity, maxCapacity);
        return 0;
    }
    maxCapacity = newCapacity;
    return 1;
}

int main(int argc, char *argv[])
{
    /* Optional: change the maximum capacity from the command line */
    if (argc > 1) {
        setCapacity(atoi(argv[1]));
    }

    if (gpioInitialise() < 0) {
        printf("Failed to initialize GPIO\n");
        return 1;
    }

    /* Let Ctrl+C stop the main loop cleanly */
    gpioSetSignalFunc(SIGINT, stopProgram);

    /* Buttons are inputs with pull-ups, so they read 1 when released and 0 when pressed */
    gpioSetMode(ENTER_PIN, PI_INPUT);
    gpioSetPullUpDown(ENTER_PIN, PI_PUD_UP);
    gpioSetMode(EXIT_PIN, PI_INPUT);
    gpioSetPullUpDown(EXIT_PIN, PI_PUD_UP);
    gpioSetMode(LED_PIN, PI_OUTPUT);

    printf("Study Room Tracker started. Press Ctrl+C to quit.\n");
    logEvent("START");
    showStatus();

    int lastEnter = 1;
    int lastExit = 1;

    while (running) {
        int enterNow = gpioRead(ENTER_PIN);
        int exitNow = gpioRead(EXIT_PIN);

        /* Only act at the moment a button goes from released (1) to pressed (0) */
        if (lastEnter == 1 && enterNow == 0) {
            personEnters();
        }
        if (lastExit == 1 && exitNow == 0) {
            personExits();
        }

        lastEnter = enterNow;
        lastExit = exitNow;

        gpioDelay(50000); /* wait 50 ms (also debounces the buttons) */
    }

    printf("\nShutting down.\n");
    logEvent("STOP");
    gpioWrite(LED_PIN, 0);
    gpioTerminate();
    return 0;
}
