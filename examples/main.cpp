#include <iostream>

#include "../ansijson.h"
#include "helpers.h"

#include "../lib/cJSON.c"


#include <time.h>
clock_t start_time, end_time;
double cpu_time_used;


int main(int argc, char *argv[])
{
    char *file = readFile("D:\\Dev\\ANSIJSON\\examples\\json\\DamagedHelmet.gltf");

    start_time = clock();
    cJSON *cjson = cJSON_Parse(file);
    end_time = clock();
    std::cout << "cJSON Completed in " << (((double)(clock() - start_time) * 1000.0) / CLOCKS_PER_SEC) << "ms" << std::endl;

    start_time = clock();
    aJSON *ajson = decodeAJSON(file);
    end_time = clock();
    std::cout << "aJSON Completed in " << (((double)(clock() - start_time) * 1000.0) / CLOCKS_PER_SEC) << "ms" << std::endl;

    return 0;
}