CPE-2600 Systems Programming - Jonathan Vang
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



