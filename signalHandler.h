#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#ifndef SIGNAL_HANDLE
#define SIGNAL_HANDLE

/*
This module handles signals, particularly SIGINT (Ctrl+C), 
and provides a prompt to confirm program termination decided by user responce.
It uses internal flags to check/reset signal states.
*/

int check_sigint();

void init_sigaction();

int prompt_for_int_signal();

#endif