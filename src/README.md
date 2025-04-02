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
parser.c and parser.h completed. defines structs for train and intersection objects. Extracts text from the respective text files located in the text_files directory. Each line is used to create a new instance of train or intersection struct. - Jarett