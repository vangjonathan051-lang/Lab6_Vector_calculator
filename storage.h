/*************************************
 * @file storage.h
 * @brief Stores Vectors
 * @author Jonathan Vang
 *  @date 10/20
 *  @version 2.0
 **********************************/
#ifndef STORAGE_H
#define STORAGE_H

 #include "vector.h"

 typedef struct
{
    vector *data;    /* heap block holding the vectors */
    int count;       /* vectors in use */
    int capacity;    /* vectors the block has room for */
} vectStore;

/* add or replace a vector by name. returns 0 on success, -1 if full */
int addvect(vectStore *s, vector v);

/* copy the named vector into *out. returns 0 if found, -1 if not */
int findvect(const vectStore *s,const char *name, vector *out);

/* empty the storage array */
void clear(vectStore *s);

/* number of vectors currently stored */
int countvect(const vectStore *s);

/* copy the n-th stored vector (0..countvect()-1) into *out. 0 ok, -1 bad index */
int getvect(const vectStore *s,int n, vector *out);

#endif