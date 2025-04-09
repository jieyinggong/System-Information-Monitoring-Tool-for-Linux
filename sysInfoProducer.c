#define _DEFAULT_SOURCE
#include "sysInfoProducer.h"
#include <unistd.h>

void child_memory_writer(int total_samples, int tdelay, int mem_write_fd){
    ///_|> descry: child process that fetches memory usage samples and writes them to a pipe
    ///_|> total_samples: number of samples to collect, type int
    ///_|> tdelay: delay between each sample in microseconds, type int
    ///_|> mem_write_fd: write-end fd for memory pipe, type int
    ///_|> returning: this function does not return; it exits the process
    for (int i = 0; i < total_samples; i++){
        usleep(tdelay); // wait between samples
        MemoryInfo* curr_memory = get_MemoryInfo();
        if (curr_memory == NULL){
            fprintf(stderr, "Failed to get memory information in child process \n");
            safe_close(&mem_write_fd);
            free(curr_memory);
            exit(EXIT_FAILURE);
        }
        // write memory info to pipe
        if (write(mem_write_fd, curr_memory, sizeof(MemoryInfo)) == -1){
            perror("write memory to pipe failed");
            safe_close(&mem_write_fd);
            free(curr_memory);
            exit(EXIT_FAILURE);
        }
        free(curr_memory);
    }
    safe_close(&mem_write_fd); // close write-end after done
    exit(EXIT_SUCCESS); // exit the process when done
}

void child_cpu_writer(int total_samples, int tdelay, int cpu_write_fd){
    ///_|> descry: child process that computes CPU utilization and writes results to a pipe
    ///_|> total_samples: number of samples to collect, type int
    ///_|> tdelay: delay between samples in microseconds, type int
    ///_|> cpu_write_fd: write-end fd for CPU pipe, type int
    ///_|> returning: this function does not return; it exits the process
    cpuDelta* cpu_delta = (cpuDelta *)malloc(sizeof(cpuDelta));
    cpu_delta->before = (cpuInfo*)malloc(sizeof(cpuInfo));
    cpu_delta->after = (cpuInfo*)malloc(sizeof(cpuInfo));

    cpuInfo* init_cpu = get_cpuInfo(); // initial cpu infomation

    for (int i = 0; i < total_samples; i++){
        usleep(tdelay); // wait between samples
        cpuInfo* curr_cpuInfo = get_cpuInfo();
        if (curr_cpuInfo == NULL){
            fprintf(stderr, "Failed to get CPU information in process \n");
            safe_close(&cpu_write_fd);
            free(init_cpu);
            free(curr_cpuInfo);
            free_cpu_delta(cpu_delta);
            exit(EXIT_FAILURE);
        }
        // for first iteration, use initial state
        if (i == 0){
            memcpy(cpu_delta->before, init_cpu, sizeof(cpuInfo));
            memcpy(cpu_delta->after, curr_cpuInfo, sizeof(cpuInfo));
        }else{
            update_delta(cpu_delta, curr_cpuInfo);
        }

        float curr_cpu_utiliz = calcu_cpu_utiliz(cpu_delta);
        // write utilization value to pipe
        if (write(cpu_write_fd, &curr_cpu_utiliz, sizeof(float)) == -1){
            perror("write cpu_utiliz to pipe failed");
            safe_close(&cpu_write_fd);
            free(init_cpu);
            free(curr_cpuInfo);
            free_cpu_delta(cpu_delta);
            exit(EXIT_FAILURE);
        }
        free(curr_cpuInfo);
    }
    safe_close(&cpu_write_fd); // close write-end after done
    free(init_cpu);
    free_cpu_delta(cpu_delta);
    exit(EXIT_SUCCESS);  // exit the process when done
}

