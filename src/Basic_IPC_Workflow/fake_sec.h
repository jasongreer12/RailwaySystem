#ifndef FAKE_SEC_H
#define FAKE_SEC_H

/*
getTime can be called within any of the print function calls to get the current time and display it.
If the time needs to be incremented, set the increment parameter to a value greater than 0. Otherwise, set it to 0.
It can be called directly or as a parameter to the print function.
example:
    printf("Time: %s\n", getTime(0)); //Gets current time
    printf("Time: %s\n", getTime(1)); //Increments time by 1 second
    printf("Time: %s\n", getTime(5)); //Increments time by 5 seconds

    -OR-

    printf(getTime(0)); //Gets current time
    printf(getTime(1)); //Increments time by 1 second
    printf(getTime(5)); //Increments time by 5 second
*/
const char* getFakeTime();
void setFakeSec(int increment);

#endif