/* ANSI JSON 0.1
 * https://ansijson.com
 * This code is licensed under MIT
 *
 * Written by : Chester Abrahams
 * Portfolio  : https://atomiccoder.com
 * LinkedIn   : https://www.linkedin.com/in/atomiccoder/
 */

#ifndef ANSIJSON
#define ANSIJSON

struct aJSON {
  struct aJSON  *next, *prev, **list,
                *child, *parent;
  union { double *number; char *string; };
  unsigned char *key, type;
  unsigned int  index;
};

long *ansijson (unsigned char action, long *data)
{
  /* Subroutine types:   (<-5)=-*Array, (>5)=*Object, 0=Array, 1=Object, -2=Member, 2=Number, 3=String, 4=Bool */
  long   i_SP,*_SP =     (long*) malloc((i_SP=0xF)*sizeof(long)), *SP=_SP;
  struct aJSON *parse =  (struct aJSON*) calloc(1,sizeof(struct aJSON)), /* Parse buffer */ *_parse=parse;
  char   *src=data,*RTN, /* Encoding result */ *_src=src, *__src=src;
  double A,X,Y;

  switch (action)
  {
    case 0: goto _DECODE;       /* Decode */
    case 1: X=1; goto _ENCODE;  /* Encode formatted */
    case 2: X=0; goto _ENCODE;  /* Encode minified */
    case 3:;                    /* Format */
    case 4:;                    /* Minimize */
  }

 _DECODE:
  {
    *SP=-1; /* Define stack bottom as -1 */
    while (*src<0x21) src++; /* Find first data structure */
    if (*src==0x5B) { *(++SP)=-(long)parse; action=1; goto _LEX_CONTAINER; }
    if (*src==0x7B) { action=2; goto _LEX_CONTAINER; }
      else goto _ERROR;

   _RTS:
    switch (*(--SP))
    {
      case 0: case 1:
      _LEX_ARRAY: _LEX_OBJECT: _LEX_CONTAINER:
        if (!*src) goto _RTS;
        while (*src<0x21) src++;
        if ((*src==0x5D || *src==0x7D) && src++) goto _RTS;
        if (*SP==1 || *SP>5 || *src==0x7B) { Y=0; goto _LEX_MEMBER; } /* Y=Bool: Key accepted */
        else goto _LEX_ELEMENT;

      case -1: default:
        if (((*src!=0x2C && !parse->next)
             || (*src==0x2C && parse->child)
             || !(*src==0x2C && !parse->next && !parse->child)
            ) && (*SP<5||*SP>5))
          parse = *SP<0?-*SP:*SP;

        if (*src==0x7D || *src==0x5D || (*src==0x2C && parse->child && src++))
        {
          if (*src!=0x2C && !*(++src)) goto _EOF;
          while (*src<0x21) src++;
         _PRTS: if (*(SP-1)!=-1 && *(SP)!=(long)parse && -*(SP)!=(long)parse) { SP--; goto _PRTS; }
          goto _RTS;
        }

        if (*SP<-5) {
          if (*src==0x5D) src++;
          if (((struct aJSON*)-*SP)->next && parse!=((struct aJSON*)-*SP)->next) parse=(struct aJSON*)-*SP;
          if (parse->next) goto _RTS; else goto _LEX_ARRAY; } else
        if (*SP>5) {
          if (parse->next && parse!=((struct aJSON*)*SP)->next) parse=(struct aJSON*)*SP;
          if (parse->next) goto _RTS; else
          if (!parse->number&&!parse->string&&!parse->child) goto _LEX_MEMBER;
          else goto _LEX_OBJECT; }

      _EOF: return (long*) (action==1 ? _parse : _parse->child);
    }

   _LEX_ELEMENT:
    if (*src==0x5B) src++;
    while (*src<0x21 || *src==0x2C) src++;
    goto _LEX_VALUE;

   _LEX_MEMBER:
    while (*src<0x21 || *src==0x3A || *src==0x2C || *src==0x5C) src++;
    if (parse->key && (parse->number||parse->string||parse->child)) { Y=0; /* MAlloc if member key string originates from a subroutine  */
      parse->next = (struct aJSON*) malloc(sizeof(struct aJSON));
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

      /* MAlloc a new list */
      if (!parse->list) (parse->list=(struct aJSON**)malloc(0x7F*sizeof(struct aJSON)))[0] = parse;

      /* Next member/element */
      if (parse->number || parse->string || parse->child) {
        (parse->next=(struct aJSON*) malloc(sizeof(struct aJSON)))->prev = parse;
        parse=parse->next;
        parse->parent=parse->prev->parent;
        (parse->list=parse->prev->list)[parse->index=parse->prev->index+1] = parse;
        *SP = *SP<-5?-(long)parse:(long)parse;
      }

      /* Container nesting */
      if ((*src==0x5B || *src==0x7B) && !parse->child) {
        *(++SP)=parse;
        (parse->child=(struct aJSON*)malloc(sizeof(struct aJSON)))->parent=parse;
        parse=parse->child;
      }

      switch (*src)
      {
        case 0x5B: *(++SP)=-(long)parse; src++; goto _LEX_ARRAY;
        case 0x7B: *(++SP)=(long)parse; src++; goto _LEX_OBJECT;
        case 0x22: *(++SP)=3; parse->type=2; src++; goto _LEX_STRING;
        case 0x2D: case 0x30 ... 0x39: parse->type=1; *(++SP)=2; goto _LEX_NUMBER;
        case 0x61 ... 0x7A: parse->type=3; *(++SP)=4; goto _LEX_BOOL;
        case 0x00: goto _EOF;
        default: _ERROR: X=Y=1; /* X=Line, Y=Column */
          while (_src<src) if (*(_src++)!=0x0A) Y++; else { X++; Y=1; } /* Find line/column */
          snprintf(RTN=(char*) malloc(0x7F), 0x7F, "Fatal error: unexpected '%c' at %d:%d\n", *src, (int)X, (int)Y);
          perror(RTN); free(RTN); return (long*) 1;
      }
    }

   _LEX_NUMBER: /* Y=Point, X=Sign */
    {
      parse->number = (double*)malloc(sizeof(double));
      Y=1; X=*src==0x2D&&src++?-1:1;

     _PARSE_DIGIT:
      while(*src==0x2E||*src==0x45||*src==0x65||*src==0x2B||*src==0x2D) if(*(src++)==0x2E && Y==1) Y=-1;
      if (Y<1) Y/=10;
      *parse->number = *parse->number*10+*(src++)-0x30;
      if (*src>0x20 && *src!=0x2C && *src!=0x5D && *src!=0x7D) goto _PARSE_DIGIT;

      *parse->number *= (double)Y*(Y<1?-X:X);
      goto _RTS;
    }

   _LEX_STRING: /* A=MAlloc Size, X=Char Counter */
    {
      /* MAlloc if member key string is next member of object */
      if (*SP==-2 && (parse->string || parse->number)) {
        parse->next = (struct aJSON*) malloc(sizeof(struct aJSON));
        parse->next->prev=parse; parse=parse->next;
        (parse->list=parse->prev->list)[parse->index=parse->prev->index+1] = parse;
      }

      if (*src==0x22) src++;
      parse->string = (char*)malloc(A=0x7F); X=-1;

     _PARSE_CHAR:
      if (++X>=A) parse->string = (char*) realloc(parse->number, A+=0x7F);
      if (*src!=0x22) parse->string[(int)X] = *(src++);
      if (*src!=0x22) goto _PARSE_CHAR; src++;

      if (*SP==-2) { SP--; /* Return member key string */
        parse->key=parse->string;
        parse->number=0; parse->string=0;
        goto _LEX_MEMBER;
      } else goto _RTS;
    }

   _LEX_BOOL: parse->number=(double*)malloc(1);
    if (!strncmp((char*)src, "null", 4) && (src+=4)) *parse->number=0; else
    if (!strncmp((char*)src, "true", 4) && (src+=4)) *parse->number=1; else
    if (!strncmp((char*)src, "false", 5) && (src+=5)) *parse->number=0; else
      goto _ERROR; goto _RTS;
  }

 _ENCODE:
  {
    *SP=-1; /* Define stack bottom as -1 */
    parse = data;
    RTN = src = (char*)malloc(Y=0xFF);
    _src=__src = (char*)malloc(0xFF);

   _ERTS:
    if (src-RTN>Y) RTN=realloc(RTN,Y+=0xFF);
    if (!parse) goto _ERTN;

    /* Left */
    if (!parse->prev) *(src++) = !((struct aJSON*)(*(++SP)=(long)parse))->key?'[':'{';

    /* Key */
    if (parse->key) {
      sprintf(_src,"\"%s\": ",parse->key);
      while(*_src) *(src++) = *(_src++);
      _src = __src;
    }

    /* Value */
    if (parse->child) { parse=parse->child; goto _ERTS; }
    else
    {
      switch (parse->type) {
        case 1: if (*parse->number==(int)*parse->number)
            sprintf(_src,"%d",(int)*parse->number); else
            sprintf(_src,"%f",*parse->number); break;
        case 2: sprintf(_src,"\"%s\"",parse->string); break;
        case 3: _src = *parse->number?"true":"false"; break;
      }

      sprintf(src,"%s",_src);
      while(*_src) *(src++) = *(_src++);
      _src = __src;
    }

    /* Right */
   _ERIGHT:
    if (parse->next) sprintf((src+=2)-2,", ");
    else *(src++)=!((struct aJSON*)*((--SP)+1))->key?']':'}';

    /* ERTS */
    if (*SP!=-1)
      if (!parse->next && ((struct aJSON*)*SP)->next)
        {
          parse=((struct aJSON*)*SP)->next;
          if (*(SP+1)==parse->child) goto _ERIGHT;
          if (*(src-1)==']'||*(src-1)=='}') sprintf((src+=2)-2,", ");
          goto _ERTS;
        }
        else if (!parse->next) goto _ERIGHT; else { parse=parse->next; goto _ERTS; };

   _ERTN: return (long*) RTN;
  }
}

#endif
