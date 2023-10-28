/* ANSIJSON 0.3.1
 * https://ansijson.com
 * Written by : Chester Abrahams
 * Portfolio  : https://atomiccoder.com
 * LinkedIn   : https://www.linkedin.com/in/atomiccoder/ */

#ifndef ANSIJSON_H
#define ANSIJSON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef struct aJSON {
    struct aJSON    *next, *prev,           /* Double link listed neighbors */
                    *child,                 /* Pointer to the child struct if it's an object or array */
                    *parent;                /* Pointer to the parent struct  */
    union {         int integer;            /* The value represented as an integer */
                    double floatval;        /* The value represented as a floating-point number */
                    char *string; };        /* A pointer to a string (if the value is a string) */
    unsigned int    index_neighbor,         /* Index of the struct among its neighboring structs */
                    index_nested,           /* Index of the struct within its parent (child index) */
                    decimal_places;         /* The number of decimal places (if the value is a floating-point number) */
    char            *key,                   /* A pointer to a member key string if the struct is a member of an object */
                    type,                   /* Type of the value: (0=Container, 1=Number, 2=String, 3=Boolean) */
                    is_null,                /* True if the value is NULL (if the value is a boolean) */
                    is_signed,              /* True if the number value is signed (negative) */
                    has_decimal_point;      /* True if the number value has a decimal point */

#ifdef __cplusplus /* Optional C++ methods */
    char *encode(int format = 0);
    struct aJSON *access(char *path);
    struct aJSON *erase();
    struct aJSON *append(struct aJSON *src);
    struct aJSON *append(char *src);
    void free();
#endif
} aJSON;

struct aJSON *decodeAJSON (char *srcArg)
{
    unsigned        heap_struct_c = 1, heap_struct_max = 0xFF;
    struct aJSON    *heap = (struct aJSON*) calloc(heap_struct_max, sizeof(struct aJSON)),
                    *heap_current = heap,
                    *parse = heap + 1,
                    *parse0 = parse;
    char            *src = srcArg,
                    *src0 = src,
                    c;
    double          A=0,X=0,Y=0; /* Multi-purpose variables */
    int             Z=0,W=0,K=0,H=0;

    /* Init heap */
    heap->parent = heap;
    heap->child = parse;
    heap->integer = 0x1F1F;

    goto _LEX_CONTAINER; /* JSON always starts with a container */

    _RTS:
    while (*src && *src<0x21) src++;
    if (*src && *src==0x5D || *src==0x7D) src++;
    switch ((unsigned long long) parse->parent) { /* Return subroutine */
        case 0: goto _EOF;
        default: parse = parse->parent; goto _LEX_CONTAINER;
    }

    _LEX_CONTAINER:
    while (*src && *src<0x21) src++;
    if (!*src) goto _EOF;
    switch (*src) { /* Switch between element (array) or member (object) */
        case 0x5B: case 0x2C: src++; goto _LEX_VALUE;
        case 0x5D: case 0x7D:        goto _LEX_VALUE;
        case 0x7B: Y=1;              goto _LEX_MEMBER;
        default:                     goto _ERROR;
    }

    _LEX_MEMBER:
    if (*src && *src==0x5B || *src==0x7B) src++; /* Y = (bool) Is a member string: use *key as parse target and return to _LEX_MEMBER */
    while (*src && *src<0x21) src++;
    if (*src != 0x22) { parse->key = NULL; goto _LEX_VALUE; }
    else if (Y) goto _LEX_STRING;
    else        goto _LEX_VALUE;

    _LEX_VALUE:
    while (*src && *src<0x21) src++;
    if (*src==0x2C) src++;
    while (*src && *src<0x21) src++;
    if (*src == 0x5D || *src == 0x7D) goto _RTS; /* Return subroutine if closing delimiter is encountered */

    /* Allocate new heap page when it's exceeded heap_struct_max */
    if (heap_struct_c+2 >= heap_struct_max)
    {
        heap_current->next = (struct aJSON*) calloc(heap_struct_max, sizeof(struct aJSON));
        heap_current->next->prev = heap_current;
        heap_current = heap_current->next;

        heap_current->parent = heap;
        heap_current->child = parse;
        heap_current->integer = 0x1F1F;
        heap_struct_c = 0;

        (heap_current+1)->index_neighbor = parse->index_neighbor;
        (heap_current+1)->prev = parse->prev;
        (heap_current+1)->key = parse->key;
    }

