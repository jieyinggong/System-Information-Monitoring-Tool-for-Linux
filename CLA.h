#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifndef CLA_HEADER
#define CLA_HEADER

/*
This module provides a utility function for parsing command-line arguments and 
configuring runtime parameters such as graph type, number of samples, and delay between updates.
*/

typedef struct {
    int graph_flag[3];
    int samples;
    int tdelay;
}CLAInfo;
///_|> descry: Stores the runtime configuration parsed from command-line input
///_|> members:
///_|>     - graph_flag: array of 3 integers, each indicating whether a specific graph
///_|>                    is enabled (0: memory, 1: CPU, 2: cores), type int[3]
///_|>     - samples: number of samples to collect and show, type int
///_|>     - tdelay: delay between samples in microseconds, type int

void read_CLA(int argc, char** argv, CLAInfo* info);

#endif