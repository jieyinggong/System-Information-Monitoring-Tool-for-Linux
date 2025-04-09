#include "pipeTool.h"

int wait_for_children(pid_t children){
    ///_|> descry: waits for a specific child process and checks if it exited normally
    ///_|> children: PID of the child process to wait for, type pid_t
    ///_|> returning: returns 0 on normal exit, -1 on error
    if (children == -1) return 0; // skip waiting for invalid PID
    int status;
    pid_t pid = waitpid(children, &status, 0);
    if (pid != -1){
        // check if child exited successfully
        if (!WIFEXITED(status) || WEXITSTATUS(status) != EXIT_SUCCESS){
            return -1;
        }     
    }else{
        perror("wait failed");
        exit(EXIT_FAILURE);
    }

    return 0;
}

void safe_close(int* fd){
    ///_|> descry: safely closes a file descriptor if it is open and resets it to -1
    ///_|> fd: pointer to file descriptor to close, type int*
    ///_|> returning: this function does not return anything
    if (*fd != -1){
        close(*fd); // close if valid
        *fd = -1;   // reset to prevent reuse
    }
}

void exit_failure_with_two_pipe_close(int* fd1, int* fd2){
    ///_|> descry: closes two pipes (each with two ends) and exits the process with failure
    ///_|> fd1: pointer to first pipe array of two fds [0]=read, [1]=write, type int*
    ///_|> fd2: pointer to second pipe array of two fds, type int*
    ///_|> returning: this function does not return; it terminates the process with EXIT_FAILURE
    safe_close(&fd1[0]);
    safe_close(&fd1[1]);
    safe_close(&fd2[0]);
    safe_close(&fd2[1]);
    exit(EXIT_FAILURE);
}

void kill_all_children(pid_t child1, pid_t child2){
    ///_|> descry: sends SIGTERM to both child process groups if their PIDs are valid
    ///_|> child1: PID of the first child process (group leader), type: pid_t
    ///_|> child2: PID of the second child process (group leader), type: pid_t
    ///_|> returning: this function does not return anything
    if (child1 != -1){
        if (kill(-child1, SIGTERM) == -1 && errno != ESRCH) {
            perror("Failed to send SIGTERM to child1 group");
        }
    }
    if (child2 != -1){
        if (kill(-child2, SIGTERM) == -1 && errno != ESRCH) {
            perror("Failed to send SIGTERM to child2 group");
        }
    }
}