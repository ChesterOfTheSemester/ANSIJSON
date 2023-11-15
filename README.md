# ANSIJSON 0.3.2 Library Documentation

## Overview

ANSIJSON is a C/C++ (C90/CXX98 and above) library for parsing, encoding and manipulating JSON data. It allows you to work with JSON data structures in your C and C++ programs, providing functions for decoding JSON strings into data structures and encoding data structures into JSON strings.

## Library Features

- **JSON Parsing**: ANSIJSON provides the `decodeAJSON` function to parse JSON strings and create a structured representation of the data.
- **JSON Encoding**: You can use the `encodeAJSON` function to convert ANSIJSON data structures back into JSON strings.
- **Data Structure**: ANSIJSON defines a data structure called `aJSON` for representing JSON data. It includes features like nested objects, arrays, numbers, strings, boolean values, and optional C++ methods.
- **Optional C++ methods**: If compiled with C++, the structure `aJSON` will contain a number of object-oriented methods for working with `aJSON` elements.

## Data Structure

The core data structure used in ANSIJSON is the `aJSON` struct, which represents JSON data. It has the following members:

```c++
/* Decoded values struct */
typedef struct aJSON {
    struct aJSON *next, *prev,      /* Double link listed neighbors */
                 *child,            /* Pointer to the child struct if it's an object or array */
                 *parent;           /* Pointer to the parent struct  */
    union {      int integer;       /* The value represented as an integer */
                 double floatval;   /* The value represented as a floating-point number */
                 char *string; };   /* A pointer to a string (if the value is a string) */
    unsigned int index_neighbor,    /* Index of the struct among its neighboring structs */
                 index_nested,      /* Index of the struct within its parent (child index) */
                 decimal_places;    /* The number of decimal places (if the value is a floating-point number) */
    char         *key,              /* A pointer to a member key string if the struct is a member of an object */
                 type,              /* Type of the value: (0=Container, 1=Number, 2=String, 3=Boolean) */
                 is_null,           /* True if the value is NULL (if the value is a boolean) */
                 is_signed,         /* True if the number value is signed (negative) */
                 has_decimal_point, /* True if the number value has a decimal point */
                 has_key;           /* True if the value structure has a key */

#ifdef __cplusplus /* Optional C++ methods */
    char *encode(int format = 0);
    struct aJSON *access(char *path);
    struct aJSON *erase();
    void append(struct aJSON *src);
    void append(char *src);
#endif
} aJSON;
```

## Functions

### `struct aJSON *decodeAJSON (char *src)`

This function parses a JSON string and returns a pointer to the root of the resulting `aJSON` data structure. It takes a JSON string `src` as input and returns a pointer to the root element. If parsing fails, it returns `NULL`.

Usage Example:
```c
char *json = "{\"name\": \"Chester\", \"age\": 32, \"favorite_numbers\": [17, 42, 51, 32]}";
struct aJSON *root = decodeAJSON(json); 
// `root` now points to the root of the parsed JSON data
```

### `char *encodeAJSON (struct aJSON *src, unsigned int format)`

This function encodes an `aJSON` data structure into a JSON string. It takes an `aJSON` element and an optional `format` parameter to control formatting (0 for unformatted, 1 for formatted). It returns a dynamically allocated string representing the JSON data.

Usage Example:
```c
char *json = encodeAJSON(root, 1);
// `json` now contains a formatted JSON string representing the `root` structure
```

### `void freeAJSON (struct aJSON *srcArg)`

This function will recursively deallocate a `aJSON` data structure. It takes an `aJSON` element.

Usage Example:
```c
freeAJSON(root);
// The heap of structure `root` has now been deallocated.
```

### `struct aJSON *accessAJSON (struct aJSON *target, char *path)`

This function allows you to access a specific `aJSON` element within the data structure by providing a search path `path`. It returns a pointer to the found element. You can use this function to navigate and manipulate the data structure.

Usage Example:
```c
struct aJSON *age = accessAJSON(root, "[\"age\"]");
// `age` now points to the specified element (second element of member-object "favorite_numbers")
struct aJSON *number = accessAJSON(root, "[\"favorite_numbers\"][2]");
// `number` now points to the specified element (the third element of member-object "favorite_numbers")
```

### `struct aJSON *eraseAJSON (struct aJSON *src)`

This function removes an `aJSON` element and its children from the data structure. It takes an `aJSON` element `src` as input and returns a pointer to the first element after the removed one.

Usage Example:
```c
root = eraseAJSON(root, number);
// The expected contents of `root` is now {"name": "Chester", "age": 32, "favorite_numbers": [17, 42, 32]}
root = eraseAJSON(root, age);
// The expected contents of `root` is now {"name": "Chester", "favorite_numbers": [17, 42, 32]}
```

### `void appendAJSON (struct aJSON *target, struct aJSON *src)`

This function appends a new `aJSON` element `src` to the elements after the `target` element. It links the elements, sets their properties, and adjusts index values as necessary.

Usage Example:
```c
appendAJSON(root, decodeAJSON("{\"birthday\": \"Saturday\"}"));
// Member ["birthday"] is appended to the `root` structure`
```


`char *encodeAJSONUnformatted (struct aJSON *src)` This is function is a wrapper for the function `encodeAJSON` with the optional format parameter set to 1.

`char *encodeAJSONFormatted (struct aJSON *src)` This is function is a wrapper for the function `encodeAJSON` with the optional format parameter set to 0.



### Optional C++ Methods

In addition to the C functions, ANSIJSON provides the following C++ methods for working with `aJSON` elements:

- `char *aJSON::encode(int format)`: Encodes the element and its children into a JSON string, allowing you to specify the formatting.
- `struct aJSON *aJSON::access(char *path)`: Allows you to access a specific element within the `aJSON` data structure by providing a search path.
- `struct aJSON *aJSON::erase()`: Removes the element and its children from the data structure.
- `void aJSON::append(struct aJSON *src)`: Appends a new `aJSON` element to the elements after the current element.
- `void aJSON::append(char *src)`: Appends a new `aJSON` element by parsing a JSON string.

These C++ methods provide a convenient and object-oriented way to work with ANSIJSON.

## MIT License

> Copyright (c) 2023 Chester Abrahams
> 
> Permission is hereby granted, free of charge, to any person obtaining a copy
> of this software and associated documentation files (the "Software"), to deal
> in the Software without restriction, including without limitation the rights
> to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
> copies of the Software, and to permit persons to whom the Software is
> furnished to do so, subject to the following conditions:
> 
> The above copyright notice and this permission notice shall be included in all
> copies or substantial portions of the Software.
> 
> THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
> IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
> FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
> AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
> LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
> OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
> SOFTWARE.

> 

## About the Author

- **Author**: Chester Abrahams
- **Portfolio**: [atomiccoder.com](https://atomiccoder.com)
- **LinkedIn**: [linkedin.com/in/atomiccoder](https://www.linkedin.com/in/atomiccoder)

Official [Github](https://ansijson.com).

---

This README provides an overview of the ANSIJSON library and its usage. For more in-depth information and usage examples, refer to the `examples/` directory in the official [Github](https://ansijson.com) page.