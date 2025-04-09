#define _POSIX_C_SOURCE 199309L
#include <signal.h>
#include "signalHandler.h"

typedef int (*CheckSignal)();

typedef void (*ResetSignal)();

// this part won't be used in out case (only handle sigint). It is just a idea for future extend if we want
// to deal with multiple signals. we can use this struct (maybe with a little adjustment). 
// and some extra implements of check and reset signals.
// And then pass the struct as a parameter to the functions in this module. 

// typedef struct signal_handle{
//         int * state; // maybe a bitvecotr showing which signal been received
//         CheckSignal check;
//         ResetSignal reset;
// }signalHandle;

// signalHandle init_signal_handle(){
//     signalHandle handle;
//     handle.check = check_sigal;
//     handle.reset = reset_sigal;
//     return handle;
// }

static volatile sig_atomic_t sigint_received = 0;
///_|> descry: internal state flag indicating whether SIGINT (Ctrl+C) has been received

static void sigint_handler(int signum){
    ///_|> descry: custom signal handler that sets the sigint_received flag to 1
    ///_|> signum: received signal number (unused), type int
    ///_|> returning: this function does not return anything
    (void)signum;
    // printf("SIGINT received,");
    sigint_received = 1;
    // printf("check sigint_received: %d \n", sigint_received);
}

int check_sigint(){
    ///_|> descry: checks whether SIGINT has been received
    ///_|> returning: returns 1 if SIGINT received, 0 otherwise
    // printf("check signal \n");
    return sigint_received == 1;
}

static void reset_signal(){
    ///_|> descry: resets the SIGINT received flag to 0
    ///_|> returning: this function does not return anything
    sigint_received = 0;
}

void init_sigaction(){
    ///_|> descry: sets up signal handling: custom handler for SIGINT and ignores SIGTSTP
    ///_|> returning: this function does not return anything
    struct sigaction sa_int;
    sa_int.sa_handler = sigint_handler; // custom handler for SIGINT
    sa_int.sa_flags = 0;
    sigemptyset(&sa_int.sa_mask);
    sigaction(SIGINT, &sa_int, NULL);

    struct sigaction sa_stp;
    sa_stp.sa_handler = SIG_IGN;  // ignore SIGTSTP (Ctrl+Z)
    sa_stp.sa_flags = 0;
    sigemptyset(&sa_stp.sa_mask);
    sigaction(SIGTSTP, &sa_stp, NULL);
    // printf("check init sigaction\n");
}

int prompt_for_int_signal(){
    ///_|> descry: if SIGINT was received, prompts user to confirm whether to quit the program
    ///_|> returning: returns 1 if user confirms exit, 0 otherwise
    CheckSignal check_signal = check_sigint;
    ResetSignal reset_sigint_received = reset_signal;

    // printf("prompt: checking sigint: %d \n", sigint_received);

    if (check_signal && check_signal()){;
        char ans[20];
        printf("\nDo you want to quit the program? (y/n): ");
        fflush(stdout);
        if (fgets(ans, sizeof(ans), stdin) && (ans[0] == 'y')){
            printf("Exiting...\n");
            fflush(stdout);
            return 1;
        }else{
            printf("Continue...\n");
            // Clear the two lines of prompt 
            printf("\033[2A");     
            printf("\033[2K");    
            printf("\033[1B");     
            printf("\033[2K");
            fflush(stdout);
        }
        if (reset_sigint_received) reset_sigint_received();
    }
    return 0;
}