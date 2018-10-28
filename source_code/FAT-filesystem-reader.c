
#include<stdio.h> // printf, gets_s
#include<string.h>  //strcmp



static void print_root_files(void)
{
    // TODO: put all necessary logic here, and then split it into functional parts

    printf(" print_root_files example output: \n\n");

    printf(" TYPE SIZE NAME  \n");
    printf("  dir    0  Dir1  \n");
    printf("  dir    0  Dir2  \n");
    printf(" file   22  File1 \n");
}


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
