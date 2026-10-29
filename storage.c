/** ********************************
* @file storage.c
* @brief storage layer - dynamic array that grows with realloc
* @author Jonathan Vang
* @date 9/29/2026
* @version 2.0
***********************************/
#include <string.h>
#include <stdlib.h>
#include "storage.h"

#define INITIAL_CAPACITY 4

int addvect(vectStore *s,vector v){ 
    for (int i = 0; i < s->count; i++) {
        if (strcmp(s->data[i].name, v.name) == 0){
            s->data[i] = v;        
            return 0;
        }
    }

    if (s->count == s->capacity){
        int newCap;
        if (s->capacity == 0){
          newCap = INITIAL_CAPACITY;
        } else {
            newCap = s->capacity * 2;
        }
        vector *bigger = realloc(s->data, (size_t)newCap * sizeof(vector));
        if (bigger == NULL) return -1;
        s->data = bigger;
        s->capacity = newCap;
    }      
    s->data[s->count++] = v;
    return 0;
}

int findvect(const vectStore *s,const char *name, vector *out){
    for (int i = 0; i < s->count; i++){
        if (strcmp(s->data[i].name, name) == 0){
            *out = s->data[i];
            return 0;
        }
    }
    return -1;
}

void clear(vectStore *s){
    free(s->data);
    s->data = NULL;
    s->count = 0;
    s->capacity = 0;
}

int countvect(const vectStore *s){
    return s->count;
}

int getvect(const vectStore *s,int n, vector *out) {
   if (n <0 || n >= s->count) return -1;
    *out = s->data[n];
         
    return 0;
}