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
#include "sysInfoProducer.h"
#include "pipeTool.h"
#include "signalHandler.h"

#ifndef SYSINFO_CONSUMER_HEADER
#define SYSINFO_CONSUMER_HEADER

/*
This module acts as the consumer in the system monitoring pipeline.
It is responsible for receiving system utilization and core information from child processes via pipes,
and rendering visual output using graph drawing functions.
*/

int parent_utilization_reader_printer(int utiliz_read_fd, UtilizInfo* info, CLAInfo* cla);

int parent_core_info_reader_printer(int core_read_fd, coreInfo* info);

#endif




