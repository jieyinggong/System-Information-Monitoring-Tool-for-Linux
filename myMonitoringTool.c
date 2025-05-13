#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "memory.h"
#include "cpu.h"
#include "core.h"
#include "CLA.h"
#include "sysInfoConsumer.h"
#include "sysInfoProducer.h"
#include "pipeTool.h"
#include "signalHandler.h"

// notice: the  tall  of the terminal should be bigger than 33, in the case let it bigger than 40!
int main(int argc,char** argv){
    // Set up custom signal handling for SIGINT (Ctrl+C) and SIGSTP(Ctrl + Z)
    init_sigaction();
    // Allocate memory for CLA information
    CLAInfo* cla = (CLAInfo*)malloc(sizeof(CLAInfo));
    if (!cla){
        perror("Memory allocation for cla failed in main");
        return 1;
    }
    // Parse command-line arguments and populate CLAInfo
    // cla->graph_flag: index 0 = memory, 1 = cpu, 2 = cores
    read_CLA(argc, argv, cla);

    int total_samples = cla->samples;

    // Clear the terminal screen and move cursor to top
    printf("\033[2J");
    printf("\033[1;1H"); 
    printf("Number of samples: %d, --every %d microSecs (%.3f secs)\n\n", total_samples, cla->tdelay, \
    (float)cla->tdelay/1000000.0);
    // initialize fd with -1
    int utiliz_fd[2] = {-1, -1}; // pipe for CPU/memory utilization
    int core_fd[2] = {-1, -1}; // pipe for core info

    // Create pipe for utilization info
    if (pipe(utiliz_fd) == -1){
        perror("pipt created for utilization failed in main");
        free(cla);
        return 1;
    }

    // Create pipe for core info
    if (pipe(core_fd) == -1){
        perror("pipt created for core information failed in main");
        free(cla);
        return 1;
    }

    // initalize the children process pid
    pid_t utiliz_pid = -1;
    pid_t core_pid = -1;

    // if need a memory or cpu graph
    if (cla->graph_flag[0]  || cla->graph_flag[1]){
        utiliz_pid = fork();
        if (utiliz_pid == 0){
            //Child process: set itself be the leader of a process group
            setpgid(0, 0);
            // child process: close unused FDs, start utilization producer
            safe_close(&core_fd[0]);
            safe_close(&core_fd[1]);
            safe_close(&utiliz_fd[0]);
            fetch_utilization_with_pipe(cla, utiliz_fd[1]);
        }
        else if (utiliz_pid < 0){
            perror("fork for utilization in main failed");
            free(cla);
            exit_failure_with_two_pipe_close(utiliz_fd, core_fd);
        }
    }

    if (cla->graph_flag[2]){
        core_pid = fork();
        if (core_pid == 0){
            //Child process: set itself be the leader of a process group
            setpgid(0, 0);
            // child process: close unused FDs, start core info producer
            safe_close(&utiliz_fd[0]);
            safe_close(&utiliz_fd[1]);
            safe_close(&core_fd[0]);
            fetch_core_info_with_pipe(core_fd[1]);
        }
        else if (core_pid < 0){
            perror("fork for core in main failed");
            free(cla);
            exit_failure_with_two_pipe_close(utiliz_fd, core_fd);
        }
    }

    // parent process: close write ends, keep read ends
    safe_close(&utiliz_fd[1]);
    safe_close(&core_fd[1]);
    // read and print memory / cpu graphs if needed
    if (cla->graph_flag[0] || cla->graph_flag[1]){
        UtilizInfo utiliz_info;
        utiliz_info.cpu_utiliz = -1;
        utiliz_info.mem_utiliz.total_memory = -1;
        utiliz_info.mem_utiliz.used_memory = -1;
        if (parent_utilization_reader_printer(utiliz_fd[0], &utiliz_info, cla) == 1 ){
            safe_close(&utiliz_fd[0]);
            safe_close(&core_fd[0]);
            free(cla);
            kill_all_children(utiliz_pid, core_pid); // kill all the children and grandchildren processes
            return 1;
        }; 
    }

    // prompt again if Ctrl+C was triggered before core info
    if (prompt_for_int_signal()){
        safe_close(&utiliz_fd[0]);
        safe_close(&core_fd[0]);
        free(cla);
        kill_all_children(utiliz_pid, core_pid); // kill all the children and grandchildren processes
        return 1;
    }
    // read and print core info if needed
    if (cla->graph_flag[2]){
        coreInfo core_info;
        core_info.cores_num = -1;
        core_info.max_freq = -1;
        if (parent_core_info_reader_printer(core_fd[0], &core_info) == 1){
            safe_close(&utiliz_fd[0]);
            safe_close(&core_fd[0]);
            free(cla);
            kill_all_children(utiliz_pid, core_pid); // kill all the children and grandchildren processes
            return 1;
        }
    }
    // Clean up remaining read ends
    safe_close(&utiliz_fd[0]);
    safe_close(&core_fd[0]);

    //Wait for core child process first to avoid zombie process during execution
    if (wait_for_children(core_pid) == -1){
        perror("child process of main for getting core information exited abnormally.");
        free(cla);
        return 1;
    }

    // Wait for child processes
    if (wait_for_children(utiliz_pid) == -1){
        utiliz_pid = -1;
        perror("child process of main for getting utilization exited abnormally.");
        free(cla);
        kill_all_children(utiliz_pid, core_pid); // kill all the children and grandchildren processes
        return 1;
    }


    free(cla);

    return 0;
}