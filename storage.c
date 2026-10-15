/** ********************************
* @file storage.c
* @brief storage layer - array is static (file scope), not a global
* @author Jonathan Vang
* @date 9/29/2026
* @version 1.0
***********************************/
#include <string.h>
#include "storage.h"

static vector storage[MAX_VECTORS];

int addvect(vector v)
{
    int empty = -1;
    for (int i = 0; i < MAX_VECTORS; i++)
    {
        if (storage[i].name[0] == '\0')
        {
            if (empty < 0) empty = i;
        }
        else if (strcmp(storage[i].name, v.name) == 0)
        {
            storage[i] = v;        
            return 0;
        }
    }
    if (empty < 0) return -1;       
    storage[empty] = v;
    return 0;
}

int findvect(const char *name, vector *out)
{
    for (int i = 0; i < MAX_VECTORS; i++)
    {
        if (storage[i].name[0] != '\0' && strcmp(storage[i].name, name) == 0)
        {
            *out = storage[i];
            return 0;
        }
    }
    return -1;
}

void clear(void)
{
    memset(storage, 0, sizeof(storage));
}

int countvect(void)
{
    int n = 0;
    for (int i = 0; i < MAX_VECTORS; i++)
        if (storage[i].name[0] != '\0') n++;
    return n;
}

int getvect(int n, vector *out)
{
    int seen = 0;
    for (int i = 0; i < MAX_VECTORS; i++)
    {
        if (storage[i].name[0] != '\0')
        {
            if (seen == n) { *out = storage[i]; return 0; }
            seen++;
        }
    }
    return -1;
}