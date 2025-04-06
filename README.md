# Train Simulation
## File Structure

src
|------text_files
|      |--intersections.txt
|      |--trains.txt
|
|--main.c
|
|  //Parser modules
|--parser.c
|--parser.h
|--parser_test.c //unit test file for parser
|--MakeFile //compiles parser_test.c
|--ptest //exe compiled from parser_test.c. generates and prints successfully


# Development Timeline
## parser.c
### 4.1.2025
Configured to print parsed text. Can be tested directly by running the .exe with command line arguments. - Jarett
### 4.2.2025
parser.c and parser.h completed. defines structs for train and intersection objects. Extracts text from the respective text files located in the text_files directory. Each line is used to create a new instance of train or intersection struct.
Separated parser files into separate directory and updated main to look there for the header file. This way, if changes are made to these functions, the local make file can be run to recompile and perform isolated unit testing
Compilation of main is successful after final push to main. - Jarett
### 4.3.2025
server.c is completed. It is set up to handle ACQUIRE/RELEASE requests from trains via message queues and grants access for intersections. This is some of the code used in Railway_System.c - Jason Greer
### 4.4.2025
Memory_Segments.c completed. Implements a shared memory segment for intersection synchronization, initializing mutexes and named semaphores. - Steve
## Railway_System.c
This is the main file. This file makes calls to the libraries created by the members of the group.
### Compilation
use the Makefile to compile and run for testing. In the terminal within the src directory, enter the following
```bash
make clean
make
```
When compilation is complete run in the first terminal
```bash
./iLikeTrains
```
and in the second terminal run
```bash
./train_sim
```
