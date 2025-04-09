
#include "core.h"
#include <string.h>


int get_cores_num(){
    ///_|> descry: this function retrieves the number of physical cores from /proc/cpuinfo
    ///_|> returning: returns the number of physical CPU cores as an integer; returns -1 on failure
    FILE* cpuinfo = fopen("/proc/cpuinfo","r");
    if ( cpuinfo == NULL ){
        perror("Failed to open /proc/cpuinfo");
        return -1;
    }

    char read[100];
    int core_num = -1;
    while(fgets(read, sizeof(read), cpuinfo) != NULL){
        // Search for line containing "cpu cores"
        if (strstr(read, "cpu cores") != NULL){
            // Parse the number of cores from the line containing “cpu cores”
            if( sscanf(read, "cpu cores : %d", &core_num) == 1){
                break;
            }
        }
    }
    fclose(cpuinfo);
    return core_num;
}

float get_max_freq(){
    ///_|> descry: this function reads the maximum frequency of CPU 
    ///_|> returning: returns the max frequency of the CPU in GHz as float; returns -1 on failure
    FILE* cpufreq = fopen("/sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq", "r");
    if ( cpufreq == NULL ){
        perror("Failed to open /sys/devices/system/cpu/cpu0/cpufreq/cpuinfo_max_freq");
        return -1;
    }

    int max_freq_kHz;
    if (fscanf(cpufreq, "%d", &max_freq_kHz) != 1){
        fclose(cpufreq);
        perror("Failed to read cpuinfo_max_freq");
        return -1;
    }

    fclose(cpufreq);
    return ((float)max_freq_kHz)/1000000.0;  
}

coreInfo* get_coreinfo(){
    ///_|> descry: this function allocates and populates a coreInfo struct with core number and max frequency
    ///_|> returning: returns a pointer to a dynamically allocated coreInfo struct containing core info
    coreInfo* info = (coreInfo*)malloc(sizeof(coreInfo));
    info->cores_num = get_cores_num();
    info->max_freq = get_max_freq();
    return info;
}

coreInfo* combine_coreinfo(int core_num, float max_freq){
    ///_|> descry: this function creates a coreInfo struct with user-specified core number and max frequency
    ///_|> core_num: number of cores to store, type integer
    ///_|> max_freq: maximum frequency to store, type float
    ///_|> returning: returns a pointer to a dynamically allocated coreInfo struct
    coreInfo* info = (coreInfo*)malloc(sizeof(coreInfo));
    info->cores_num = core_num;
    info->max_freq = max_freq;
    return info;
}

// // up \033[nA down\033[nB right\033[nC left\033[nD 
// void draw_single_core(int horiz_term){
//     printf("+--+\n");
//     printf("|  |\n");
//     printf("+--+\n");
//     printf("\n");
//     // here we are at the line 5 at the begining
//     printf("\033[%dA", 4);
//     printf("\033[%dC", horiz_term * 6 + 1);
// }

void draw_single_core_line1(int horiz_term){
    ///_|> descry: this function prints the top and bottom line of core boxes for visualization
    ///_|> horiz_term: number of cores to draw horizontally in one line, type integer
    ///_|> returning: this function does not return anything
    for (int i = 0; i<horiz_term; i++){
        printf("+--+  ");
    }
    printf("\n");
}

void draw_single_core_line2(int horiz_term){
    ///_|> descry: this function prints the middle line of core boxes for visualization
    ///_|> horiz_term: number of cores to draw horizontally in one line, type integer
    ///_|> returning: this function does not return anything
    for (int i = 0; i<horiz_term; i++){
        printf("|  |  ");
    }
    printf("\n");
}

void draw_all_core(coreInfo* info){
    ///_|> descry: this function prints a graph of all CPU cores based on coreInfo
    ///_|> info: pointer to a coreInfo struct containing number of cores and max frequency, type coreInfo*
    ///_|> returning: this function does not return anything
    if (info == NULL){
        perror("draw_all_core(): info == NULL");
        return;
    }
    int num = info->cores_num;
    float freq = info->max_freq;
    printf("v Number of Cores: %d @ %.2f GHz\n", num, freq);
    // Draw in rows of 4 cores per line
    while (num > 0){
        int i = (num >= 4) ? 4 : num;
        
        draw_single_core_line1(i); // top border
        draw_single_core_line2(i); // content
        draw_single_core_line1(i); // bottom border
        printf("\n");
        num = num - 4;
    }
}

//quick test
// int main(){
//     coreInfo* info = (coreInfo*)malloc(sizeof(coreInfo));;
//     info->cores_num = 14;
//     info->max_freq = 4.8;

//     draw_all_core(info);
//     free(info);
// }
