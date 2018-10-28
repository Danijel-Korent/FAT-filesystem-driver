
#include<stdio.h>   //printf, gets_s
#include<string.h>  //strcmp

#include "../fat_images/FAT12_3-clusters-clean.h"


static void print_root_files(void)
{
    // TODO: put all necessary logic here, and then split it into functional parts

    // Info:
    // https://en.wikipedia.org/wiki/Design_of_the_FAT_file_system

    // COPY-PASTER:
    //      - smallest possible FAT12 file system (!! Needs modification in mkfs.fat source to work! )
    //          - 3k total size, with 3 clusters * 512bytes = 1536bytes free space
    //          - works in linux (r/w), can read on windows10, but sometimes crashes explorer.exe on write
    //
    //          /home/danijel/fat_proj/dosfstools/src/mkfs.fat -vvv -F 12 -f 1 -r 16 -R 1 -n POKUS -s 1 -C FAT12_3-clusters-clean 3 > FAT12_3-clusters-clean_info.txt
    //

    /*************************************************************************************
    * OVERALL STRUCTURE OF FAT FS
    *
    *  RESERVED SECTORS
    *      - VBR - Volume boot record / partition boot sector (1sector/512bytes)
    *      - FS Information Sector (FAT32 only)
    *      - Rest of reserved sectors
    *  FAT TABLE #1
    *  FAT TABLE #2        (optional)
    *  ROOT DIRECTORY AREA (FAT12/16 only)
    *  DATA AREA
    *       - Cluser_01
    *       - Cluser_xy -> until the end of partition
    *
    */

    /*************************************************************************************
    * OVERALL STRUCTURE OF Volume Boot Record (VBR) - the first 512 bytes of a FS (not MBR)
    *
    *     0 -   2  Boot code jump instruction / EB 3C 90 -> jmp 0x3e, nop
    *     3 -   8  OEM Name string
    *     9 -  37  BIOS Parametar block ??
    *    62 - 510  Boot code (bootloader) -> conflicting info sources, but the jump instraction for the FAT12 points to 0x3e

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
        else if (0 == user_input[0])
        {
            // Empty input - do nothing
        }
        else if ('c' == user_input[0] && 'd' == user_input[1]) // cd
        {
            execute_command_cd(NULL, 0);
        }
        else if ('l' == user_input[0] && 's' == user_input[1]) // ls
        {
            execute_command_ls(NULL, 0);
        }
        else
        {
            printf("%s: Unknown command", user_input);
        }

        printf("\nshell$ ");
    }
}

static void execute_command_cd(const char *args, int args_lenght)
{
    printf("cd unimplemented");
}

static void execute_command_ls(const char *args, int args_lenght)
{
    printf(" ls example output: \n\n");

    printf(" %s %06i  %s \n", "d", 0, "DIR_5");
    printf(" %s %06i  %s \n", "f", 5, "FILE_6");
    printf(" %s %06i  %s \n", "f", 255, "FILE_7");
}

int main(void)
{
    print_root_files();

    run_pseudo_shell();

    return 0;
}
