/** ********************************
* @file main.c
* @brief controller for vector calculator
* @author Jonathan Vang
* @date 9/29/2026
* @version 1.0
***********************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ui.h"
#include "storage.h"



int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        if (strcmp(argv[1], "h") == 0)
        {
            ui_print_help();
            return 0;
        }
        printf("Unknown option '%s'. Use h for help\n", argv[1]);
        return 1;
    }
 
    ui_run();
    return 0;
}