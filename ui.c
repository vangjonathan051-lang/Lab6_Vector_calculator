#define _POSIX_C_SOURCE 200809L   /* for sigaction */
/** ********************************
* @file ui.c
* @brief user interface layer - all console input/output lives here
* @author Jonathan Vang
* @date 9/29/2026
* @version 3.0
*
**************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include "ui.h"
#include "vector.h"
#include "storage.h"

#define LINE_LEN 128
#define DELIMS " ,\t\r\n"

typedef struct{
    int isa_vector;
    vector v;
    double num;
} operand;

static void print_vector(const char *label, vector v){
    printf("%s = %g %g %g\n", label, v.x, v.y, v.z);
}

static int bool_reserved(const char *s){
    return strcmp(s, "quit") == 0 || strcmp(s, "list") == 0 ||
           strcmp(s, "exit") == 0 || strcmp(s, "clear") == 0 ||
           strcmp(s, "load") == 0 || strcmp(s, "save") == 0 ||
           strcmp(s, "fill") == 0;
}

/* token is exactly one of + - * */
static int bool_OP(const char *t){
    return t != NULL && t[1] == '\0' &&
           (t[0] == '+' || t[0] == '-' || t[0] == '*');
}

static int bool_name(const char *s){
    size_t n = strlen(s);
    if (n == 0 || n >= NAME_LEN) return 0;
    if (!isalpha((unsigned char)s[0])) return 0;
    for (size_t i = 1; i < n; i++)
    {
        if (!isalnum((unsigned char)s[i]) && s[i] != '_') return 0;
    }
    return !bool_reserved(s);
}

static int parse_number(const char *text, double *out){
    char *end;
    if (*text == '\0') return -1;
    *out = strtod(text, &end);
    return (*end == '\0') ? 0 : -1;
}