    /* Next element/member */
    if (parse->type || parse->floatval || parse->string || parse->child) {
        parse->next = heap_current+(++heap_struct_c);
        heap_current->index_neighbor++;
        if (!heap_current->child) heap_current->child = parse->next;
        parse->next->prev = parse;
        parse = parse->next;
        parse->index_neighbor = parse->prev->index_neighbor+1;
        parse->index_nested = parse->prev->index_nested;
        parse->parent = parse->prev->parent;
        if (parse->prev && parse->prev->key) { Y=1; goto _LEX_MEMBER; } /* Lex member (object) if previous struct has a key */
    }

    /* Child nesting */
    if (!parse->child && (*src==0x5B || *src==0x7B)) {
        parse->child = heap_current+(++heap_struct_c);
        heap_current->index_neighbor++;
        if (!heap_current->child) heap_current->child = parse->child;
        parse->child->parent = parse;
        parse = parse->child;
        parse->index_nested = parse->parent->index_nested+1;
        if (*src==0x7B) goto _LEX_CONTAINER; else { src++; goto _LEX_VALUE; } /* Switch between lexing member (object) or element (array) */
    }

    /* Switch to specific value and jump to value lexer */
    switch (*src) {
        /* Container */    case 0x5B: case 0x7B:           goto _LEX_CONTAINER;
        /* Number */       case 0x2D: case 0x30 ... 0x39:  parse->type=1; goto _LEX_NUMBER;
        /* String */       case 0x22:                      parse->type=2; goto _LEX_STRING;
        /* Boolean */      case 0x61 ... 0x7A:             parse->type=3; goto _LEX_BOOLEAN;
        /* End of file */  case 0x00:                      goto _EOF;
        /* Error */        default:                        goto _ERROR;
    }

    _LEX_NUMBER: /* Y=Decimal point location, X=Sign flag */
    X = *src==0x2D && src++?-1:1; Y = 1;
    if (*src<0x30 || *src>0x39) goto _ERROR;
    if (X<0) parse->is_signed = 1;

    _PARSE_DIGIT:
    while (*src==0x2E || *src==0x45 || *src==0x65 || *src==0x2B || *src==0x2D) if (*(src++)==0x2E && Y==1) Y=-1; /* Determine sign */
    if (Y<1) { Y /= 10; parse->decimal_places++; }; /* Locate decimal placement */
    parse->floatval = parse->floatval * 10 + *(src++) - 0x30; /* Parse digit */
    if (*src>0x20 && *src!=0x2C && *src!=0x5D && *src!=0x7D) goto _PARSE_DIGIT;
    parse->floatval *= (double) Y * (Y<1?-X:X); /* Flip to positive number */
    if (Y<1) parse->has_decimal_point=1; else parse->integer = (int) parse->floatval;
    X=Y=0; goto _LEX_VALUE;

    _LEX_STRING: /* A=Allocation Size, X=Char Counter, Y=Is_Key Boolean */
    if (*src == 0x22) src++;
    parse->string = (char*) calloc((size_t) (A=0xFFF), sizeof(char)); X=-1;
    _PARSE_CHAR: /* Character parsing loop */
    if (*src==0x5C) { /* Escape sequences and converting UNICODE characters */
        src++; X++;
        switch (*src) {
            case '\\': case '/': case 'b':
            case 'f':  case 'n':  case 'r' :
            case 't' : case '\"':
                parse->string[(int)X++] = '\\';
                parse->string[(int)X] = *src++; break;
            case 'u': Z = *(++src); W = 0; W = 0;
                for (H = 0; H < 4; H++) { /* Convert hex characters to int */
                    W = W * 16 + ((*src >= 'A' && *src <= 'F') ? *src - 'A' + 10 :
                                  (*src >= 'a' && *src <= 'f') ? *src - 'a' + 10 :
                                  (*src >= '0' && *src <= '9') ? *src - '0' : -1);
                    Z = *(++src); }
                Z = (W <= 0x7F) ? 1 : (W <= 0x7FF) ? 2 : (W <= 0xFFFF) ? 3 : 4; /* Count number of bytes used */
                K = (Z == 1) ? 0x00 : (Z == 2) ? 0xC0 : (Z == 3) ? 0xE0 : 0xF0; /* Find leading byte */
                for (H = Z - 1; H > 0; H--) parse->string[(int)X + H] = (char)((W & 0x3F) | 0x80), W >>= 6; /* Encode bytes individually */
                c = (char)(W | K);
                if (Z == 1 && (c=='\\'&&(c='\\'))||(c=='\/'&&(c='/'))||(c=='\b'&&(c='b'))||(c=='\f'&&(c='f'))||(c=='\n'&&(c='n'))||(c=='\r'&&(c='r'))||(c=='\t'&&(c='t'))||(c=='\"'&&(c='"'))) parse->string[(int)X++] = '\\';
                else if (Z == 1) { for (Z=0,src-=6;Z<6;Z++) parse->string[(int)X++] = *(src++); X--; goto _PARSE_CHAR; } /* Don't proceed with encoding if it's 1 byte */
                parse->string[(int)X] = c; /* Encode first byte of UTF-8 encoding */
                X += Z-1; goto _PARSE_CHAR;
            default: parse->string[(int)X] = *src++; break;
        }
    }
    if (++X+1 > A) parse->string = (char*) realloc(parse->string, sizeof(char) * (int)(A+=0xFFFF)); /* Extend string buffer */
    if (*src != 0x22) parse->string[(int)X] = (char) *(src++);
    if (*src != 0x22) goto _PARSE_CHAR;
    parse->string[(int)++X] = 0x00; /* Insert NUL terminator (end of string) and then resize */
    if (X < A) parse->string = (char*) realloc(parse->string, sizeof(char) * (int)X+1); src++;

