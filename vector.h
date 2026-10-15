/** ********************************
* @file vector.h
* @brief controller for vector calculator
* @author Jonathan Vang
* @date 9/29/2026
* @version 1.0
***********************************/

#ifndef VECTOR_H
#define VECTOR_H

#define NAME_LEN 8

typedef struct
{
char name[8];
double x;
double y;
double z;

} vector;

vector add(vector a, vector b);
vector sub(vector a, vector b);
vector scale(vector a, double s);

#endif