static int get_operand(const vectStore *st, const char *text, operand *o){
    if (isalpha((unsigned char)*text))
    {
        o->isa_vector = 1;
        if (findvect(st, text, &o->v) != 0)
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

/* is the + or - at in[i] an operator (vs. a sign like -3 or 1e-5)? */
static int sign_op(const char *in, int i){
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
 * UPDATE (Req #5 assignment, #7 addition, #8 subtraction,
 *         #9 scalar multiplication, #10 operation plus assignment):
 *   the spec lets us require spaces around '=' and the operator. We don't
 *   require them; this function adds them if the user left them out.
 */
static void preformat(const char *in, char *out, size_t outsz){
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

/* computes  tok1 op tok3 . 0 ok, -1 error (prints) */
static int evaluate(const vectStore *st, const char *tok1, const char *op,
                    const char *tok3, vector *result) {
    operand l, r;
    if (get_operand(st, tok1, &l) != 0) return -1;
    if (get_operand(st, tok3, &r) != 0) return -1;

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

/* prints out all the vectors stored in the storage */
static void list(const vectStore *st)
{
    int n = countvect(st);
    if (n == 0)
    {
        printf("No vectors stored\n");
        return;
    }
    for (int i = 0; i < n; i++)
    {
        vector v;
        if (getvect(st, i, &v) == 0) print_vector(v.name, v);
    }
}

/* stores result under name and displays it (displayed even if memory full) */
static void show_stored(vectStore *st, const char *name, vector result)
{
    strncpy(result.name, name, NAME_LEN - 1);
    result.name[NAME_LEN - 1] = '\0';

    if (addvect(st, result) != 0)
    {
        printf("Error: out of memory, vector '%s' was not stored\n", name);
    }
    print_vector(name, result);
}

/*
 * Handles   name = ...   (tokens after the '=' are token3..token5)
 *   a = b          copy
 *   a = 1 2        x y, z = 0
 *   a = 1 2 3      x y z
 *   c = a + b      operation
 */
static void assignments(vectStore *st, char *name, char *token3,
                        char *token4, char *token5)
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

    if (bool_OP(token4) && token5 != NULL)                /* Req #10 */
    {
        if (evaluate(st, token3, token4, token5, &result) != 0) return;
    }
    else if (token4 == NULL)                               /* a = b */
    {
        operand o;
        if (get_operand(st, token3, &o) != 0) return;
        if (!o.isa_vector)
        {
            printf("Error: invalid assignment. Example: a = 1 2 3\n");
            return;
        }
        result = o.v;
    }
    else                                                   /* Req #5: a = x y [z] */
    {
        if (parse_number(token3, &result.x) != 0 ||
            parse_number(token4, &result.y) != 0 ||
            (token5 != NULL && parse_number(token5, &result.z) != 0))
        {
            printf("Error: invalid assignment. Example: a = 1 2 3\n");
            return;
        }
    }

    show_stored(st, name, result);
}

// -------------------------- file I/O and fill (Lab 7) 
static void rtrim(char *s)
{
    size_t n = strlen(s);
    while (n > 0 && isspace((unsigned char)s[n - 1])) s[--n] = '\0';
}

static char *ltrim(char *s)
{
    while (isspace((unsigned char)*s)) s++;
    return s;
}

/*
 * load <fname>: reads a CSV file (name,x,y,z per line).
 */
static void do_load(vectStore *st, const char *fname)
{
    FILE *fp = fopen(fname, "r");
    char buf[256];
    int lineno = 0, loaded = 0, bad = 0;

    if (fp == NULL) // or (!fp)
    {
        printf("Error: cannot open file '%s'\n", fname);
        return;
    }

    while (fgets(buf, sizeof(buf), fp) != NULL)
    {
        char *p = buf;
        char *fields[5];
        int n = 0;
        vector v;
        lineno++;

        /* line longer than the buffer: count it as bad and skip the rest */
        if (strchr(buf, '\n') == NULL && !feof(fp))
        {
            int ch;
            while ((ch = fgetc(fp)) != '\n' && ch != EOF) { }
            printf("Line %d: line too long, skipped\n", lineno);
            bad++;
            continue;
        }

        /* skip a UTF-8 byte order mark on the first line */
        if (lineno == 1 && (unsigned char)p[0] == 0xEF &&
            (unsigned char)p[1] == 0xBB && (unsigned char)p[2] == 0xBF)
            p += 3;

        rtrim(p);                       /* removes \n, \r and trailing spaces */
        if (*ltrim(p) == '\0') continue; /* blank line */

        /* split on commas (keeps empty fields so "a,,1,2" is caught) */
        char *cur = p;
        while (n < 5)
        {
            fields[n++] = cur;
            char *comma = strchr(cur, ',');
            if (comma == NULL) break;
            *comma = '\0';
            cur = comma + 1;
        }

        memset(&v, 0, sizeof(v));
        if (n != 4)
        {
            printf("Line %d: expected name,x,y,z - skipped\n", lineno);
            bad++;
            continue;
        }
        for (int i = 0; i < 4; i++)
        {
            fields[i] = ltrim(fields[i]);
            rtrim(fields[i]);
        }
        if (!bool_name(fields[0]) ||
            parse_number(fields[1], &v.x) != 0 ||
            parse_number(fields[2], &v.y) != 0 ||
            parse_number(fields[3], &v.z) != 0)
        {
            printf("Line %d: invalid name or number - skipped\n", lineno);
            bad++;
            continue;
        }

        strncpy(v.name, fields[0], NAME_LEN - 1);
        v.name[NAME_LEN - 1] = '\0';
        if (addvect(st, v) != 0)
        {
            printf("Error: out of memory, stopped loading\n");
            break;
        }
        loaded++;
    }
    fclose(fp);

    printf("Loaded %d vector(s) from '%s'", loaded, fname);
    if (bad > 0) printf(" (%d bad line(s) skipped)", bad);
    printf("\n");
}

/* save <fname>: writes every stored vector as name,x,y,z (overwrites the file) */
static void do_save(const vectStore *st, const char *fname)
{
    FILE *fp = fopen(fname, "w");
    int n = countvect(st);

    if (fp == NULL)
    {
        printf("Error: cannot write file '%s'\n", fname);
        return;
    }
    for (int i = 0; i < n; i++)
    {
        vector v;
        if (getvect(st, i, &v) == 0)
            fprintf(fp, "%s,%.15g,%.15g,%.15g\n", v.name, v.x, v.y, v.z);
    }
    if (fclose(fp) != 0)
    {
        printf("Error: problem writing '%s'\n", fname);
        return;
    }
    printf("Saved %d vector(s) to '%s'\n", n, fname);
}

/* fill <count>: makes vectors v1..vN to exercise dynamic memory.
   N is limited to 999999 */
static void do_fill(vectStore *st, const char *arg)
{
    char *end;
    long n = strtol(arg, &end, 10);
    long made = 0;

    if (*end != '\0' || n < 1 || n > 999999)
    {
        printf("Error: fill needs a count from 1 to 999999\n");
        return;
    }
    for (long i = 1; i <= n; i++)
    {
        vector v;
        char tmp[24];
        memset(&v, 0, sizeof(v));
        snprintf(tmp, sizeof(tmp), "v%ld", i);
        strncpy(v.name, tmp, NAME_LEN - 1);
        v.x = (double)i;
        v.y = (double)i * 2.0;
        v.z = (double)i * 3.0;
        if (addvect(st, v) != 0)
        {
            printf("Error: out of memory after %ld vectors\n", made);
            return;
        }
        made++;
    }
    printf("Filled %ld vector(s): v1 to v%ld\n", made, n);
}

/* returns 1 if the program should quit */
static int vectCalc_UI(vectStore *st, char *line)
{
    char spaced[LINE_LEN * 3 + 1];
    char *token1;
    char *token2;
    char *token3;
    char *token4;
    char *token5;
    char *extra;
    vector result;

    /* load / save / fill take a file name or count, so they are handled
       BEFORE preformat (a name like my-data.csv must not be split at '-') */
    {
        char raw[LINE_LEN + 1];
        strncpy(raw, line, LINE_LEN);
        raw[LINE_LEN] = '\0';
        char *cmd = strtok(raw, " \t\r\n");
        if (cmd != NULL && (strcmp(cmd, "load") == 0 ||
                            strcmp(cmd, "save") == 0 ||
                            strcmp(cmd, "fill") == 0))
        {
            char *arg = strtok(NULL, " \t\r\n");
            char *more = strtok(NULL, " \t\r\n");
            if (arg == NULL || more != NULL)
                printf("Usage: %s <%s>\n", cmd,
                       strcmp(cmd, "fill") == 0 ? "count" : "fname");
            else if (strcmp(cmd, "load") == 0) do_load(st, arg);
            else if (strcmp(cmd, "save") == 0) do_save(st, arg);
            else                               do_fill(st, arg);
            return 0;
        }
    }

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

    if (token2 == NULL)                       /* one word: command or name */
    {
        vector v;
        if (strcmp(token1, "quit") == 0 || strcmp(token1, "exit") == 0) return 1;
        if (strcmp(token1, "-h") == 0 ){

         ui_print_help();
         return 0;
        }
        if (strcmp(token1, "clear") == 0)
        {
            clear(st);
            printf("Memory cleared\n");
        }
        else if (strcmp(token1, "list") == 0) list(st);
        else if (!bool_name(token1)) printf("Error: unknown command '%s'\n", token1);
        else if (findvect(st, token1, &v) != 0) printf("Vector '%s' does not exist\n", token1);
        else print_vector(v.name, v);
        return 0;
    }

    if (strcmp(token2, "=") == 0)             /* assignment */
    {
        assignments(st, token1, token3, token4, token5);
        return 0;
    }

    if (bool_OP(token2) && token3 != NULL && token4 == NULL)
    {
        if (evaluate(st, token1, token2, token3, &result) == 0)
            print_vector("ans", result);
        return 0;
    }

    printf("Error: invalid expression\n");
    return 0;
}

void ui_print_help(void)
{
    printf("minimat - 3D vector calculator\n");
    printf("Usage: minimat [-h]\n\n");
    printf("Commands:\n");
    printf("  a = 1 2 3        create/replace a vector (spaces or commas; z optional)\n");
    printf("  a                show vector a\n");
    printf("  a + b            add two vectors\n");
    printf("  a - b            subtract two vectors\n");
    printf("  a * 2  /  2 * a  scalar multiplication\n");
    printf("  c = a + b        do an operation and store the result\n");
    printf("  list             show all stored vectors\n");
    printf("  clear            empty vector memory\n");
    printf("  load <fname>     load vectors from a CSV file (name,x,y,z)\n");
    printf("  save <fname>     save all vectors to a CSV file\n");
    printf("  fill <count>     make vectors v1..vN (test dynamic memory)\n");
    printf("  quit             exit the program\n");
    printf("No fixed vector limit. Names: letter first, max %d characters.\n", NAME_LEN - 1);
}

/* Ctrl-C: nothing to do here. It only has to exist so the signal does not
   kill the program; it makes fgets return, ui_run frees the storage and
   returns, and main exits normally. */
static void on_sigint(int sig)
{
    (void)sig;
}

void ui_run(void)
{
    char line[LINE_LEN];
    struct sigaction sa;
    vectStore store = {NULL, 0, 0};   /* local - owned here, passed by pointer */

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_sigint;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);      /* no SA_RESTART, so fgets returns */

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
        if (vectCalc_UI(&store, line)) break;
    }
    if (ferror(stdin)) printf("\n");   /* fgets was interrupted by Ctrl-C */
    clear(&store);                     /* free all vector memory before returning */
}