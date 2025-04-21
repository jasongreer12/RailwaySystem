#ifndef FAKE_SEC_H
#define FAKE_SEC_H

#include "../Shared_Memory_Setup/Memory_Segments.h"

/*
TIMESTAMPING FOR LOGGING
getFakeTime can be used in either of the formats below.
example:
    printf("Time: %s\n", getFakeTime()); //Gets current time
    printf(getFakeTime()); //Gets current time

setFakeSec can be used to increment the time by a given integer amount.
example:
    setFakeSec(1); //increments time by 1 second
    setFakeSec(4); //increments time by 1 minute
  
*/

/* 
NEW FORMAT: Moving seconds, minutes, and hours to the shared memory structure
*/
void setFakeSec(int seconds);
const char* getFakeTime(void);

#endif