ssize_t parent_memory_reader(int mem_read_fd, MemoryInfo* mem_info){
    ///_|> descry: reads one memory usage sample from the memory pipe into provided struct
    ///_|> mem_read_fd: read-end fd of the memory pipe, type int
    ///_|> mem_info: pointer to a MemoryInfo struct to populate, type MemoryInfo*
    ///_|> returning: number of bytes read, or -1 for signal, or exit when error
    ssize_t read_memory = read(mem_read_fd, mem_info, sizeof(MemoryInfo));
    if (read_memory == -1){
        // check if interrupted by signal (i.e SIGINT), avoid exit
        if (check_sigint()) return -1;
        perror("read memory from pipe failed");
        safe_close(&mem_read_fd);
        exit(EXIT_FAILURE);
    }
    return read_memory;
} 

ssize_t parent_cpu_reader(int cpu_read_fd, float* cpu_utilize){
    ///_|> descry: reads one CPU utilization sample from the CPU pipe into a float
    ///_|> cpu_read_fd: read-end fd of the CPU pipe, type int
    ///_|> cpu_utilize: pointer to a float to store CPU utilization, type float*
    ///_|> returning: number of bytes read, or -1 for signal, or exit when error
    ssize_t read_cpu = read(cpu_read_fd, cpu_utilize, sizeof(float));
    if (read_cpu == -1){
        // check if interrupted by signal (i.e SIGINT), avoid exit
        if (check_sigint()) return -1;
        perror("read cpu from pipe failed");
        safe_close(&cpu_read_fd);
        exit(EXIT_FAILURE);
    } 
    return read_cpu; 
}

void child_max_freq_writer(int max_freq_write_fd){
    ///_|> descry: child process that writes maximum CPU frequency to a pipe
    ///_|> max_freq_write_fd: write-end fd for frequency pipe, type int
    ///_|> returning: this function does not return; it exits the process
    float max_freq = get_max_freq();
    if (max_freq < 0){
        fprintf(stderr, "get max_freq failed in children \n");
        safe_close(&max_freq_write_fd);
        exit(EXIT_FAILURE);
    }

    if (write(max_freq_write_fd, &max_freq, sizeof(float)) == -1){
        perror("write max_freq to pipe failed");
        safe_close(&max_freq_write_fd);
        exit(EXIT_FAILURE);
    }
    safe_close(&max_freq_write_fd); // close write-end after done
    exit(EXIT_SUCCESS);  // exit the process when done
}

void child_core_count_writer(int core_count_write_fd){
    ///_|> descry: child process that writes core count to a pipe
    ///_|> core_count_write_fd: write-end fd for core count pipe, type int
    ///_|> returning: this function does not return; it exits the process
    int core_count = get_cores_num();
    if (core_count < 0){
        fprintf(stderr, "get core_count failed in children process \n");
        safe_close(&core_count_write_fd);
        exit(EXIT_FAILURE);
    }
    if (write(core_count_write_fd, &core_count, sizeof(int)) == -1){
        perror("write core count to pipe failed");
        safe_close(&core_count_write_fd);
        exit(EXIT_FAILURE);
    }
    safe_close(&core_count_write_fd); // close write-end after done
    exit(EXIT_SUCCESS); // exit the process when done
}

void parent_max_freq_reader(int max_freq_read_fd, float* max_freq){
    ///_|> descry: parent process reads the maximum frequency value from pipe
    ///_|> max_freq_read_fd: read-end fd for frequency pipe, type int
    ///_|> max_freq: pointer to float to store the received frequency, type float*
    ///_|> returning: this function does not return anything
    if (read(max_freq_read_fd, max_freq, sizeof(float)) == -1){
        perror("read max_freq from pipe failed");
        safe_close(&max_freq_read_fd);
        exit(EXIT_FAILURE);
    }
    safe_close(&max_freq_read_fd); // close read-end after done
}

