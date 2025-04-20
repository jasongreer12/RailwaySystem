#include <stdio.h>
#include <string.h>
#include "fake_sec.h"

static char timeString[11]; // string format: [HH:MM:SS] - Keeping local and static for thread safety

void setFakeSec(int increment) {
    if (!shared_intersections) return;
    
    pthread_mutex_lock(&shared_intersections[0].mutex);
    shared_intersections[0].fakeSec += increment;
    shared_intersections[0].fakeHour = shared_intersections[0].fakeSec / 3600;
    shared_intersections[0].fakeMin = (shared_intersections[0].fakeSec % 3600) / 60;
    shared_intersections[0].fakeMinSec = shared_intersections[0].fakeSec % 60;
    pthread_mutex_unlock(&shared_intersections[0].mutex);
}

const char* getFakeTime(void) {
    if (!shared_intersections) return "[00:00:00]";
    
    pthread_mutex_lock(&shared_intersections[0].mutex);
    snprintf(timeString, sizeof(timeString), "[%02d:%02d:%02d]", 
             shared_intersections[0].fakeHour, 
             shared_intersections[0].fakeMin, 
             shared_intersections[0].fakeMinSec);
    pthread_mutex_unlock(&shared_intersections[0].mutex);
    
    return timeString;
}