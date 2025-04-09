#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>    
#include <sys/wait.h>

#ifndef PIPE_TOOL_HEADER
#define PIPT_TOOL_HEADER

/*
This module contains helper functions to simplify and secure low-level system operations 
such as process handling and pipe/file descriptor cleanup.
It is mainly used by producer/consumer modules and main process to manage forked children and 
pipe-based inter-process communication safely.
*/

int wait_for_children(pid_t children);

void safe_close(int* fd);

void exit_failure_with_two_pipe_close(int* fd1, int* fd2);

void kill_all_children(pid_t child1, pid_t child2);

#endif