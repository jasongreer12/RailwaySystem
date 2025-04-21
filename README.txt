# Train Simulation
## File Structure
There are files included that were used to test certain functions. These files are generally not listed. If a file is not listed, it is not critical to the operation of the program.
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
|      |--trainsRandomizer.c // non-essential file that updates trains.txt have a random route
|      |--intersectionsRandomizer.c // non-essential file that updates intersections.txt to have a random size
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
|      |--intersection_locks.c
|      |--intersection_locks.h
|      |--ipc.c
|      |--ipc.h
|      |--Train_Movement_Simulation.c
|      |--Train_Movement_Simulation_Test.c //Non-essential file that can be used in place of Train_Movement_Simulation 
|                                          //for testing that trains fork successfully and that message queues are working.
|
|------logger
       |--logger.c
       |--logger.h
       |--csv_logger.c
       |--csv_logger.h

```


### Compilation
use the Makefile to compile and run for testing. In the terminal within the src directory, enter the following


make clean
make

When compilation is complete run in the first terminal

./iLikeTrains

and in a second terminal run

./train_sim

These will run and print information to the screen as well as update the simulation.log file int the src folder.

### Compilation Testing
#### 4.20.2025
Compilation was tested locally and confirmed working on csx1.cs.okstate.edu