    if (Y) { /* Continue to lex member if this string was a key */
        parse->key = parse->string;
        parse->string = 0; Y = 0;
        while (*src && *src==0x3A || *src<0x21) src++;
        goto _LEX_VALUE;
    } else goto _LEX_VALUE;

    _LEX_BOOLEAN:
    for (Y=0, X=1; X && src[(int)Y] && "true"[(int)Y];  Y++) if (src[(int)Y] != "true"[(int)Y])  X=0; else if (!"true"[(int)Y+1])  { src+=4; parse->floatval=(double)1; goto _LEX_VALUE; }
    for (Y=0, X=1; X && src[(int)Y] && "false"[(int)Y]; Y++) if (src[(int)Y] != "false"[(int)Y]) X=0; else if (!"false"[(int)Y+1]) { src+=5; parse->floatval=(double)0; goto _LEX_VALUE; }
    for (Y=0, X=1; X && src[(int)Y] && "null"[(int)Y];  Y++) if (src[(int)Y] != "null"[(int)Y])  X=0; else if (!"null"[(int)Y+1])  { src+=4; parse->floatval=(double)0; parse->is_null=1; goto _LEX_VALUE; }

    _ERROR: X=Y=1; /* X=Line, Y=Column */
    while (src0<src) if (*(src0++)!=0x0A) Y++; else { X++; Y=1; } /* Find line/column */
    fprintf(stderr, "ANSIJSON Fatal error: unexpected '%c' at %d:%d\n", *src, (int)X, (int)Y);
    return 0;

    _EOF: return parse0;
}

