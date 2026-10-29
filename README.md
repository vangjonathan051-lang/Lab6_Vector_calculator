Vector Calculator

CPE-2600 Systems Programming - Jonathan Vang

Description:

An interactive command-line calculator for 3D vectors, similar to a small Matlab. Each vector has a name and three double values (x, y, z). At the minimat> prompt you can create vectors, add and subtract them, multiply them by a number, and save or load them as CSV files.

How it works:

main.c handles the -h option, then starts the user interface.
ui.c reads a line, splits it into tokens with strtok, and runs the command.
vector.c does the math (add, sub, scale).
storage.c keeps the vectors in memory.

Spaces around =, +, - and * are optional, so c=a+b and c = a + b both work.

Building:

Requirements: gcc and make.

Go to the project folder.
Run make clean to remove old build files.
Run make to build. The program is called vectorCalculator.
Running
./vectorCalculator

Command line options:

-h - print a help summary and exit

To check for memory leaks:

valgrind --leak-check=full ./vectorCalculator
Commands

At the minimat> prompt:

a = 1 2 3 - create or replace vector a (spaces or commas; z is optional and defaults to 0)
a - show vector a
a + b - add two vectors
a - b - subtract two vectors
a * 2 or 2 * a - scalar multiplication
c = a + b - do an operation and store the result in c
list - show all stored vectors
clear - delete all stored vectors and free their memory
load <fname> - load vectors from a CSV file
save <fname> - save all vectors to a CSV file (overwrites the file)
fill <count> - create v1 to vN (1 to 999999) to test dynamic memory
quit - exit the program

Notes:

Vector names start with a letter and are at most 7 characters.
load, save, fill, list, clear, quit and exit cannot be used as vector names.
A result that is not assigned is shown as ans.
CSV format

One vector per line: name,x,y,z

a,1,2,3
b,4.4,5.5,6.6
load merges the file into memory. A vector with the same name is replaced.
load accepts \n or \r\n line endings and ignores blank lines.
A missing file or a badly formatted line is reported and the program keeps running.
Dynamic Memory

The vectors are stored in a dynamic array that grows as needed, so there is no fixed limit on how many vectors can be stored.

A vectStore struct holds a pointer to the array (data), the number of vectors in use (count), and the number of slots allocated (capacity).
ui_run creates the vectStore as a local variable and passes a pointer to it to every function that needs it. There are no global variables.
The array starts empty. The first vector allocates 4 slots with realloc.
When the array is full, realloc doubles its size (4, 8, 16, ...). If realloc fails, the old data is kept and an error is shown.
clear calls free on the array and resets the struct.
ui_run calls clear before it returns, so all memory is freed when the program quits, reaches end of input, or is stopped with Ctrl-C.
Valgrind reports no leaks at exit.
Files
main.c - program start and -h option
ui.c, ui.h - user interface, parsing, load and save
storage.c, storage.h - dynamic vector storage
vector.c, vector.h - vector type and math
Makefile - build instructions
Releases
v1.0 - Lab 5: fixed storage for 10 vectors
v2.0 - Lab 7: dynamic memory and CSV load/saveCPE-2600 Systems Programming - Jonathan Vang
Vector Calculator:
FILES:
main.c
ui.c
vector.c
storage.c

commands that can be done in Calculator:
a = 1 2 3 - create or replace vector a (spaces or commas; z is optional and defaults to 0)
a - show vector a
a + b - add two vectors
a - b - subtract two vectors
a * 2 or 2 * a - scalar multiplication
c = a + b - do an operation and store the result in c
list - show all stored vectors
clear - delete all stored vectors and free their memory
load <fname> - load vectors from a CSV file
save <fname> - save all vectors to a CSV file (overwrites the file)
fill <count> - create v1 to vN (1 to 999999) to test dynamic memory
quit - exit the program



