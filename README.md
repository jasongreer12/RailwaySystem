# Train Simulation
## File Structure
```
src
|
|  //main operational files
|--|Railway_System.c
|--|Makefile
|--|server.c //original code used as server. this has 
|   been copied into Railway_System.c. server.c is no longer
|   used in running the file.
|
|
|  //text files for import
|------text_files
|      |--intersections.txt
|      |--trains.txt
|
|  //Parser Modules
|------parser
|      |--parser.c
|      |--parser.h
|      |--parser_test.c //unit test file for parser (not used in compiled product)
|      |--MakeFile //compiles parser_test.c (not used in compiled product)
|      |--ptest //exe compiled from parser_test.c. generates and print
|         successfully (not used in compiled product)
|
|  //Shared Memory Modules 
|------Shared_Memory_Setup
|      |--Memory_Segments.c
|      |--Memory_Segments.h
|
|  //IPC modules
|------Basic_IPC_Workflow
       |--intersection_locks.c
       |--intersection_locks.h
       |--ipc.c
       |--ipc.h
       |--Train_Movement_Simulation.c
       |--Train_Movement_Simulation_Test.c //Non-essential file that can be used in place of Train_Movement_Simulation 
                                           //for testing that trains fork successfully and that message queues are working.
```

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
