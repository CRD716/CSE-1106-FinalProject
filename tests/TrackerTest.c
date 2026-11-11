/*
 * TrackerTest.c - Simple tests for Tracker.c
 *
 * These tests run on any computer (no Raspberry Pi needed) because they use
 * the fake pigpio.h in this tests folder instead of the real library.
 *
 * Build (from the repo root):  gcc -Wall -Itests -o tracker_test tests/TrackerTest.c
 * Run:                         ./tracker_test
 */

#include <string.h>

/* Rename Tracker.c's main() so this file can have its own main() */
#define main trackerMain
#include "../src/Tracker.c"
#undef main

int failures = 0;

/* Print PASS or FAIL for one check */
void check(int condition, const char *testName)
{
    if (condition) {
        printf("PASS: %s\n", testName);
    } else {
        printf("FAIL: %s\n", testName);
        failures++;
    }
}

/* Put the tracker back to its starting state before each test */
void reset(void)
{
    occupancy = 0;
    maxCapacity = 5;
    fakeLed = 0;
}

void testEnterAddsOne(void)
{
    reset();
    personEnters();
    check(occupancy == 1, "Enter adds one person");
}

void testExitRemovesOne(void)
{
    reset();
    personEnters();
    personEnters();
    personExits();
    check(occupancy == 1, "Exit removes one person");
}

void testNeverNegative(void)
{
    reset();
    personExits();
    check(occupancy == 0, "Exit on an empty room keeps count at 0");
}

void testNeverOverCapacity(void)
{
    reset();
    for (int i = 0; i < 7; i++) {
        personEnters();
    }
    check(occupancy == 5, "Count stops at capacity of 5");
}

void testLedOnWhenFull(void)
{
    reset();
    for (int i = 0; i < 5; i++) {
        personEnters();
    }
    check(fakeLed == 1, "LED is ON when room is full");
}

void testLedOffWhenNotFull(void)
{
    reset();
    for (int i = 0; i < 5; i++) {
        personEnters();
    }
    personExits();
    check(fakeLed == 0, "LED turns OFF when someone leaves a full room");
}

void testChangeCapacity(void)
{
    reset();
    check(setCapacity(2) == 1, "Capacity can be changed to 2");
    personEnters();
    personEnters();
    personEnters();
    check(occupancy == 2 && fakeLed == 1, "New capacity of 2 is enforced");
}

void testInvalidCapacity(void)
{
    reset();
    check(setCapacity(0) == 0 && maxCapacity == 5, "Capacity of 0 is rejected");
    check(setCapacity(-3) == 0 && maxCapacity == 5, "Negative capacity is rejected");
}

void testLogFileWritten(void)
{
    reset();
    remove(logFileName);
    personEnters();

    char line[100] = "";
    FILE *file = fopen(logFileName, "r");
    if (file != NULL) {
        fgets(line, sizeof(line), file);
        fclose(file);
    }
    check(strstr(line, ",ENTER,1,5") != NULL, "Entry is saved to the log file");
}

int main(void)
{
    /* Write test logs to a separate file so the real log is not touched */
    logFileName = "test_log.csv";

    testEnterAddsOne();
    testExitRemovesOne();
    testNeverNegative();
    testNeverOverCapacity();
    testLedOnWhenFull();
    testLedOffWhenNotFull();
    testChangeCapacity();
    testInvalidCapacity();
    testLogFileWritten();

    remove(logFileName);

    printf("\n%d test(s) failed\n", failures);
    return failures;
}
