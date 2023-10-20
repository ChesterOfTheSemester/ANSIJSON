/* ANSI JSON 0.3
 * https://ansijson.com
 * Written by : Chester Abrahams
 * Portfolio  : https://atomiccoder.com
 * LinkedIn   : https://www.linkedin.com/in/atomiccoder/ */

#ifndef ANSIJSON_H
#define ANSIJSON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Decoded values struct */
typedef struct aJSON {
    struct aJSON    *next, *prev,           /* Doubly-link listed neighbors */
                    *child,                 /* Pointer to child-nested struct */
                    *parent;                /* Pointer to parent struct  */
    char            *key,                   /* Member-key string if struct is type Container */
                    type;                   /* Type of value: 0=Container, 1=Number, 2=String, 3=Bool */
    unsigned int    index_neighbor,         /* Indexed by neighbor */
                    index_nested,           /* Indexed by child-nesting (child-index) */
                    is_null,                /* True if the value is NULL (in case type is Bool) */
                    is_signed,              /* True if the number value is signed (negative) */
                    has_decimal_point,      /* True if the number value has a decimal point */
                    decimal_places;         /* The number of decimal places if the value is a float */
    union {         int integer;            /* The value */
                    double floatval;
                    char *string; };
} aJSON;

struct aJSON *decodeAJSON(char *src)
{
    struct aJSON    *pool_struct = (struct aJSON*) calloc(0x40, sizeof(struct aJSON)),
                    *parse = &pool_struct[0],
                    *parse0 = parse;
    unsigned        pool_struct_c = 1, pool_struct_mul = 1;
    char            *src0 = src, c;
    double          A=0,X=0,Y=0;
    int             Z=0,W=0,K=0;

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
    if (Y) goto _LEX_STRING;
    else   goto _LEX_VALUE;

    _LEX_VALUE:
    while (*src && *src<0x21) src++;
    if (*src==0x2C) src++;
    while (*src && *src<0x21) src++;
    if (*src == 0x5D || *src == 0x7D) goto _RTS; /* Return subroutine if closing delimiter is encountered */

    /* Reset structure pool when it's exceeded  */
    if (pool_struct_c+2 >= 0x40) {
        pool_struct = (struct aJSON*) calloc(0x40, sizeof(struct aJSON));
        pool_struct_c = 0;
    }

    /* Next element/member */
    if (parse->type || parse->floatval || parse->string || parse->child) {
        parse->next = &pool_struct[pool_struct_c++];
        parse->next->prev = parse;
        parse = parse->next;
        parse->index_neighbor = parse->prev->index_neighbor+1;
        parse->index_nested = parse->prev->index_nested;
        parse->parent = parse->prev->parent;
        if (parse->prev && parse->prev->key) { Y=1; goto _LEX_MEMBER; } /* Lex member (object) if previous struct has a key */
    }

    /* Child nesting */
    if (!parse->child && (*src==0x5B || *src==0x7B)) {
        parse->child = &pool_struct[pool_struct_c++];
        parse->child->parent = parse;
        parse = parse->child;
        parse->index_nested = parse->parent->index_nested+1;
        if (*src==0x7B) goto _LEX_CONTAINER; else { src++; goto _LEX_VALUE; } /* Switch between lexing member (object) or element (array) */
    }

    /* Switch to specific value and jump to value lexer */
    switch (*src) {
        /* Container */         case 0x5B: case 0x7B:           goto _LEX_CONTAINER;
            /* Number */        case 0x2D: case 0x30 ... 0x39:  parse->type=1; goto _LEX_NUMBER;
            /* String */        case 0x22:                      parse->type=2; goto _LEX_STRING;
            /* Boolean */       case 0x61 ... 0x7A:             parse->type=3; goto _LEX_BOOLEAN;
            /* End of file */   case 0x00:                      goto _EOF;
            /* Error */         default:                        goto _ERROR;
    }

    _LEX_NUMBER: /* Y=Decimal point location, X=Sign flag */
    X = *src==0x2D && src++?-1:1; Y = 1;
    if (*src<0x30 || *src>0x39) goto _ERROR;
    if (X<0) parse->is_signed=1;

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
            case '\\': case '\/': case 'b':
            case 'f':  case 'n':  case 'r' :
            case 't' : case '\"':
                parse->string[(int)X++] = '\\';
                parse->string[(int)X] = *src++; break;
            case 'u': Z = *(++src); W = 0; W = 0;
                for (int i = 0; i < 4; i++, Z = *(++src)) /* Convert hex characters to int */
                    W = W * 16 + ((*src >= 'A' && *src <= 'F') ? *src - 'A' + 10 :
                                  (*src >= 'a' && *src <= 'f') ? *src - 'a' + 10 :
                                  (*src >= '0' && *src <= '9') ? *src - '0' : -1);
                Z = (W <= 0x7F) ? 1 : (W <= 0x7FF) ? 2 : (W <= 0xFFFF) ? 3 : 4; /* Count number of bytes used */
                K = (Z == 1) ? 0x00 : (Z == 2) ? 0xC0 : (Z == 3) ? 0xE0 : 0xF0; /* Find leading byte */
                for (int i = Z - 1; i > 0; i--) parse->string[(int)X + i] = (char)((W & 0x3F) | 0x80), W >>= 6; /* Encode bytes individually */
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

char *encodeAJSON(struct aJSON *srcArg, unsigned int format)
{
    struct aJSON    *src = srcArg;
    long            rtn_max = 0xFFF,
                    rtn_pos,
                    mul_string = 1,
                    mul_rts = 1,
                    i;
    char            *rtn = (char*) calloc(rtn_max, sizeof(char)),
                    *rtn0 = rtn,
                    sample[0x20],
                    *c;

    _RTS:
    /* Extend return string */
    if ((rtn-rtn0)+(sizeof(char)*3) >= rtn_max) {
        rtn_pos = rtn - rtn0;
        c = calloc(rtn_pos + (rtn_max+=(0xFFFF * mul_rts++)), sizeof(char));
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
    if (src->key) {
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
    {
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
    }

    /* Add comma */
    if (src->next) {
        *(rtn++) = 0x2C;
        /* Add formatting */
        if (format && (*(rtn++)=0xA)) for (i=0;i<src->index_nested+1;i++) *(rtn++)=0x9;
        else *(rtn++) = 0x20;
    }

    /* Add closing delimiter and return routine */
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

char *encodeAJSONUnformatted(struct aJSON *srcArg) {
    return encodeAJSON(srcArg, 0);
}

char *encodeAJSONFormatted(struct aJSON *srcArg) {
    return encodeAJSON(srcArg, 1);
}

#endif
