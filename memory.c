#include <math.h>
#include "memory.h"
#define VERTICAL_DIV 12

typedef struct {
    unsigned long total_memory_kb;    // total memory in kb
    unsigned long free_memory_kb;     // free memory in kb
} OriginMemInfo;

void fetch_memory_info(OriginMemInfo* info){
    ///_|> descry: update the OriginMemInfo struct with total and free memory using sysinfo
    ///_|> info: pointer to OriginMemInfo struct to be filled, type OriginMemInfo*
    ///_|> returning: this function does not return anything
    if (info == NULL){
        perror("fetch_memory_info(): info == NULL");
        return;
    }
    struct sysinfo sys_info;
    if (sysinfo(&sys_info) != 0) {
        perror("sysinfo");
        exit(EXIT_FAILURE);
    }
    // Convert bytes to kb
    info->total_memory_kb = sys_info.totalram/ 1024;
    info->free_memory_kb = sys_info.freeram/ 1024;
}


OriginMemInfo* get_originMem_info(){
    ///_|> descry: allocates and returns a populated OriginMemInfo struct with memory in kb
    ///_|> returning: returns pointer to dynamically allocated OriginMemInfo; exits on failure
    OriginMemInfo* newInfo = (OriginMemInfo*)malloc(sizeof(OriginMemInfo));
    if (!newInfo) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }

    fetch_memory_info(newInfo);
    return newInfo;
}


MemoryInfo* calcu_MemoryInfo_GB(OriginMemInfo* origin){
    ///_|> descry: converts OriginMemInfo values from KB to GB and calculates used memory
    ///_|> origin: pointer to a populated OriginMemInfo struct, type OriginMemInfo*
    ///_|> returning: pointer to dynamically allocated MemoryInfo struct in GB; NULL on error
    if (origin == NULL){
        perror("calcu_MemoryInfo_GB(): origin == NULL");
        return NULL;
    }
    MemoryInfo* MemInfo = (MemoryInfo*)malloc(sizeof(MemoryInfo));
    if (!MemInfo) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
     // Convert total and free memory from KB to GB
    MemInfo->total_memory = origin->total_memory_kb/(1024.0 * 1024.0);
    float free_memory = origin->free_memory_kb/(1024.0 * 1024.0);
    MemInfo->used_memory = MemInfo->total_memory - free_memory; 
    return MemInfo;
}

MemoryInfo* get_MemoryInfo(){
    ///_|> descry: retrieves memory info and returns a MemoryInfo struct with values in GB
    ///_|> returning: pointer to a dynamically allocated MemoryInfo struct
    OriginMemInfo* origin = get_originMem_info();
    MemoryInfo* info = calcu_MemoryInfo_GB(origin);
    free(origin);
    return info;
}

// up \033[nA down\033[nB right\033[nC left\033[nD 
void draw_memory_chart(float *samples, int sample_count, int total, float total_memory) {
    ///_|> descry: draws a graph to visualize memory usage over time
    ///_|> samples: array of memory usage values in GB that already stored, type float*
    ///_|> sample_count: number of memory samples alreday stored, type int
    ///_|> total: total number of samples, type int
    ///_|> total_memory: total system memory in GB for scaling, type float
    ///_|> returning: this function does not return anything

    float current_memory = samples[sample_count-1];

    printf("v Memory %5.2f GB\n", current_memory);
    printf(" %3d GB | \n", (int)round(total_memory)); // width = 6 for " %2d GB"
    // vertical axis
    for (int i = 0; i < VERTICAL_DIV-1; i++) {
        printf("\033[%dC", 8);
        printf("|\n");
    }
    // horizontal axis
    printf(" %3d GB ", 0);
    for (int i = 0; i < total+1; i++) {
        printf("—");
    }

    printf("\033[%dD", total);
    //plot points
    for (int i = 0; i < sample_count; i++) {
        float used_mem = samples[i];
        float scaled_to12 = used_mem / total_memory * VERTICAL_DIV;
        int plot_row = (int)ceil(scaled_to12);

        if (plot_row <= 0){
            printf("\033[C"); 
            continue;
        }
        if (plot_row > VERTICAL_DIV) plot_row = VERTICAL_DIV;

        printf("\033[%dA", plot_row);
        printf("#");
        printf("\033[%dB", plot_row);   
        // printf("\033[C");    
    } 

    printf("\n");
}