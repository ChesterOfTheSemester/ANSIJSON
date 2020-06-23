# ANSI JSON  0.1 (Work In Progress)
[*ANSI JSON*](https://ansijson.com) is a single-file, portable & lightweight [JSON](https://json.org) parser/encoder/decoder written in ANSI C. It is compatible with ANSI C/C++ compilers and above.

I wrote this library for a few upcoming projects. Feel free to use this in your own project.
If you give credit to using ANSI JSON, let me know so I can add your project (proj name & url) to the list of references.

## Usage
There are **two** definitions involved in *ANSI JSON*:


```c
  /* Data structure of a value, generated by *ansijson */
  struct aJSON {
    struct aJSON  *next, *prev,               /* Doubly-linked list */
                  **list,                     /* Array of pointers to neighboring structs */
                  *child,                     /* Pointer to child-nested struct if the value is of type object or element */
                  *parent;                    /* Pointer to parent-nested struct  */
    unsigned char *key,                       /* String if struct is of type object-member */
                  type;                       /* Type of value: 0=Container, 1=Number, 2=String, 3=Bool */
    unsigned int  index;                      /* All structs are indexed by nesting level */
    union { double *number; char *string; };  /* The value, union */
  }
```

```c
  /* Main function used for decoding & encoding
  *  The value of action determines the function's behavior and return value
  *    0) Decode: decodes JSON *String* (char*) to *Struct* (struct aJSON*) 
  *    1) Encode formatted: encodes *Struct* (struct aJSON*) to JSON *String* (char*)
  *    2) Encode minified: encodes *Struct* (struct aJSON*) to JSON *String* (char*)
    
  long *ansijson (unsigned char action, long *data)
```

### How to incorporate ANSI JSON in your project
* Simply include the code from *ansijson.h*
```c
#include "ansijson.h"
```
* Or copy the code in between the `#ifndef` directives in *ansijson.h*

### Parsing a *string* to *structures*
```c
struct aJSON *_data = ansijson(0, "{ \"test\": [ 1, 2, 3 ] }");
```

### Parsing *structures* to a formatted *string*
```c
char *_string = ansijson(1, _data);
```

### Parsing *structures* to a minified *string*
```c
char *_string = ansijson(2, _data);
```
## Utility
Here are some helpful functions that go along with aJSON:
### Lookup
```c
/* Finding an aJSON struct from a lookup pattern */
struct aJSON *ajsonLookup(struct aJSON *json, char *query[], unsigned int len)
{ /* Example query: {"index/key"} */
  int i=0; ajson_find:
  if ( (json->key && strcmp(json->key,query[i])==0)
      || (json->index==(int)query[i]) )
    if (i+1>len-1) return json;
    else { i++; json=json->child; goto ajson_find; }
  else if (json->next) { json=json->next; goto ajson_find; }
  else return (struct aJSON*)0;
}
```
```c
struct aJSON *_data = ansijson(0, "{ \"node1\": { \"node2\": { \"node3\": [ 11, 22, 33 ] } } }");
struct aJSON *_result = ajsonLookup(_data, (char*[]){"node1","node2","node3",0}, 4);
```