char *encodeAJSON (struct aJSON *srcArg, unsigned int format)
{
    struct aJSON *src = srcArg;
    long         rtn_max = 0xFFF,
                 rtn_pos,
                 mul_string = 1,
                 mul_rts = 1,
                 i;
    char         *rtn = (char*) calloc(rtn_max, sizeof(char)),
                 *rtn0 = rtn,
                 sample[0x20],
                 *c;

    _RTS:
    /* Extend return string */
    if ((rtn-rtn0)+(sizeof(char)*3) >= rtn_max) {
        rtn_pos = rtn - rtn0;
        c = (char*) calloc(rtn_pos + (rtn_max+=(0xFFFF * mul_rts++)), sizeof(char));
        for (rtn=c;*rtn0;rtn0++) *(c++) = *rtn0;
        c = rtn;
        rtn0 = c;
        rtn = rtn0 + rtn_pos;
    }

    /* Add opening delimiter */
    if (!src->prev) {
        *(rtn++) = src->key ? 0x7B : 0x5B;

        /* Add formatting */
        if (format && (*(rtn++)=0xA)) for (i=0;i<src->index_nested+1;i++) *(rtn++)=0x9;
        else *(rtn++) = 0x20;
    }

    /* Encode key */
    if (src->key && src->key != (char*) 1) {
        c = src->key;
        *(rtn++) = 0x22;
            while (*c) {
                *(rtn++) = *(c++);
                /* Extend return string */
                if ((rtn-rtn0)+(sizeof(char)*3) >= rtn_max) {
                    rtn_pos = rtn - rtn0;
                    rtn0 = (char*) realloc(rtn0, sizeof(char) * (unsigned int) (rtn_max+=0xFFF));
                    rtn = rtn0 + rtn_pos;
                }
            }
        *(rtn++) = 0x22;
        *(rtn++) = 0x3A;
        *(rtn++) = 0x20;
    }

    /* Encode child */
    if (src->child) {
        src = src->child;
        goto _RTS;
    }

    /* Encode value */
    switch (src->type) {
        case 1: /* Number (integer/floatval) */
            memset(sample, 0, 20);
            if (src->has_decimal_point) sprintf(sample, "%.*f", src->decimal_places, src->floatval);
            else sprintf(sample, "%d", src->integer);
            sprintf(rtn, "%s", sample);
            while (*(rtn+1)) rtn++;
            rtn++;
            break;

        case 2: /* String */
            c = src->string;
            *(rtn++) = 0x22;
            while (*c) {
                *(rtn++) = *(c++);
                /* Extend return string */
                if ((rtn-rtn0)+(sizeof(char)*3) >= rtn_max) {
                    rtn_pos = rtn - rtn0;
                    rtn0 = (char*) realloc(rtn0, sizeof(char) * (unsigned int) (rtn_max+=(0xFFF*mul_string++)));
                    rtn = rtn0 + rtn_pos;
                }
            }
            *(rtn++) = 0x22;
            break;

        case 3: /* Boolean (true/false/null) */
            memset(sample, 0, 20);
            if (src->integer)       sprintf(sample, "%s", "true");
            else if (!src->is_null) sprintf(sample, "%s", "false");
            else                    sprintf(sample, "%s", "null");
            sprintf(rtn, "%s", sample);
            while (*(rtn+1)) rtn++;
            rtn++;
            break;
    }

    /* Add comma */
    if (src->next) {
        *(rtn++) = 0x2C;
        /* Add formatting */
        if (format && (*(rtn++)=0xA)) for (i=0;i<src->index_nested+1;i++) *(rtn++)=0x9;
        else *(rtn++) = 0x20;
    }

    /* Add closing delimiter and return subroutine */
    if (!src->next) {
        /* Add formatting */
        if (format && (*(rtn++)=0xA)) for (i=0;i<src->index_nested-0;i++) *(rtn++)=0x9;
        else *(rtn++) = 0x20;
        *(rtn++) = src->key ? 0x7D : 0x5D;

        /* Recursively return to parents and add closing delimiters until EOF, or proceed with next neighbor */
        while (src->parent) {
            src = src->parent;

            if (src->next) {
                src = src->next;
                *(rtn++) = 0x2C;

                /* Add formatting */
                if (format && (*(rtn++)=0xA)) for (i=0;i<src->index_nested+1;i++) *(rtn++)=0x9;
                goto _RTS;
            }

            /* Add formatting */
            if (format && (*(rtn++)=0xA)) for (i=0;i<src->index_nested-(src->index_nested-1>-1?src->index_nested-1:0);i++) *(rtn++)=0x9;
            else *(rtn++) = 0x20;

            *(rtn++) = src->key ? 0x7D : 0x5D;
        }
        goto _EOF;
    } else if (!src->child) {
        src = src->next;
        goto _RTS;
    }

    /* Resize return string before return */
    _EOF:
    /* Add formatting */
    if (format && (*(rtn++)=0xA)) for (i=0;i<src->index_nested+1;i++) *(rtn++)=0x9;
    else *(rtn++) = 0x20;
    rtn_pos = rtn - rtn0;
    rtn0 = (char*) realloc(rtn0, sizeof(char) * (unsigned int) (rtn-rtn0));
    *(rtn0 + rtn_pos) = 0;
    return rtn0;
}

char *encodeAJSONUnformatted (struct aJSON *srcArg) {
    return encodeAJSON(srcArg, 0);
}

char *encodeAJSONFormatted (struct aJSON *srcArg) {
    return encodeAJSON(srcArg, 1);
}

struct aJSON *eraseAJSON (struct aJSON *srcArg)
{
    struct aJSON *src = srcArg,
                 *first = src;

    /* Traverse to first struct */
    while (first->parent || first->prev)
        first = first->parent ? first->parent : first->prev;

    /* Remove from neighbors */
    if (src->next || src->prev) {
        if (src->next) src->next->prev = src->prev;
        if (src->prev) src->prev->next = src->next;
    }

    /* Remove from parent */
    if (src->parent && src->parent->child == srcArg)
        if (src->parent->child == srcArg)
            src->parent->child = src->prev ? src->prev : src->next;
        else
            src->parent->child = src->parent->child == src && src->next ? src->next : NULL;

