/* ANSI JSON 0.2
 * https://ansijson.com
 *
 * Written by : Chester Abrahams
 * Portfolio  : https://atomiccoder.com
 * LinkedIn   : https://www.linkedin.com/in/atomiccoder/ */

#ifndef ANSIJSON
#define ANSIJSON

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ANSIJSON_DECODE 0
#define ANSIJSON_ENCODE 1
#define ANSIJSON_MINIFY 2

struct aJSON {
  struct aJSON  *next, *prev, **list, *child, *parent;
  union { double *number; char *string; };
  char *key, type; /* Types: ((>5)=*Container, 0=Array, 1=Object, -2=Member, 2=Number, 3=String, 4=Bool */
  unsigned int  index;
};

long *ansijson (char action, long *data)
{
  long   i_SP,*_SP =     (long*) calloc(2,(i_SP=0xF)*sizeof(long)), *SP=_SP;
  struct aJSON *parse =  (struct aJSON*) calloc(2, sizeof(struct aJSON)), /* Parse buffer */ *_parse=parse;
  char   *src=(char*)data,*RTN, /* Encoding result */ *_src=src, *__src=src;
  double A,X,Y;

  switch (action) {
    case 0: goto _DECODE;           /* Decode */
    case 1: action=1; goto _ENCODE; /* Encode formatted */
    case 2: action=0; goto _ENCODE; /* Encode minified */
  }

 _DECODE:
  {
    *SP=-1; /* Define stack bottom as -1 */
    while (*src<0x21) src++; /* Find first data structure */
    if (*src!=0x5B && *src!=0x7B) goto _ERROR;
    (parse->parent=(struct aJSON*)calloc(sizeof(struct aJSON),1))->type=*src==0x7B; /* Allocate head container */
    parse->parent->child=parse;
    parse->type=*src==0x7B;
    src++; goto _LEX_CONTAINER;

   _RTS:
    switch (*(--SP)) {
      case 0: case 1:
      _LEX_ARRAY: _LEX_OBJECT: _LEX_CONTAINER:
        while (*src&&*src<0x21) src++;
        if (!*src) goto _EOF;
        if (parse->parent && !parse->parent->type)
          goto _LEX_ELEMENT; else { Y=0; goto _LEX_MEMBER; } /* Y=Bool: Member-key accepted */

      case -1: default:
        while (*src&&*src<0x21) src++;
        switch (*src) {
          case 0x2C: src++; goto _LEX_CONTAINER;
          case 0x5D: case 0x7D: src++; parse=parse->parent; while(*SP!=-1&&*SP!=(long)parse) SP--; goto _RTS;
          case 0x00: goto _EOF;
          default: goto _ERROR;
        }

      _EOF: return (long*) _parse;
    }

   _LEX_ELEMENT:
    while (*src<0x21 || *src==0x2C) src++;
    goto _LEX_VALUE;

   _LEX_MEMBER:
    while (*src<0x21 || *src==0x3A || *src==0x2C || *src==0x5C) src++;
    if (parse->key && (parse->number||parse->string||parse->child)) { Y=0; /* Allocate neighbor if member key string originates from a subroutine  */
      parse->next = (struct aJSON*) calloc(2, sizeof(struct aJSON));
      parse->next->prev = parse; parse = parse->next;
      parse->parent = parse->prev->parent;
      (parse->list=parse->prev->list)[parse->index=parse->prev->index+1] = parse;
    }
    if (!Y && *src==0x22 && (*(++SP)=-2) && ++Y) goto _LEX_STRING; /* Lex key:value */
    goto _LEX_VALUE;

   _LEX_VALUE:
    {
      /* Expand stack */
      if (SP-_SP>=i_SP) _SP=(long*)realloc(_SP,(i_SP+=0xF)*sizeof(long));

      /* Allocate a new list */
      if (!parse->list) (parse->list=(struct aJSON**)calloc(2, 0x7F*sizeof(struct aJSON)))[0] = parse;

      /* Next member/element */
      if (parse->number || parse->string || parse->child) {
        (parse->next=(struct aJSON*) calloc(2, sizeof(struct aJSON)))->prev = parse;
        parse=parse->next;
        parse->parent=parse->prev->parent;
        (parse->list=parse->prev->list)[parse->index=parse->prev->index+1] = parse;
        *SP=(long)parse;
      }

      /* Allocate new child struct */
      if ((*src==0x5B || *src==0x7B) && !parse->child) {
        *(++SP)=(long)parse;
        (parse->child=(struct aJSON*)calloc(2, sizeof(struct aJSON)))->parent=parse;
        parse=parse->child;
      }

      switch (*src) {
        case 0x5B: case 0x7B: *(++SP)=(long)parse; (Y?parse->parent:parse)->type=*src==0x5B?0:1; src++; goto _LEX_CONTAINER;
        case 0x22: *(++SP)=3; parse->type=3; src++; goto _LEX_STRING;
        case 0x2D: case 0x30 ... 0x39: parse->type=2; *(++SP)=2; goto _LEX_NUMBER;
        case 0x61 ... 0x7A: parse->type=4; *(++SP)=4; goto _LEX_BOOL;
        case 0x00: goto _EOF;
        default: _ERROR: X=Y=1; /* X=Line, Y=Column */
          while (_src<src) if (*(_src++)!=0x0A) Y++; else { X++; Y=1; } /* Find line/column */
          snprintf(RTN=(char*) calloc(2, 0x7F), 0x7F, "Fatal error: unexpected '%c' at %d:%d\n", *src, (int)X, (int)Y);
          perror(RTN); free(RTN); return (long*) 1;
      }
    }

   _LEX_NUMBER: /* Y=Point, X=Sign */
    {
      parse->number = (double*)calloc(2, sizeof(double));
      Y=1; X=*src==0x2D&&src++?-1:1;

     _PARSE_DIGIT:
      while(*src==0x2E||*src==0x45||*src==0x65||*src==0x2B||*src==0x2D) if(*(src++)==0x2E && Y==1) Y=-1;
      if (Y<1) Y/=10;
      *parse->number = *parse->number*10+*(src++)-0x30;
      if (*src>0x20 && *src!=0x2C && *src!=0x5D && *src!=0x7D) goto _PARSE_DIGIT;

      *parse->number *= (double)Y*(Y<1?-X:X);
      goto _RTS;
    }

   _LEX_STRING: /* A=Allocation Size, X=Char Counter */
    {
      if (*SP==-2 && (parse->string || parse->number)) { /* Allocate if member key string is next member of object */
        parse->next = (struct aJSON*) calloc(2, sizeof(struct aJSON));
        parse->next->prev=parse; parse=parse->next;
        (parse->list=parse->prev->list)[parse->index=parse->prev->index+1] = parse;
      }

      if (*src==0x22) src++;
      parse->string = (char*)calloc((size_t) (A=0x7F), 1); X=-1;

     _PARSE_CHAR:
      if (++X>=A) parse->string = (char*) realloc(parse->number, (size_t) (A+=0x7F));
      if (*src!=0x22) parse->string[(int)X] = *(src++);
      if (*src!=0x22) goto _PARSE_CHAR; src++;

      if (*SP==-2) { SP--; /* Return member key string */
        parse->key=parse->string;
        parse->number=0; parse->string=0;
        goto _LEX_MEMBER;
      } else goto _RTS;
    }

   _LEX_BOOL: parse->number=(double*)calloc(2, 1);
    if (!strncmp((char*)src, "null", (size_t) 4) && (src+=4)) *parse->number=0; else
    if (!strncmp((char*)src, "true", (size_t) 4) && (src+=4)) *parse->number=1; else
    if (!strncmp((char*)src, "false", (size_t) 5) && (src+=5)) *parse->number=0; else
      goto _ERROR; goto _RTS;
  }

 _ENCODE:
  {
    A=X=1; parse=_parse= (struct aJSON*) data;
    RTN = src = (char*) calloc(2, (size_t)(Y=0xFFFF));
    _src=__src = (char*) calloc(2, 0xFF);

   _ERTS:
    /* Expand buffer */
    if (src-RTN>Y) RTN=(char*) realloc(RTN,(size_t) (Y+=0xFF));
    if (!parse->parent) goto _ERTN;

    /* Left */
    if (!parse->prev) {
      *(src++) = !parse->parent->type?0x5B:0x7B;
      if (action) { *(src++)=0xA; for (X=0;X<A;X++) sprintf((src+=2)-2,"  "); }
    }

    /* Key */
    if (parse->key) {
      sprintf(_src,"\"%s\": ",parse->key);
      while(*_src) *(src++) = *(_src++);
      _src = __src;
    }

    /* Value */
    if (parse->child) { A++; parse=parse->child; goto _ERTS; }
    else {
      switch (parse->type) {
        case 2: if (*parse->number==(int)*parse->number)
            sprintf(_src,"%d",(int)*parse->number); else
            sprintf(_src,"%f",*parse->number); break;
        case 3: sprintf(_src,"\"%s\"",parse->string); break;
        case 4: _src = (char*)(*parse->number?"true":"false"); break;
      }

      sprintf(src,"%s",_src);
      while(*_src) *(src++) = *(_src++);
      _src = __src;
    }

   _ERIGHT: /* Next / Right / ERTS / Return */
    if (parse->next) {
      parse=parse->next; sprintf((src+=2)-2,", ");
      if (action) { *(src++)=0xA; for (X=0;X<A;X++) sprintf((src+=2)-2,"  "); }
      goto _ERTS;
    }

    if (!parse->parent || _parse==parse) goto _ERTN;
    if (action) { *(src++)=0xA; for (X=0;X<A-1;X++) sprintf((src+=2)-2,"  "); }
    *(src++) = !parse->parent->type?0x5D:0x7D;
    if (parse->parent) { A--; parse=parse->parent; goto _ERIGHT; }

   _ERTN: return (long*) RTN;
  }
}

#endif
