#define _DEFAULT_SOURCE
#include "sysInfoConsumer.h"
#include <unistd.h>

int parent_utilization_reader_printer(int utiliz_read_fd, UtilizInfo* info, CLAInfo* cla){
    ///_|> descry: reads system utilization (CPU & memory) from a pipe and visualizes it using charts
    ///_|> utiliz_read_fd: fd to read UtilizInfo from child process, type int
    ///_|> info: pointer to UtilizInfo struct that receives data from pipe, type UtilizInfo*
    ///_|> cla: pointer to CLAInfo controlling number of samples and which graphs to show, type CLAInfo*
    ///_|> returning: returns 0 on success; 1 on error or signal
    int total_samples = cla->samples;
    float * memory_samples = (float *) malloc(sizeof(float)*total_samples);
    float * cpu_samples = (float *) malloc(sizeof(float)*total_samples);

    if (!memory_samples || !cpu_samples) {
        perror("Memory allocation failed for utilization reader printer");
        free(memory_samples);
        free(cpu_samples);
        return 1;
    }

    int i = 0; 
    int num_of_sigint = 0;
    while (true){
        // printf("loop iteration in utilization reader: %d \n", i);
        // check if SIGINT signal was received, if received, handling it.
        if (prompt_for_int_signal()){
            free(memory_samples);
            free(cpu_samples);
            safe_close(&utiliz_read_fd);
            return 1;
        }
        // read one sample from pipe
        ssize_t read_utiliz = read(utiliz_read_fd, info, sizeof(UtilizInfo));
        if (read_utiliz > 0){
            // check value's validation
            if (info->cpu_utiliz < 0 && (info->mem_utiliz.used_memory < 0 || info->mem_utiliz.total_memory < 0)){
                fprintf(stderr, "Utilization info get from utilization reader is invalid, cpu: %.2f, memory: %.2f, %.2f \n", \
                info->cpu_utiliz, info->mem_utiliz.total_memory, info->mem_utiliz.used_memory);
                safe_close(&utiliz_read_fd);
                free(memory_samples);
                free(cpu_samples);
                return 1;
            }
            // prevent overflow
            if ( i >= total_samples + num_of_sigint){
                fprintf(stderr, "Utilization reader receives information more than total samples times: %d \n", i);
                safe_close(&utiliz_read_fd);
                free(memory_samples);
                free(cpu_samples);
                return 1;
            }

            UtilizInfo current;
            current.cpu_utiliz = info->cpu_utiliz;
            current.mem_utiliz = info->mem_utiliz;

            printf("\033[3;1H");
            // if need memory graph, render memory graph
            if (cla->graph_flag[0]){
                memory_samples[i] = current.mem_utiliz.used_memory;
                draw_memory_chart(memory_samples, i+1, total_samples, current.mem_utiliz.total_memory);
                printf("\n");
            }
            // if need cpu graph, render CPU graph
            if (cla->graph_flag[1]){
                cpu_samples[i] = current.cpu_utiliz;
                draw_cpu_chart(cpu_samples, i+1, total_samples);
                printf("\n");
            }
            i++;
        }
        else if (read_utiliz == 0){
            // if EOF, end the loop
            break;
        }
        else{
            // handle with SIGINT
            if (check_sigint()){
                // handle SIGINT: ignore this failed read and continue
                num_of_sigint ++;
                continue;
             } 
            perror("read utilization information from pipe failed");
            free(memory_samples);
            free(cpu_samples);
            safe_close(&utiliz_read_fd); // close the read end fd after done
            return 1;
        }
    }
    free(memory_samples);
    free(cpu_samples);
    safe_close(&utiliz_read_fd);
    return 0;
}

int parent_core_info_reader_printer(int core_read_fd, coreInfo* info){
    ///_|> descry: reads coreInfo from pipe and draws a visualization of CPU cores
    ///_|> core_read_fd: fd for reading core information from child, type int
    ///_|> info: pointer to coreInfo struct to populate and render, type coreInfo*
    ///_|> returning: returns 0 on success; 1 on error
    ssize_t read_core = read(core_read_fd, info, sizeof(coreInfo));
    if (read_core > 0){
        //check value validation
        if (info->cores_num < 0 || info->max_freq < 0){
            fprintf(stderr, "core information get from reader is invalid: %d, %.2f \n", \
                    info->cores_num, info->max_freq);
            safe_close(&core_read_fd);
            return 1;
        }
        draw_all_core(info);
    }
    else if (read_core == -1){
        perror("read utilization information from pipe failed"); 
        safe_close(&core_read_fd);
        return 1;
    }

    safe_close(&core_read_fd); // close read-end fd after done
    return 0;
}