    return first == srcArg ? first->next : first;
}

void appendAJSON (struct aJSON *targetArg, struct aJSON *srcArg)
{
    struct aJSON *tmp = targetArg->next ? targetArg->next : NULL;

    /* Link neighbors */
    targetArg->next = srcArg;
    srcArg->prev = targetArg;
    srcArg->next = tmp;

    /* Set other properties */
    srcArg->index_nested = targetArg->index_nested;
    srcArg->index_neighbor = targetArg->index_neighbor+1;

    /* Decrement index_neighbor of all right-hand neighbors */
    if (tmp)
        while (tmp) {
            tmp->index_neighbor--;
            tmp = tmp->next;
        }

    /* Bind heaps */
    while (targetArg->prev) targetArg = targetArg->prev;
    tmp = targetArg - 1;
    while (tmp->next) tmp = tmp->next;
    tmp->next = srcArg - 1;
    (srcArg-1)->prev = tmp;
    (srcArg-1)->index_neighbor = (srcArg-1)->prev->index_neighbor + 1;
}

struct aJSON *accessAJSON (struct aJSON *targetArg, char *path)
{
    struct aJSON *i = targetArg,
                 *rtn;
    const char   *start = path,
                 *end;
    int          len,
                 index;

    while ((start = strchr(start, '[')) != NULL)
    {
        rtn = NULL;
        start++; /* Move past the opening bracket */
        if ((end = strchr(start, ']')) == NULL) break;

        if ((len = end - start) > 0)
        {
            char substring[len + 1];
            strncpy(substring, start, len);
            substring[len] = '\0';

            /* Find member */
            if (substring[0] == '\"' && substring[len - 1] == '\"') {
                memmove(substring, substring + 1, len - 2);
                substring[len - 2] = '\0';

                /* Left-hand search */
                if (i->prev)
                    while (i) {
                        if (!strcmp(substring, i->key)) {
                            rtn = i;
                            break;
                        }
                        i = i->prev;
                    }

                /* Right-hand search */
                if (i->next)
                    while (i) {
                        if (!strcmp(substring, i->key)) {
                            rtn = i;
                            break;
                        }
                        i = i->next;
                    }

                if (i->child) i = i->child;
            }

            /* Find element */
            else {
                index = atoi(substring);

                /* Left-hand search */
                if (i->prev)
                    while (i) {
                        if (index == i->index_neighbor) {
                            rtn = i;
                            break;
                        }
                        i = i->prev;
                    }

                /* Right-hand search */
                if (i->next)
                    while (i) {
                        if (index == i->index_neighbor) {
                            rtn = i;
                            break;
                        }
                        i = i->next;
                    }

                if (i->child) i = i->child;
            }
        }
        start = end+1;
    }

    return rtn ? (rtn->child ? rtn->child : rtn) : NULL;
}

void freeAJSON(struct aJSON *srcArg)
{
    unsigned dealloc_i = 0;
    struct aJSON *heap_root = srcArg - 1,
                 *heap_current = heap_root,
                 *heap_tmp,
                 *element = heap_root + 1;

    while (heap_current)
    {
        // Deallocate values
        while (element)
        {
            if (element->type == 2 && element->string)
                free(element->string);
            if (element->key > (char*) 0x20 && element->key[0] != '\0' && element->key[0] > 0)
                free(element->key);

            if (dealloc_i >= heap_current->index_neighbor)
                break;

            element++;
            dealloc_i++;
        }

        // Jump to next heap
        heap_tmp = heap_current;
        heap_current = heap_current->next;
        element = heap_current + 1;

        if (!heap_current || !heap_current->index_neighbor)
            break;

        dealloc_i = 0;
    }

    // Deallocate heaps
    heap_current = heap_root;
    while (heap_current) {
        heap_tmp = heap_current;
        heap_current = heap_current->next;
        free(heap_tmp);
    }
}

#ifdef __cplusplus
char *aJSON::encode(int format) { return encodeAJSON(this, format); }
struct aJSON *aJSON::access(char *path) { return accessAJSON(this, path); }
struct aJSON *aJSON::erase() { return eraseAJSON(this); }
struct aJSON *aJSON::append(struct aJSON *src) { appendAJSON(this, src); return this; }
struct aJSON *aJSON::append(char *src) { appendAJSON(this, decodeAJSON(src)); return this; }
void aJSON::free() { freeAJSON(this); }
#endif

#endif

#include <immintrin.h>