void parent_core_count_reader(int core_count_read_fd, int* core_count){
    ///_|> descry: parent process reads the number of CPU cores from pipe
    ///_|> core_count_read_fd: read-end fd for core count pipe, type int
    ///_|> core_count: pointer to int to store received core count, type int*
    ///_|> returning: this function does not return anything
    if (read(core_count_read_fd, core_count, sizeof(int)) == -1){
        perror("read core count from pipe failed");
        safe_close(&core_count_read_fd);
        exit(EXIT_FAILURE);
    }
    safe_close(&core_count_read_fd); // close read-end after done
}

void core_info_writer(int core_info_write_fd, coreInfo* core_info){
    ///_|> descry: writes a coreInfo struct to parent process through a pipe
    ///_|> core_info_write_fd: write-end fd, type int
    ///_|> core_info: pointer to the coreInfo struct to send, type coreInfo*
    ///_|> returning: this function does not return anything
    if (write(core_info_write_fd, core_info, sizeof(coreInfo)) == -1){
        perror("fail to write core infomation to parent main");
        safe_close(&core_info_write_fd);
        exit(EXIT_FAILURE);
    }
    safe_close(&core_info_write_fd); // close write-end after done
}

void fetch_utilization_with_pipe(CLAInfo* cla_info, int write_to_parent_fd){
    ///_|> descry: launches child processes to collect CPU and memory utilization data and send it to parent
    ///_|> cla_info: pointer to parsed command-line arguments controlling sampling, type CLAInfo*
    ///_|> write_to_parent_fd: write-end fd used to send data to parent process, type int
    ///_|> returning: this function does not return; it exits after completion or failure
    int total_samples = cla_info->samples;
    int tdelay = cla_info->tdelay;
    bool if_memory = cla_info->graph_flag[0];
    bool if_cpu = cla_info->graph_flag[1];
    // initialize fd with -1
    int mem_fd[2] = {-1, -1}; // memory pipe: [0] = read-end, [1] = write-end
    int cpu_fd[2] = {-1, -1}; // cpu pipe

    // initialize memory and utlization information struct
    MemoryInfo mem_info;
    mem_info.total_memory = -1;
    mem_info.used_memory = -1;
    UtilizInfo utitiz_info;
    utitiz_info.cpu_utiliz = -1;
    utitiz_info.mem_utiliz = mem_info;
    // create pipes
    if (pipe(mem_fd) == -1){
        perror("pipe for memory created failed");
        exit(EXIT_FAILURE);
    }
    if (pipe(cpu_fd) == -1){
        perror("pipe for cpu created failed");
        exit_failure_with_two_pipe_close(mem_fd, cpu_fd);
    }

    // skip if both CPU and memory are disabled
    if (!if_memory && !if_cpu)return;

    pid_t mem_pid = -1;
    if(if_memory){
        mem_pid = fork();
        if (mem_pid == 0){
            // child process for memory writer
            safe_close(&write_to_parent_fd);
            safe_close(&cpu_fd[0]);
            safe_close(&cpu_fd[1]);
            safe_close(&mem_fd[0]);
            child_memory_writer(total_samples, tdelay, mem_fd[1]);
        }
        else if (mem_pid < 0){
            perror("fork failed for memory");
            exit_failure_with_two_pipe_close(mem_fd, cpu_fd);
        }
    }

    pid_t cpu_pid = -1;
    if (if_cpu){
        cpu_pid = fork();
        if (cpu_pid == 0){
            // child process for CPU writer
            safe_close(&write_to_parent_fd);
            safe_close(&mem_fd[0]);
            safe_close(&mem_fd[1]);
            safe_close(&cpu_fd[0]);
            child_cpu_writer(total_samples, tdelay, cpu_fd[1]);
        }
        else if (cpu_pid < 0){
            perror("fork failed for cpu");
            exit_failure_with_two_pipe_close(mem_fd, cpu_fd);
        }
    }

    // parent process
    ssize_t read_memory = 0;
    ssize_t read_cpu = 0;

    safe_close(&mem_fd[1]);
    safe_close(&cpu_fd[1]);
    while(true){
        // read from memory and cpu pipes
        if (if_memory){
            read_memory = parent_memory_reader(mem_fd[0], &utitiz_info.mem_utiliz);
        }
        if (if_cpu){
            read_cpu = parent_cpu_reader(cpu_fd[0], &utitiz_info.cpu_utiliz);
        }

        // break on EOF
        if (read_memory == 0 && read_cpu == 0) break;
        // // skip this iteration on  SIGINT received
        if (read_memory < 0 || read_cpu < 0)continue;

        // write utilization information to upper parent process main process
        if (write(write_to_parent_fd, &utitiz_info, sizeof(UtilizInfo)) == -1){
            perror("fail to write utilization infomation to parent");
            safe_close(&mem_fd[0]);
            safe_close(&cpu_fd[0]);
            safe_close(&write_to_parent_fd);
            exit(EXIT_FAILURE);
        }
    }
    // cleanup
    safe_close(&write_to_parent_fd);
    safe_close(&mem_fd[0]);
    safe_close(&cpu_fd[0]);
    // wait for children to exit
    if (if_memory && wait_for_children(mem_pid) == -1){
        perror("child process for getting memory utilization exited abnormally.");
        exit(EXIT_FAILURE);
    }
    if (if_cpu && wait_for_children(cpu_pid) == -1){
        perror("child process for getting cpu utilization exited abnormally.");
        exit(EXIT_FAILURE);
    }
    
    exit(EXIT_SUCCESS); // exit the process when done
}

