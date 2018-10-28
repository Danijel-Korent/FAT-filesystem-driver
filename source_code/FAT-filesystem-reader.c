
#include<stdio.h>   //printf, gets_s
#include<string.h>  //strcmp



static void print_root_files(void)
{
    // TODO: put all necessary logic here, and then split it into functional parts


    /*************************************************************************************
    * OVERALL STRUCTURE OF FAT FS
    *
    *  Boot sector (1sector/512bytes)
    *  Reserved area
    *  FAT TABLE #1
    *  FAT TABLE #2
    *  ROOT DIRECTORY AREA
    *  DATA AREA
    *       - Cluser_01
    *       - Cluser_xy
    *
    */

    /*************************************************************************************
    * OVERALL STRUCTURE OF BOOT SECTOR - the first 512 bytes of a FS (not MBR)
    *
    *     0 -   3  Boot code jump instruction
    *     4 -  37  FAT parameters ??
    *    90 - 510  Boot code (bootloader) ??

    *   511-512 - Boot sector signature
    */

    printf(" print_root_files example output: \n\n");

    printf(" TYPE SIZE NAME  \n");
    printf("  dir    0  Dir1  \n");
    printf("  dir    0  Dir2  \n");
    printf(" file   22  File1 \n");
}

static void execute_command_cd(const char *args, int args_lenght);
static void execute_command_ls(const char *args, int args_lenght);

static void run_pseudo_shell(void)
{
    // TODO: Implement a simple shell-like interface that supports cd, ls and cat commands

    char user_input[100] = { 0 };

    printf("\n\n\nshell$ ");

    while (NULL != gets_s(user_input, sizeof(user_input)))
    {
        if (0 == strcmp(user_input, "exit"))
        {
            break;
        }
        else if ('q' == user_input[0] && 0 == user_input[1])
        {
            break;
        }
        else
        {
            printf("%s: Unknown command", user_input);
        }

        printf("\nshell$ ");
    }
}


int main(void)
{
    print_root_files();

    run_pseudo_shell();

    return 0;
}
