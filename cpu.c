#include <sys/resource.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "cpu.h"
#define VERTICAL_DIV 12

// typedef struct {
//     int total_time;
//     int idle_time;
// }cpuInfo;

// typedef struct{
//     cpuInfo* before;
//     cpuInfo* after;
// }cpuDelta;

//cpu  2178222523 28813 15411241 13053426780 996126 0 70929 (0 0 0)
//user + nice + system + idle + iowait + irq + softirq
// idle = idle = 13053426780 

cpuInfo* get_cpuInfo(){
    ///_|> descry: retrieves current total and idle CPU times by parsing /proc/stat
    ///_|> returning: returns a dynamically allocated cpuInfo struct; returns NULL on failure
    cpuInfo* info = (cpuInfo*)malloc(sizeof(cpuInfo));
    if (info == NULL){
        perror("Failed to allocate for cpuInfo");
        return NULL;
    }
    long long total_time = 0;
    long long idle_time = 0;

    FILE* stat = fopen("/proc/stat", "r");
    if ( stat == NULL ){
        perror("Failed to open /proc/stat");
        free(info);
        return NULL;
    }

    long long cpu_data[7];
    char read_line[256];

    if (fgets(read_line, sizeof(read_line), stat)){
         // Parse first line of /proc/stat for CPU values
        int count = sscanf(read_line, "cpu  %lld %lld %lld %lld %lld %lld %lld", &cpu_data[0], &cpu_data[1], \
        &cpu_data[2], &cpu_data[3], &cpu_data[4], &cpu_data[5], &cpu_data[6]);
        if (count < 7) { 
            free(info);
            fclose(stat);
            perror("Failed to parse CPU data");
            return NULL;
        }
    }else{
        free(info);
        fclose(stat);
        perror("Failed to read cpu information from /proc/stat");
        return NULL;
    }
    // Sum all CPU time fields to get total_time
    for (int i = 0; i < 7; i++){
        total_time += cpu_data[i];
    }
    // get idle_time (4th field)
    idle_time = cpu_data[3];
    info->total_time = total_time;
    info->idle_time = idle_time;

    fclose(stat);
    return info;
}

void update_delta(cpuDelta* delta, cpuInfo* new_info){
    ///_|> descry: updates the cpuDelta struct with new CPU usage data
    ///_|> delta: pointer to a cpuDelta struct containing previous and current cpuInfo, type cpuDelta*
    ///_|> new_info: pointer to the latest cpuInfo struct, type cpuInfo*
    ///_|> returning: this function does not return anything
    if (delta == NULL){
        perror("update_delta(): delta == NULL");
        return;
    }
    if (delta->before == NULL){
        perror("update_delta(): delta->before == NULL");
        return;
    }
    if (delta->after == NULL){
        perror("update_delta(): delta->after == NULL");
        return;
    }
    // Free previous 'before' info and update it with old 'after' data
    if (delta->before != NULL) {
        free(delta->before);
    }
    delta->before = (cpuInfo*) malloc(sizeof(cpuInfo));
    if (delta->before == NULL) {
        perror("Memory allocation failed for delta->before");
        return;
    }

    memcpy(delta->before, delta->after, sizeof(cpuInfo));
    memcpy(delta->after, new_info, sizeof(cpuInfo));
}

float calcu_cpu_utiliz(cpuDelta* delta){
    ///_|> descry: calculates CPU utilization percentage based on the delta between two cpuInfo states
    ///_|> delta: pointer to a cpuDelta struct holding before and after cpuInfo, type cpuDelta*
    ///_|> returning: returns CPU utilization as a percentage in float
    if (delta == NULL){
        perror("update_delta(): delta == NULL");
        return 0.0;
    }
    // Compute cpu utiliz before and after
    long long utiliz_before = delta->before->total_time - delta->before->idle_time;
    long long utiliz_after = delta->after->total_time - delta->after->idle_time;
    long long utiliz_delta = utiliz_after - utiliz_before;
    long long total_delta = delta->after->total_time - delta->before->total_time;

    // Avoiding divided by zero
    if (total_delta <= 0) return 0.0;
    // U = T - I
    // cpu utilization = U2 - U1 / T2 - T1 
    return ((float)(utiliz_delta))/((float)total_delta) * 100.0;
}

void draw_cpu_chart(float *samples, int sample_count, int total) {
    ///_|> descry: renders a graph of in CPU utilization samples
    ///_|> samples: pointer to an array of float CPU usage samples, type float*
    ///_|> sample_count: number of samples currently stored, type int
    ///_|> total: total number of columns (width of chart), type int
    ///_|> returning: this function does not return anything
    printf("v CPU %5.2f %% \n", samples[sample_count-1]);
    printf("  %3d %% | \n", 100); 
    // vertical axis
    for (int i = 0; i < VERTICAL_DIV-1; i++) {
        printf("\033[%dC", 8);
        printf("|\n");
    }
    // horizontal axis
    printf("  %3d %% ", 0);
    for (int i = 0; i < total+1; i++) {
        printf("—");
    }

    printf("\033[%dD", total);
    //plot points
    for (int i = 0; i < sample_count; i++) {
        float cpu_utiliz = samples[i];
        float scaled_to12 = cpu_utiliz / 100.0 * VERTICAL_DIV;
        int plot_row = (int)ceil(scaled_to12);

        if (plot_row <= 0){
            printf("\033[C"); 
            continue;
        }
        if (plot_row > VERTICAL_DIV) plot_row = VERTICAL_DIV;

        printf("\033[%dA", plot_row);
        printf(":");
        printf("\033[%dB", plot_row);   
        // printf("\033[C");    
    } 
    printf("\n");
}

void free_cpu_delta(cpuDelta* delta){
    ///_|> descry: frees memory associated with a cpuDelta struct
    ///_|> delta: pointer to a cpuDelta struct to be deallocated, type cpuDelta*
    ///_|> returning: this function does not return anything
    free(delta->after);
    free(delta->before);
    free(delta);
}

