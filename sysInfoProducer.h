#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>

#include "memory.h"
#include "cpu.h"
#include "core.h"
#include "CLA.h"
#include "pipeTool.h"
#include "signalHandler.h"

#ifndef SYSINFO_PRODUCER_HEADER
#define SYSINFO_PRODUCER_HEADER
/*
This module serves as the producer in this system monitoring tool.
It is responsible for spawning child processes to collect real-time system 
information such as memory usage, CPU utilization, and core statistics.
These values are sampled over time and passed back to the main process 
via pipe-based inter-process communication (IPC). The collected data is 
then consumed by the display .
*/
typedef struct utiization_info{
    float cpu_utiliz;
    MemoryInfo mem_utiliz;
}UtilizInfo;
//_|> descry: Combines both memory and CPU usage information into a single struct
///_|> members:
///_|>     - cpu_utiliz: current CPU usage in percentage, type float
///_|>     - mem_utiliz: MemoryInfo struct containing total and used memory in GB, type MemoryInfo

void fetch_utilization_with_pipe(CLAInfo* cla_info, int write_to_parent_fd);

void fetch_core_info_with_pipe(int write_fd_to_parent);

#endif