#include <sys/resource.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef MEMORY_HEADER
#define MEMORY_HEADER
/*
This module provides functions for retrieving and visualizing memory usage statistics, 
including total and used memory, and drawing a graph of usage over time.
*/
typedef struct {
    float total_memory;           //  (GB)
    float used_memory;            //  (GB)     
} MemoryInfo;
///_|> descry: Holds converted memory usage values in gigabytes (GB)
///_|> members:
///_|>     - total_memory: total system memory in GB, type float
///_|>     - used_memory: memory currently in use in GB, type float


MemoryInfo* get_MemoryInfo();

void draw_memory_chart(float *samples, int sample_count, int total, float total_memory);

#endif