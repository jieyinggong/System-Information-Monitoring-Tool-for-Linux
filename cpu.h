#include <sys/resource.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  
#include <math.h> 

#ifndef CPU_HEADER
#define CPU_HEADER
/*
This module provides utilities for tracking and visualizing CPU usage over time.
It supports reading system CPU time statistics, calculating CPU usage deltas, 
and rendering a real-time cpu utilization graph.
*/

typedef struct {
    int total_time;
    int idle_time;
}cpuInfo;
///_|> descry: Represents snapshot data of total and idle CPU times
///_|> members:
///_|>     - total_time: accumulated CPU time including all states (type int)
///_|>     - idle_time: portion of total_time when CPU was idle (type int)

typedef struct{
    cpuInfo* before;
    cpuInfo* after;
}cpuDelta;
///_|> descry: Holds two snapshots (before and after) of CPU time data to
///_|>         calculate usage deltas between sampling intervals
///_|> members:
///_|>     - before: pointer to previous cpuInfo state (type cpuInfo*)
///_|>     - after: pointer to current cpuInfo state (type cpuInfo*)

cpuInfo* get_cpuInfo();

void update_delta(cpuDelta* delta, cpuInfo* new_info);

float calcu_cpu_utiliz(cpuDelta* delta);

void draw_cpu_chart(float *samples, int sample_count, int total);

void free_cpu_delta(cpuDelta* delta);


#endif