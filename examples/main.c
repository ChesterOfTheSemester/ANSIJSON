#include <stdio.h>

#include "../ansijson.h"
#include "helpers.h"

int main(int argc, char *argv[])
{
    // "Hello World!"
    struct aJSON *example1_data = decodeAJSON(readFile("../examples/json/hello_world.json"));
    appendAJSON(example1_data->next, decodeAJSON("{\"one_more\": \"!\"}"));
    printf("Example 1:\n%s\n", encodeAJSONUnformatted(example1_data));
    freeAJSON(example1_data); // Deallocate

    // Print a slightly more complex JSON file containing dimensions, floats, unicodes and more
    struct aJSON *example2_data = decodeAJSON(readFile("../examples/json/complex.json"));
    printf("Example 2:\n%s\n", encodeAJSONFormatted(example2_data));

    // Print a GLTF file containing embedded geometry for a square
    struct aJSON *example3_data = decodeAJSON(readFile("../examples/json/box.gltf"));
    printf("Example 3:\n%s\n", encodeAJSONFormatted(example3_data));

    return 0;
}