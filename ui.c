/** ********************************
* @file ui.c
* @brief user interface layer - all console input/output lives here
* @author Jonathan Vang
* @date 10/10/2026
**************************************/
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "ui.h"
#include "vector.h"
#include "storage.h"
 
#define LINE_LEN 128
#define DELIMS " ,\t\r\n"
 
typedef struct
{
    int isa_vector;
    vector v;
    double num;
} operand;
 
static void print_vector(const char *label, vector v)
{
    printf("%s = %g %g %g\n", label, v.x, v.y, v.z);
}
 
static int bool_reserved(const char *s)
{
    return strcmp(s, "quit") == 0 || strcmp(s, "list") == 0 ||
           strcmp(s, "exit") == 0 || strcmp(s, "clear") == 0;
}
 
// token is exactly one of + - * 
static int bool_OP(const char *t)
{
    return t != NULL && t[1] == '\0' &&
           (t[0] == '+' || t[0] == '-' || t[0] == '*');
}
 
static int bool_name(const char *s)
{
    size_t n = strlen(s);
    if (n == 0 || n >= NAME_LEN) return 0;
    if (!isalpha((unsigned char)s[0])) return 0;
    for (size_t i = 1; i < n; i++)
    {
        if (!isalnum((unsigned char)s[i]) && s[i] != '_') return 0;
    }
    return !bool_reserved(s);
}
 
static int parse_number(const char *text, double *out)
{
    char *end;
    if (*text == '\0') return -1;
    *out = strtod(text, &end);
    return (*end == '\0') ? 0 : -1;
}
 
static int get_operand(const char *text, operand *o)
{
    if (isalpha((unsigned char)*text))
    {
        o->isa_vector = 1;
        if (findvect(text, &o->v) != 0)
        {
            printf("Error: vector '%s' does not exist\n", text);
            return -1;
        }
        return 0;
    }
    o->isa_vector = 0;
    if (parse_number(text, &o->num) != 0)
    {
        printf("Error: '%s' is not a vector or a number\n", text);
        return -1;
    }
    return 0;
}
 
// is the + or - at in[i] an operator (vs. a sign like -3 or 1e-5)? 
static int sign_op(const char *in, int i)
{
    int j = i - 1;
    while (j >= 0 && isspace((unsigned char)in[j])) j--;
    if (j < 0) return 0;
 
    int k = j;
    while (k >= 0 && (isalnum((unsigned char)in[k]) || in[k] == '_' || in[k] == '.')) k--;
    if (k == j) return 0;
    return isalpha((unsigned char)in[k + 1]);
}
 
/*
 * Copies in to out, putting spaces around = * + - so strtok can split them.
 */
static void preformat(const char *in, char *out, size_t outsz)
{
    size_t o = 0;
    for (int i = 0; in[i] != '\0' && o + 4 < outsz; i++)
    {
        char c = in[i];
        int spaced = (c == '=' || c == '*' ||
                      ((c == '+' || c == '-') && sign_op(in, i)));
        if (spaced) out[o++] = ' ';
        out[o++] = c;
        if (spaced) out[o++] = ' ';
    }
    out[o] = '\0';
}
 
//computes  tok1 op tok3 . 0 ok, -1 error (prints) 
static int evaluate(const char *tok1, const char *op, const char *tok3, vector *result)
{
    operand l, r;
    if (get_operand(tok1, &l) != 0) return -1;
    if (get_operand(tok3, &r) != 0) return -1;
 
    if (op[0] == '*')
    {
        if (l.isa_vector && !r.isa_vector)
        {
            *result = scale(l.v, r.num);
        }
        else if (!l.isa_vector && r.isa_vector)
        {
            *result = scale(r.v, l.num);
        }
        else
        {
            printf("Error: '*' needs one vector and one number\n");
            return -1;
        }
        return 0;
    }
 
    if (!l.isa_vector || !r.isa_vector)
    {
        printf("Error: '%c' needs two vectors\n", op[0]);
        return -1;
    }
    *result = (op[0] == '+') ? add(l.v, r.v) : sub(l.v, r.v);
    return 0;
}
 
// prints out all the vectors stored in the storage
static void list(void)
{
    int n = countvect();
    if (n == 0)
    {
        printf("No vectors stored\n");
        return;
    }
    for (int i = 0; i < n; i++)
    {
        vector v;
        if (getvect(i, &v) == 0) print_vector(v.name, v);
    }
}
/*
static void print(void)
{


}
 */

// stores result under name and displays it (displayed even if memory full) 
static void show_stored(const char *name, vector result)
{
    strncpy(result.name, name, NAME_LEN - 1);
    result.name[NAME_LEN - 1] = '\0';
 
    if (addvect(result) != 0)
    {
        printf("Error: memory full (max %d vectors). Use 'clear' to empty it\n", MAX_VECTORS);
    }
    print_vector(name, result);
}
 
