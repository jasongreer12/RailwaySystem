#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fake_sec.h"
#include "Memory_Segments.h"

/*
OFFLOADED TO SHARED MEMORY
static int fakeSec = 0;
static int fakeMin = 0;
static int fakeMinSec = 0;
static int fakeHour = 0;
*/
static char timeString[11]; // string format: [HH:MM:SS]



void setFakeSec(int seconds) { // made public. removed static
    fakeSec += seconds;
    fakeHour = fakeSec / 3600;
    fakeMin = (fakeSec % 3600) / 60;
    fakeMinSec = fakeSec % 60;
}

//SHARED MEMORY VERSION
void setFakeSec(SharedIntersection *si, int increment){
    pthread_mutex_lock(&si->mutex);
    si->sim_time += increment;
    fakeHour = si->sim_time / 3600;
    fakeMin = (si->sim_time % 3600) / 60;
    fakeMinSec = si->sim_time % 60;
    pthread_mutex_unlock(&si->mutex);
}

const char* getFakeTime() {
    snprintf(timeString, sizeof(timeString), "[%02d:%02d:%02d]", fakeHour, fakeMin, fakeMinSec);
    return timeString;
}