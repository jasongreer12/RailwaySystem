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
static char timeString[11]; // string format: [HH:MM:SS] - Keeping local and static for thread safety

//ORIGINAL LOCAL VERSION
// void setFakeSec(int seconds) { // made public. removed static
//     fakeSec += seconds;
//     fakeHour = fakeSec / 3600;
//     fakeMin = (fakeSec % 3600) / 60;
//     fakeMinSec = fakeSec % 60;
// }
static SharedIntersection *si = NULL;  // Global static pointer to first intersection

// Helper to ensure si is initialized
static inline void init_si() {
    if (!si) {
        si = &shared_intersections[0];  // Use first intersection for global time
    }
}
//SHARED MEMORY VERSION
void setFakeSec(int increment){
    init_si();
    pthread_mutex_lock(&si->mutex);
    si->fakeSec += increment;
    si->fakeHour = si->fakeSec / 3600;
    si->fakeMin = (si->fakeSec % 3600) / 60;
    si->fakeMinSec = si->fakeSec % 60;
    pthread_mutex_unlock(&si->mutex);
}

const char* getFakeTime(void) {
    init_si();
    pthread_mutex_lock(&si->mutex);
    snprintf(timeString, sizeof(timeString), "[%02d:%02d:%02d]", si->fakeHour, si->fakeMin, si->fakeMinSec);
    pthread_mutex_unlock(&si->mutex);
    return timeString;
}