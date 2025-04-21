/*
Author: Jarett Woodard
Group: B
Email: jarett.woodard@okstate.edu
Date: 4.4.2025
*/
#include <stdio.h>
#include <string.h>
#include "fake_sec.h"

/*
This file contains the functions to manage the time simulation. It can be called
from any process to either increment the time or get the current time. To ensure
all processes are working from the sae time, ie pulling their value from the same
memory, the time is stored only on the parent process.
*/

static char timeString[11]; // string format: [HH:MM:SS]. Static and local because this memory is only accessed by the functions in this file

void setFakeSec(int increment) {
    if (!shared_intersections) return;
    //increments seconds when called by {increment} amount
    //writes to parent process which is accessible to all children (trains)
    pthread_mutex_lock(&shared_intersections[0].mutex);
    shared_intersections[0].fakeSec += increment;
    shared_intersections[0].fakeHour = shared_intersections[0].fakeSec / 3600;
    shared_intersections[0].fakeMin = (shared_intersections[0].fakeSec % 3600) / 60;
    shared_intersections[0].fakeMinSec = shared_intersections[0].fakeSec % 60;
    pthread_mutex_unlock(&shared_intersections[0].mutex);
}

const char* getFakeTime(void) {
    if (!shared_intersections) return "[00:00:00]";
    //creates string in [HH:MM:SS] format

    pthread_mutex_lock(&shared_intersections[0].mutex);
    snprintf(timeString, sizeof(timeString), "[%02d:%02d:%02d]", 
             shared_intersections[0].fakeHour, 
             shared_intersections[0].fakeMin, 
             shared_intersections[0].fakeMinSec);
    pthread_mutex_unlock(&shared_intersections[0].mutex);
    
    return timeString;
}