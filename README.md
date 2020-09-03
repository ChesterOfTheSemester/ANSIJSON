# ANSI JSON  0.2 (Work In Progress)
[*ANSI JSON*](https://ansijson.com) is a single-file, portable & lightweight [JSON](https://json.org) parser/encoder/decoder written in ANSI C. It is compatible with ANSI C/C++ compilers and above.

I wrote this library for a few personal upcoming projects. Feel free to use this in your own project.

## Usage
There are **two** primary definitions involved in *ANSIJSON*, the struct and main function:

```c
  /* Data structure of a value, generated by *ansijson */
  struct aJSON {
    struct aJSON  *next, *prev,               /* Doubly-linked list */
                  **list,                     /* Array of pointers to neighboring structs */
                  *child,                     /* Pointer to child-nested struct if the value is of type object or element */
                  *parent;                    /* Pointer to parent-nested struct  */
    char          *key,                       /* String if struct is of type object-member */
                  type;                       /* Type of value: 0=Container, 1=Number, 2=String, 3=Bool */
    unsigned int  index;                      /* All structs are indexed by nesting level */
    union { double *number; char *string; };  /* The value, union */
  }
```

```c
  /* Main function used for decoding & encoding
  *  The value of action determines the function's behavior and return value
  *    0) Decode: decodes (char*) to (struct aJSON*) 
  *    1) Encode formatted: encodes (struct aJSON*) to (char*)
  *    2) Encode minified: encodes (struct aJSON*) to (char*) */
    
  long *ansijson (char action, long *data);
```

### How to incorporate ANSI JSON in your project
* Simply include the code from *ansijson.h*
```c
#include "ansijson.h"
```
* Or copy the code in between the `#ifndef` directives in *ansijson.h*

### Decode (char*) to (struct aJSON*) 
Make use of casting in order to avoid compiler specific warnings.
```c
struct aJSON *_data = (struct aJSON*) ansijson(0, (long*) "{ \"test\": [ 1, 2, 3 ] }");
```

### Encode (struct aJSON*) to a formatted (char*)
```c
char *_string = (char*) ansijson(1, (long*) _data);
```

### Encode (struct aJSON*) to a minified (char*)
```c
char *_string = (char*) ansijson(2, (long*) _data);
```
## Utility
Here are some helpful functions that go along with aJSON:
### Lookup
```c
/* Finding an aJSON struct from a lookup pattern */
struct aJSON *ajsonLookup(struct aJSON *json, char *query[], unsigned int len)
{ /* Example query: {"index/key"} */
  int i=0; ajson_find:
  if ( (json->key && query[i]>0xFFFFFF && strcmp(json->key,query[i])==0)
      || (json->index==(int)query[i]) )
    if (i+1>len-1) return json;
    else { i++; json=json->child; goto ajson_find; }
  else if (json->next) { json=json->next; goto ajson_find; }
  else return (struct aJSON*)0;
}
```
```c
/* Example: {"node1":{"node2":{"node3":[11,22,33]}}} */
struct aJSON *_data = (struct aJSON*) ansijson(0, (long*) "{ \"node1\": { \"node2\": { \"node3\": [ 11, 22, 33 ] } } }");

/* This lookup is equivalent to a JavaScript lookup: _data["node1"]["node2"]["node3"][1] */
struct aJSON *_result = (struct aJSON*) ajsonLookup(_data, (char*[]){"node1","node2","node3",1}, 4);
printf("%d", result->number); /* The expected output is 22 */

/* Tip: You can use mixed keys & indexes */
struct aJSON *_data = (struct aJSON*) ansijson(0, (long*) "{ \"node0\": { \"node1\": 1, \"node2\": { \"node3\": [ 11, 22, 33 ] } } }");
struct aJSON *_result = (struct aJSON*) ajsonLookup(_data, (char*[]){"node0",1,"node3",1}, 4);
```
