#include <sys/resource.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef CORE_HEADER
#define CORE_HEADER

/*
This module provides functions to retrieve and visualize core-related information from the system, 
including the number of physical cores and the maximum frequency of a CPU. 
It also includes function to display core data specific format.
*/

typedef struct {
    int cores_num;
    float max_freq;
}coreInfo;
///_|> Struct: coreInfo
///_|> descry: A struct representing basic core information for a CPU
///_|> members:
///_|>     - cores_num: number of physical cores (int)
///_|>     - max_freq: maximum core frequency in GHz (float)

float get_max_freq();

int get_cores_num();

coreInfo* get_coreinfo();

coreInfo* combine_coreinfo(int core_num, float max_freq);

void draw_all_core(coreInfo* info);

#endif