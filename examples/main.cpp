#include <iostream>

#include "../ansijson.h"
#include "helpers.h"

int main(int argc, char *argv[])
{
    // "Hello World!"
    struct aJSON *example1_data = decodeAJSON(readFile("../examples/json/hello_world.json"));
    example1_data->next->append("{\"one_more\": \"!\"}");
    std::cout << "Example 1:\n" << example1_data->encode() << "\n" << std::endl;

    // Print a slightly more complex JSON file containing dimensions, floats, unicodes and more
    std::cout << "Example 2:\n" << decodeAJSON(readFile("../examples/json/complex.json"))->encode(1) << "\n" << std::endl;

    // Print a GLTF file containing embedded geometry for a square
    std::cout << "Example 3:\n" << decodeAJSON(readFile("../examples/json/box.gltf"))->encode(1) << "\n" << std::endl;

    return 0;
}