void fetch_core_info_with_pipe(int write_fd_to_parent){
    ///_|> descry: launches two child processes to fetch max frequency and core count concurrently,
    ///_|>         then sends the combined coreInfo struct to the parent process
    ///_|> write_fd_to_parent: write-end fd to send coreInfo to parent, type int
    ///_|> returning: this function does not return; it exits after completion or error
    // initialize fd with -1
    int max_freq_fd[2] = {-1, -1}; // pipe for max frequency
    int core_count_fd[2] = {-1, -1};  // core count
    // create pipes
    if (pipe(max_freq_fd) == -1 || pipe(core_count_fd) == -1){
        perror("pipe for core_info created failed");
        exit(EXIT_FAILURE);
    }

    coreInfo core_info;
    core_info.cores_num = -1;
    core_info.max_freq = -1;

    pid_t max_freq_pid = fork();
    if (max_freq_pid == 0){
        // child process to get max frequency
        safe_close(&write_fd_to_parent);
        safe_close(&core_count_fd[0]);
        safe_close(&core_count_fd[1]);
        safe_close(&max_freq_fd[0]);
        child_max_freq_writer(max_freq_fd[1]);
    }
    else if (max_freq_pid < 0){
        perror("fork failed for max frequency");
        exit_failure_with_two_pipe_close(core_count_fd, max_freq_fd);
    }

    pid_t core_count_pid = fork();
    if (core_count_pid == 0){
        // child process to get core count
        safe_close(&write_fd_to_parent);
        safe_close(&max_freq_fd[0]);
        safe_close(&max_freq_fd[1]);
        safe_close(&core_count_fd[0]);
        child_core_count_writer(core_count_fd[1]);
    }
    else if (core_count_pid < 0){
        perror("fork failed for core count");
        exit_failure_with_two_pipe_close(core_count_fd, max_freq_fd);
    }
    // parent process: read both values from respective pipes
    safe_close(&max_freq_fd[1]);
    parent_max_freq_reader(max_freq_fd[0], &core_info.max_freq);
    safe_close(&core_count_fd[1]);
    parent_core_count_reader(core_count_fd[0], &core_info.cores_num);

    // send core info to main parent process
    core_info_writer(write_fd_to_parent, &core_info);

    // wait for both child processes
    if (wait_for_children(max_freq_pid) == -1){
        perror("child process of get max frequency exited abnormally.");
        exit(EXIT_FAILURE);
    }

    if (wait_for_children(core_count_pid) == -1){
        perror("child process of get core count exited abnormally.");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);  // exit the process when done     
}





