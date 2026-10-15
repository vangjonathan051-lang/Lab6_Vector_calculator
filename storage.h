/** ********************************
* @file storage.h
* @brief storage layer for the vector array
* @author Jonathan Vang
* @date 9/29/2026
* @version 1.0
***********************************/
#include "vector.h"

#define MAX_VECTORS 10

/* add or replace a vector by name. returns 0 on success, -1 if full */
int addvect(vector v);

/* copy the named vector into *out. returns 0 if found, -1 if not */
int findvect(const char *name, vector *out);

/* empty the storage array */
void clear(void);

/* number of vectors currently stored */
int countvect(void);

/* copy the n-th stored vector (0..countvect()-1) into *out. 0 ok, -1 bad index */
int getvect(int n, vector *out);

