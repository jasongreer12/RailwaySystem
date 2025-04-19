#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fake_sec.h"

static int fakeSec = 0;
static int fakeMin = 0;
static int fakeMinSec = 0;
static int fakeHour = 0;
static char timeString[11];  // "MM:SS\0"

static void setFakeSec(int seconds) {
    fakeSec = seconds;
    fakeHour = fakeSec / 3600;
    fakeMin = (fakeSec % 3600) / 60;
    fakeMinSec = fakeSec % 60;
}


const char* getFakeTime(int increment) {

    if(increment > 0) {
        setFakeSec(fakeSec + increment);
    }
    snprintf(timeString, sizeof(timeString), "[%02d:%02d:%02d]", fakeMin, fakeMinSec);
    return timeString;
}