/*
 * Handles:
 *   a = b          copy
 *   a = 1 2        x y, z = 0
 *   a = 1 2 3      x y z
 *   c = a + b      operation
 */
static void assignments(char *name, char *token3, char *token4, char *token5)
{
    vector result;
    memset(&result, 0, sizeof(result));
 
    if (!bool_name(name))
    {
        printf("Error: '%s' is not a valid vector name\n", name);
        return;
    }
    if (token3 == NULL)
    {
        printf("Error: nothing to assign. Example: a = 1 2 3\n");
        return;
    }
 
    if (bool_OP(token4) && token5 != NULL)               
    {
        if (evaluate(token3, token4, token5, &result) != 0) return;
    }
    else if (token4 == NULL)                          
    {
        operand o;
        if (get_operand(token3, &o) != 0) return;
        if (!o.isa_vector)
        {
            printf("Error: invalid assignment. Example: a = 1 2 3\n");
            return;
        }
        result = o.v;
    }
    else                                                 
    {
        if (parse_number(token3, &result.x) != 0 ||
            parse_number(token4, &result.y) != 0 ||
            (token5 != NULL && parse_number(token5, &result.z) != 0))
        {
            printf("Error: invalid assignment. Example: a = 1 2 3\n");
            return;
        }
    }
 
    show_stored(name, result);
}
 
// returns 1 if the program should quit 
static int vectCalc_UI(char *line)
{
    char spaced[LINE_LEN * 3 + 1];
    char *token1;
    char *token2;
    char *token3;
    char *token4;
    char *token5;
    char *extra;
    vector result;
 
    preformat(line, spaced, sizeof(spaced));
 
    token1 = strtok(spaced, DELIMS);
    token2 = strtok(NULL, DELIMS);
    token3 = strtok(NULL, DELIMS);
    token4 = strtok(NULL, DELIMS);
    token5 = strtok(NULL, DELIMS);
    extra  = strtok(NULL, DELIMS);
 
    if (token1 == NULL) return 0;
    if (extra != NULL)
    {
        printf("Error: too many values in expression\n");
        return 0;
    }
 
    if (token2 == NULL)                       // one word: command or vector name 
    {
        vector v;
        if (strcmp(token1, "quit") == 0 ) return 1;
        if (strcmp(token1, "-h") == 0 ){

         ui_print_help();
         return 0;
        }
        if (strcmp(token1, "clear") == 0)
        {
            clear();
            printf("Memory cleared\n");
        }
        else if (strcmp(token1, "list") == 0) list();
       /* .cvs load and print command
        else if (strcmp(token1, "print") == 0) print();
        else if (strcmp(token1, "load") == 0) load();
        */
        else if (!bool_name(token1) && !strcmp(token1, "-h") == 0) printf("Error: unknown command '%s'\n", token1);
        else if (findvect(token1, &v) != 0) printf("Vector '%s' does not exist\n", token1);
        else print_vector(v.name, v);
        return 0;
    }
 
    if (strcmp(token2, "=") == 0)        /* assignment */
    {
        assignments(token1, token3, token4, token5);
        return 0;
    }
 
      // operator is token2, found even if the user typed no spaces. 
    if (bool_OP(token2) && token3 != NULL && token4 == NULL)
    {
        if (evaluate(token1, token2, token3, &result) == 0)
            print_vector("ans", result);
        return 0;
    }
 
    printf("Error: invalid expression\n");
    return 0;
}
 
void ui_print_help(void)
{
    printf("minimat - 3D vector calculator\n");
    printf("Commands:\n");
    printf("  a = 1 2 3        create/replace a vector. use " " or ,'s\n");
    printf("  a                show's vector \n");
    printf("  a + b            add two vectors\n");
    printf("  a - b            subtract two vectors\n");
    printf("  c = a + b        stores result of 2 vector product\n");
    printf("  list             show all stored vectors\n");
    printf("  clear            empty vector memory\n");
    printf("  quit            exit the program\n");
    printf("Up to %d vectors. Names: letter first, max %d characters.\n", MAX_VECTORS, NAME_LEN - 1);
}
 // runs calculator
void ui_run(void)
{
    char line[LINE_LEN];
    while (1)
    {
        printf("minimat> ");
        fflush(stdout);
        if (fgets(line, sizeof(line), stdin) == NULL) break;
        if (strchr(line, '\n') == NULL && !feof(stdin))
        {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF) { }
            printf("Error: line too long\n");
            continue;
        }
        if (vectCalc_UI(line)) break;
    }
}