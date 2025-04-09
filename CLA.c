#include "CLA.h"

int isdigit_cla(char* str){
    ///_|> descry: checks whether the given string consists only of digits
    ///_|> str: input string to check, type char*
    ///_|> returning: returns 1 if string contains only digits, otherwise 0

    while (*str != '\0'){
        if (isdigit(*str) == 0 ) return 0;
        str++;
    }
    return 1;
}

void read_CLA(int argc, char** argv, CLAInfo* info){
    ///_|> descry: parses command-line arguments and updates fields in the CLAInfo struct
    ///_|> argc: number of command-line arguments, type int
    ///_|> argv: array of command-line argument strings, type char**
    ///_|> info: pointer to CLAInfo struct to populate with parsed values, type CLAInfo*
    ///_|> returning: this function does not return anything
    info->samples = 20;   //default value
    info->tdelay = 500000; //default value
    info->graph_flag[0] = 0;  // memory graph off
    info->graph_flag[1] = 0;  // CPU graph off
    info->graph_flag[2] = 0;  // cores graph off
    
    for (int i = 1; i < argc; i++){
        int parsed;
        if (strcmp(argv[i], "--memory") == 0){
            info->graph_flag[0] = 1;
            continue;
        }
        else if (strcmp(argv[i], "--cpu") == 0){
            info->graph_flag[1] = 1;
            continue;
        }
        else if (strcmp(argv[i], "--cores") == 0){
            info->graph_flag[2] = 1;
            continue;
        }
        else if ((sscanf(argv[i], "--samples=%d",&parsed) == 1) && parsed > 0){
            info->samples = parsed;
            continue;
        }
        else if ((sscanf(argv[i], "--tdelay=%d",&parsed) == 1) && parsed > 0){
            info->tdelay = parsed;
            continue;
        }
        else if (i == 1 && isdigit_cla(argv[i])){
            info->samples = atoi(argv[i]);
            continue;
        }
        else if (i == 2 && isdigit_cla(argv[i])){
            info->tdelay = atoi(argv[i]);
            continue;
        } 
        // fix issue for a1: add the invalid cla case!
        else{
            // Invalid CLA detected
            printf("Invalid CLA: %s\n", argv[i]);
            continue;
        }
    }

    //default case 
    if (info->graph_flag[0] == 0 && info->graph_flag[1] == 0 && info->graph_flag[2] == 0){
        info->graph_flag[0] = 1;
        info->graph_flag[1] = 1;
        info->graph_flag[2] = 1;
    } 
}
