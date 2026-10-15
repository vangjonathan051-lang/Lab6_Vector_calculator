/** ********************************
* @file vectCalc.c
* @brief controller for vector calculator
* @author Jonathan Vang
* @date 9/29/2026
***********************************/

#include <string.h>
#include "vector.h"



vector add(vector a, vector b){

    vector returnval;

    returnval.x = a.x + b.x;
    returnval.y = a.y + b.y;
    returnval.z = a.z + b.z;

    return returnval;
}

vector sub(vector a, vector b){

    vector returnval;

    returnval.x = a.x - b.x;
    returnval.y = a.y - b.y;
    returnval.z = a.z - b.z;

    return returnval;
}

vector scale(vector a, double s)
{
    vector returnval;
    memset(&returnval, 0, sizeof(returnval));
    returnval.x = a.x * s;
    returnval.y = a.y * s;
    returnval.z = a.z * s;
    return returnval;
}